/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "BLI_sys_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UnityCExportParams {
  char filepath[1024];
  bool apply_modifiers;
  bool triangulate;
  float global_scale;
  bool unity_axes; /* Z forward, Y up */
} UnityCExportParams;

bool UNITYC_export_scene(struct bContext *C, const UnityCExportParams *params);

#ifdef __cplusplus
}
#endif
