// Laba 4(Copying).cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Synchronizer.h"
#include <string>
#include <vector>
#include <wchar.h>
#include <map>

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hDlg;
HWND hSrcPath;
HWND hDstPath;
HWND hFilter;

#define IDC_SRC 1001
#define IDC_DST 1002
#define IDC_MASK 1003
#define IDC_SYMM 1004
#define IDC_FILTER 1008
#define IDC_STATE 1007
#define IDD_DIALOG 1000
#define IDC_START 1010

// ������������� ����������
bool DirectoryExists(const wstring &szPath)
{
  // �������� �������� ����������
  DWORD dwAttrib = GetFileAttributes(szPath.c_str());
  // ���� ��������� ���������, ���������� TRUE, ����� - FALSE
  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
// ��������� ���� UNICODE-�����(������� ��� wcsicmp)
inline bool ieq(const wstring &str1, const wstring &str2)
{
	return ((!wcsicmp(str1.c_str(),str2.c_str()))?true:false);
}
// ��������� ������ ��������� ��� ���������� ��������
bool getDirs(vector<wstring> &buf, const wstring &path)
{
	// ���������, ���������� �� �������
	if(!DirectoryExists(path.c_str()))
		return false;

	// ����������� ����������, ������ ��� ������������ ����������� ��������
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	wstring szDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	// ��������� ����� ��� ������
	szDir = path + L"\\*";

	// ������� ������ "�����"
	hFind = FindFirstFile(szDir.c_str(), &ffd);

	// ���������, �������� �� ������
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return false;
	} 
	
	// ���������� �������� ����� �� -2, ����� ���������� ������ ��� ��������
	// ��� ��� �������� ��������� �� ������� � ������������ ��������
	int flag = -2;
	do
	{
		// ���� ����� ���� ����������
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// ���� ��� ���������� �������� ��������
			if(!flag)
			{
				// ��������� ������� � ������
				buf.push_back(ffd.cFileName);
			}
			// ����� ���������� � ����� �������
			else flag++;
		}
	}
	// ������, ���� ������� �� ��������
	while (FindNextFile(hFind, &ffd) != 0);
	
	// ����� ������ ������ �� ������ � ������
	szDir.clear();

	// ���������, ������� �� ������
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return false;
	}

	// ������� "�����"
	FindClose(hFind);
	// ���������� ����� "�� ������"
	return true;	
}
// ��������� ������ ������
bool getFiles(map<wstring,FILETIME> &buf, const wstring &path, const wstring &filter)
{
	// ��������� �� ������������
	if(!DirectoryExists(path.c_str()))
		return false;

	// ����������
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	wstring szDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	// ������� �����
	szDir = path + L"\\" + filter;

	// ������� ������ "�����"
	hFind = FindFirstFile(szDir.c_str(), &ffd);

	// ���������, ������ �� ������
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return false;
	} 
	
	// ���� �� �������� ��������
	do
	{
		// ���� ����� ���� �� �������
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// ��������� � ������ ��� ����� � ���� ���������� �������
			buf.insert(pair<wstring,FILETIME>(ffd.cFileName,ffd.ftLastAccessTime));
		}
	}
	// � ���, ���� �� �������� �����
	while (FindNextFile(hFind, &ffd) != 0);
	
	// ������ �����
	szDir.clear();

	// ���������, �� ���� �� ������
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return false;
	}

	// ������� ������ "�����"
	FindClose(hFind);

	// ���������� "�� ������"
	return true;	
}
// �������� �������� ������ � �������������
bool rmdir(const wstring &path)
{
	// ���������, ���������� ��
	if(!DirectoryExists(path.c_str()))
		return false;

	// ����������
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	wstring szDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	// ������� �����
	szDir = path + L"\\*";

	// ������ "�����"
	hFind = FindFirstFile(szDir.c_str(), &ffd);

	// ���������, �������� �� "�����"
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return false;
	} 
	
	// ���� ��� �������� ���� ������ ���������
	int flag = -2;
	// ���� �� ���������
	do
	{
		// ���� �� �������
		wstring tmp = (path + L"\\") + ffd.cFileName;
		// ���� ����������
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// ���� ���������� �������
			if(flag)
				// ����������
				flag++;
			else // �����
				// �������
				rmdir(tmp);
		}else{
			// ���� ����, �� �������
			DeleteFile(tmp.c_str());
		}
		// ������ ������ � ����
		tmp.clear();
	}
	// ���� �� �������� �������
	while (FindNextFile(hFind, &ffd) != 0);
	// ������ ������ � ������
	szDir.clear();
	// ��������� �� ������
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return false;
	}
	// ������� "�����"
	FindClose(hFind);
	// ������� ��������� �������
	RemoveDirectory(path.c_str());
	// ���������� "�� ������"
	return true;
}
// �������������
bool sync(const wstring &src, const wstring &dst, bool symm, const wstring &filter)
{
	// �������� ����������?
	if(!DirectoryExists(src))
		return false;
	if(!DirectoryExists(dst))
		return false;
	
	// ������� ������ ����������
	vector<wstring> srcDirs, dstDirs;
	map<wstring,FILETIME> srcFiles, dstFiles;
	wstring t0,t1;

	// �������� ������ ������ � ���������� ��� ������� �����, ��������� ������
	getDirs(srcDirs,src);
	getDirs(dstDirs,dst);
	getFiles(srcFiles,src,filter);
	getFiles(dstFiles,dst,filter);

	// ���� �� ������ ��������-���������
	for(auto s = srcDirs.begin(); s != srcDirs.end(); s++)
	{
		// ����, ����������, ������ �� ������� ������� � ��������-���������
		bool f = false;
		// ���� �� ��������� ������ ���������
		for(auto d = dstDirs.begin(); d != dstDirs.end(); d++)
		{
			// ���� ����� �������
			if(ieq((*s),(*d)))
			{
				// ������ ����
				f = true;
				// ������� ��� �� ���������
				dstDirs.erase(d);
				// ������� �� �����
				break;
			}
		}
		// ��������� ���� �� ���������
		t0 = src + L"\\" + *s;
		t1 = dst + L"\\" + *s;
		// ���� � ��������� ��� ������ ��������
		if(!f)
		{
			// �� ��������
			CreateDirectory(t1.c_str(),NULL);
		}
		// �������������
		sync(t0,t1,symm,filter);
	}
	// ���� �� ���������� � ��������� ���������
	for(auto d = dstDirs.begin(); d != dstDirs.end(); d++)
	{
		// ��������� ����
		t1 = dst + L"\\" + *d;
		// ���� ������������� ������������
		if(symm)
		{
			// ��������� ���� �� �������� ������ ���������
			t0 = src + L"\\" + *d;
			// ������� �������
			CreateDirectory(t0.c_str(),NULL);
			// ��������� ��������������
			sync(t0,t1,true,filter);
		}else{ // �����
			// ������� ������� �� ���������
			rmdir(t1);
		}
	}
	// ���� �� ������ ���������
	for(auto s = srcFiles.begin(); s != srcFiles.end(); s++)
	{
		// ����, ���������� ����������� �����������
		bool f = true;
		// ���� �� ������ ���������
		for(auto d = dstFiles.begin(); d != dstFiles.end(); d++)
		{
			// ���� ����� ���������
			if(ieq((*s).first,(*d).first))
			{
				// ��������� ����
				if(*(unsigned __int64*)&((*s).second) < *(unsigned __int64*)&((*d).second))
					// ���� ���� ��������, ���������� ����
					f = false;
				// ������� ��� �� ���������
				dstFiles.erase(d);
				// ������� �� �����
				break;
			}
		}
		
		// ��������� ����
		t0 = src + L"\\" + (*s).first;
		t1 = dst + L"\\" + (*s).first;
		// ���� ����������� ������������(��������->��������)
		if(f)
		{
			// �������� ���� �� ��������� � ��������
			CopyFile(t0.c_str(),t1.c_str(),FALSE);
		}
		else // ���� ������������
		{
			// � ������������� �����������
			if(symm)
				// �������� ���� �� ��������� � ��������
				CopyFile(t1.c_str(),t0.c_str(),FALSE);
		}
	}
	// ���� �� ������ ���������
	for(auto d = dstFiles.begin(); d != dstFiles.end(); d++)
	{
		// ����
		t1 = dst + L"\\" + (*d).first;
		// ������������� �����������?
		if(symm)
		{
			// ����� ���������� ���� � ���������
			t0 = src + L"\\" + (*d).first;
			// � ��������� ���� �� ���������� � ��������
			CopyFile(t1.c_str(),t0.c_str(),FALSE);
		}else{
			//����� ������� ���� �� ���������
			DeleteFile(t1.c_str());
		}
	}
	// ������ ������
	t0.clear();
	t1.clear();
	srcDirs.clear();
	srcFiles.clear();
	dstDirs.clear();
	dstFiles.clear();
	// ���������� "�� ������"
	return true;
}

