#pragma once

#include <windows.h>

#define RECIEVEBUTTON_ID 50001

extern HWND hRecieveButton;

HFONT SetButtonFont(LPCTSTR face, int h, int angle);

HWND CreateRecieveButton(HWND hWnd, HINSTANCE hInst);

void DeleteRecieveButton();

void SwitchRecieveStatus();