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

// существование директории
bool DirectoryExists(const wstring &szPath)
{
  // получаем атрибуты директории
  DWORD dwAttrib = GetFileAttributes(szPath.c_str());
  // если результат корректен, возвращаем TRUE, иначе - FALSE
  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
// сравнение двух UNICODE-строк(обертка для wcsicmp)
inline bool ieq(const wstring &str1, const wstring &str2)
{
	return ((!wcsicmp(str1.c_str(),str2.c_str()))?true:false);
}
// получение списка каталогов для некоторого каталога
bool getDirs(vector<wstring> &buf, const wstring &path)
{
	// проверяем, существует ли каталог
	if(!DirectoryExists(path.c_str()))
		return false;

	// настраиваем переменные, нужные для перечисления содержимого каталога
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	wstring szDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	// формируем маску для поиска
	szDir = path + L"\\*";

	// создаем объект "поиск"
	hFind = FindFirstFile(szDir.c_str(), &ffd);

	// проверяем, создался ли объект
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return false;
	} 
	
	// выставляем значение флага на -2, чтобы пропустить первые два каталога
	// эти два каталога указывают на текущий и родительский каталоги
	int flag = -2;
	do
	{
		// если перед нами директория
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// если два проблемных каталога пройдены
			if(!flag)
			{
				// добавляем каталог в список
				buf.push_back(ffd.cFileName);
			}
			// иначе прибавляем к флагу единицу
			else flag++;
		}
	}
	// делаем, пока объекты не кончатся
	while (FindNextFile(hFind, &ffd) != 0);
	
	// далее чистим память от строки с маской
	szDir.clear();

	// проверяем, имеются ли ошибки
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return false;
	}

	// удаляем "поиск"
	FindClose(hFind);
	// возвращаем ответ "всё хорошо"
	return true;	
}
// получение списка файлов
bool getFiles(map<wstring,FILETIME> &buf, const wstring &path, const wstring &filter)
{
	// проверяем на существовние
	if(!DirectoryExists(path.c_str()))
		return false;

	// переменные
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	wstring szDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	// готовим маску
	szDir = path + L"\\" + filter;

	// создаем объект "поиск"
	hFind = FindFirstFile(szDir.c_str(), &ffd);

	// проверяем, создан ли объект
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return false;
	} 
	
	// цикл по найденым объектам
	do
	{
		// если перед нами не каталог
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// добавляем в список имя файла и дату последнего доступа
			buf.insert(pair<wstring,FILETIME>(ffd.cFileName,ffd.ftLastAccessTime));
		}
	}
	// и так, пока не кончатся файлы
	while (FindNextFile(hFind, &ffd) != 0);
	
	// чистим маску
	szDir.clear();

	// проверяем, не было ли ошибок
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return false;
	}

	// удаляем объект "поиск"
	FindClose(hFind);

	// возвращаем "всё хорошо"
	return true;	
}
// удаление каталога вместе с подкаталогами
bool rmdir(const wstring &path)
{
	// проверяем, существует ли
	if(!DirectoryExists(path.c_str()))
		return false;

	// переменные
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	wstring szDir;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	// готовим маску
	szDir = path + L"\\*";

	// объект "поиск"
	hFind = FindFirstFile(szDir.c_str(), &ffd);

	// проверяем, создался ли "поиск"
	if (INVALID_HANDLE_VALUE == hFind) 
	{
		return false;
	} 
	
	// флаг для пропуска двух первых каталогов
	int flag = -2;
	// цикл по найденому
	do
	{
		// путь до объекта
		wstring tmp = (path + L"\\") + ffd.cFileName;
		// если директория
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// если проблемный каталог
			if(flag)
				// пропускаем
				flag++;
			else // иначе
				// удаляем
				rmdir(tmp);
		}else{
			// если файл, то удаляем
			DeleteFile(tmp.c_str());
		}
		// чистим строку с путём
		tmp.clear();
	}
	// пока не кончатся объекты
	while (FindNextFile(hFind, &ffd) != 0);
	// чистим строку с маской
	szDir.clear();
	// проверяем на ошибки
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
		return false;
	}
	// удаляем "поиск"
	FindClose(hFind);
	// удаляем очищенный каталог
	RemoveDirectory(path.c_str());
	// возвращаем "всё хорошо"
	return true;
}
// синхронизация
bool sync(const wstring &src, const wstring &dst, bool symm, const wstring &filter)
{
	// каталоги существуют?
	if(!DirectoryExists(src))
		return false;
	if(!DirectoryExists(dst))
		return false;
	
	// создаем нужные переменные
	vector<wstring> srcDirs, dstDirs;
	map<wstring,FILETIME> srcFiles, dstFiles;
	wstring t0,t1;

	// получаем списки файлов и директорий для входных путей, используя фильтр
	getDirs(srcDirs,src);
	getDirs(dstDirs,dst);
	getFiles(srcFiles,src,filter);
	getFiles(dstFiles,dst,filter);

	// цикл по папкам каталога-источника
	for(auto s = srcDirs.begin(); s != srcDirs.end(); s++)
	{
		// флаг, означающий, найден ли текущий каталог в каталоге-приемнике
		bool f = false;
		// цикл по каталогам внутри приемника
		for(auto d = dstDirs.begin(); d != dstDirs.end(); d++)
		{
			// если имена совпали
			if(ieq((*s),(*d)))
			{
				// ставим флаг
				f = true;
				// удаляем имя из приемника
				dstDirs.erase(d);
				// выходим из цикла
				break;
			}
		}
		// формируем пути до каталогов
		t0 = src + L"\\" + *s;
		t1 = dst + L"\\" + *s;
		// если в приемнике нет такого каталога
		if(!f)
		{
			// то создадим
			CreateDirectory(t1.c_str(),NULL);
		}
		// синхронизация
		sync(t0,t1,symm,filter);
	}
	// цикл по оставшимся в приемнике каталогам
	for(auto d = dstDirs.begin(); d != dstDirs.end(); d++)
	{
		// формируем путь
		t1 = dst + L"\\" + *d;
		// если синхронизация симметричная
		if(symm)
		{
			// формируем путь до каталога внутри источника
			t0 = src + L"\\" + *d;
			// создаем каталог
			CreateDirectory(t0.c_str(),NULL);
			// выполняем синхронизакцию
			sync(t0,t1,true,filter);
		}else{ // иначе
			// удаляем каталог из приемника
			rmdir(t1);
		}
	}
	// цикл по файлам источника
	for(auto s = srcFiles.begin(); s != srcFiles.end(); s++)
	{
		// флаг, означающий направление копирования
		bool f = true;
		// цикл по файлам приемника
		for(auto d = dstFiles.begin(); d != dstFiles.end(); d++)
		{
			// если имена совпадают
			if(ieq((*s).first,(*d).first))
			{
				// проверяем даты
				if(*(unsigned __int64*)&((*s).second) < *(unsigned __int64*)&((*d).second))
					// если даты нравятся, сбрасываем флаг
					f = false;
				// удаляем имя из приемника
				dstFiles.erase(d);
				// выходим из цикла
				break;
			}
		}
		
		// формируем пути
		t0 = src + L"\\" + (*s).first;
		t1 = dst + L"\\" + (*s).first;
		// если направление положительно(источник->приемник)
		if(f)
		{
			// копируем файл из источника в приемник
			CopyFile(t0.c_str(),t1.c_str(),FALSE);
		}
		else // если отрицательно
		{
			// и синхронизация симметрична
			if(symm)
				// копируем файл из приемника в источник
				CopyFile(t1.c_str(),t0.c_str(),FALSE);
		}
	}
	// цикл по файлам приемника
	for(auto d = dstFiles.begin(); d != dstFiles.end(); d++)
	{
		// путь
		t1 = dst + L"\\" + (*d).first;
		// синхронизация симметрична?
		if(symm)
		{
			// тогда сформируем путь в источнике
			t0 = src + L"\\" + (*d).first;
			// и скопируем файл из приемникам в источник
			CopyFile(t1.c_str(),t0.c_str(),FALSE);
		}else{
			//иначе удаляем файл из приемника
			DeleteFile(t1.c_str());
		}
	}
	// чистим память
	t0.clear();
	t1.clear();
	srcDirs.clear();
	srcFiles.clear();
	dstDirs.clear();
	dstFiles.clear();
	// возвращаем "всё хорошо"
	return true;
}

