#include "codemsg.h"





#define DEVICE L"\\Device\\MyDevece.com"
#define DosDEVICE L"\\DosDevices\\MyDosDevece.com"


//ж������
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING DosDeviceName;

	//ɾ����������
	RtlInitUnicodeString(&DosDeviceName, DosDEVICE);
	IoDeleteSymbolicLink(&DosDeviceName);

	//ɾ���豸����
	if (DriverDeviceObject != NULL)
	{
		IoDeleteDevice(DriverDeviceObject);
	}
	DbgPrint("ж������ִ�гɹ�!");

	

}


NTSTATUS IODispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;

	//������̣���ʾ�������Ѿ�����˶�ָ���������д������
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}




BOOLEAN ValidateWCHARString(WCHAR *pwzStr, ULONG_PTR Length)
{
	ULONG i;
	__try
	{
		//��һ���ж�ָ���Ƿ�ΪNULL��
		if (*pwzStr == NULL || Length == 0)
		{
			return FALSE;
		}
		for (i = 0; i < Length; i++)
		{
			//���ÿ���ֽ������ֵ�Ƿ��ǺϷ��ģ��ɶ��� 
			if (!MmIsAddressValid((PUCHAR)pwzStr + i))
			{
				//ֻҪ��һ���ֽ��ǷǷ��ģ����߲��ɶ��ģ��ͷ��ش���
				return FALSE;
			}
		}


	}
	__except (EXCEPTION_EXECUTE_HANDLER){ // �������쳣�ͳ�ȥ
		return FALSE;
	}

	return TRUE;

}

//����IRPͨ������
NTSTATUS IOManager(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	//��ȡ��ǰIrpStack��ͨ����ȡ��ṹ��Ա��ȡ��������Ҫ�Ŀ��ƴ���IRPcode
	PIO_STACK_LOCATION StackLocation = IoGetCurrentIrpStackLocation(Irp);
	ULONG IRPcode = StackLocation->Parameters.DeviceIoControl.IoControlCode;
	WCHAR *buf;
	SIZE_T size;
	WCHAR *pwzCopyBuf = NULL;

	//���Ǵ�Ӧ�ò㴫�����Ļ�������Լ���С
	buf = (WCHAR*)Irp->AssociatedIrp.SystemBuffer;
	size = Irp->Size;

	//IRP���ݻ�ȡ�ɹ���־
	Irp->IoStatus.Status = STATUS_SUCCESS;

	//����IRPCode�Ĳ�ͬ���зֱ���
	switch (IRPcode)
	{
	case CODEMSG(INIT_FILE_NAME):
			//��Ӧ�ò�������������buff�����Ͻ���֤
			__try
			{
				if (ValidateWCHARString(buf, size)){
					//�����Լ���Ӧ�ò����ݴ������ں˲�
					DbgPrint("Buf == > %ws:%d\r\n");
					//�����ڴ���ַ�����������
					pwzCopyBuf = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, size, 'fp');

					if (pwzCopyBuf)
					{
						//copy��������������ڴ�ռ���
						memset(pwzCopyBuf, 0, size);
						memcpy(pwzCopyBuf, buf, size);

						DbgPrint("CopyBuf===> %ws\r\n", pwzCopyBuf);

						//�ǵ��ͷ�
						ExFreePool(pwzCopyBuf);
					}

				}
			}

			__except (EXCEPTION_EXECUTE_HANDLER){
				Irp->IoStatus.Status = GetExceptionCode();
			}
			break;
			
	
	}
	return STATUS_SUCCESS;



}




//�����൱��Ӧ�ò��main���� �����
//��������PDRIVER_OBJECT 
/*kd> dt _DRIVER_OBJECT
nt!_DRIVER_OBJECT
   +0x000 Type             : Int2B
   +0x002 Size             : Int2B
   +0x008 DeviceObject     : Ptr64 _DEVICE_OBJECT
   +0x010 Flags            : Uint4B
   +0x018 DriverStart      : Ptr64 Void                 ��������Ҫ���ĵĵط�������������Ŀ�ʼ��ַ���൱��PEͷλ��
   +0x020 DriverSize       : Uint4B                     ������С
   +0x028 DriverSection    : Ptr64 Void
   +0x030 DriverExtension  : Ptr64 _DRIVER_EXTENSION
   +0x038 DriverName       : _UNICODE_STRING            ��������
   +0x048 HardwareDatabase : Ptr64 _UNICODE_STRING
   +0x050 FastIoDispatch   : Ptr64 _FAST_IO_DISPATCH
   +0x058 DriverInit       : Ptr64     long 
   +0x060 DriverStartIo    : Ptr64     void 
   +0x068 DriverUnload     : Ptr64     void              ������ж������
   +0x070 MajorFunction    : [28] Ptr64     long */


extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath) // �������
{
	
	UNICODE_STRING DeviceName;  //�豸��������
	UNICODE_STRING DosDeviceName;//��֮��Ӧ�ķ�����������
	NTSTATUS status;




	//��ʼ��������������
	//UNICODE_STRING�Ǹ��ṹ�壬��Ҫ����������д���ַ���

	RtlInitUnicodeString(&DeviceName, DEVICE);
	RtlInitUnicodeString(&DosDeviceName, DosDEVICE);


	//���Ҫ���û���ͨѶ����Ҫһ���豸����Ϊ����ǲſ��Խ���IRP���൱��Ӧ�ò����Ϣ��
	status = IoCreateDevice(
		pDriverObject,     //ָ������ߵ���������ָ��
		0,
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DriverDeviceObject);      //���صĴ����õ��豸����


	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(DriverDeviceObject); // �����豸
		return status;  // �ɴ��󷵻ػ�ȥ
	}
	//����һ����������
	status = IoCreateSymbolicLink(&DosDeviceName, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		//�ǵ�ɾ���豸
		IoDeleteDevice(DriverDeviceObject);
		return status;
	}




	//����ͨѶ���� ��дж������
	pDriverObject->DriverUnload = DriverUnload;

	//��д��������ķַ���������Щ������������ֻ���������̣�
	//windows���豸������IRP��Ϣ��Ȼ���豸�����IRP��Ϣ�����Լ���������������ķַ�����ȥ����
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IODispatch;

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IODispatch;
	pDriverObject->MajorFunction[IRP_MJ_READ] = IODispatch;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = IODispatch;

	//һ�����Ǹ�Ӧ�ò�ͨ�ţ�����ͨ��IRP_MJ_DEVICE_CONTROL ������̣�������̶�Ӧ����Ӧ�ò��µ�DeviceIoConTrol
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOManager;



	//����API��ӡ
	KdPrint(("Hello! My First Driver"));
	
	return STATUS_SUCCESS;



}



