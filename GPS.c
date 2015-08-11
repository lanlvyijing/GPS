/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
This example will open the GPS device and receive the current position.
Firstly it will open GPS with vm_gps_open. It then reads the GPS data and analyzes its GPGGA data.
Before you run this example, you need to insert a GPS antenna. 
Once the position is fixed, the example prints the number of satellites, latitude and longitude to monitor tool.

*/
#include <string.h>
#include "vmtype.h"
#include "vmsystem.h"
#include "vmlog.h"
#include "vmgps.h"

/* Parse token */
const VMCHAR *next_token(const VMCHAR* src, VMCHAR* buf) {

   VMINT i = 0; 

   while(src[i] != 0 && src[i] != ',') 
     i++; 
   
   if(buf){ 
     strncpy(buf, src, i); 
     buf[i] = 0; 
   } 
   
   if(src[i]) 
     i++; 
   return src+i; 
} 
 
/* Parse and print GPS GGA data */
/**************************************************************
 *
 * $GPGGA 语句包括17个字段：
 * 语句标识头，世界时间，纬度，纬度半球，经度，经度半球，定位质量指示，使用卫星数量，水平精确度，海拔高度，
 * 高度单位，大地水准面高度，高度单位，差分GPS数据期限，差分参考基站标号，
 * 校验和结束标记(用回车符<CR>和换行符<LF>)，分别用14个逗号进行分隔
 * ***********************************************************/


void gps_print_gpgga(const VMCHAR* str){

  VMCHAR latitude[20]; 
  VMCHAR longitude[20]; 
  VMCHAR buf[20]; 
  const VMCHAR* p = str; 
   
  p = next_token(p, 0); /* GGA */       //next_token找用“，”分开的字符串
  p = next_token(p, 0); /* Time */
  p = next_token(p, latitude); /* Latitude */
  p = next_token(p, 0); /* N */
  p = next_token(p, longitude); /* Longitude */
  p = next_token(p, 0); /* E */
  p = next_token(p, buf); /* fix quality */
   
  if(buf[0] == '1') 
  { 
    /* GPS fix */
    p = next_token(p, buf); /* number of satellites */
    vm_log_info("GPS is fixed: %d satellite(s) found!", atoi(buf)); 
    vm_log_info("Latitude:%d", latitude); 
    vm_log_info("Longitude:%d",longitude); 
  } 
  else 
  { 
    vm_log_info("GPS is not fixed yet."); 
  } 
} 

/* GPS callback. */
static void gps_callback(VM_GPS_MESSAGE message, void* data, void* user_data){

     switch(message){
     case VM_GPS_OPEN_RESULT:            //打开GPS
    {
             VMINT result = (VMINT)data;
             if(result == 1)
             {
                vm_log_info("open failed");
             }
             else
             {
                 vm_log_warn("open success");
                 /* set report period as 2 seconds */
                 vm_gps_set_parameters(VM_GPS_SET_LOCATION_REPORT_PERIOD, 2, NULL);//2分钟收集一次
             }
         }
         break;
     case VM_GPS_SENTENCE_DATA:          //接收GPS的汇报
         {
        	/* GPS data arriving */
            vm_gps_sentence_info_t info;
            memcpy(&info, (vm_gps_sentence_info_t*)data, sizeof(vm_gps_sentence_info_t));
            gps_print_gpgga(info.GPGGA);
         }
         break;
     }
 }

/* GPS power on. */
void gps_power_on(void)
{
  VMINT result;

  result = vm_gps_open( VM_GPS_ONLY, gps_callback, NULL);// GPS can be enhanced with the optional support of Beidou and GLONASS
  if(result == VM_SUCCESS)
  {
      vm_log_info("GPS open success");
  }
  else
  {
      vm_log_info("GPS open failed");
  }
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
  switch (message) {
  case VM_EVENT_CREATE:
    gps_power_on();
    break;
  case VM_EVENT_QUIT:
    vm_gps_close();
    break;  
  }
}

/* Entry point */
void vm_main(void) {
    vm_pmng_register_system_event_callback(handle_sysevt);
}