INT_PTR CALLBACK DialogProc(
  _In_  HWND hwndDlg,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
)
{
	// пути до приемника/источника + фильтр
	wchar_t *src_path, *dst_path, *filter;
	// флаги фильтра и симметричности
	bool use_filter = false, symm = false;
	// временная переменная
	int tmp;
	switch(uMsg) // обрабатываем сообщения
	{
	case WM_INITDIALOG: // инициализация диалога
		// получаем дескрипторы наших полей ввода
		hSrcPath = GetDlgItem(hwndDlg,IDC_SRC);
		hDstPath = GetDlgItem(hwndDlg,IDC_DST);
		hFilter = GetDlgItem(hwndDlg,IDC_MASK);
		break;
	case WM_COMMAND: // в случае команды
		switch(wParam)
		{
		case IDC_START: // если была нажата кнопка "старт"
			// получить флаги
			if(IsDlgButtonChecked(hwndDlg,IDC_FILTER))
				use_filter = true;
			if(IsDlgButtonChecked(hwndDlg,IDC_SYMM))
				symm = true;
			// получить пути
			tmp = GetWindowTextLength(hSrcPath) + 1;
			src_path = new wchar_t[tmp];
			SendMessage(hSrcPath,WM_GETTEXT,tmp, (LPARAM)src_path);

			tmp = GetWindowTextLength(hDstPath) + 1;
			dst_path = new wchar_t[tmp];
			SendMessage(hDstPath,WM_GETTEXT,tmp, (LPARAM)dst_path);
			// если используется фильтр
			if(use_filter)
			{
				// полуачем строку фильтра
				tmp = GetWindowTextLength(hFilter) + 1;
				filter = new wchar_t[tmp];
				SendMessage(hFilter,WM_GETTEXT,tmp, (LPARAM)filter);

				// выполняем синхронизацию
				sync(src_path,dst_path,symm,filter);

				// освобождаем память
				delete filter;
			}else{
				// иначе синхронизация по всем файлам
				sync(src_path,dst_path,symm,L"*");
			}
			// освобождаем память
			delete src_path;
			delete dst_path;
		}
		break;

	case WM_CLOSE: // если закрытие
		DestroyWindow(hwndDlg); // рушим окно
		return TRUE;

	case WM_DESTROY: // окно рушится
		PostQuitMessage(0); // завершаем работу
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
	// создаем диалоговое окно
	DialogBoxParam(hInst,(LPCWSTR)IDD_DIALOG,0,DialogProc,0);
	// выводим на экран
	ShowWindow(hDlg,nCmdShow);
	
	BOOL ret;
	MSG msg;
	// цикл получения сообщений и посылки их на обработку
	while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
	  if(ret == -1) /* error found */
		return -1;

	  if(!IsDialogMessage(hDlg, &msg)) {
		TranslateMessage(&msg); /* translate virtual-key messages */
		DispatchMessage(&msg); /* send it to dialog procedure */
	  }
	}
}