/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "io_unity_c_export.h"

#include "BKE_context.h"
#include "BKE_deform.h"
#include "BKE_layer.h"
#include "BKE_mesh.h"
#include "BKE_object.h"

#include "DEG_depsgraph.h"

#include "DNA_object_types.h"
#include "DNA_mesh_types.h"

#include "BLI_fileops.h"
#include "BLI_listbase.h"
#include "BLI_math.h"
#include "BLI_path_util.h"
#include "BLI_string.h"
#include "BLI_utildefines.h"

#include <stdio.h>

/* Simple binary format:
 * header: 'UMSH' (4 bytes)
 * uint32 version = 1
 * float scale
 * uint32 mesh_count
 *
 * For each mesh:
 *   uint32 name_len, char name[name_len] (no null)
 *   uint32 vertex_count
 *   uint32 index_count (tri indices)
 *   float3 vertices[vertex_count]   (x,y,z)
 *   float3 normals[vertex_count]
 *   uint32 indices[index_count]     (triangles)
 */

static void axis_convert_unity(float r_co[3], const float co[3], bool unity_axes)
{
  if (!unity_axes) {
    copy_v3_v3(r_co, co);
    return;
  }
  /* Blender: +Y forward, +Z up
   * Unity:   +Z forward, +Y up
   * One mapping: (x, y, z) -> (x, z, -y)  (approx; choose what matches your pipeline)
   */
  r_co[0] = co[0];
  r_co[1] = co[2];
  r_co[2] = -co[1];
}

static bool write_u32(FILE *f, unsigned int v) { return fwrite(&v, 4, 1, f) == 1; }
static bool write_f32(FILE *f, float v) { return fwrite(&v, 4, 1, f) == 1; }
static bool write_f32v(FILE *f, const float *v, int n) { return fwrite(v, 4, n, f) == (size_t)n; }

static void triangulate_mesh_if_needed(Mesh *me_dst)
{
  /* Minimalistic triangulation: ensure looptris are calculated. */
  /* Blender keeps polys (ngons). We export via looptris for indices. */
  BKE_mesh_runtime_looptri_ensure(me_dst);
}

static bool export_one_object(FILE *f,
                              Depsgraph *depsgraph,
                              Object *ob,
                              const UnityCExportParams *p)
{
  if (ob->type != OB_MESH) {
    return true; /* skip non-mesh, not an error */
  }

  /* Evaluated object (with modifiers if viewport set up). */
  Object *ob_eval = DEG_get_evaluated_object(depsgraph, ob);
  Mesh *me_eval = BKE_object_get_evaluated_mesh(ob_eval);

  Mesh *me = NULL;
  if (me_eval) {
    me = me_eval;
  }
  else {
    /* fallback: create mesh from object (applies modifiers if requested). */
    me = BKE_object_to_mesh(NULL, ob, false);
  }
  if (!me) {
    return true; /* nothing to export */
  }

  /* Apply simple triangulation via looptris. */
  triangulate_mesh_if_needed(me);

  /* Collect counts. */
  const int totvert = me->totvert;
  const int totlooptri = BKE_mesh_runtime_looptri_len(me);
  const MLoopTri *looptris = BKE_mesh_runtime_looptri_ensure(me);

  /* Write name. */
  const char *name = ob->id.name + 2; /* skip ID prefix "OB" */
  const unsigned int name_len = (unsigned int)strlen(name);
  if (!write_u32(f, name_len)) return false;
  if (fwrite(name, 1, name_len, f) != name_len) return false;

  /* Write counts (triangles => index_count = looptri_num * 3). */
  if (!write_u32(f, (unsigned int)totvert)) return false;
  if (!write_u32(f, (unsigned int)(totlooptri * 3))) return false;

  /* Export vertices & normals in object space, scaled & axis-converted. */
  const float(*positions)[3] = BKE_mesh_vert_positions(me);
  const float(*normals)[3] = CustomData_get_layer(&me->vert_data, CD_NORMAL);
  bool need_temp_normals = false;

  if (!normals) {
    /* compute vertex normals if missing */
    BKE_mesh_calc_normals_split(me);
    normals = CustomData_get_layer(&me->vert_data, CD_NORMAL);
    if (!normals) {
      need_temp_normals = true;
    }
  }

  for (int i = 0; i < totvert; i++) {
    float co[3], n[3], out[3];

    copy_v3_v3(co, positions[i]);
    mul_v3_fl(co, p->global_scale);

    /* transform by object matrix? Here we keep object space;
     * for world space: mul_m4_v3(ob_eval->object_to_world, co)
     * For Unity, world space often easier—choose what you need.
     */
    axis_convert_unity(out, co, p->unity_axes);
    if (!write_f32v(f, out, 3)) return false;

    if (need_temp_normals) {
      zero_v3(n);
    }
    else {
      copy_v3_v3(n, normals[i]);
    }
    axis_convert_unity(out, n, p->unity_axes);
    if (!write_f32v(f, out, 3)) return false;
  }

  /* Export triangle indices. */
  const MLoop *mloop = me->mloop;
  for (int i = 0; i < totlooptri; i++) {
    const MLoopTri *lt = &looptris[i];
    unsigned int i0 = mloop[lt->tri[0]].v;
    unsigned int i1 = mloop[lt->tri[1]].v;
    unsigned int i2 = mloop[lt->tri[2]].v;
    if (!write_u32(f, i0) || !write_u32(f, i1) || !write_u32(f, i2)) return false;
  }

  /* If we created temp mesh, free it. */
  if (me != me_eval && me != NULL) {
    BKE_id_free(NULL, me);
  }

  return true;
}

bool UNITYC_export_scene(bContext *C, const UnityCExportParams *params)
{
  if (!params || params->filepath[0] == '\0') {
    return false;
  }

  Depsgraph *depsgraph = CTX_data_ensure_evaluated_depsgraph(C);
  Scene *scene = CTX_data_scene(C);
  ViewLayer *view_layer = CTX_data_view_layer(C);

  /* Open file */
  BLI_make_existing_file(params->filepath);
  FILE *f = BLI_fopen(params->filepath, "wb");
  if (!f) return false;

  /* Header */
  fwrite("UMSH", 1, 4, f);
  write_u32(f, 1); /* version */
  write_f32(f, params->global_scale);
  /* We'll count meshes on the fly; first pass to count. */
  unsigned int mesh_count = 0;

  /* Count pass */
  FOREACH_BASE_IN_LAYER (view_layer, base) {
    Object *ob = base->object;
    if ((base->flag & BASE_ENABLED_VIEWPORT) == 0) continue;
    if (ob->type == OB_MESH) mesh_count++;
  }
  write_u32(f, mesh_count);

  /* Export pass */
  FOREACH_BASE_IN_LAYER (view_layer, base) {
    Object *ob = base->object;
    if ((base->flag & BASE_ENABLED_VIEWPORT) == 0) continue;
    if (!export_one_object(f, depsgraph, ob, params)) {
      fclose(f);
      return false;
    }
  }

  fclose(f);
  return true;
}
