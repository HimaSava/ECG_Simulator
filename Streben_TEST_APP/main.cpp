/*******************************************************
 *File name: main.cpp
 *
 *Description: This is the test app created for the ECG
 *             Simulator made by Himanshu S.
 *
 ******************************************************/

#include<windows.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<fstream>

#define FILE_MENU_START     1
#define FILE_MENU_EXIT      2
#define COM_5               3
#define COM_6               4
#define COM_7               5
#define COM_8               6


LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void AddMenu(HWND);
void SerialConnect();
void StartECG();
void decode(char[]);
void RecordECG(int[]);

HMENU hMenu;
DWORD dwEventMask;
HANDLE hComm;
HANDLE hECGData;
HWND hWnd;
char ComPortName[5];


int WINAPI WinMain (HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR args,int ncmdshow)
{
    WNDCLASSW                   wc = {0};

    MSG                         msg = {0};

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW ;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"TEST_APP";
    wc.lpfnWndProc = WndProc;

    if(!RegisterClassW(&wc))
        return(-1);

    hWnd = CreateWindow((LPCSTR)"TEST_APP",(LPCSTR)"TEST_APP",WS_OVERLAPPEDWINDOW | WS_VISIBLE,500,500,200,200,NULL,NULL,NULL,NULL);

    ShowWindow(hWnd,ncmdshow);
    UpdateWindow(hWnd);


    while(GetMessage(&msg,NULL,NULL,NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp)
{

    switch(msg)
    {
    case WM_COMMAND:

        switch(wp)
        {
        case FILE_MENU_START:
            MessageBeep(MB_OK);
            StartECG();
            break;
        case FILE_MENU_EXIT:
            MessageBeep(MB_OK);
            DestroyWindow(hWnd);
            break;
        case COM_5:
            strcpy(ComPortName,"COM5");
            SerialConnect();
            break;
        case COM_6:
            strcpy(ComPortName,"COM6");
            SerialConnect();
            break;
        case COM_7:
            strcpy(ComPortName,"COM7");
            SerialConnect();
            break;
        case COM_8:
            strcpy(ComPortName,"COM8");
            SerialConnect();
            break;
        }
        break;
    case WM_CREATE:
        AddMenu(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
            return DefWindowProcW(hWnd,msg,wp,lp);
    }
}

void AddMenu(HWND hWnd)
{
    hMenu= CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hComPort = CreateMenu();
    AppendMenu(hMenu,MF_POPUP,(UINT_PTR)hFileMenu,(LPCSTR)"File");
        AppendMenu(hFileMenu,MF_POPUP,(UINT_PTR)hComPort,(LPCSTR)"COM");
            AppendMenu(hComPort,MF_STRING,COM_5,(LPCSTR)"COM5");
            AppendMenu(hComPort,MF_STRING,COM_6,(LPCSTR)"COM6");
            AppendMenu(hComPort,MF_STRING,COM_7,(LPCSTR)"COM7");
            AppendMenu(hComPort,MF_STRING,COM_8,(LPCSTR)"COM8");
        AppendMenu(hFileMenu,MF_STRING,FILE_MENU_START,(LPCSTR)"Start");
        AppendMenu(hFileMenu,MF_SEPARATOR,NULL,NULL); // This statement adds a line between the menu components above and below the stat.
        AppendMenu(hFileMenu,MF_STRING,FILE_MENU_EXIT,(LPCSTR)"Exit");
    AppendMenu(hMenu,MF_STRING,2,(LPCSTR)"Help");
    SetMenu(hWnd,hMenu);
}

void SerialConnect()
{
	BOOL  Status;

	int i = 0;
	hComm = CreateFile( ComPortName,
		    GENERIC_READ | GENERIC_WRITE,
            0,
			NULL,
			OPEN_EXISTING,
		    0,
		    NULL);


			/*------------------------------- Setting the Parameters for the SerialPort ------------------------------*/

	DCB dcbSerialParams = { 0 };                         // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	Status = GetCommState(hComm, &dcbSerialParams);      //retreives  the current settings

	if (Status == FALSE)
    {
        MessageBox(hWnd,(LPCSTR)"ERROR OPENING!!",(LPCSTR)"COM PORT",MB_OK);
	}

	dcbSerialParams.BaudRate = CBR_115200;      // Setting BaudRate = 115200
	dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;        // Setting Parity = None

	Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB

	if (Status == FALSE)
	{
        MessageBox(hWnd,(LPCSTR)"ERROR OPENING!!",(LPCSTR)"COM PORT",MB_OK);
	}

			/*------------------------------------ Setting Timeouts --------------------------------------------------*/

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 50;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(hComm, &timeouts) == FALSE)
	{
        MessageBox(hWnd,(LPCSTR)"ERROR OPENING!!",(LPCSTR)"COM PORT",MB_OK);
	}

			/*------------------------------------ Setting Receive Mask ----------------------------------------------*/

	Status = SetCommMask(hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

	if (Status == FALSE)
    {
        MessageBox(hWnd,(LPCSTR)"ERROR OPENING!!",(LPCSTR)"COM PORT",MB_OK);
    }
    else
    {
        MessageBox(hWnd,(LPCSTR)"PORT OPENED SUCCESSFULLY",(LPCSTR)"COM PORT",MB_OK);
    }


           /*------------------------------------ Setting WaitComm() Event   ----------------------------------------*/

}

void StartECG()
{
    int flag, flag2, i;
    char        Temp;
    char        SerialBuffer[255];
    DWORD       NoBytesRead;
    char Name[] = "ECG.txt";
    WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received
    flag = 0;
    hECGData = CreateFile(Name,GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	while(flag!=100)
    {

        SerialBuffer[0]='/0';
		i=0;
		ReadFile(hComm, &Temp, sizeof(Temp), &NoBytesRead, NULL);
		if((int)Temp==-128)
        {
            flag++;
            do
            {
                ReadFile(hComm, &Temp, sizeof(Temp), &NoBytesRead, NULL);
                if( NoBytesRead >= 1)
                {
                    SerialBuffer[i] = Temp;
                    i++;
                }
            }
            while (i!=132);
            decode(SerialBuffer);
        }
    }
    CloseHandle(hComm);//Closing the Serial Port
    CloseHandle(hECGData);
}

void decode(char RawData[])
{
    int Packet[10], i, j, temp;
    Packet[0]=(((int)RawData[0] & 0x3F) << 6) + ((int)RawData[1] & 0x3F) - 2048;
    j=24;
    for(i=1;i<10;i++)
    {
        temp = (int)RawData[j] & 0x3F;
        if((int)RawData[j] >63)
        {
            temp *= -1;
        }
        Packet[i] = Packet[i-1] + temp;
        j += 12;
    }
    RecordECG(Packet);
    for(i=0;i<10;i++)
    {
        printf("%d ", Packet[i]);
    }
    printf("\n\n\n\n");
}

void RecordECG(int Packet[])
{
    int i;
    char c=',';
    for(i=0;i<10;i++)
    {
        WriteFile(hECGData, (PCVOID)Packet[i],(DWORD)sizeof((char)Packet[i]),NULL,NULL);
        WriteFile(hECGData, &c,(DWORD)sizeof(c),NULL,NULL);
    }

}
