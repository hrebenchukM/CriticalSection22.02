#include "CriticalSectionDlg.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <tchar.h>
#include <ctime>
#include <string>
CriticalSectionDlg* CriticalSectionDlg::ptr = NULL;

CRITICAL_SECTION cs;//УСЛОВИЕ1 - Флажок переменная типа критикалсекшон

CriticalSectionDlg::CriticalSectionDlg(void)
{
	ptr = this;
}

CriticalSectionDlg::~CriticalSectionDlg(void)
{
	DeleteCriticalSection(&cs);//Когда закроем диалог то освободим ресурс єтот
}

void CriticalSectionDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CriticalSectionDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	InitializeCriticalSection(&cs);//обнуления флажка инициализация . Только после инициализации можно использовать переменную ,до нельзя 
	return TRUE;
}

void MessageAboutError(DWORD dwError)
{
	LPVOID lpMsgBuf = NULL;
	TCHAR szBuf[300]; 

	BOOL fOK = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if(lpMsgBuf != NULL)
	{
		wsprintf(szBuf, TEXT("Ошибка %d: %s"), dwError, lpMsgBuf); 
		MessageBox(0 ,szBuf, TEXT("Сообщение об ошибке"), MB_OK | MB_ICONSTOP); 
		LocalFree(lpMsgBuf); 
	}
}

DWORD WINAPI Write(LPVOID lp)//функция записи в файл.
{
	int numCopies = *reinterpret_cast<int*>(lp);// Получаем количество копий из параметров

	srand(time(0));
	EnterCriticalSection(&cs);

	for (int i = 0; i < numCopies; ++i)
	{
		ofstream out(TEXT("array") + to_wstring(i) + TEXT(".txt"));
		if (!out.is_open())
		{
			MessageAboutError(GetLastError());
			LeaveCriticalSection(&cs);
			return 1;
		}

		out << "HELLO WORLD" << ' ';
		out.close();
	}

	LeaveCriticalSection(&cs);


	MessageBox(0, TEXT("Поток записал информацию в файлы"), TEXT("Критическая секция"), MB_OK);
	return 0;





	//srand(time(0));
	//EnterCriticalSection(&cs); // захватили флажок себе
	//ofstream out(TEXT("array.txt"));
	//if(!out.is_open())
	//{
	//	MessageAboutError(GetLastError());
	//	return 1;
	//}
	//out << "HELLO WORLD" << ' ';
	////int A[100];
	////for(int i = 0; i < 100; i++)
	////{
	////	A[i] = rand()%50;
	////	out << A[i] << ' ';//массив записали в файл
	////}
	//out.close();
	//LeaveCriticalSection(&cs); // освобождение флажка. Поставили на место флажок назад чтоб брали другие.
	//MessageBox(0, TEXT("Поток записал информацию в файл"), TEXT("Критическая секция"), MB_OK);
	//return 0;
}

DWORD WINAPI Read(LPVOID lp)
{


	int numCopies = *reinterpret_cast<int*>(lp);// Получаем количество копий из параметров

	EnterCriticalSection(&cs);

	ofstream resultFile("result.txt", ios::app); // Открываем результирующий файл для дозаписи

	for (int i = 0; i < numCopies; ++i)
	{
		ifstream in(TEXT("array") + to_wstring(i) + TEXT(".txt"));
		if (!in.is_open())
		{
			MessageAboutError(GetLastError());
			LeaveCriticalSection(&cs);
			return 1;
		}

		string word;
		in >> word;
		resultFile << word << ' ';

		in.close();
	}

	resultFile.close();
	LeaveCriticalSection(&cs);

	MessageBox(0, TEXT("Поток прочитал информацию из файлов и записал в результирующий файл"), TEXT("Критическая секция"), MB_OK);
	return 0;





	//EnterCriticalSection(&cs);//захватили флажок после того как там освободили флажок.
	//ifstream in(TEXT("array.txt"));
	//if(!in.is_open())
	//{
	//	MessageAboutError(GetLastError());
	//	return 1;
	//}
	//string word;
	//in >> word;
	//
	////int B[100];
	////int sum = 0;
	////for(int i = 0; i < 100; i++)
	////{
	////	in >> B[i];//показали
	////	sum += B[i];
	////}
	//in.close();
	//LeaveCriticalSection(&cs);//освободили флажок
	//MessageBox(0, TEXT("Поток прочитал информацию из файла"), TEXT("Критическая секция"), MB_OK);
	////TCHAR str[30];
 //  //	wsprintf(str, TEXT("Сумма чисел равна %d"));
	////MessageBox(0, str, TEXT("Критическая секция"), MB_OK);
	//return 0;
}


void CriticalSectionDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BUTTON1)
	{
		int numCopies = 3; 
		HANDLE hThread = CreateThread(NULL, 0, Write, reinterpret_cast<LPVOID>(&numCopies), 0, NULL);
		//HANDLE hThread = CreateThread(NULL, 0, Write, 0, 0, NULL);
		CloseHandle(hThread);
		hThread = CreateThread(NULL, 0, Read, reinterpret_cast<LPVOID>(&numCopies), 0, NULL);
		CloseHandle(hThread);
	}
}


BOOL CALLBACK CriticalSectionDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}