#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#define SERVERIP   "127.0.0.1"          //실습할시 서버의 ip입력해주면됨
#define SERVERPORT 9000
#define BUFSIZE    512

#include <Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define MistakeWidth   9
#define MistakeHeight 32
#define WindowFullWidth 1000
#define WindowFullHeight 800

#define TIMERSEC 80
//윈도우 이름



//전역변수로 쓸것


//서버통신관련 변수들
HANDLE hThread_1, hThread_2;
SOCKET sock;
int retval;
char buf[BUFSIZE + 1];
//char* buf;          버프쪽왜?


int J_send_key;
char* J_send_char;



int gamestate = 0;

int sendtime;
int receivetime;



TCHAR* str;


int pnum;
HANDLE Event;








//게임내 전역변수끝


struct Pos {
    int x;
    int y;


};
struct Camera {
    Pos pos;



};

struct Item {
    bool exist;
    int itemtype;
    int itempower;
    RECT position;
    Pos size;



};
struct Obstacle
{
    int obstacletype;
    RECT position;




};
struct Bullet {
    RECT position;
    int direction;
    int power;
    int speed;
    int survivaltime;
    bool exist;
    Bullet()
    {
        direction = 0;
        exist = true;
        speed = 30;
        power = 10;
        survivaltime = 4000;
    }

    ~Bullet()
    {

    }


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






//게임내 전역변수들

HDC hdc, memdc;
PAINTSTRUCT ps;





Bullet* Bullets[100];    //총알들의 모음
Obstacle* Obstacles[100]; //장애물들의 모음
Item* Items[100]; //아이템 들의 모음
Player* Objects[100];    //오브젝트들의 모음




int objectnum;
int bulletnum;
int obstaclenum;
int itemnum;

Player* ENEMY = new Player;
Player* ME = new Player;

Camera maincamera;
Pos MapMaxSize;
Pos MapMinSize;

int Itemcreatetime = 8000;
int nowitemgauge = 0;
bool obstaclecreate = true;

//HBITMAP start_bitmap = (HBITMAP)LoadImage(NULL, "..\start.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
HBITMAP startimg, OldBitmap;
BITMAP BIT;

RECT soloplayimg_pos;
int mouseposx;
int mouseposy;

RECT teamplayimg_pos;
RECT readyimg_pos;


int keyinputtime;





//서버통신시작
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}



DWORD WINAPI KeySend(LPVOID arg)
{
    int retval;

    SOCKET server_sock = (SOCKET)arg;

    //값 보내준다.
    J_send_char = (char*)"zero";
    while (1) {

        // if 

        if (sendtime >= TIMERSEC)
        {
            //8초마다 u보내주고 업데이트자료보내줌
            // n초마다 보내주는건 send  recv는 u그다음에받는식 ㅇㅋ
            J_send_char = (char*)"U";  //숫자보내는건 어케하지?  // char형으으로 1[ 이렇게해서 끝자리 [인경우     
            retval = send(sock, J_send_char, strlen(J_send_char), 0);
            J_send_char = (char*)"zero";

            //위치정보들바꾼다고 보내준후에 한다
            retval = send(server_sock, (char*)ME, sizeof(*ME), 0);   //me니까 몇짜리크기

            sendtime = 0;
        }

        if (J_send_key == 1) //센드키가 뭔가있는경우 센드키보내줌
        {

            if (J_send_key == 1)
            {
                J_send_char = (char*)"C";  //숫자보내는건 어케하지?  // char형으으로 1[ 이렇게해서 끝자리 [인경우                 
                str = TEXT(J_send_char);
                retval = send(sock, J_send_char, strlen(J_send_char), 0);
                J_send_char = (char*)"zero";
                J_send_key = 0;
            }

            if (J_send_key != 0)
            {

                retval = send(server_sock, (char*)&J_send_key, sizeof(int), 0);
            }

        }

        if (J_send_char != "zero")   //보낼값이 변경될경우 send보내줌
        {
            //r보내지나확인

            str = TEXT(J_send_char);

            retval = send(server_sock, J_send_char, strlen(J_send_char), 0); // char값보내줌.

            J_send_char = (char*)"zero";
        }

    }

    closesocket(server_sock);

    return 0;
}




