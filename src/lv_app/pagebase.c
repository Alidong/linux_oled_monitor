
#include "pagebase.h"
#include <stdlib.h>
#include "stdio.h"
#define TAG "pagemanege"
Page_maneger *root_page;
static lv_anim_timeline_t *anim_timeline = NULL;
Page_Status_t Pop_page(void)
{
    // anim_timeline = lv_anim_timeline_create();
    Page_maneger *page = Get_stackTop();
    if (page == root_page)
    {
        return REACH_BOTTOM;
    }
    if (page->onDisappearing != NULL)
    {
        page->is_Quit = true;       //��ջ���ٶ���
        page->onDisappearing(page); //�ڶ�����ɻص���ɾ�����󣬷�����ɶ����޷���������
    }
    page->pre_page->hidePage(page->pre_page, false);
    if (page->pre_page->onAppearing != NULL)
    {
        page->pre_page->onAppearing(page->pre_page);
    }
    page->pre_page->next_page = NULL;
    page->next_page = NULL;
    page->pre_page = NULL;
    // lv_anim_timeline_start(anim_timeline);
    page->onRelease(page); //�޹��ɶ���ֱ��ɾ��
    return NOERR;
}
Page_Status_t Push_page(Page_maneger *page)
{
    //page = (Page_maneger *)malloc(sizeof(Page_maneger));
    // anim_timeline = lv_anim_timeline_create();
    page->pre_page = Get_stackTop();
    page->pre_page->next_page = page;
    page->next_page = NULL;
    if (page->pre_page->onDisappearing != NULL)
    {
        page->pre_page->is_Quit = false;
        page->pre_page->onDisappearing(page->pre_page);
    }
    else
    {
        page->pre_page->hidePage(page->pre_page, true);
    }
    page->onCreate(page);

    page->pre_page->hidePage(page->pre_page, false);
    if (page->onAppearing != NULL)
    {
        page->onAppearing(page);
    }
    // lv_anim_timeline_start(anim_timeline);
    return NOERR;
}
Page_Status_t Page_stackInit(Page_maneger *page)
{
    root_page = page;
    page->pre_page = NULL;
    page->next_page = NULL;
    page->onCreate(page);
    if (page->onAppearing != NULL)
    {
        page->onAppearing(page);
    }
    return NOERR;
}
Page_maneger *Get_stackTop(void)
{

    // ESP_LOGI(TAG,"root=%d", (uint32_t)root);
    Page_maneger *page = root_page;
    while (page->next_page != NULL)
    {
        page = page->next_page;
    }
    return page;
}
Page_maneger *Get_stackBottom(Page_maneger *page)
{
    while (page->pre_page != NULL)
    {
        page = page->pre_page;
    }
    return page;
}
Page_Status_t Add_anime(lv_anim_t *a, uint32_t start_time)
{
    lv_anim_timeline_add(anim_timeline, start_time, a);
}
Page_Status_t Pop_Allpages(void)
{
    Page_maneger *page = Get_stackTop();
    if (page == root_page)
    {
        return REACH_BOTTOM;
    }
    while (page != root_page)
    {
        page->pre_page->next_page = NULL;
        page->pre_page = NULL;
        page->next_page = NULL;
        page->is_Quit = true;
        page->onRelease(page);
        //Pop_page();
        page = Get_stackTop();
    }
    page->onAppearing(page); //��ʾ��ҳ��
    return NOERR;
}