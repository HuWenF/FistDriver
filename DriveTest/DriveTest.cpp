#include "codemsg.h"
#include "Ntstrsafe.h"





#define MAX_COM_NUMBER 1
static PDEVICE_OBJECT s_fltobj[MAX_COM_NUMBER] = { 0 };
static PDEVICE_OBJECT s_nextobj[MAX_COM_NUMBER] = { 0 };

//获得串口设备
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

	//打开设备对象
	*status = IoGetDeviceObjectPointer(&com_Name, FILE_ALL_ACCESS, &fileobj, &devobj);

	//如果打开成功，记得把文件对象解除引用，切记
	if (*status == STATUS_SUCCESS)
	{
		ObDereferenceObject(fileobj);
	}

	return devobj;


}

//绑定设备
NTSTATUS cppAttachDevice(PDRIVER_OBJECT driver, PDEVICE_OBJECT oldobj, PDEVICE_OBJECT *fltobj, PDEVICE_OBJECT *next)
{
	NTSTATUS status;

	PDEVICE_OBJECT topdev = NULL;

	//生成设备,然后绑定
	status = IoCreateDevice(driver,
		0,
		NULL,
		oldobj->DeviceType,
		0, FALSE, fltobj);

	if (status != STATUS_SUCCESS)
	{
		return status;
	}
	//拷贝重要标志位

	if (oldobj->Flags & DO_BUFFERED_IO)
		(*fltobj)->Flags |= DO_BUFFERED_IO;
	if (oldobj->Flags * DO_DIRECT_IO)
		(*fltobj)->Flags |= DO_DIRECT_IO;
	if (oldobj->Characteristics & FILE_DEVICE_SECURE_OPEN)
		(*fltobj)->Characteristics |= FILE_DEVICE_SECURE_OPEN;

	(*fltobj)->Flags |= DO_POWER_PAGABLE;

	//将一个设备绑定到另一个设备上
	status = IoAttachDeviceToDeviceStackSafe(*fltobj, oldobj, &topdev);


	if (status != STATUS_SUCCESS)
	{
		//失败后
		IoDeleteDevice(*fltobj);
		*fltobj = NULL;
		status = STATUS_UNSUCCESSFUL;
		return status;
	}
	*next = topdev;

	//这个设备以及启动
	(*fltobj)->Flags = (*fltobj)->Flags & ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;

}


//绑定所有端口
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

//开始编写分发函数
NTSTATUS cppDispatch(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
	NTSTATUS status;
	ULONG i, j;
	for (i = 0; i < MAX_COM_NUMBER; i++)
	{
		if (s_fltobj[i] == device)
		{
			//所有电源操作全部放过
			if (irpsp->MajorFunction == IRP_MJ_POWER)
			{
				//直接发送，然后返回说已经处理好了
				PoStartNextPowerIrp(irp);
				IoSkipCurrentIrpStackLocation(irp);
				return PoCallDriver(s_nextobj[i], irp);
			}
			//此外我们只过滤请求，写请求，获得缓冲区及其长度
			if (irpsp->MajorFunction == IRP_MJ_WRITE)
			{
				//如果是写,先获得其长度
				ULONG Len = irpsp->Parameters.Write.Length;

				//然后获得缓冲区
				PUCHAR buf = NULL;
				if (irp->MdlAddress != NULL)
					buf = (PUCHAR)MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
				else
					buf = (PUCHAR)irp->UserBuffer;
				if (buf == NULL)
				{
					buf = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
				}
				//打印内容
				for (j = 0; j < Len; j++)
				{
					DbgPrint("Comcap:Send Data:%2x \r\n", buf[j]);
				}
			}
			IoSkipCurrentIrpStackLocation(irp);
			return IoCallDriver(s_nextobj[i], irp);
		}
		

	}
	//如果根本就不在被绑定设备中，那就是有问题的，直接返回错误
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;



}




#define DELAY_ONE_MICROSEOND (-10)
#define DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSEOND*1000)
#define DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)
//卸载例程
VOID cppDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	ULONG i;
	LARGE_INTEGER interval;

	//首先解除绑定
	for (i = 0; i < MAX_COM_NUMBER; i++)
	{
		if (s_nextobj[i] != NULL)
			IoDetachDevice(s_nextobj[i]);

	}
	//沉睡5秒
	interval.QuadPart = (5 * 1000 * DELAY_ONE_MILLISECOND);
	KeDelayExecutionThread(KernelMode, FALSE, &interval);

	//删除这些设备
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
	//所有分发函数设置都是一样的
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = cppDispatch;
	}

	//支持动态卸载
	pDriverObject->DriverUnload = cppDriverUnload;
	
	//绑定所有串口
	cppAttachAllCom(pDriverObject);

	return STATUS_SUCCESS;


}