// pipe_server.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

int main(int argc, char *argv[]) {
  BOOL nowait = FALSE;
  TCHAR pipe_name[256];
  for(int i(1); 1; i++){
    if (i > (argc - 1)) {
      fprintf(stderr, "Usage: %s pipe_name\n", argv[0]);
      exit(-1);
    }
    if(strcmp(argv[i], "--nowait") == 0){
      nowait = TRUE;
    }else{
      _stprintf_s(pipe_name, _T("\\\\.\\pipe\\%hs"), argv[i]);
      break;
    }
  }

  _ftprintf(stderr, _T("Target NamedPipe: %Ts%Ts\n"), pipe_name,
      (nowait ? _T(" (nowait)") : _T("")));

  HANDLE hPipe = INVALID_HANDLE_VALUE;
  hPipe = CreateNamedPipe(pipe_name, //lpName
      PIPE_ACCESS_DUPLEX,            // dwOpenMode
      PIPE_TYPE_BYTE                 // dwPipeMode
        | (nowait ? PIPE_NOWAIT : PIPE_WAIT),
      1,                             // nMaxInstances
      0x1000,                        // nOutBufferSize
	    0x1000,                        // nInBufferSize
      100,                           // nDefaultTimeOut
      NULL);                         // lpSecurityAttributes

  if (hPipe == INVALID_HANDLE_VALUE) {
    _ftprintf(stderr, _T("Couldn't create %Ts.\n"), pipe_name);
    return 1;
  }
  if (nowait
      ? ((ConnectNamedPipe(hPipe, NULL) != 0) || (GetLastError() != ERROR_IO_PENDING))
      : (!ConnectNamedPipe(hPipe, NULL))) {
    _ftprintf(stderr, _T("Couldn't connect to %Ts.\n"), pipe_name);
    CloseHandle(hPipe);
    return 1;
  }

  while (1) {
    DWORD buf_count;
#if 1
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        break;
      }
    }
#endif
    if (!PeekNamedPipe(hPipe, NULL, 0, NULL, &buf_count, NULL)) {
      DWORD dwError = GetLastError();

      if ((dwError == ERROR_BROKEN_PIPE) ||
          (dwError == ERROR_PIPE_NOT_CONNECTED) ||
          (dwError == ERROR_INVALID_HANDLE)){
        break;
      }

      buf_count = 0;
    }
    if (buf_count <= 0) {
      Sleep(10);
      continue;
    }
    while (buf_count > 0) {
      BYTE buf[256];
      DWORD buf_size(sizeof(buf) - 1), buf_filled;
      if (buf_count < buf_size) {
        buf_size = buf_count;
      }
      if (!ReadFile(hPipe, buf, buf_size, &buf_filled, NULL)) {
        return -1;
      }
      buf_count -= buf_filled;
      buf[buf_filled] = '\0';
      printf("%s", buf);
    }
    fflush(stdout);
  }
  _ftprintf(stderr, _T("%Ts will be closed."), pipe_name);
  FlushFileBuffers(hPipe);
  DisconnectNamedPipe(hPipe);
  CloseHandle(hPipe);

  return 0;
}
