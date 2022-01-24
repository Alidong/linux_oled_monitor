#ifndef PAGEBASE_H
#define PAGEBASE_H
#ifdef __cplusplus
extern "C"
{
#endif
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif
    typedef enum
    {
        NOERR = 0,
        ERR = 1,
        REACH_BOTTOM = 2,
    } Page_Status_t;
    typedef struct Page_maneger
    {
        struct Page_maneger *pre_page;
        struct Page_maneger *next_page;
        lv_obj_t *Page_obj;
        void (*pageConfig)(struct Page_maneger *page_maneger);
        void (*onCreate)(struct Page_maneger *page_maneger);       // ����ҳ��
        void (*onAppearing)(struct Page_maneger *page_maneger);    // ��ʾ���ȶ���
        void (*onDisappearing)(struct Page_maneger *page_maneger); // ��ʾ���ȶ���
        void (*onRelease)(struct Page_maneger *page_maneger);      // ���ٽ��棬�Լ��ͷŽ���������Ԫ����ռ�õ��ڴ�
        void (*hidePage)(struct Page_maneger *page_maneger, bool en);
        void (*event_handler)(lv_event_t *event); // ��һ�������� �������ͣ�˵���ǰ�����һ���������ڶ����ǰ������������»��ǵ���
        bool is_Quit;
    } Page_maneger;
    Page_Status_t Pop_Allpages(void);
    Page_Status_t Page_stackInit(Page_maneger *page);  //����һ��ջ�ף���ҳ���޷���ջ
    Page_maneger *Get_stackTop(void);                  //��ȡջ��
    Page_maneger *Get_stackBottom(Page_maneger *page); //��ȡ��ҳ��
    Page_Status_t Push_page(Page_maneger *page);       //ҳ����ջ
    Page_Status_t Pop_page(void);                      //ҳ���ջ
    Page_Status_t Add_anime(lv_anim_t *a, uint32_t start_time);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif