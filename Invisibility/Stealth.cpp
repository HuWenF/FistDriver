#include "ntddk.h"


VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Hello World!!\r\n");

}



//这是相当于应用层的main函数 是入口
//驱动对象
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath) // 驱动入口
{
	//调用API打印
	KdPrint(("Hello! My First Driver"));
	pDriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;



}



