#pragma once

#include <windows.h>

// �ȉ��^�[�Q�b�g�̃N���X�����A�v���Z�X�l�[��(�̈ꕔ)�̂ǂ��炩�킩���Ă�΂Ȃ�Ƃ��Ȃ�B
#define TARGET_CLASS_WND "KOEI_NOBU8WINDOW"

// �����֐�
void InitModDebugger(HWND hEdit);

// ���[�v
int ProcModDebugger(HWND hEdit);


// �X���b�h�p�֐��̐錾
unsigned __stdcall ThreadExternalProcModDebugger(void *);

extern BOOL isActiveRecieveFlag;