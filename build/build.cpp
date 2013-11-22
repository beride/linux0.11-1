#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

DWORD g_dwFileHeader[1024] = {0};    //The PE file's header will be read into this buffer.

typedef struct __tagFILE_HEADER{
	unsigned char ucNop[4];
	DWORD         dwJmpAddr;
}__FILL_HEADER;

__FILL_HEADER g_FillHeader = {0x90,0x90,0x90,0xe9,0x00000000};    //This structure will be
                                                                  //written to target file.

char* g_lpszTargetPath = "E:\\book\\Temp\\linux011VC\\VC\\Release\\system";  //Target file¡¯s path and name.

void main(int argc,char argv[])
{
	IMAGE_DOS_HEADER*       ImageDosHeader = NULL;
	IMAGE_NT_HEADERS*       ImageNtHeader = NULL;
	IMAGE_OPTIONAL_HEADER*  ImageOptionalHeader = NULL;
	HANDLE                  hFile = INVALID_HANDLE_VALUE;
	DWORD                   dwReadBytes = 0L;
	BOOL                    bResult = FALSE;
	DWORD                   dwActualBytes = 0L;
	DWORD                   dwOffset = 0L;
	UCHAR*                  lpucSource = NULL;
	UCHAR*                  lpucDes    = NULL;
	DWORD                   dwLoop     = 0;

	hFile = CreateFile(                //Open the target file.
		g_lpszTargetPath,
		GENERIC_READ | GENERIC_WRITE,
		0L,
		NULL,
		OPEN_ALWAYS,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		printf("Can not open the target file to read.");
		goto __TERMINAL;
	}

	dwReadBytes = 4096;               //Read 4k bytes from target file.
	bResult = ReadFile(hFile,g_dwFileHeader,dwReadBytes,&dwActualBytes,NULL);
	if(!bResult)
		goto __TERMINAL;

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	//
	//The following code locates the entry point of the PE file,and modifies it.
	//
	ImageDosHeader = (IMAGE_DOS_HEADER*)&g_dwFileHeader[0];
	dwOffset = ImageDosHeader->e_lfanew;

	ImageNtHeader = (IMAGE_NT_HEADERS*)((UCHAR*)&g_dwFileHeader[0] + dwOffset);
	ImageOptionalHeader = &(ImageNtHeader->OptionalHeader);

	g_FillHeader.dwJmpAddr = ImageOptionalHeader->AddressOfEntryPoint;
	printf("    Entry Point : %d\r\n",ImageOptionalHeader->AddressOfEntryPoint);
	g_FillHeader.dwJmpAddr -= sizeof(__FILL_HEADER);    //Calculate the target address will
	                                                    //jump to.
	                                                    //Because we have added some nop instruc-
	                                                    //tions in front of the target file,so
	                                                    //we must adjust it.

	lpucSource = (UCHAR*)&g_FillHeader.ucNop[0];
	lpucDes    = (UCHAR*)&g_dwFileHeader[0];

	for(dwLoop = 0;dwLoop < sizeof(__FILL_HEADER);dwLoop ++)  //Modify the target file's header.
	{
		*lpucDes = *lpucSource;
		lpucDes ++;
		lpucSource ++;
	}

	hFile = CreateFile(                //Open the target file to write.
		g_lpszTargetPath,
		GENERIC_READ | GENERIC_WRITE,
		0L,
		NULL,
		OPEN_ALWAYS,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		printf("Can not open the target file to write.");
		goto __TERMINAL;
	}

	WriteFile(hFile,(LPVOID)&g_dwFileHeader[0],sizeof(__FILL_HEADER),&dwActualBytes,
		NULL);

	printf("SectionAligment : %d\r\n",ImageOptionalHeader->SectionAlignment);
	printf("   FileAligment : %d\r\n",ImageOptionalHeader->FileAlignment);

__TERMINAL:
	if(INVALID_HANDLE_VALUE != hFile)
		CloseHandle(hFile);
}
