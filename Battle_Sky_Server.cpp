#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

int buffer[BUFSIZE];
SOCKET player1_sock;
SOCKET player2_sock;
SOCKADDR_IN player1_addr;
SOCKADDR_IN player2_addr;
RECT player1_pos;
RECT player2_pos;
RECT Obstacles[];

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
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[])
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
    
    int addrlen;
    char buf[BUFSIZE + 1];

    while (1) {
        // player1 accept()
        addrlen = sizeof(player1_addr);
        player1_sock = accept(listen_sock, (SOCKADDR*)&player1_addr, &addrlen);
        if (player1_sock == INVALID_SOCKET) {
            err_display((char*)"accept()");
            break;
        }

        // player2 accept()
        addrlen = sizeof(player2_addr);
        player2_sock = accept(listen_sock, (SOCKADDR*)&player2_addr, &addrlen);
        if (player2_sock == INVALID_SOCKET) {
            err_display((char*)"accept()");
            break;
        }

        // 플레이어 정보 초기화


        // 클라이언트와 데이터 통신
        while (1) {

        }

        // closesocket()
        closesocket(player1_sock);
        closesocket(player2_sock);
    }

    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}