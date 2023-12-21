#pragma once

#include <windows.h>

// 以下ターゲットのクラス名か、プロセスネーム(の一部)のどちらかわかってればなんとかなる。
#define TARGET_CLASS_WND "KOEI_NOBU8WINDOW"

// 初期関数
void InitModDebugger(HWND hEdit);

// ループ
int ProcModDebugger(HWND hEdit);


// スレッド用関数の宣言
unsigned __stdcall ThreadExternalProcModDebugger(void *);

extern BOOL isActiveRecieveFlag;