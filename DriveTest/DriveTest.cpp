#include "codemsg.h"


VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	//ж������
	DbgPrint("Unload Driver");

}



extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject, 
	IN PUNICODE_STRING pRegistryPath)
{

	DbgPrint("Hello World");

	//��װж������
	pDriverObject->DriverUnload = DriverUnload;

	return STATUS_SUCCESS;


}
















