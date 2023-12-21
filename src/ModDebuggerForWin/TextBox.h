#pragma once

#include <windows.h>
#include <richedit.h>
#include <vector>
#include <string>

using namespace std;

const int TextBoxPosX = 5;
const int TextBoxPosY = 30;
const int TextBoxLimitSize = 1024 * 1024 * 10; // 10M


extern HWND hEdit;

HWND CreateTextBox(HWND hWnd, HINSTANCE hInst);
void DeleteTextBox();

void MoveTextBox(HWND hWnd);

BOOL SetInitialFont(HWND hWnd);
BOOL SetForegroundFont(HWND);
BOOL ChangeBackColor( HWND hWnd, COLORREF clr );
BOOL WriteRichEdit(HWND hWnd, char* pszBuf);

// テキストボックスをAllRecievedLogに従って書き換え
BOOL ReWriteAllRichEdit(HWND hWnd);

// テキストボックスをクリア
BOOL ClearRichEdit(HWND hWnd);

#define OVER_ALL_RECIEVEDLOG_SIZE	2000
#define JUST_ALL_RECIEVEDLOG_SIZE	1000
extern vector<string> AllRecievedLog; // フィルター無しで受信した全てのログ(SJIS済)
