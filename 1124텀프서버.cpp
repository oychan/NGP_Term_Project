
#define _CRT_SECURE_NO_WARNINGS         // 최신 VC++ 컴파일 시 경고 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define TIMERSEC    100

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char* fmt, ...);
// 오류 출력 함수
void err_quit(char* msg);
void err_display(char* msg);
// 소켓 통신 스레드 함수
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);

HINSTANCE hInst; // 인스턴스 핸들
HWND hEdit; // 편집 컨트롤
CRITICAL_SECTION cs; // 임계 영역


//클라이언트의 자료형들
struct Pos {
    int x;
    int y;


};

struct Player {
    RECT position;
    int direction;
    int hp;
    int speed;
    int power;
    int powertype;
    int bulletcreatetime;
    Pos size;
    Player()
    {
        direction = 0;           //0오른쪽 1왼쪽
        hp = 100;
        speed = 10;
        power = 10;
        powertype = 1;
        bulletcreatetime = TIMERSEC;

    }

};



int clientuser;

int P1_ready = 0;
int P2_ready = 0;
Player serverME;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;
    InitializeCriticalSection(&cs);

    // 윈도우 클래스 등록
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "MyWndClass";
    if (!RegisterClass(&wndclass)) return 1;

    // 윈도우 생성
    HWND hWnd = CreateWindow("MyWndClass", "TCP 서버", WS_OVERLAPPEDWINDOW,
        0, 0, 600, 200, NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) return 1;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 소켓 통신 스레드 생성
    CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

    // 메시지 루프
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DeleteCriticalSection(&cs);
    return msg.wParam;
}

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        hEdit = CreateWindow("edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_HSCROLL |
            WS_VSCROLL | ES_AUTOHSCROLL |
            ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
            0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL);
        return 0;
    case WM_SIZE:
        MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    case WM_SETFOCUS:
        SetFocus(hEdit);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 편집 컨트롤 출력 함수
void DisplayText(char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[BUFSIZE + 256];
    vsprintf(cbuf, fmt, arg);

    EnterCriticalSection(&cs);
    int nLength = GetWindowTextLength(hEdit);
    SendMessage(hEdit, EM_SETSEL, nLength, nLength);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
    LeaveCriticalSection(&cs);

    va_end(arg);
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    DisplayText((char*)"[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// TCP 서버 시작 부분
DWORD WINAPI ServerMain(LPVOID arg)              //servermain 여기서 멀티쓰레드를또생성해줌
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit((char*)"socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit((char*)"bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit((char*)"listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display((char*)"accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        DisplayText((char*)"\r\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\r\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}

// 클라이언트와 데이터 통신        멀티쓰레드
DWORD WINAPI ProcessClient(LPVOID arg)
{
                         
    //w누를때마다 
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];
    int strlen;
    Player * svp; //원하는 ME임
   
    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    int count = 0;
    int thisclientnum = 0;
    
    //구조체 데이터 받는구조

    /*
    while (1) {
        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);

        // 받은 것을 형 변환으로 원하는 값으로 바꾼다. 
        buf[retval] = '\0';
    
        svp = (Player*)buf;
      
        DisplayText((char*)"%d  \n",svp->speed);
        DisplayText((char*)" count =%d ", count);
        count++;

        //그럼 계속 받기대기 상태에서 쭉사는건가?
    }

    */
    
    //문자열데이터 받는구조
    while (1) {
        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);    //길이가 반환됨.
        if (retval == SOCKET_ERROR) {
            err_display((char*)"recv()");
            break;
        }
        else if (retval == 0)
            break;

        

        buf[retval] = '\0';
        //받은 buf형 문자열출력
        DisplayText((char*)"[TCP/%s:%d] %s\r\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);
        //int gili = strlen("하이요");
       
      //  DisplayText((char*)"%s가 보내졌다",buf);
        if ( buf[retval-1]=='C')
        {
            
      
            DisplayText((char*)"클라이언트 연결완료");
            clientuser += 1;
            
            thisclientnum = clientuser;

            DisplayText((char*)"매칭된 유저수 %d",clientuser);


           
            //연결되면 데이터보냄
        }


        if (buf[retval - 1] == 'R')
        {


            DisplayText((char*)"클라이언트 레디완료");
            if (thisclientnum == 1)
            {
                P1_ready = 1;

               

                //1피레디한경우 다했다 치고 일단 넘어가게끔

            }
            if (thisclientnum == 2)
            {
                P2_ready = 1;

            }

            DisplayText((char*)"레디한 유저번호 %d", thisclientnum);


            if (P1_ready == 1 && P2_ready == 1)
            {
               // DisplayText((char*)"모두가 레디하여 게임실행 %d", thisclientnum);
                char* buf = (char*)"G";           //GO 시작하라고 값보냄 어디서받아야할까
                retval = send(client_sock, buf, retval, 0);    //매새지보낸다는의미
                if (retval == SOCKET_ERROR) {
                    err_display((char*)"send()");
                    break;
                }

            }


        }

        if(buf[retval - 1] == 'S')
        {


            DisplayText((char*)"대화보냄");
        }

        /*
        retval = send(client_sock, buf, retval, 0);    //매새지보낸다는의미
        if (retval == SOCKET_ERROR) {
            err_display((char*)"send()");
            break;
        }
        */
       
    }

    
    // closesocket()
    closesocket(client_sock);
    DisplayText((char*)"[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\r\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}