#include "cpu_page.h"
#include "stdio.h"
#include <sys/time.h>
#include <unistd.h>
#include "cpu_data.h"
#define ANIM_TIME 300
#define ANIM_DELAY 100
#define ANIM_Y 32

static Page_maneger *local_subMenu;
static Page_maneger *local_subMenu_1;
static lv_obj_t *lv_cpu_temp_lable, *lv_cpu_load_lable, *lv_mem_status_lable,
    *lv_fdisk_status_lable, *lv_ip_status_lable, *lv_localtime_lable;
static lv_anim_timeline_t *anim_timeline = NULL;
static char cpu_load_str[10], cpu_temp_str[10], mem_data_str[10], fdisk_data_str[20], ip_data_str[50];
static double cpu_load, cpu_temp;
static MEM_OCCUPY mem_data;
static FDISK_OCCUPY fdisk_data;
static TIME_DATA time_data;
static void timer_cb(lv_timer_t *timer)
{
    /*Use the user_data*/
    getCpu_data(&cpu_load, &cpu_temp);
    getMem_data(&mem_data);
    get_disk_occupy(&fdisk_data);
    char ip_str1[20], ip_str2[20];
    get_local_ip("eth0", ip_str1);
    get_local_ip("wlan0", ip_str2);
    getTime_data(&time_data);
    sprintf(fdisk_data_str, "%s/%s", fdisk_data.used, fdisk_data.size);
    sprintf(cpu_load_str, "%.0f", cpu_load);
    sprintf(cpu_temp_str, "%.1fC", cpu_temp);
    sprintf(ip_data_str, "0:%s\n1:%s", ip_str1, ip_str2);
    long mem_used_mb = (mem_data.total - mem_data.MemAvailable) / 1024;
    if (mem_used_mb > 1000)
        sprintf(mem_data_str, "%.1fGB", (float)mem_used_mb / 1024.0);
    else
        sprintf(mem_data_str, "%dMB", mem_used_mb);
    if (lv_cpu_load_lable != NULL)
        lv_label_set_text(lv_cpu_load_lable, cpu_load_str);
    if (lv_cpu_temp_lable != NULL)
        lv_label_set_text(lv_cpu_temp_lable, cpu_temp_str);
    if (lv_mem_status_lable != NULL)
        lv_label_set_text(lv_mem_status_lable, mem_data_str);
    if (lv_fdisk_status_lable != NULL)
        lv_label_set_text(lv_fdisk_status_lable, fdisk_data_str);
    if (lv_ip_status_lable != NULL)
        lv_label_set_text(lv_ip_status_lable, ip_data_str);
    if (lv_localtime_lable != NULL)
        lv_label_set_text(lv_localtime_lable, time_data.time);
}

static void event_handler(lv_event_t *e)
{
    /*The original target of the event. Can be the buttons or the container*/
    // lv_obj_t *target = lv_event_get_target(e);
    // lv_event_code_t code = lv_event_get_code(e);
    // if (target == btn1 && code == LV_EVENT_CLICKED)
    // {
    //     Pop_page();
    // }
    // else if (target == btn2 && code == LV_EVENT_CLICKED)
    // {
    //     Push_page(local_subMenu_1);
    // }
    // else if (code == LV_EVENT_VALUE_CHANGED)
    // {
    //     printf("Toggled\n");
    // }
}
static void set_angle(void *obj, int32_t v)
{
    lv_arc_set_value(obj, v);
}
static lv_point_t line_points[] = {{0, 0}, {64, 0}};

