#include "RecieveButton.h"
#include "TextBox.h"
#include "ModDebugger.h"

HWND hRecieveButton = NULL;

static HFONT hFont = NULL;

HWND CreateRecieveButton(HWND hWnd, HINSTANCE hInst ) {

	 hRecieveButton = CreateWindow(
		"BUTTON",								 // �E�B���h�E�N���X��
		"��M��~",								 // �L���v�V����
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,   // �X�^�C���w��
		TextBoxPosX, 5,                                  // ���W
		100, 20,                                  // �T�C�Y
		hWnd,                                    // �e�E�B���h�E�̃n���h��
		(HMENU)RECIEVEBUTTON_ID,                 // ���j���[�n���h��
		hInst,                                   // �C���X�^���X�n���h��
		NULL                                     // ���̑��̍쐬�f�[�^
	);

	hFont = SetButtonFont("�l�r �S�V�b�N", 13, 0);
	SendMessage(hRecieveButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	return hRecieveButton;
}

void DeleteRecieveButton() {
	DeleteObject(hFont);
	DestroyWindow(hRecieveButton);
}


HFONT SetButtonFont(LPCTSTR face, int h, int angle) {
    HFONT hFont;
    hFont = CreateFont(h,    //�t�H���g����
        0,                    //������
        angle,                    //�e�L�X�g�̊p�x
        0,                    //�x�[�X���C���Ƃ����Ƃ̊p�x
        FW_REGULAR,            //�t�H���g�̏d���i�����j
        FALSE,                //�C�^���b�N��
        FALSE,                //�A���_�[���C��
        FALSE,                //�ł�������
        SHIFTJIS_CHARSET,    //�����Z�b�g
        OUT_DEFAULT_PRECIS,    //�o�͐��x
        CLIP_DEFAULT_PRECIS,//�N���b�s���O���x
        PROOF_QUALITY,        //�o�͕i��
        FIXED_PITCH | FF_MODERN,//�s�b�`�ƃt�@�~���[
        face);    //���̖�
    return hFont;
}


void SwitchRecieveStatus() {
	if ( isActiveRecieveFlag ) {
		isActiveRecieveFlag = FALSE;
		SetWindowText( hRecieveButton, "��M�ĊJ");
} else {
		isActiveRecieveFlag = TRUE;
		SetWindowText( hRecieveButton, "��M��~");
	}
}