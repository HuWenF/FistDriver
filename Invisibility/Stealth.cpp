#include "ntddk.h"


VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Hello World!!\r\n");

}



//�����൱��Ӧ�ò��main���� �����
//��������
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath) // �������
{
	//����API��ӡ
	KdPrint(("Hello! My First Driver"));
	pDriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;



}