static void onCreate_subMenu(Page_maneger *sub_page)
{
    lv_obj_t *page = lv_obj_create(lv_scr_act());
    sub_page->Page_obj = page;
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_size(page, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_align(page, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_flex_align(page, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    LV_IMG_DECLARE(cpuload_image);
    lv_obj_t *img1 = lv_img_create(page);
    lv_img_set_src(img1, &cpuload_image);
    lv_obj_align(img1, LV_ALIGN_LEFT_MID, 12, 0);
    /*Create an Arc*/
    lv_obj_t *arc = lv_arc_create(page);
    lv_obj_align(arc, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
    lv_cpu_load_lable = lv_label_create(arc);
    lv_obj_align(lv_cpu_load_lable, LV_ALIGN_CENTER, 0, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_start(&a);

    /*CPU temp*/
    LV_IMG_DECLARE(cputemp_image);
    lv_obj_t *img2 = lv_img_create(page);
    lv_img_set_src(img2, &cputemp_image);
    lv_img_set_zoom(img2, 240);
    lv_img_set_offset_y(img2, -2);
    lv_obj_align(img2, LV_ALIGN_OUT_LEFT_MID, -36, 0);

    lv_cpu_temp_lable = lv_label_create(page);
    lv_obj_align(lv_cpu_temp_lable, LV_ALIGN_RIGHT_MID, 36, 0);

    /*MEM status*/
    LV_IMG_DECLARE(mem_image);
    lv_obj_t *img3 = lv_img_create(page);
    lv_img_set_src(img3, &mem_image);
    lv_img_set_offset_y(img3, -3);
    lv_obj_align(img3, LV_ALIGN_OUT_LEFT_MID, -36, 0);
    lv_mem_status_lable = lv_label_create(page);
    lv_obj_align(lv_mem_status_lable, LV_ALIGN_RIGHT_MID, 36, 0);

    /*FDISK status*/
    LV_IMG_DECLARE(fdisk_image);
    lv_obj_t *img4 = lv_img_create(page);
    lv_img_set_src(img4, &fdisk_image);
    lv_img_set_zoom(img4, 240);
    lv_img_set_offset_y(img4, -3);
    lv_obj_align(img4, LV_ALIGN_OUT_LEFT_MID, -36, 0);
    lv_fdisk_status_lable = lv_label_create(page);
    lv_obj_align(lv_fdisk_status_lable, LV_ALIGN_RIGHT_MID, 36, 0);
    /*IP status*/
    lv_ip_status_lable = lv_label_create(page);
    lv_obj_align(lv_ip_status_lable, LV_ALIGN_CENTER, 0, 0);
    /*Local time*/
    lv_localtime_lable = lv_label_create(page);
    lv_obj_set_style_text_font(lv_localtime_lable, &lv_font_unscii_16, 0);
    lv_obj_align(lv_localtime_lable, LV_ALIGN_CENTER, 0, 0);
}
static void onRelease_subMenu(Page_maneger *page)
{
    printf("onRelease_subMenu\r\n");
    lv_obj_del(page->Page_obj);
    lv_cpu_load_lable = NULL;
    lv_cpu_temp_lable = NULL;
}
static void submenu_hide(Page_maneger *page, bool en)
{
    //lv_event_send(page->Page_obj,lv_event_hi);
    if (en)
    {
        lv_obj_add_flag(page->Page_obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(page->Page_obj, LV_OBJ_FLAG_HIDDEN);
    }
    //lv_obj_set_hidden(, en);
}
static void onAppearing(Page_maneger *page)
{

    lv_anim_t a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    lv_obj_t *child_1 = lv_obj_get_child(page->Page_obj, 0);
    lv_obj_t *child_2 = lv_obj_get_child(page->Page_obj, 1);
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, child_1);
    lv_anim_set_time(&a1, ANIM_TIME);
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a1, lv_anim_path_ease_out);
    lv_anim_set_values(&a1,
                       lv_obj_get_x(child_1) - 36,
                       18);
    lv_anim_set_playback_delay(&a1, 1000);
    lv_anim_set_playback_time(&a1, ANIM_TIME);
    lv_anim_set_repeat_count(&a1, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a1, 1600 * 5);

    lv_anim_init(&a2);
    lv_anim_set_var(&a2, child_2);
    lv_anim_set_time(&a2, ANIM_TIME);
    lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
    lv_anim_set_values(&a2,
                       lv_obj_get_x(child_2) + 36,
                       -18);
    lv_anim_set_playback_delay(&a2, 1000);
    lv_anim_set_playback_time(&a2, ANIM_TIME);
    lv_anim_set_repeat_count(&a2, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a2, 1600 * 5);
    /*Temp*/
    lv_obj_t *child_3 = lv_obj_get_child(page->Page_obj, 2);
    lv_anim_init(&a3);
    lv_anim_set_var(&a3, child_3);
    lv_anim_set_time(&a3, ANIM_TIME);
    lv_anim_set_exec_cb(&a3, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a3, lv_anim_path_ease_out);
    lv_anim_set_values(&a3,
                       lv_obj_get_x(child_3) - 36,
                       18);
    lv_anim_set_playback_delay(&a3, 1000);
    lv_anim_set_playback_time(&a3, ANIM_TIME);
    lv_anim_set_repeat_count(&a3, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a3, 1600 * 5);

    lv_obj_t *child_4 = lv_obj_get_child(page->Page_obj, 3);
    lv_anim_init(&a4);
    lv_anim_set_var(&a4, child_4);
    lv_anim_set_time(&a4, ANIM_TIME);
    lv_anim_set_exec_cb(&a4, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a4, lv_anim_path_ease_out);
    lv_anim_set_values(&a4,
                       lv_obj_get_x(child_4) + 42,
                       -12);
    lv_anim_set_playback_delay(&a4, 1000);
    lv_anim_set_playback_time(&a4, ANIM_TIME);
    lv_anim_set_repeat_count(&a4, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a4, 1600 * 5);

    /*Mem*/
    lv_obj_t *child_5 = lv_obj_get_child(page->Page_obj, 4);
    lv_anim_init(&a5);
    lv_anim_set_var(&a5, child_5);
    lv_anim_set_time(&a5, ANIM_TIME);
    lv_anim_set_exec_cb(&a5, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a5, lv_anim_path_ease_out);
    lv_anim_set_values(&a5,
                       lv_obj_get_x(child_5) - 36,
                       18);
    lv_anim_set_playback_delay(&a5, 1000);
    lv_anim_set_playback_time(&a5, ANIM_TIME);
    lv_anim_set_repeat_count(&a5, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a5, 1600 * 5);

    lv_obj_t *child_6 = lv_obj_get_child(page->Page_obj, 5);
    lv_anim_init(&a6);
    lv_anim_set_var(&a6, child_6);
    lv_anim_set_time(&a6, ANIM_TIME);
    lv_anim_set_exec_cb(&a6, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a6, lv_anim_path_ease_out);
    lv_anim_set_values(&a6,
                       lv_obj_get_x(child_6) + 42,
                       -12);
    lv_anim_set_playback_delay(&a6, 1000);
    lv_anim_set_playback_time(&a6, ANIM_TIME);
    lv_anim_set_repeat_count(&a6, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a6, 1600 * 5);
    /*Fdisk*/
    lv_obj_t *child_7 = lv_obj_get_child(page->Page_obj, 6);
    lv_anim_init(&a7);
    lv_anim_set_var(&a7, child_7);
    lv_anim_set_time(&a7, ANIM_TIME);
    lv_anim_set_exec_cb(&a7, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a7, lv_anim_path_ease_out);
    lv_anim_set_values(&a7,
                       lv_obj_get_x(child_7) - 36,
                       12);
    lv_anim_set_playback_delay(&a7, 1000);
    lv_anim_set_playback_time(&a7, ANIM_TIME);
    lv_anim_set_repeat_count(&a7, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a7, 1600 * 5);

    lv_obj_t *child_8 = lv_obj_get_child(page->Page_obj, 7);
    lv_anim_init(&a8);
    lv_anim_set_var(&a8, child_8);
    lv_anim_set_time(&a8, ANIM_TIME);
    lv_anim_set_exec_cb(&a8, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a8, lv_anim_path_ease_out);
    lv_anim_set_values(&a8,
                       lv_obj_get_x(child_8) + 72,
                       -8);
    lv_anim_set_playback_delay(&a8, 1000);
    lv_anim_set_playback_time(&a8, ANIM_TIME);
    lv_anim_set_repeat_count(&a8, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a8, 1600 * 5);
    /*IP status*/
    lv_obj_t *child_9 = lv_obj_get_child(page->Page_obj, 8);
    lv_anim_init(&a9);
    lv_anim_set_var(&a9, child_9);
    lv_anim_set_time(&a9, ANIM_TIME);
    lv_anim_set_exec_cb(&a9, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a9, lv_anim_path_ease_out);
    lv_anim_set_values(&a9,
                       lv_obj_get_y(child_9) - 32,
                       0);
    lv_anim_set_playback_delay(&a9, 1000);
    lv_anim_set_playback_time(&a9, ANIM_TIME);
    lv_anim_set_repeat_count(&a9, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a9, 1600 * 5);
    /*Local time*/
    lv_obj_t *child_10 = lv_obj_get_child(page->Page_obj, 9);
    lv_anim_init(&a10);
    lv_anim_set_var(&a10, child_10);
    lv_anim_set_time(&a10, ANIM_TIME);
    lv_anim_set_exec_cb(&a10, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a10, lv_anim_path_ease_out);
    lv_anim_set_values(&a10,
                       lv_obj_get_y(child_9) + 32,
                       0);
    lv_anim_set_playback_delay(&a10, 1000);
    lv_anim_set_playback_time(&a10, ANIM_TIME);
    lv_anim_set_repeat_count(&a10, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a10, 1600 * 5);
    /* Create anim timeline */
    anim_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_timeline, 0, &a1);
    lv_anim_timeline_add(anim_timeline, 0, &a2);
    lv_anim_timeline_add(anim_timeline, 1600, &a3);
    lv_anim_timeline_add(anim_timeline, 1600, &a4);
    lv_anim_timeline_add(anim_timeline, 1600 * 2, &a5);
    lv_anim_timeline_add(anim_timeline, 1600 * 2, &a6);
    lv_anim_timeline_add(anim_timeline, 1600 * 3, &a7);
    lv_anim_timeline_add(anim_timeline, 1600 * 3, &a8);
    lv_anim_timeline_add(anim_timeline, 1600 * 4, &a9);
    lv_anim_timeline_add(anim_timeline, 1600 * 5, &a10);
    lv_anim_timeline_start(anim_timeline);
}
static void onDisappearing_ready_cb(lv_anim_t *a)
{
    printf("onDisappearing_ready_cb\r\n");

    // if (local_subMenu->is_Quit != true)
    // {
    //     local_subMenu->next_page->onCreate(local_subMenu->next_page);
    // }
    // else
    {
        printf("onDisappearing_release\r\n");
        local_subMenu->onRelease(local_subMenu);
    }
}
static void onDisappearing(Page_maneger *main_page)
{
    // printf("cpu onDisappearing get =%d\r\n", lv_obj_get_child_cnt(main_page->Page_obj));
    // lv_obj_t *child_1 = lv_obj_get_child(main_page->Page_obj, 0);
    // lv_obj_t *child_2 = lv_obj_get_child(main_page->Page_obj, 1);
    // lv_anim_t a1, a2;
    // lv_anim_init(&a1);
    // lv_anim_set_var(&a1, child_1);
    // lv_anim_set_time(&a1, ANIM_TIME);
    // // lv_anim_set_delay(&a1, ANIM_DELAY);
    // lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_x);
    // lv_anim_set_values(&a1, lv_obj_get_x(child_1),
    //                    -30);

    // lv_anim_init(&a2);
    // lv_anim_set_var(&a2, child_2);
    // // lv_anim_set_ready_cb(&a2, onDisappearing_ready_cb);
    // lv_anim_set_time(&a2, ANIM_TIME);
    // lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_x);
    // lv_anim_set_values(&a2, lv_obj_get_x(child_2),
    //                    30);
    // printf("Add_anime\r\n");
    // Add_anime(&a1, 0);
    // Add_anime(&a2, 0);
}

void subMenu_init(Page_maneger *sub_page)
{
    local_subMenu = sub_page;
    sub_page->onCreate = onCreate_subMenu;
    sub_page->onRelease = onRelease_subMenu;
    sub_page->hidePage = submenu_hide;
    sub_page->event_handler = event_handler;
    sub_page->onAppearing = onAppearing;
    sub_page->onDisappearing = NULL;
    sub_page->is_Quit = false;
    lv_timer_t *timer = lv_timer_create(timer_cb, 1000, NULL);
}
