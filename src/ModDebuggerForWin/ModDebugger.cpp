#define _CRT_SECURE_NO_WARNINGS

/*++

OutputDebugString�̕��������o���B
32bit/64bit�ǂ���ł��R���p�C���\�Ȃ悤�ɂ��Ă���B(�\�����[�V������v���W�F�N�g�̐ݒ�I�ɂ�)
�E�^�[�Q�b�g�̃v���Z�X�̢�N���X������킩���Ă���Ȃ�΁A�r�b�g���̕ǂ��z������B
�E�^�[�Q�b�g�̃t�@�C���������������Ă��Ȃ��Ȃ�΁A�r�b�g���̕ǂ͒������Ȃ��B
�@���̏ꍇ�A�^�[�Q�b�g�̃r�b�g��(32 or 64)�Ɠ����r�b�g���ŃR���p�C�����Ă����K�v������B
�Ȃ��AOutputDebugString���̂̎擾�́A64bit��32bit���݂ł����Ȃ�����Ă����B

--*/

#include "WinTarget.h"
//---------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "ModDebugger.h"
#include "GetProcess.h"
#include "OnigRegex.h"
#include "TextBox.h"
#include "RegexBox.h"

using namespace std;

#pragma comment (lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")


BOOL isActiveRecieveFlag = TRUE;


HANDLE AckEvent;
HANDLE ReadyEvent;
HANDLE SharedFile;
LPVOID SharedMem;
LPSTR  String;
DWORD  ret;
DWORD  LastPid;
LPDWORD pThisPid;
BOOL    DidCR;

SECURITY_ATTRIBUTES sa;
SECURITY_DESCRIPTOR sd;


BOOL WriteRichEdit(HWND hWnd, char* pszBuf);

// ���C���֐�
void InitModDebugger(HWND hEdit) {

	//--------------- �Z�L�����e�B
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;

	if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
		fprintf(stderr,"unable to InitializeSecurityDescriptor, err == %dn",
			GetLastError());
		exit(1);
	}

	if(!SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE)) {
		fprintf(stderr,"unable to SetSecurityDescriptorDacl, err == %dn",
			GetLastError());
		exit(1);
	}

	// OutputDebugString�ŏ������񂾂�[�Ƃ����C�x���g���擾���邽�߂ɃC�x���g�쐬�B
	//--------------- ACK�C�x���g(�V�O�i����ԂŁA����OutputDebugString()�p��)
	AckEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_BUFFER_READY");

	if (!AckEvent) {
		fprintf(stderr,	"ModDebugger: Unable to create synchronization object, err == %dn",
		GetLastError());
		exit(1);
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// fprintf(stderr, "ModDebugger: already runningn");
		// exit(1);
	}

	//--------------- READY�C�x���g(�V�O�i����Ԃ�OutputDebugString()�o�͂�����)
	ReadyEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_DATA_READY");

	if (!ReadyEvent) {
		fprintf(stderr,
			"ModDebugger: Unable to create synchronization object, err == %dn",
			GetLastError());
		exit(1);
	}

	// OutputDebugString()�ŏ������܂��o�b�t�@�����L�������ŊJ��
	//--------------- "DBWIN_BUFFER"�Ƃ������̂̋��L������(4096bytes)
	SharedFile = CreateFileMapping(
		(HANDLE)-1,
		&sa,
		PAGE_READWRITE,
		0,
		4096,
		"DBWIN_BUFFER");

	if (!SharedFile) {
		fprintf(stderr,
			"ModDebugger: Unable to create file mapping object, err == %dn",
			GetLastError());
		exit(1);
	}

	SharedMem = MapViewOfFile(
		SharedFile,
		FILE_MAP_READ,
		0,
		0,
		512);

	if (!SharedMem) {
		fprintf(stderr,
			"ModDebugger: Unable to map shared memory, err == %dn",
			GetLastError());
		exit(1);
	}

	//--------------- �擪DWORD���v���Z�XID�A�ȉ����i�[������
	String = (LPSTR)SharedMem + sizeof(DWORD);
	pThisPid = (LPDWORD)SharedMem;

	LastPid = 0xffffffff;
	DidCR = TRUE;

	// ���������܂ő҂�
	SetEvent(AckEvent);




}


// �X���b�h
unsigned __stdcall ThreadExternalProcModDebugger(void *lpx)
{
	for ( ;; ) {
		ret = WaitForSingleObject(ReadyEvent, INFINITE);

		if (ret != WAIT_OBJECT_0) {

			fprintf(stderr, "ModDebugger: wait failed; err == %dn", GetLastError());
			exit(1);

		} else {
			// �ȉ��󂯎������������ǂ��������鏈��

			// �v���Z�X�h�c���ω������B
			if (LastPid != *pThisPid) {

				LastPid = *pThisPid;
				if (!DidCR) {
					// putchar('n');
					DidCR = TRUE;
				}
			}

			// ��M�{�^���̎�M���L���ȏ�ԂȂ�΁B
			if ( isActiveRecieveFlag ) {

				// �^�[�Q�b�g��ProcessID�̂��̂����\������B
				// ���x���x�`�F�b�N����K�v������B

				// �^�[�Q�b�g�̃v���Z�X�h�c
				int iTargetProcessID = 0;
				HWND hTargetWnd = FindWindow( TARGET_CLASS_WND, NULL );
				GetWindowThreadProcessId(hTargetWnd, (LPDWORD)&iTargetProcessID);

				// ���łɎ�M�̃o�b�t�@�[���傫���Ȃ肷���Ă���Ȃ�΁A��x�i��
				if ( AllRecievedLog.size() > OVER_ALL_RECIEVEDLOG_SIZE ) { // 2000�ȏ�ɂȂ�����
					// �Ō��1000�s�����擾
					vector<string> tmp;
					for ( UINT i=AllRecievedLog.size()-JUST_ALL_RECIEVEDLOG_SIZE; i < AllRecievedLog.size(); i++) {
						tmp.push_back(AllRecievedLog[i]);
					}
					AllRecievedLog = tmp; // �Ō��1000�s���Ń��O���㏑��

					// �\���̈���P����o�͂��Ȃ���
					ReWriteAllRichEdit(hEdit);
				}


				string msg = string(String);
				// �Ō�̕��������s�ł͂Ȃ��ꍇ
				/*
				if ( String[strlen(String)-1] != '\n' ) {
					msg += "\n";
				}
				*/
				// �^�[�Q�b�g�v���Z�X�������オ���Ă��Ȃ����́A�Ȃ�ł��o���B(�ז��ɂȂ�Ȃ�����)
				if ( !iTargetProcessID ) {

					AllRecievedLog.push_back( msg );

					WriteRichEdit(hEdit, (char *)msg.c_str());

				// �^�[�Q�b�g�v���Z�X�������オ���Ă��鎞�ɂ́A�^�[�Q�b�g�v���Z�X�ȊO�̂��̂͏o�͂��Ȃ��悤�ɂ���(�ז��ɂȂ邩��)
				} else if ( iTargetProcessID && iTargetProcessID == *pThisPid ) {
					
					AllRecievedLog.push_back( msg );

					WriteRichEdit(hEdit, (char *)msg.c_str());
				}
			}

			DidCR = (*String && (String[strlen(String) - 1] == 'n'));

			SetEvent(AckEvent);
		}

	}

	return 1;
}

