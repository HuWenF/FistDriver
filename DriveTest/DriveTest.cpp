#include "codemsg.h"


VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	//卸载例程
	DbgPrint("Unload Driver");

}



extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject, 
	IN PUNICODE_STRING pRegistryPath)
{

	DbgPrint("Hello World");

	//安装卸载例程
	pDriverObject->DriverUnload = DriverUnload;

	return STATUS_SUCCESS;


}
















