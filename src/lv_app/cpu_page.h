#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif
#include "pagebase.h"
    void cpuPage_init(Page_maneger *sub_page);
  //  void subMenu1_init1(Page_maneger *sub_page);
#ifdef __cplusplus
} /* extern "C" */
#endif