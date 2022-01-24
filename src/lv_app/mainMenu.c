#include "mainMenu.h"
#include "stdio.h"
#include "lv_app.h"
#define ANIM_TIME 100
#define ANIM_DELAY 100
#define ANIM_Y 50
static Page_maneger *local_mainMenu;
static lv_obj_t *btn1;
static lv_obj_t *btn2;
static lv_obj_t *label;
static void event_handler(lv_event_t *event)
{
    /*The original target of the event. Can be the buttons or the container*/
    lv_obj_t *target = lv_event_get_target(event);
    lv_event_code_t code = lv_event_get_code(event);
    if (target == btn1 && code == LV_EVENT_CLICKED)
    {
        Push_page(&subMenu);
    }
    else if (target == btn2 && code == LV_EVENT_CLICKED)
    {
        Push_page(&subMenu_1);
    }
}
static void onCreate_mainMenu(Page_maneger *main_page)
{ /*Create a page*/
    main_page->pre_page = NULL;
    main_page->next_page = NULL;

    lv_obj_t *page = lv_obj_create(lv_scr_act());
    main_page->Page_obj = page;
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_size(page, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(page);

    label = lv_label_create(page);
    lv_label_set_text(label, "[Main]");
    lv_obj_add_flag(label, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align_to(label, page, LV_ALIGN_TOP_MID, 0, 2);
}
static void hide_mainMenu(Page_maneger *page_maneger, bool en)
{
    // printf("hide_mainMenu");
    if (en)
    {
        lv_obj_add_flag(page_maneger->Page_obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(page_maneger->Page_obj, LV_OBJ_FLAG_HIDDEN);
    }
}
static void onAppearing(Page_maneger *main_page)
{
    //lv_obj_align(main_page->Page_obj, LV_ALIGN_CENTER, 0, -ANIM_Y);
    // lv_obj_clear_flag(main_page->Page_obj, LV_OBJ_FLAG_HIDDEN);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, main_page->Page_obj);
    lv_anim_set_time(&a, ANIM_TIME);
    lv_anim_set_delay(&a, ANIM_DELAY);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&a, lv_obj_get_y(main_page->Page_obj) - ANIM_Y,
                       0);
    lv_anim_start(&a);
}
static void onDisappearing_ready_cb(lv_anim_t *a)
{
    printf("onDisappearing_ready_cb\r\n");
    //Page_maneger *maneger=Get_stackTop();
    ////lv_obj_fade_out(local_mainMenu->Page_obj, ANIM_TIME, 0);
    //maneger ->onCreate(maneger);
    //if (maneger->onAppearing!=NULL)
    //{
    //    maneger->onAppearing(maneger);
    //}
}
static void onDisappearing(Page_maneger *main_page)
{

    printf("onDisappearing\r\n");
    //lv_obj_fade_out(main_page->Page_obj, ANIM_TIME, ANIM_DELAY);
}
void mainMenu_init(Page_maneger *main_page)
{
    local_mainMenu = main_page;
    main_page->pre_page = NULL;
    main_page->next_page = NULL;
    main_page->onCreate = onCreate_mainMenu;
    main_page->hidePage = hide_mainMenu;
    main_page->event_handler = event_handler;
    main_page->onAppearing = onAppearing;
    main_page->onDisappearing = NULL;
}