//recv쓰레드
DWORD WINAPI PositionRecv(LPVOID arg)     //값받는다.
{
    int retval;

    char buf[BUFSIZE + 1]; //이걸안하면 꼬임 왜그럴까..;;

    //값을받아보자

    SOCKET client_sock = (SOCKET)arg;


    while (1) {

        // retval = recv(client_sock, buf, BUFSIZE, 0);    //길이가 반환됨.
        retval = recv(client_sock, buf, 1, 0);                         //하나씩받아서그런가? 뭉텅이로받아볼까?
       // retval = recv(client_sock, buf, 1, 0); //하나씩받아준다.
        if (retval == SOCKET_ERROR) {
            break;
        }
        else if (retval == 0)
            break;

        //  buf[retval] = '\0';        //뭔가나온값의 마지막꼬리를 '\0'해준다.
                   //recv buf 판별이이상해서꼬이는건가?

            //
        if (retval != 0) //뭔가값을받은경우.
        {


            int len = strlen(buf);

            if (buf[0] == 'R')
            {
                // exit(1);
                gamestate = 2;
                Player* ptemp = new Player;
                retval = recv(client_sock, (char*)ptemp, sizeof(*ptemp), 0);
                //값만넘겨줌

                ME->hp = ptemp->hp;      //데이터만고쳐주자
                ME->position = ptemp->position;


                delete ptemp;


            }


            if (buf[0] == 'O')
            {

                //장애물 생성키인경우 장애물생성


                Obstacle* obstacle = new Obstacle;
                Obstacle* otemp = new Obstacle;
                retval = recv(client_sock, (char*)otemp, sizeof(*otemp), 0);
                //값만넘겨줌

                obstacle->obstacletype = otemp->obstacletype;
                obstacle->position = otemp->position;

                Obstacles[obstaclenum] = obstacle;

                obstaclenum++;


                delete otemp;
                //  ME->hp = ptemp->hp;      //데이터만고쳐주자
                //  ME->position = ptemp->position;


                //  delete ptemp;


            }


            if (buf[0] == 'I')
            {

                //아이템 생성키인경우 아이템생성


                Item* item = new Item;
                Item* itemp = new Item;
                retval = recv(client_sock, (char*)itemp, sizeof(*itemp), 0);
                //값만넘겨줌


                item->itemtype = itemp->itemtype; //0파워업 1스피드업
                item->itempower = itemp->itempower;
                item->size = itemp->size;
                item->position = itemp->position;


                Items[itemnum] = item;

                itemnum++;


                delete itemp;
                //  ME->hp = ptemp->hp;      //데이터만고쳐주자
                //  ME->position = ptemp->position;


                //  delete ptemp;


            }


            if (buf[0] == 'B')
            {

                //불렛받은경우 불렛생성


                Bullet* bullet = new Bullet;
                Bullet* btemp = new Bullet;
                retval = recv(client_sock, (char*)btemp, sizeof(*btemp), 0);
                //값만넘겨줌


                bullet->direction = btemp->direction; //0파워업 1스피드업
                bullet->power = btemp->power;
                bullet->position = btemp->position;
                //  item->position = ibemp->position;


                Bullets[bulletnum] = bullet;

                bulletnum++;


                delete btemp;


            }



            //        //buf[0]=='P' 인경우인데 왜
            if (buf[0] == 'U') //12345678 에서 1인경우 
            {
                // wsprintf(str, TEXT((char*)"현재buf는 %d입니다."), 100 );
               //  str = TEXT((char*)"hiruhiru %d");
                pnum = buf[0];                //도대체 buf값에 R이 왜들어가냐고 이해가안가네

                Player* ptemp = new Player;
                retval = recv(client_sock, (char*)ptemp, sizeof(*ptemp), 0);
                //값만넘겨줌


                ME->position = ptemp->position;
                ME->direction = ptemp->direction;

                retval = recv(client_sock, (char*)ptemp, sizeof(*ptemp), 0);

                ENEMY->position = ptemp->position;

                delete ptemp;

                Camera* ctemp = new Camera;
                retval = recv(client_sock, (char*)ctemp, sizeof(*ctemp), 0);
                maincamera.pos = ctemp->pos;

                delete ctemp;

            }

            SetEvent(Event);    //  여기아니면 그밑에

        }


    }

    closesocket(client_sock);

    return 0;
}






//

// 0타이틀화면 1게임시작누른후 게임로딩창  2게임실제플레이


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//Player 구조체
// 위치4군대, 공격타입,아이템관련,

