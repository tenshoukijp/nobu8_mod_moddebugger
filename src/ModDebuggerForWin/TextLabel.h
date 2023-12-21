#include <windows.h>


// 正規表現受信フィルター
void CreateRegexBoxTextLabel(HWND hWnd);
void DeleteRegexBoxTextLabel();
HFONT SetLabelFont(HDC hdc, LPCTSTR face, int h, int angle);