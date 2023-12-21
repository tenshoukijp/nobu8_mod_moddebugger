#define _CRT_SECURE_NO_WARNINGS

/*++

OutputDebugStringの文字を取り出す。
32bit/64bitどちらでもコンパイル可能なようにしている。(ソリューションやプロジェクトの設定的にも)
・ターゲットのプロセスの｢クラス名｣がわかっているならば、ビット数の壁を越えられる。
・ターゲットのファイル名しか分かっていないならば、ビット数の壁は超えられない。
　この場合、ターゲットのビット数(32 or 64)と同じビット数でコンパイルしておく必要がある。
なお、OutputDebugString自体の取得は、64bitと32bit混在でも問題なく取ってこれる。

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

// メイン関数
void InitModDebugger(HWND hEdit) {

	//--------------- セキュリティ
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

	// OutputDebugStringで書き込んだよーというイベントを取得するためにイベント作成。
	//--------------- ACKイベント(シグナル状態で、次のOutputDebugString()用意)
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

	//--------------- READYイベント(シグナル状態でOutputDebugString()出力が完了)
	ReadyEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_DATA_READY");

	if (!ReadyEvent) {
		fprintf(stderr,
			"ModDebugger: Unable to create synchronization object, err == %dn",
			GetLastError());
		exit(1);
	}

	// OutputDebugString()で書き込まれるバッファを共有メモリで開く
	//--------------- "DBWIN_BUFFER"という名称の共有メモリ(4096bytes)
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

	//--------------- 先頭DWORDがプロセスID、以下が格納文字列
	String = (LPSTR)SharedMem + sizeof(DWORD);
	pThisPid = (LPDWORD)SharedMem;

	LastPid = 0xffffffff;
	DidCR = TRUE;

	// 準備完了まで待つ
	SetEvent(AckEvent);




}


// スレッド
unsigned __stdcall ThreadExternalProcModDebugger(void *lpx)
{
	for ( ;; ) {
		ret = WaitForSingleObject(ReadyEvent, INFINITE);

		if (ret != WAIT_OBJECT_0) {

			fprintf(stderr, "ModDebugger: wait failed; err == %dn", GetLastError());
			exit(1);

		} else {
			// 以下受け取った文字列をどうこうする処理

			// プロセスＩＤが変化した。
			if (LastPid != *pThisPid) {

				LastPid = *pThisPid;
				if (!DidCR) {
					// putchar('n');
					DidCR = TRUE;
				}
			}

			// 受信ボタンの受信が有効な状態ならば。
			if ( isActiveRecieveFlag ) {

				// ターゲットのProcessIDのものだけ表示する。
				// 毎度毎度チェックする必要がある。

				// ターゲットのプロセスＩＤ
				int iTargetProcessID = 0;
				HWND hTargetWnd = FindWindow( TARGET_CLASS_WND, NULL );
				GetWindowThreadProcessId(hTargetWnd, (LPDWORD)&iTargetProcessID);

				// すでに受信のバッファーが大きくなりすぎているならば、一度絞る
				if ( AllRecievedLog.size() > OVER_ALL_RECIEVEDLOG_SIZE ) { // 2000以上になったら
					// 最後の1000行分を取得
					vector<string> tmp;
					for ( UINT i=AllRecievedLog.size()-JUST_ALL_RECIEVEDLOG_SIZE; i < AllRecievedLog.size(); i++) {
						tmp.push_back(AllRecievedLog[i]);
					}
					AllRecievedLog = tmp; // 最後の1000行分でログを上書き

					// 表示領域を１から出力しなおし
					ReWriteAllRichEdit(hEdit);
				}


				string msg = string(String);
				// 最後の文字が改行ではない場合
				/*
				if ( String[strlen(String)-1] != '\n' ) {
					msg += "\n";
				}
				*/
				// ターゲットプロセスが立ち上がっていない時は、なんでも出す。(邪魔にならないから)
				if ( !iTargetProcessID ) {

					AllRecievedLog.push_back( msg );

					WriteRichEdit(hEdit, (char *)msg.c_str());

				// ターゲットプロセスが立ち上がっている時には、ターゲットプロセス以外のものは出力しないようにする(邪魔になるから)
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

