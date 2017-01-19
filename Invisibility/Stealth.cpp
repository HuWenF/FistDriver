#include "codemsg.h"





#define DEVICE L"\\Device\\MyDevece.com"
#define DosDEVICE L"\\DosDevices\\MyDosDevece.com"


//卸载驱动
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING DosDeviceName;

	//删除符号链接
	RtlInitUnicodeString(&DosDeviceName, DosDEVICE);
	IoDeleteSymbolicLink(&DosDeviceName);

	//删除设备对象
	if (DriverDeviceObject != NULL)
	{
		IoDeleteDevice(DriverDeviceObject);
	}
	DbgPrint("卸载流程执行成功!");

	

}


NTSTATUS IODispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;

	//完成例程，表示调用者已经完成了对指定请求所有处理操作
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}




BOOLEAN ValidateWCHARString(WCHAR *pwzStr, ULONG_PTR Length)
{
	ULONG i;
	__try
	{
		//第一步判断指针是否为NULL，
		if (*pwzStr == NULL || Length == 0)
		{
			return FALSE;
		}
		for (i = 0; i < Length; i++)
		{
			//检查每个字节里面的值是否是合法的，可读的 
			if (!MmIsAddressValid((PUCHAR)pwzStr + i))
			{
				//只要有一个字节是非法的，或者不可读的，就返回错误
				return FALSE;
			}
		}


	}
	__except (EXCEPTION_EXECUTE_HANDLER){ // 触发了异常就出去
		return FALSE;
	}

	return TRUE;

}

//进行IRP通信例程
NTSTATUS IOManager(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	//获取当前IrpStack，通过读取其结构成员，取出我们需要的控制代码IRPcode
	PIO_STACK_LOCATION StackLocation = IoGetCurrentIrpStackLocation(Irp);
	ULONG IRPcode = StackLocation->Parameters.DeviceIoControl.IoControlCode;
	WCHAR *buf;
	SIZE_T size;
	WCHAR *pwzCopyBuf = NULL;

	//这是从应用层传进来的缓冲变量以及大小
	buf = (WCHAR*)Irp->AssociatedIrp.SystemBuffer;
	size = Irp->Size;

	//IRP数据获取成功标志
	Irp->IoStatus.Status = STATUS_SUCCESS;

	//对于IRPCode的不同进行分别处理
	switch (IRPcode)
	{
	case CODEMSG(INIT_FILE_NAME):
			//从应用层吧这个传进来的buff进行严谨验证
			__try
			{
				if (ValidateWCHARString(buf, size)){
					//至此以及吧应用层数据传到了内核层
					DbgPrint("Buf == > %ws:%d\r\n");
					//申请内存吧字符串拷贝过来
					pwzCopyBuf = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, size, 'fp');

					if (pwzCopyBuf)
					{
						//copy到我们新申请的内存空间来
						memset(pwzCopyBuf, 0, size);
						memcpy(pwzCopyBuf, buf, size);

						DbgPrint("CopyBuf===> %ws\r\n", pwzCopyBuf);

						//记得释放
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




//这是相当于应用层的main函数 是入口
//驱动对象PDRIVER_OBJECT 
/*kd> dt _DRIVER_OBJECT
nt!_DRIVER_OBJECT
   +0x000 Type             : Int2B
   +0x002 Size             : Int2B
   +0x008 DeviceObject     : Ptr64 _DEVICE_OBJECT
   +0x010 Flags            : Uint4B
   +0x018 DriverStart      : Ptr64 Void                 这里是需要关心的地方，是驱动程序的开始地址，相当于PE头位置
   +0x020 DriverSize       : Uint4B                     驱动大小
   +0x028 DriverSection    : Ptr64 Void
   +0x030 DriverExtension  : Ptr64 _DRIVER_EXTENSION
   +0x038 DriverName       : _UNICODE_STRING            驱动名字
   +0x048 HardwareDatabase : Ptr64 _UNICODE_STRING
   +0x050 FastIoDispatch   : Ptr64 _FAST_IO_DISPATCH
   +0x058 DriverInit       : Ptr64     long 
   +0x060 DriverStartIo    : Ptr64     void 
   +0x068 DriverUnload     : Ptr64     void              驱动的卸载例程
   +0x070 MajorFunction    : [28] Ptr64     long */


extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath) // 驱动入口
{
	
	UNICODE_STRING DeviceName;  //设备链接名字
	UNICODE_STRING DosDeviceName;//与之对应的符号链接名字
	NTSTATUS status;




	//初始化驱动符号名字
	//UNICODE_STRING是个结构体，主要用作驱动编写的字符串

	RtlInitUnicodeString(&DeviceName, DEVICE);
	RtlInitUnicodeString(&DosDeviceName, DosDEVICE);


	//如果要和用户层通讯就需要一个设备，因为这个是才可以接受IRP（相当于应用层的消息）
	status = IoCreateDevice(
		pDriverObject,     //指向调用者的驱动对象指针
		0,
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DriverDeviceObject);      //返回的创建好的设备对象


	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(DriverDeviceObject); // 销毁设备
		return status;  // 吧错误返回回去
	}
	//创建一个符号链接
	status = IoCreateSymbolicLink(&DosDeviceName, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		//记得删除设备
		IoDeleteDevice(DriverDeviceObject);
		return status;
	}




	//驱动通讯例程 填写卸载驱动
	pDriverObject->DriverUnload = DriverUnload;

	//编写驱动对象的分发函数，这些函数用来进行只掉调用例程，
	//windows向设备对象发送IRP信息，然后设备对象吧IRP信息交给自己所属的驱动对象的分发函数去处理
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IODispatch;

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IODispatch;
	pDriverObject->MajorFunction[IRP_MJ_READ] = IODispatch;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = IODispatch;

	//一般我们跟应用层通信，都是通过IRP_MJ_DEVICE_CONTROL 这个例程，这个例程对应的是应用层下的DeviceIoConTrol
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOManager;



	//调用API打印
	KdPrint(("Hello! My First Driver"));
	
	return STATUS_SUCCESS;



}



