#include "codemsg.h"
#include "Ntstrsafe.h"





#define MAX_COM_NUMBER 1
static PDEVICE_OBJECT s_fltobj[MAX_COM_NUMBER] = { 0 };
static PDEVICE_OBJECT s_nextobj[MAX_COM_NUMBER] = { 0 };

//��ô����豸
PDEVICE_OBJECT cppGetCom(ULONG id, NTSTATUS *status)
{
	
	ULONG i = 0;
	static WCHAR name[32] = { 0 };
	UNICODE_STRING com_Name;
	PFILE_OBJECT fileobj = NULL;
	PDEVICE_OBJECT devobj = NULL;

	
	memset(name, 0, sizeof(WCHAR)* 32);
	RtlStringCchPrintfW(
		name, 32,
		L"\\Device\\Serial%d", id);
	RtlInitUnicodeString(&com_Name, name);

	//���豸����
	*status = IoGetDeviceObjectPointer(&com_Name, FILE_ALL_ACCESS, &fileobj, &devobj);

	//����򿪳ɹ����ǵð��ļ����������ã��м�
	if (*status == STATUS_SUCCESS)
	{
		ObDereferenceObject(fileobj);
	}

	return devobj;


}

//���豸
NTSTATUS cppAttachDevice(PDRIVER_OBJECT driver, PDEVICE_OBJECT oldobj, PDEVICE_OBJECT *fltobj, PDEVICE_OBJECT *next)
{
	NTSTATUS status;

	PDEVICE_OBJECT topdev = NULL;

	//�����豸,Ȼ���
	status = IoCreateDevice(driver,
		0,
		NULL,
		oldobj->DeviceType,
		0, FALSE, fltobj);

	if (status != STATUS_SUCCESS)
	{
		return status;
	}
	//������Ҫ��־λ

	if (oldobj->Flags & DO_BUFFERED_IO)
		(*fltobj)->Flags |= DO_BUFFERED_IO;
	if (oldobj->Flags * DO_DIRECT_IO)
		(*fltobj)->Flags |= DO_DIRECT_IO;
	if (oldobj->Characteristics & FILE_DEVICE_SECURE_OPEN)
		(*fltobj)->Characteristics |= FILE_DEVICE_SECURE_OPEN;

	(*fltobj)->Flags |= DO_POWER_PAGABLE;

	//��һ���豸�󶨵���һ���豸��
	status = IoAttachDeviceToDeviceStackSafe(*fltobj, oldobj, &topdev);


	if (status != STATUS_SUCCESS)
	{
		//ʧ�ܺ�
		IoDeleteDevice(*fltobj);
		*fltobj = NULL;
		status = STATUS_UNSUCCESSFUL;
		return status;
	}
	*next = topdev;

	//����豸�Լ�����
	(*fltobj)->Flags = (*fltobj)->Flags & ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;

}


//�����ж˿�
void cppAttachAllCom(PDRIVER_OBJECT driver)
{
	ULONG i ;
	PDEVICE_OBJECT com_obj;
	NTSTATUS status;
	for (i = 0; i < MAX_COM_NUMBER; i++)
	{
		com_obj = cppGetCom(i, &status);
		if (com_obj == NULL)
		{
			continue;
		}
		cppAttachDevice(driver, com_obj, &s_fltobj[i], &s_nextobj[i]);
	}


}

//��ʼ��д�ַ�����
NTSTATUS cppDispatch(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
	NTSTATUS status;
	ULONG i, j;
	for (i = 0; i < MAX_COM_NUMBER; i++)
	{
		if (s_fltobj[i] == device)
		{
			//���е�Դ����ȫ���Ź�
			if (irpsp->MajorFunction == IRP_MJ_POWER)
			{
				//ֱ�ӷ��ͣ�Ȼ�󷵻�˵�Ѿ��������
				PoStartNextPowerIrp(irp);
				IoSkipCurrentIrpStackLocation(irp);
				return PoCallDriver(s_nextobj[i], irp);
			}
			//��������ֻ��������д���󣬻�û��������䳤��
			if (irpsp->MajorFunction == IRP_MJ_WRITE)
			{
				//�����д,�Ȼ���䳤��
				ULONG Len = irpsp->Parameters.Write.Length;

				//Ȼ���û�����
				PUCHAR buf = NULL;
				if (irp->MdlAddress != NULL)
					buf = (PUCHAR)MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
				else
					buf = (PUCHAR)irp->UserBuffer;
				if (buf == NULL)
				{
					buf = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
				}
				//��ӡ����
				for (j = 0; j < Len; j++)
				{
					DbgPrint("Comcap:Send Data:%2x \r\n", buf[j]);
				}
			}
			IoSkipCurrentIrpStackLocation(irp);
			return IoCallDriver(s_nextobj[i], irp);
		}
		

	}
	//��������Ͳ��ڱ����豸�У��Ǿ���������ģ�ֱ�ӷ��ش���
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;



}




#define DELAY_ONE_MICROSEOND (-10)
#define DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSEOND*1000)
#define DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)
//ж������
VOID cppDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	ULONG i;
	LARGE_INTEGER interval;

	//���Ƚ����
	for (i = 0; i < MAX_COM_NUMBER; i++)
	{
		if (s_nextobj[i] != NULL)
			IoDetachDevice(s_nextobj[i]);

	}
	//��˯5��
	interval.QuadPart = (5 * 1000 * DELAY_ONE_MILLISECOND);
	KeDelayExecutionThread(KernelMode, FALSE, &interval);

	//ɾ����Щ�豸
	for (i = 0; i < MAX_COM_NUMBER; i++)
	{
		if (s_fltobj[i] != NULL)
			IoDeleteDevice(s_fltobj[i]);
	}

}


extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject, 
	IN PUNICODE_STRING pRegistryPath)
{

	size_t i;
	//���зַ��������ö���һ����
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = cppDispatch;
	}

	//֧�ֶ�̬ж��
	pDriverObject->DriverUnload = cppDriverUnload;
	
	//�����д���
	cppAttachAllCom(pDriverObject);

	return STATUS_SUCCESS;


}