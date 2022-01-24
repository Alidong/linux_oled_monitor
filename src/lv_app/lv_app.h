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
    extern Page_maneger mainMenu;
    extern Page_maneger subMenu;
    extern Page_maneger subMenu_1;
    void lvapp_init(void);
#ifdef __cplusplus
} /* extern "C" */
#endif