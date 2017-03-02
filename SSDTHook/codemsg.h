#include "ntddk.h"


#ifndef _DEFINE_H_
#define _DEFINE_H_

// _number:    0 -> 2047 : reserved for Microsoft 微软保留
//             2047 -> 4095 : reserved for OEMs 用户自定义     
#define CODEMSG(_number) CTL_CODE(FILE_DEVICE_UNKNOWN, _number , METHOD_BUFFERED,\
	FILE_READ_DATA | FILE_WRITE_DATA)


PDEVICE_OBJECT DriverDeviceObject; // 定义一个设备对象，用于设备创建

//定义控制码
#define INIT_FILE_NAME 2047


//定义一个变量来指定hook类型  有两种，一种直接修改ssdt表，一种mdl映射方式
BOOLEAN book_hook_type = TRUE;
#endif
