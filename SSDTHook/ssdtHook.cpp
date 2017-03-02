#include "codemsg.h"
#include "Ntstrsafe.h"



//直接修改SSDT表来进行hook
NTSTATUS Cr0SSDTHook(int *index, ULONG_PTR *ul_save_real_address, ULONG_PTR ul_hook_address)
{







}







//卸载例程
VOID cppDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	

}


extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath)
{


	//支持动态卸载
	pDriverObject->DriverUnload = cppDriverUnload;


	//定义一个BOOLEAN 
	if (book_hook_type)
	{
		


	}







	return STATUS_SUCCESS;


}


























