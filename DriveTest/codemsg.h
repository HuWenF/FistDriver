#include "ntddk.h"


#ifndef _DEFINE_H_
#define _DEFINE_H_

// _number:    0 -> 2047 : reserved for Microsoft ΢����
//             2047 -> 4095 : reserved for OEMs �û��Զ���     
#define CODEMSG(_number) CTL_CODE(FILE_DEVICE_UNKNOWN, _number , METHOD_BUFFERED,\
	FILE_READ_DATA | FILE_WRITE_DATA)


PDEVICE_OBJECT DriverDeviceObject; // ����һ���豸���������豸����

//���������
#define INIT_FILE_NAME 2047

#endif
