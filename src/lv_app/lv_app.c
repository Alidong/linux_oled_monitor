#include "lv_app.h"
#include "stdio.h"
#include "mainMenu.h"
#include "cpu_page.h"
#include "lvgl.h"
#include "lv_examples.h"
#include "lv_port_disp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h> //如果你用了epoll 或者select可添加此头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/kd.h>
#include <linux/input.h> //如果你是自己处理去抖动那直接读取触摸屏事件可以添加此头文件
#include <time.h>
#include "unistd.h"
#include "cpu_data.h"
#define PERIOD 10

Page_maneger mainMenu;
Page_maneger subMenu;
Page_maneger subMenu_1;
static int count = 2;
static void timer_cb(lv_timer_t *timer)
{
	/*Use the user_data*/
	if (count % 2)
		Push_page(&subMenu);
	else
		Pop_page();
	count++;
}
void lvapp_init(void)
{
	lv_theme_mono_init(NULL, true, &lv_font_unscii_8);
	// mainMenu_init(&mainMenu);
	subMenu_init(&subMenu);
	// subMenu1_init1(&subMenu_1);
	Page_stackInit(&subMenu);
	// lv_timer_t *timer = lv_timer_create(timer_cb, 2000, NULL);
}
int main(void)
{

	cpu_init();
	lv_init();			 //lvgl 系统初始化
	lv_port_disp_init(); //lvgl 显示接口初始化,放在 lv_init()的后面
	//lv_example_spinner_1();
	//lv_example_arc_2();
	lvapp_init();
	//lv_obj_set_size(img1, 32, 32);
	while (1)
	{
		lv_tick_inc(PERIOD);
		usleep(PERIOD * 1000);
		lv_task_handler();
	}
}
