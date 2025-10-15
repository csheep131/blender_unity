/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "io_unity_c_export.h"
#include "BKE_context.h"
#include "RNA_access.h"
#include "RNA_define.h"
#include "WM_api.h"
#include "WM_types.h"
#include "ED_fileselect.h"
#include "EM_guardedalloc.h"
#include <string.h>
typedef struct ExportUnityCOp {
  UnityCExportParams params;
} ExportUnityCOp;

static bool export_unity_c_exec(bContext *C, wmOperator *op)
{
  ExportUnityCOp *eu = op->customdata;
  RNA_string_get(op->ptr, "filepath", eu->params.filepath);
  eu->params.apply_modifiers = RNA_boolean_get(op->ptr, "apply_modifiers");
  eu->params.triangulate     = RNA_boolean_get(op->ptr, "triangulate");
  eu->params.global_scale    = RNA_float_get(op->ptr, "global_scale");
  eu->params.unity_axes      = RNA_boolean_get(op->ptr, "unity_axes");

  if (!UNITYC_export_scene(C, &eu->params)) {
    BKE_report(op->reports, RPT_ERROR, "Unity C export failed");
    return OPERATOR_CANCELLED;
  }
  return OPERATOR_FINISHED;
}

static int export_unity_c_invoke(bContext *C, wmOperator *op, const wmEvent *UNUSED(event))
{
  ExportUnityCOp *eu = MEM_callocN(sizeof(*eu), __func__);
  op->customdata = eu;

  RNA_string_set(op->ptr, "filepath", "untitled.umesh");
  WM_event_add_fileselect(C, op);
  return OPERATOR_RUNNING_MODAL;
}

static void export_unity_c_cancel(bContext *UNUSED(C), wmOperator *op)
{
  if (op->customdata) {
    MEM_freeN(op->customdata);
    op->customdata = NULL;
  }
}

static void export_unity_c_ui(bContext *UNUSED(C), wmOperator *op)
{
  /* File browser sidebar props. */
  uiLayout *layout = op->layout;
  uiLayoutSetPropSep(layout, true);
  uiLayoutSetPropDecorate(layout, false);

  uiItemR(layout, op->ptr, "apply_modifiers", 0, NULL, ICON_NONE);
  uiItemR(layout, op->ptr, "triangulate", 0, NULL, ICON_NONE);
  uiItemR(layout, op->ptr, "global_scale", 0, NULL, ICON_NONE);
  uiItemR(layout, op->ptr, "unity_axes", 0, NULL, ICON_NONE);
}

void EXPORT_OT_unity_c(wmOperatorType *ot)
{
  ot->name = "Unity C Export (.umesh)";
  ot->idname = "EXPORT_SCENE_unity_c";
  ot->description = "Export scene meshes to Unity-friendly binary format (.umesh)";

  ot->invoke = export_unity_c_invoke;
  ot->exec   = export_unity_c_exec;
  ot->cancel = export_unity_c_cancel;

  ot->flag = OPTYPE_REGISTER | OPTYPE_PRESET;

  /* File browser */
  WM_operator_properties_filesel(
      ot, FILE_TYPE_FOLDER | FILE_TYPE_OPERATOR, FILE_SPECIAL, FILE_SAVE, WM_FILESEL_FILEPATH,
      FILE_DEFAULTDISPLAY, FILE_SORT_DEFAULT);

  /* Props */
  RNA_def_string_file_path(ot->srna, "filepath", NULL, 0, "File Path", "Output .umesh file");
  RNA_def_boolean(ot->srna, "apply_modifiers", true, "Apply Modifiers", "Export evaluated meshes");
  RNA_def_boolean(ot->srna, "triangulate", true, "Triangulate", "Force triangle export");
  RNA_def_float(ot->srna, "global_scale", 1.0f, 0.0001f, 1000.0f, "Scale", "", 0.01f, 100.0f);
  RNA_def_boolean(ot->srna, "unity_axes", true, "Unity Axes (Z Fwd, Y Up)", "Convert axes");

  ot->ui = export_unity_c_ui;
}
