#include "codemsg.h"
#include "Ntstrsafe.h"



//ֱ���޸�SSDT��������hook
NTSTATUS Cr0SSDTHook(int *index, ULONG_PTR *ul_save_real_address, ULONG_PTR ul_hook_address)
{







}







//ж������
VOID cppDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	

}


extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath)
{


	//֧�ֶ�̬ж��
	pDriverObject->DriverUnload = cppDriverUnload;


	//����һ��BOOLEAN 
	if (book_hook_type)
	{
		


	}







	return STATUS_SUCCESS;


}


