//내꺼버전 winmain


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    static char szAppName[] = "Midterm";
    static char szTitle[] = "2020-2 networkgame";
    MSG  msg;
    HWND  hWnd;                 //여러개창만들거면 이부분수정
    WNDCLASS wc;              //여러개창만들거면이부분수정






    //이게없으면 표시안함       하고싶은것 다른콜백함수를하고싶은거였는데;





  //  wc.cbSize = sizeof(WNDCLASSEX);      //
    wc.style = CS_HREDRAW | CS_VREDRAW;    //
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;       //
    wc.cbWndExtra = 0;    //
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);      //
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szAppName;
    //   wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);        //


    RegisterClass(&wc);  //이게없으면 표시안함


    hWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, 0, WindowFullWidth + MistakeWidth, WindowFullHeight + MistakeHeight, NULL, NULL, hInstance, NULL);
    //윈도우만든다.
    //+9+32가
      //맵크기
    if (!hWnd)
        return (false);


    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}






//SPACE누를시 불렛생성
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    //이미지들 좌표모음들
    //게임모드 w누를시 게임모드0
    switch (uMsg)
    {

    case WM_CREATE:




        //포인트형 ME
        ME->position.top = 100;
        ME->position.left = 100;
        ME->position.right = 200;
        ME->position.bottom = 200;
        ME->size.x = 100;
        ME->size.y = 100;
        //
        ENEMY->position.top = 500;
        ENEMY->position.left = 800;
        ENEMY->position.right = 900;
        ENEMY->position.bottom = 600;


        //maincamera.pos.x = Me.position.left-200;
       // maincamera.pos.y = Me.position.top-400;

        //내위치가 200
        maincamera.pos.x = 0;
        maincamera.pos.y = 0;

        MapMinSize.x = 10;
        MapMinSize.y = 0;


        //아이템이랑 오브젝트 바로생성부터 ㄱ
        //
        MapMaxSize.x = 1300;
        MapMaxSize.y = 1300;
        SetTimer(hWnd, 1, TIMERSEC, NULL);




        break;
    case WM_CHAR:



        if (gamestate == 2) {      //게임상태2일때만 해당이됨.




            if (wParam == 'a' || wParam == 'A')
            {
                //총알생성

                if (ME->bulletcreatetime >= 800)   //불렛난사가 1초에 3회가 최대로 설정
                {
                    //총알생성정의는
                    ME->bulletcreatetime = 0;

                    J_send_char = (char*)"B";
                    //총알생성

                    /*
                    Bullet* bullet = new Bullet;
                    bullet->direction = ME->direction;
                    bullet->power = ME->power;
                    if (ME->direction == 0)
                    {

                        bullet->position.top = ME->position.top + 30;
                        bullet->position.left = ME->position.left + 100;
                        bullet->position.right = ME->position.left + 150;
                        bullet->position.bottom = ME->position.bottom - 30;
                    }

                    if (ME->direction == 1)
                    {
                        bullet->position.top = ME->position.top + 30;
                        bullet->position.left = ME->position.right - 150;
                        bullet->position.right = ME->position.right - 100;
                        bullet->position.bottom = ME->position.bottom - 30;



                    }
                    Bullets[bulletnum] = bullet;
                    bulletnum++;
                    */

                }

            }

        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_UP)
        {
            J_send_char = (char*)"w";

            //임시로 키눌를떄 아무반응없다 ㅇㅋ

            /*
            if (ME->position.top > MapMinSize.y)     //캐릭최소위치
            {
                ME->position.top -= ME->speed;
                ME->position.bottom -= ME->speed;

                if (ME->position.top < MapMinSize.y)
                {
                    ME->position.top = MapMinSize.y;
                    ME->position.bottom = ME->position.top + ME->size.y;
                }



            }
                 */


                 //시점관련보정
                 /*
                 if (maincamera.pos.y > MapMinSize.y)     //카메라 최소위치
                 {

                     if (ME->position.top < (MapMaxSize.y - (WindowFullHeight / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                     {
                         maincamera.pos.y -= ME->speed;
                     }

                     if (maincamera.pos.y < MapMinSize.y)
                     {
                         maincamera.pos.y = MapMinSize.y;       //-되는경우 0으로보정
                     }
                 }

                   */

                   //  InvalidateRect(hWnd, NULL, TRUE);
        }
        if (wParam == VK_DOWN)
        {

            J_send_char = (char*)"s";

            //   서버에서처리하면서 사라짐
/*
if ((ME->position.bottom) < MapMaxSize.y)     //캐릭최소위치
{
    ME->position.top += ME->speed;
    ME->position.bottom += ME->speed;

    if ((ME->position.bottom) > MapMaxSize.y)
    {
        ME->position.bottom = MapMaxSize.y;
        ME->position.top = ME->position.bottom - ME->size.y;
    }



}

        */


        /*
        if (maincamera.pos.y < (MapMaxSize.y - (WindowFullHeight)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
        {

            if (ME->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
            {
                maincamera.pos.y += ME->speed;
            }
            if (maincamera.pos.y > (MapMaxSize.y - (WindowFullHeight + 9)))
            {
                maincamera.pos.y = (MapMaxSize.y - (WindowFullHeight - 9));       //-되는경우 0으로보정
            }
        }
          */


          //  InvalidateRect(hWnd, NULL, TRUE);
        }
        if (wParam == VK_LEFT)
        {
            J_send_char = (char*)"M";
            /*
            ME->direction = 1;
            //top이아니라 left
            if (ME->position.left > MapMinSize.x)     //캐릭최소위치
            {
                ME->position.left -= ME->speed;
                ME->position.right -= ME->speed;

                if (ME->position.left < MapMinSize.x)
                {
                    ME->position.left = MapMinSize.x;
                    ME->position.right = ME->position.left + ME->size.x;
                }



            }
                  */

                  /*
                  if (maincamera.pos.x > MapMinSize.x)     //카메라 최소위치
                  {

                      if (ME->position.right < (MapMaxSize.x - (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                      {
                          maincamera.pos.x -= ME->speed;
                      }

                      if (maincamera.pos.x < MapMinSize.x)
                      {
                          maincamera.pos.x = MapMinSize.x;       //-되는경우 0으로보정
                      }
                  }
                  */

                  //InvalidateRect(hWnd, NULL, TRUE);

        }
        if (wParam == VK_RIGHT)
        {
            J_send_char = (char*)"N";
            /*
            ME->direction = 0;
            if ((ME->position.right) < MapMaxSize.x)     //일반적상황 오른키시 오른쪽으로이동
            {
                ME->position.left += ME->speed;
                ME->position.right += ME->speed;

                if ((ME->position.right) > MapMaxSize.x)  //맵을 벗어나려고하면 보정
                {
                    ME->position.right = MapMaxSize.x;
                    ME->position.left = ME->position.right - ME->size.x;
                }



            }
                */

                /*
                if (maincamera.pos.x < (MapMaxSize.x - (WindowFullWidth)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
                {

                    if (ME->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                    {
                        maincamera.pos.x += ME->speed;
                    }
                    if (maincamera.pos.x > (MapMaxSize.x - (WindowFullWidth + 9)))
                    {
                        maincamera.pos.x = (MapMaxSize.x - (WindowFullWidth - 9));       //-되는경우 0으로보정
                    }
                }
                  */
                  // InvalidateRect(hWnd, NULL, TRUE);

        }


        break;

    case WM_LBUTTONDOWN:

        mouseposx = LOWORD(lParam);
        mouseposy = HIWORD(lParam);



        if (gamestate == 0)        //타이틀화면 혼자하기,같이하기가있다.
        {


            if (mouseposx > teamplayimg_pos.left && mouseposx < teamplayimg_pos.right)
            {

                if (mouseposy > teamplayimg_pos.top && mouseposy < teamplayimg_pos.bottom)
                {

                    //서버통신시작
                    //팀플레이 하기 누른경우라 서버랑 통신시작.
                    gamestate = 1;            //서버대기창도 좀만들자

                    WSADATA wsa;
                    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
                        return 1;

                    // socket()
                    sock = socket(AF_INET, SOCK_STREAM, 0);
                    if (sock == INVALID_SOCKET) { exit(1); }

                    // connect()
                    SOCKADDR_IN serveraddr;
                    ZeroMemory(&serveraddr, sizeof(serveraddr));
                    serveraddr.sin_family = AF_INET;
                    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
                    serveraddr.sin_port = htons(SERVERPORT);
                    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
                    if (retval == SOCKET_ERROR) { exit(1); }

                    Event = CreateEvent(0, FALSE, FALSE, 0);

                    hThread_1 = CreateThread(NULL, 0, KeySend, (LPVOID)sock, 0, NULL);                    //sned쓰레드.

                    hThread_2 = CreateThread(NULL, 0, PositionRecv, (LPVOID)sock, 0, NULL);   //recv쓰레드



                    J_send_key = 1; //키값변경

                    //구조체보낼경우는
                     /*
                     Player me;
                     retval = send(sock, (char*)&me, sizeof(me), 0);

                     printf((char*)"[TCP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

                       //포인터기준
                    //retval = send(sock, (char*)ME, sizeof(*ME), 0);        //이제 데이터값보내기성공
                    */

                }
            }





            if (mouseposx > soloplayimg_pos.left && mouseposx < soloplayimg_pos.right)
            {

                if (mouseposy > soloplayimg_pos.top && mouseposy < soloplayimg_pos.bottom)
                {
                    gamestate = 2;            //서버대기창도 좀만들자

                }
            }


        }

        if (gamestate == 1)        //같이하기누른 채팅대기창.
        {


            if (mouseposx > readyimg_pos.left && mouseposx < readyimg_pos.right)
            {

                if (mouseposy > readyimg_pos.top && mouseposy < readyimg_pos.bottom)
                {



                    J_send_char = (char*)"R";

                    // J_send_key = 1; //키값변경

                }
            }
        }




        break;

    case WM_PAINT:
        //그려주는데  또머있냐..? 그려주는부분에서 차이있긴함 maincamera.x 이 2000이고 ENEMY가 2400이면  ENEMY - MAINCAMERA 이런식으로 처리한다.
        hdc = BeginPaint(hWnd, &ps);
        memdc = CreateCompatibleDC(hdc);        //더블버퍼링

       // WaitForSingleObject(Event,20);

      //  TCHAR str2[128];
      //  wsprintf(str2, TEXT((char*)"현재item갯수는 %d입니다. \n"), itemnum);
     //   TextOut(hdc, 100, 100, str2, lstrlen(str2));

       // TextOut(hdc, 400, 100, str, lstrlen(str));

        if (gamestate == 0) {               //게임타이틀화면인경우 출력


            startimg = (HBITMAP)LoadImage(NULL, TEXT("startsolo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            OldBitmap = (HBITMAP)SelectObject(memdc, startimg);



            // 비트맵 정보를 알아낸다 getobject

            GetObject(startimg, sizeof(BITMAP), &BIT);
            int xsize = BIT.bmWidth;
            int ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 300, xsize, ysize, memdc, 0, 0, SRCCOPY);

            soloplayimg_pos.top = 300;
            soloplayimg_pos.left = 600;
            soloplayimg_pos.right = 600 + xsize;
            soloplayimg_pos.bottom = 300 + ysize;



            startimg = (HBITMAP)LoadImage(NULL, TEXT("startteam.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            OldBitmap = (HBITMAP)SelectObject(memdc, startimg);



            // 비트맵 정보를 알아낸다 getobject

            GetObject(startimg, sizeof(BITMAP), &BIT);
            xsize = BIT.bmWidth;
            ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 500, xsize, ysize, memdc, 0, 0, SRCCOPY);

            teamplayimg_pos.top = 500;
            teamplayimg_pos.left = 600;
            teamplayimg_pos.right = 600 + xsize;
            teamplayimg_pos.bottom = 500 + ysize;



            SelectObject(memdc, OldBitmap); // hImage 선택을 해제하기 위해 hOldBitmap을 선택한다
            DeleteObject(startimg); // 선택 해제된 비트맵을 제거한다


            /*
            HBITMAP startimg2 = (HBITMAP)LoadImage(NULL, TEXT("startsolo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);


            GetObject(startimg2, sizeof(BITMAP), &BIT);

            xsize = BIT.bmWidth;
            ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 600, xsize, ysize, memdc, 0, 0, SRCCOPY);

            // if 내가 그 사이값안에 마우스클릭한경우


            SelectObject(memdc, OldBitmap); // hImage 선택을 해제하기 위해 hOldBitmap을 선택한다
            DeleteObject(startimg2); // 선택 해제된 비트맵을 제거한다

              */
        }

        if (gamestate == 1) {               //게임타이틀화면인경우 출력


            HBITMAP readyimg = (HBITMAP)LoadImage(NULL, TEXT("ready.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            OldBitmap = (HBITMAP)SelectObject(memdc, readyimg);



            // 비트맵 정보를 알아낸다 getobject

            GetObject(readyimg, sizeof(BITMAP), &BIT);
            int xsize = BIT.bmWidth;
            int ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 300, xsize, ysize, memdc, 0, 0, SRCCOPY);

            readyimg_pos.top = 300;
            readyimg_pos.left = 600;
            readyimg_pos.right = 600 + xsize;
            readyimg_pos.bottom = 300 + ysize;






            SelectObject(memdc, OldBitmap); // hImage 선택을 해제하기 위해 hOldBitmap을 선택한다
            DeleteObject(readyimg); // 선택 해제된 비트맵을 제거한다



        }


        if (gamestate == 2) {      //게임플레이 이미지

            if (ME != NULL)
            {
                Rectangle(hdc, ME->position.left - maincamera.pos.x, ME->position.top - maincamera.pos.y, ME->position.right - maincamera.pos.x, ME->position.bottom - maincamera.pos.y);
            }
            if (ENEMY != NULL)
            {
                Rectangle(hdc, ENEMY->position.left - maincamera.pos.x, ENEMY->position.top - maincamera.pos.y, ENEMY->position.right - maincamera.pos.x, ENEMY->position.bottom - maincamera.pos.y);
            }
            // Rectangle(hdc, Enemy.position.left, Enemy.position.top, Enemy.position.right, Enemy.position.bottom);
            for (int i = 0; i < bulletnum; i++)
            {
                Rectangle(hdc, Bullets[i]->position.left - maincamera.pos.x, Bullets[i]->position.top - maincamera.pos.y, Bullets[i]->position.right - maincamera.pos.x, Bullets[i]->position.bottom - maincamera.pos.y);


            }
            for (int i = 0; i < obstaclenum; i++)
            {
                Rectangle(hdc, Obstacles[i]->position.left - maincamera.pos.x, Obstacles[i]->position.top - maincamera.pos.y, Obstacles[i]->position.right - maincamera.pos.x, Obstacles[i]->position.bottom - maincamera.pos.y);


            }
            for (int i = 0; i < itemnum; i++)
            {
                Rectangle(hdc, Items[i]->position.left - maincamera.pos.x, Items[i]->position.top - maincamera.pos.y, Items[i]->position.right - maincamera.pos.x, Items[i]->position.bottom - maincamera.pos.y);


            }


        }
        if (gamestate == 3) //승리패배
        {

            if (ME->hp <= 0)
            {
                startimg = (HBITMAP)LoadImage(NULL, TEXT("lose.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
                OldBitmap = (HBITMAP)SelectObject(memdc, startimg);

            }


            if (ENEMY->hp <= 0)
            {
                startimg = (HBITMAP)LoadImage(NULL, TEXT("victory.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
                OldBitmap = (HBITMAP)SelectObject(memdc, startimg);

            }


            // 비트맵 정보를 알아낸다 getobject

            GetObject(startimg, sizeof(BITMAP), &BIT);
            int xsize = BIT.bmWidth;
            int ysize = BIT.bmHeight;

            BitBlt(hdc, 0, 0, xsize, ysize, memdc, 0, 0, SRCCOPY);

            /*
            teamplayimg_pos.top = 500;
            teamplayimg_pos.left = 600;
            teamplayimg_pos.right = 600 + xsize;
            teamplayimg_pos.bottom = 500 + ysize;
              */
              //10초후 사라지게만들자


            SelectObject(memdc, OldBitmap); // hImage 선택을 해제하기 위해 hOldBitmap을 선택한다
            DeleteObject(startimg); // 선택 해제된 비트맵을 제거한다



        }


        WaitForSingleObject(Event, 1);

        DeleteDC(memdc); //더블버퍼링
        EndPaint(hWnd, &ps);
        break;
    case WM_TIMER:



        //데이터를 계속받는다는식으로 말해야하나?


        if (gamestate == 1)
        {




        }
        //


        if (gamestate == 2)
        {
            keyinputtime += TIMERSEC;
            sendtime += TIMERSEC;
            receivetime += TIMERSEC;
            nowitemgauge += TIMERSEC;
            ME->bulletcreatetime += TIMERSEC;
            //8초마다 아이템생성

            /*
            if (nowitemgauge >= Itemcreatetime)
            {
                nowitemgauge = 0;

                srand(time(NULL));

                if (itemnum < 8)      //일단 8개까지생성
                {
                    Item* item = new Item;
                    item->itemtype = rand() % 2; //0파워업 1스피드업
                    item->itempower = 5;
                    item->size.x = 50;
                    item->size.y = 50;
                    //위치랜덤
                    item->position.top = rand() % 800 + 100;
                    item->position.left = rand() % 800 + 100;
                    item->position.right = item->position.left + item->size.x;
                    item->position.bottom = item->position.top + item->size.y;
                    Items[itemnum] = item;
                    itemnum++;

                }


            }
            */



            //키누르는경우 해석


            for (int i = 0; i < bulletnum; i++)
            {
                if (Bullets[i]->direction == 0)
                {
                    Bullets[i]->position.left += Bullets[i]->speed;
                    Bullets[i]->position.right += Bullets[i]->speed;
                }
                if (Bullets[i]->direction == 1)
                {
                    Bullets[i]->position.left -= Bullets[i]->speed;
                    Bullets[i]->position.right -= Bullets[i]->speed;
                }
                //0.1초당 100씩깍아줌
                Bullets[i]->survivaltime -= TIMERSEC;

                //충돌처리 bullet이 object랑 행동하는경우.

                             //원래 objectnum해서 비교이지만. 일단 enemy맞춤 ㄱ
             //충돌처리 시작

                if (ENEMY != NULL)
                {
                    //총알충돌처리시작

                    if ((ENEMY->position.left < Bullets[i]->position.left && Bullets[i]->position.left < ENEMY->position.right) || (ENEMY->position.left < Bullets[i]->position.right && Bullets[i]->position.right < ENEMY->position.right))
                    {

                        if ((ENEMY->position.top < Bullets[i]->position.top && Bullets[i]->position.top < ENEMY->position.bottom) || (ENEMY->position.top < Bullets[i]->position.bottom && Bullets[i]->position.bottom < ENEMY->position.bottom)) //비교값이 bullet이되어야함.
                        {




                            if (ENEMY != NULL)
                            {
                                ENEMY->hp -= Bullets[i]->power;
                                if (ENEMY->hp <= 0)
                                {

                                    gamestate = 3;
                                    //죽었다처리
                                   // delete ENEMY;
                                   // ENEMY = NULL;


                                }
                            }


                            Bullets[i]->exist = false;
                            delete Bullets[i];
                            // bulletnum -= 1;
                            bulletnum -= 1;
                            for (int j = 0; j < bulletnum; j++)
                            {

                                Bullets[j] = Bullets[j + 1];

                                //한칸씩땡겨준다.


                            }

                        }

                    }

                }
                //내캐릭터와 총알 충돌(적이보냄)


                if (ME != NULL)
                {
                    //총알충돌처리시작

                    if ((ME->position.left < Bullets[i]->position.left && Bullets[i]->position.left < ME->position.right) || (ME->position.left < Bullets[i]->position.right && Bullets[i]->position.right < ME->position.right))
                    {

                        if ((ME->position.top < Bullets[i]->position.top && Bullets[i]->position.top < ME->position.bottom) || (ME->position.top < Bullets[i]->position.bottom && Bullets[i]->position.bottom < ME->position.bottom)) //비교값이 bullet이되어야함.
                        {




                            if (ME != NULL)
                            {
                                ME->hp -= Bullets[i]->power;
                                if (ME->hp <= 0)
                                {
                                    //졌다 패배
                                    gamestate = 3;
                                    //죽었다처리
                              // delete ME;
                              // ME = NULL;


                                }
                            }


                            Bullets[i]->exist = false;
                            delete Bullets[i];
                            // bulletnum -= 1;
                            bulletnum -= 1;
                            for (int j = 0; j < bulletnum; j++)
                            {

                                Bullets[j] = Bullets[j + 1];

                                //한칸씩땡겨준다.


                            }

                        }

                    }

                }





                //생성시간 오버라 삭제 삭제조건이 여러개일텐데 왜 삭제조건이 여러개인데 왜꼬이냐?

                //삭제조건은 단한번??
                if ((Bullets[i]->survivaltime <= 0) && (Bullets[i]->exist == true))
                {

                    delete Bullets[i];
                    bulletnum -= 1;

                    for (int j = 0; j < bulletnum; j++)
                    {
                        Bullets[j] = Bullets[j + 1];
                        //한칸씩땡겨준다.


                    }

                }



            }

            //아이템 관련 실시간처리시작
            for (int i = 0; i < itemnum; i++)
            {

                //충돌처리 시작

                if (ME != NULL)
                {
                    //아이템충돌처리시작

                    if ((ME->position.left < Items[i]->position.left && Items[i]->position.left < ME->position.right) || (ME->position.left < Items[i]->position.right && Items[i]->position.right < ME->position.right))
                    {

                        if ((ME->position.top < Items[i]->position.top && Items[i]->position.top < ME->position.bottom) || (ME->position.top < Items[i]->position.bottom && Items[i]->position.bottom < ME->position.bottom)) //비교값이 bullet이되어야함.
                        {




                            if (ME != NULL)
                            {
                                //충돌일때 아이템임. 아이템사라지고  내 수치에맞게상향

                                if (Items[i]->itemtype == 0)  //파워업
                                {
                                    ME->power += Items[i]->itempower;
                                }


                                if (Items[i]->itemtype == 1)//스피드업
                                {
                                    ME->speed += Items[i]->itempower;
                                }

                                /*
                                ENEMY->hp -= Bullets[i]->power;
                                if (ENEMY->hp <= 0)
                                {

                                    delete ENEMY;
                                    ENEMY = NULL;


                                }
                                */
                            }

                            //닿은 충돌물(총알,아이템 삭제)
                            Items[i]->exist = false;
                            delete Items[i];
                            // bulletnum -= 1;
                            itemnum -= 1;
                            for (int j = 0; j < itemnum; j++)
                            {

                                Items[j] = Items[j + 1];

                                //한칸씩땡겨준다.


                            }

                        }

                    }

                }



                //나뿐만아니라 ENEMY도 처리시작
                if (ENEMY != NULL)
                {
                    //아이템충돌처리시작

                    if ((ENEMY->position.left < Items[i]->position.left && Items[i]->position.left < ENEMY->position.right) || (ENEMY->position.left < Items[i]->position.right && Items[i]->position.right < ENEMY->position.right))
                    {

                        if ((ENEMY->position.top < Items[i]->position.top && Items[i]->position.top < ENEMY->position.bottom) || (ENEMY->position.top < Items[i]->position.bottom && Items[i]->position.bottom < ENEMY->position.bottom)) //비교값이 bullet이되어야함.
                        {




                            if (ENEMY != NULL)
                            {
                                //충돌일때 아이템임. 아이템사라지고  내 수치에맞게상향

                                if (Items[i]->itemtype == 0)  //파워업
                                {
                                    //어차피 상대클라에서 처리함
                                    //ME->power += Items[i]->itempower;
                                }


                                if (Items[i]->itemtype == 1)//스피드업
                                {
                                    //  ME->speed += Items[i]->itempower;
                                }

                                /*
                                ENEMY->hp -= Bullets[i]->power;
                                if (ENEMY->hp <= 0)
                                {

                                    delete ENEMY;
                                    ENEMY = NULL;


                                }
                                */
                            }

                            //닿은 충돌물(총알,아이템 삭제)
                            Items[i]->exist = false;
                            delete Items[i];
                            // bulletnum -= 1;
                            itemnum -= 1;
                            for (int j = 0; j < itemnum; j++)
                            {

                                Items[j] = Items[j + 1];

                                //한칸씩땡겨준다.


                            }

                        }

                    }

                }



            }

            //장애물 관련 실시간처리시작
            for (int i = 0; i < obstaclenum; i++)
            {

                //충돌처리 시작

                if (ME != NULL)
                {

                    //이동채가 뒤로가야함 ㅇㅋ
                    if ((Obstacles[i]->position.left < ME->position.left && ME->position.left < Obstacles[i]->position.right) || (Obstacles[i]->position.left < ME->position.right && ME->position.right < Obstacles[i]->position.right))
                    {

                        if ((Obstacles[i]->position.top < ME->position.top && ME->position.top < Obstacles[i]->position.bottom) || (Obstacles[i]->position.top < ME->position.bottom && ME->position.bottom < Obstacles[i]->position.bottom)) //비교값이 bullet이되어야함.
                        {

                            //왜닿은처리가안나지??원인이머여
                         //닿은경우 me를없앤다

                            gamestate = 3;
                            ME->hp = -999;
                          
                        
                        }

                    }

                }


                //장애물과 적 충돌처리

                if (ENEMY != NULL)
                {

                    //이동채가 뒤로가야함 ㅇㅋ
                    if ((Obstacles[i]->position.left < ENEMY->position.left && ENEMY->position.left < Obstacles[i]->position.right) || (Obstacles[i]->position.left < ENEMY->position.right && ENEMY->position.right < Obstacles[i]->position.right))
                    {

                        if ((Obstacles[i]->position.top < ENEMY->position.top && ENEMY->position.top < Obstacles[i]->position.bottom) || (Obstacles[i]->position.top < ENEMY->position.bottom && ENEMY->position.bottom < Obstacles[i]->position.bottom)) //비교값이 bullet이되어야함.
                        {

                            //왜닿은처리가안나지??원인이머여
                         //닿은경우 me를없앤다

                            gamestate = 3;
                            ENEMY->hp = -999;

                            //적패배처리
                            //추후 서버와통신시 바꿈
                                //ME = NULL;

                                //패배처리
                        }

                    }

                }


            }

        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    //wm timer에서 네트워크기능수행.
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}