INT_PTR CALLBACK DialogProc(
  _In_  HWND hwndDlg,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
)
{
	// ���� �� ���������/��������� + ������
	wchar_t *src_path, *dst_path, *filter;
	// ����� ������� � ��������������
	bool use_filter = false, symm = false;
	// ��������� ����������
	int tmp;
	switch(uMsg) // ������������ ���������
	{
	case WM_INITDIALOG: // ������������� �������
		// �������� ����������� ����� ����� �����
		hSrcPath = GetDlgItem(hwndDlg,IDC_SRC);
		hDstPath = GetDlgItem(hwndDlg,IDC_DST);
		hFilter = GetDlgItem(hwndDlg,IDC_MASK);
		break;
	case WM_COMMAND: // � ������ �������
		switch(wParam)
		{
		case IDC_START: // ���� ���� ������ ������ "�����"
			// �������� �����
			if(IsDlgButtonChecked(hwndDlg,IDC_FILTER))
				use_filter = true;
			if(IsDlgButtonChecked(hwndDlg,IDC_SYMM))
				symm = true;
			// �������� ����
			tmp = GetWindowTextLength(hSrcPath) + 1;
			src_path = new wchar_t[tmp];
			SendMessage(hSrcPath,WM_GETTEXT,tmp, (LPARAM)src_path);

			tmp = GetWindowTextLength(hDstPath) + 1;
			dst_path = new wchar_t[tmp];
			SendMessage(hDstPath,WM_GETTEXT,tmp, (LPARAM)dst_path);
			// ���� ������������ ������
			if(use_filter)
			{
				// �������� ������ �������
				tmp = GetWindowTextLength(hFilter) + 1;
				filter = new wchar_t[tmp];
				SendMessage(hFilter,WM_GETTEXT,tmp, (LPARAM)filter);

				// ��������� �������������
				sync(src_path,dst_path,symm,filter);

				// ����������� ������
				delete filter;
			}else{
				// ����� ������������� �� ���� ������
				sync(src_path,dst_path,symm,L"*");
			}
			// ����������� ������
			delete src_path;
			delete dst_path;
		}
		break;

	case WM_CLOSE: // ���� ��������
		DestroyWindow(hwndDlg); // ����� ����
		return TRUE;

	case WM_DESTROY: // ���� �������
		PostQuitMessage(0); // ��������� ������
		return TRUE;
	}

  return FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	hInst = hInstance;
	// ������� ���������� ����
	DialogBoxParam(hInst,(LPCWSTR)IDD_DIALOG,0,DialogProc,0);
	// ������� �� �����
	ShowWindow(hDlg,nCmdShow);
	
	BOOL ret;
	MSG msg;
	// ���� ��������� ��������� � ������� �� �� ���������
	while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
	  if(ret == -1) /* error found */
		return -1;

	  if(!IsDialogMessage(hDlg, &msg)) {
		TranslateMessage(&msg); /* translate virtual-key messages */
		DispatchMessage(&msg); /* send it to dialog procedure */
	  }
	}
}