#pragma once

#include <windows.h>

#define CLEARBUTTON_ID 50005

const int ClearButtonPosX = 115;

extern HWND hClearButton;

HFONT SetButtonFont(LPCTSTR face, int h, int angle);

HWND CreateClearButton(HWND hWnd, HINSTANCE hInst);

void DeleteClearButton();

void CreateTextBox();