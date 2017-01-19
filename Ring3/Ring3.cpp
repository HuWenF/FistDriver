#include <windows.h>  
#include <stdio.h>  
#include "codemsg.h"

#define DosDEVICE L"\\\\DosDevices\\MyDosDevece.com"

BOOL CallDrive(char *ID,char *buffer)
{
	HANDLE device = 0;
	WCHAR ToSend[512];
	int code = -1;
	char ret[1024];
	DWORD bytes;


	device = CreateFile(DosDEVICE, GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!device || device == INVALID_HANDLE_VALUE)
	{
		printf("打开驱动失败，驱动驱动不存在");
		return FALSE;
	}

	//判断是否输入的是-file
	if (!strcmp(ID, "-file"))
	{
		code = INIT_FILE_NAME;
	}
	
	//判断驱动的控制代码是否有效
	if (code == -1)
	{
		printf("无效的ID:%d\r\n", code);
		return FALSE;
	}

	//将ascii码lpbuffer字符串转unicode码字符串
	memset(ToSend, 0, sizeof(ToSend));
	MultiByteToWideChar(CP_ACP, 0, buffer, -1, ToSend, sizeof(ToSend));

	//开始填写通讯函数
	DeviceIoControl(device,
		CODEMSG(code),
		ToSend,
		(wcslen(ToSend) + 1) * 2,
		&ret,
		sizeof(ret),
		&bytes,
		NULL
		);

	//关闭驱动文件
	CloseHandle(device);
	printf("完成!\r\n");
	return TRUE;
}

void main(int argc, char *argv[])
{
	int result = 0;
	if (argv == NULL || argc != 3)
	{
		printf("输入的参数不正确!!\n");
		return;
	}

	CallDrive(argv[1], argv[2]);







}












