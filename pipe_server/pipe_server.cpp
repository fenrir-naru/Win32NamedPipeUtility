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
  hPipe = CreateNamedPipe(pipe_name, //lpName
      PIPE_ACCESS_DUPLEX,            // dwOpenMode
      PIPE_TYPE_BYTE | PIPE_WAIT,    // dwPipeMode
      1,                             // nMaxInstances
      0x1000,                        // nOutBufferSize
	  0x1000,                        // nInBufferSize
      100,                           // nDefaultTimeOut
      NULL);                         // lpSecurityAttributes

  if (hPipe == INVALID_HANDLE_VALUE) {
    _ftprintf(stderr, _T("Couldn't create %Ts.\n"), pipe_name);
    return 1;
  }
  if (!ConnectNamedPipe(hPipe, NULL)) {
    _ftprintf(stderr, _T("Couldn't connect to %Ts.\n"), pipe_name);
    CloseHandle(hPipe);
    return 1;
  }

  while (1) {
    BYTE buf[256];
    DWORD buf_count;
    if (!ReadFile(hPipe, buf, sizeof(buf) - 1, &buf_count, NULL)) {
      if (GetLastError() != ERROR_BROKEN_PIPE) {
        return -1;
      }
      break;
    }
    buf[buf_count] = '\0';
    printf("%s", buf);
    fflush(stdout);
  }
  _ftprintf(stderr, _T("%Ts will be closed."), pipe_name);
  FlushFileBuffers(hPipe);
  DisconnectNamedPipe(hPipe);
  CloseHandle(hPipe);

  return 0;
}
