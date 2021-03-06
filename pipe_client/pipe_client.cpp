// pipe_server.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include <stdio.h>
#include <windows.h>
#include <tchar.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s pipe_name\n", argv[0]);
    exit(-1);
  }

  TCHAR pipe_name[256];
  _stprintf_s(pipe_name, _T("\\\\.\\pipe\\%hs"), argv[1]);

  _ftprintf(stderr, _T("Target NamedPipe: %Ts\n"), pipe_name);

  HANDLE hPipe = INVALID_HANDLE_VALUE;
  hPipe = CreateFile(pipe_name,
      GENERIC_WRITE,
      0,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL);

  if (hPipe == INVALID_HANDLE_VALUE) {
    _ftprintf(stderr, _T("Couldn't access %Ts.\n"), pipe_name);
    return 1;
  }

  while (1) {
    char szBuff[256];
    DWORD dwBytesWritten;
    fgets(szBuff, sizeof(szBuff), stdin);
    if (!WriteFile(hPipe, szBuff, strlen(szBuff), &dwBytesWritten, NULL)) {
      _ftprintf(stderr, _T("Couldn't write %Ts.\n"), pipe_name);
      break;
    }
  }
  CloseHandle(hPipe);

  return 0;
}
