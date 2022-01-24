#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
   typedef struct _mem_occupy
   {
      char name[20];
      long total;
      char name2[20];
      long free;
      char name3[20];
      long MemAvailable;
   } MEM_OCCUPY;
   typedef struct _fdisk_occupy
   {
      char fileSys[20];
      char size[10];
      char used[10];
      char free[10];
      char percent[10];
      char moment[20];
   } FDISK_OCCUPY;
   typedef struct _time_data
   {
      char year[20];
      char month[10];
      char day[10];
      char week[10];
      char time[20];
      char zone[10]
   } TIME_DATA;
   void cpu_init(void);
   void getCpu_data(double *cpu_usage, double *cpu_tmp);
   void getMem_data(MEM_OCCUPY *mem);
   int get_local_ip(const char *eth_inf, char *dest_str);
   // void getTime_data(TIME_DATA *time_data);
#ifdef __cplusplus
} /* extern "C" */
#endif