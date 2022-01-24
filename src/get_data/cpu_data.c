#include "cpu_data.h"
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#define MAXBUFSIZE 1024
#define WAIT_SECOND 3      //暂停时间，单位为“秒”
typedef struct cpu_occupy_ //定义一个cpu occupy的结构体
{
    char name[20];       //定义一个char类型的数组名name有20个元素
    unsigned int user;   //定义一个无符号的int类型的user
    unsigned int nice;   //定义一个无符号的int类型的nice
    unsigned int system; //定义一个无符号的int类型的system
    unsigned int idle;   //定义一个无符号的int类型的idle
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
} cpu_occupy_t;
static pthread_t tid;
static double cpu_load;
static float cpu_temp;
double cal_cpuoccupy(cpu_occupy_t *o, cpu_occupy_t *n)
{
    double od, nd;
    double id, sd;
    double cpu_use;

    od = (double)(o->user + o->nice + o->system + o->idle + o->softirq + o->iowait + o->irq); //第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double)(n->user + n->nice + n->system + n->idle + n->softirq + n->iowait + n->irq); //第二次(用户+优先级+系统+空闲)的时间再赋给od

    id = (double)(n->idle); //用户第一次和第二次的时间之差再赋给id
    sd = (double)(o->idle); //系统第一次和第二次的时间之差再赋给sd
    if ((nd - od) != 0)
        cpu_use = 100.0 - ((id - sd)) / (nd - od) * 100.00; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else
        cpu_use = 0;
    return cpu_use;
}

void get_cpuoccupy(cpu_occupy_t *cpust)
{
    FILE *fd;
    int n;
    char buff[256];
    cpu_occupy_t *cpu_occupy;
    cpu_occupy = cpust;

    fd = fopen("/proc/stat", "r");
    if (fd == NULL)
    {
        perror("fopen:");
        exit(0);
    }
    fgets(buff, sizeof(buff), fd);

    sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle, &cpu_occupy->iowait, &cpu_occupy->irq, &cpu_occupy->softirq);

    fclose(fd);
}
void *cpu_getinfo_thread(void *arg)
{
    char buf[50];
    cpu_occupy_t cpu_stat1;
    cpu_occupy_t cpu_stat2;
    FILE *fstream = NULL;
    time_t timer;
    timer = time(NULL);
    while (1)
    {
        get_cpuoccupy((cpu_occupy_t *)&cpu_stat1);
        sleep(1);
        //第二次获取cpu使用情况
        get_cpuoccupy((cpu_occupy_t *)&cpu_stat2);
        //计算cpu使用率
        cpu_load = cal_cpuoccupy((cpu_occupy_t *)&cpu_stat1, (cpu_occupy_t *)&cpu_stat2);

        if (NULL == (fstream = popen("cat /sys/class/thermal/thermal_zone0/temp", "r")))
        {
            fprintf(stderr, "execute command failed: %s", strerror(errno));
            return -1;
        }
        while (fgets(buf, sizeof(buf), fstream) != NULL)
        {
            cpu_temp = (float)atoi(buf) / 1000.f;
            usleep(1000);
        }
        pclose(fstream);
        // printf("cpu_load=%.1f cpu_temp=%.1f\r\n",cpu_load,cpu_temp);
    }
}
void getMem_data(MEM_OCCUPY *mem)
{
    FILE *fd;
    char buff[1024];
    fd = fopen("/proc/meminfo", "r");
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %ld", mem->name, &mem->total);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %ld", mem->name2, &mem->free);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %ld", mem->name3, &mem->MemAvailable);
    fclose(fd);
}
void get_disk_occupy(FDISK_OCCUPY *file_status)
{
    char currentDirectoryPath[MAXBUFSIZE];
    getcwd(currentDirectoryPath, MAXBUFSIZE);
    //printf("当前目录：%s\n",currentDirectoryPath);
    char cmd[20] = "df -h ";
    strcat(cmd, currentDirectoryPath);
    //printf("%s\n",cmd);

    char buffer[MAXBUFSIZE];
    FILE *pipe = popen(cmd, "r");

    if (!pipe)
        return;
    if (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        sscanf(buffer, "%s %s %s %s %s %s", file_status->fileSys, file_status->size, file_status->used, file_status->free, file_status->percent, file_status->moment);
    }
    if (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        sscanf(buffer, "%s %s %s %s %s %s", file_status->fileSys, file_status->size, file_status->used, file_status->free, file_status->percent, file_status->moment);
    }
    pclose(pipe);
    //printf("desk used:%s\n",percent);
    return;
}
// 获取本机ip  eth_inf网卡名称 调用方法get_local_ip("apcli0", ip);
int get_local_ip(const char *eth_inf, char *dest_str)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        strcpy(dest_str, "not conneted");
        // printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    // printf("interfac: %s, ip: %s\n", eth_inf, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    strcpy(dest_str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    shutdown(sd, SHUT_RDWR);
    close(sd);
    return 0;
}
void getTime_data(TIME_DATA *time_data)
{
    char cmd[20] = "date ";
    //printf("%s\n",cmd);
    char buffer[MAXBUFSIZE];
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        return;
    if (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        sscanf(buffer, "%s %s %s %s %s %s", time_data->week, time_data->month, time_data->day, time_data->time, time_data->zone, time_data->year);
    }
    pclose(pipe);
}
void cpu_init(void)
{
    pthread_create(&tid, NULL, cpu_getinfo_thread, NULL);
}
void getCpu_data(double *cpu_usage, double *cpu_tmp)
{
    *cpu_usage = cpu_load;
    *cpu_tmp = cpu_temp;
}