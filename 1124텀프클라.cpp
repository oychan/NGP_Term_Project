#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#define SERVERIP   "127.0.0.1"
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

#define TIMERSEC 100
//윈도우 이름

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
struct Player{
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


// 0타이틀화면 1게임시작누른후 게임로딩창  2게임실제플레이
int gamestate = 0;

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
        CW_USEDEFAULT, 0, WindowFullWidth+MistakeWidth, WindowFullHeight+MistakeHeight, NULL, NULL, hInstance, NULL);  
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
    HDC hdc,memdc;
    PAINTSTRUCT ps;


    //서버관련시작
    static SOCKET sock;
    int retval;
    char buf[BUFSIZE + 1];

   // SOCKET server_sock;

    static Bullet *Bullets[100];    //총알들의 모음
    static Obstacle* Obstacles[100]; //장애물들의 모음
    static Item* Items[100]; //아이템 들의 모음
    static Player* Objects[100];    //오브젝트들의 모음


    
  //  static Player *ENEMY;
    static int objectnum;
    static int bulletnum;
    static int obstaclenum;
    static int itemnum;

    static Player* ENEMY = new Player;
    static Player* ME = new Player;
  
    static Camera maincamera;
    static Pos MapMaxSize;
    static Pos MapMinSize;

    static int Itemcreatetime = 8000;
    static int nowitemgauge = 0;
    static bool obstaclecreate = true;

    //HBITMAP start_bitmap = (HBITMAP)LoadImage(NULL, "..\start.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    HBITMAP startimg, OldBitmap;
    BITMAP BIT;
    
    static RECT soloplayimg_pos;
    static int mouseposx;
    static int mouseposy;
    
    static RECT teamplayimg_pos;

    static RECT readyimg_pos;
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
        maincamera.pos.x =0;
        maincamera.pos.y =0;

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

                if (ME->bulletcreatetime >= 300)   //불렛난사가 1초에 3회가 최대로 설정
                {
                    ME->bulletcreatetime = 0;
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
                    //일단 총알생성인데 넣어주는것은 0~100해서 
                    //얻은점 총 데이터파일을 포인터를모아둔것들의 배열로해놔야 삭제가 자유로움.
                }

            }

        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_UP)
        {
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

          
            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (wParam == VK_DOWN)
        {
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

            if (maincamera.pos.y < (MapMaxSize.y - (WindowFullHeight)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
            {

                if (ME->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
                {
                    maincamera.pos.y += ME->speed;
                }
                if (maincamera.pos.y > (MapMaxSize.y - (WindowFullHeight+9)))
                {
                    maincamera.pos.y = (MapMaxSize.y - (WindowFullHeight-9));       //-되는경우 0으로보정
                }
            }

            //down이 문제다 ㅇㅋ
            /* down원래
            Me.position.top += Me.speed;
            Me.position.bottom += Me.speed;
            maincamera.pos.y += Me.speed;
            */
            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (wParam == VK_LEFT)
        {
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

            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (wParam == VK_RIGHT)
        {
            ME->direction = 0;
            if ( (ME->position.right) < MapMaxSize.x)     //일반적상황 오른키시 오른쪽으로이동
            {
                ME->position.left += ME->speed;
                ME->position.right += ME->speed;

                if ((ME->position.right) > MapMaxSize.x)  //맵을 벗어나려고하면 보정
                {
                    ME->position.right = MapMaxSize.x;
                    ME->position.left = ME->position.right - ME->size.x;
                }



            }

            if (maincamera.pos.x < (MapMaxSize.x-(WindowFullWidth) ) )     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
            {
          
                if (ME->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                {
                    maincamera.pos.x += ME->speed;
                }
                if (maincamera.pos.x > (MapMaxSize.x - (WindowFullWidth+9) ))
                {
                    maincamera.pos.x = (MapMaxSize.x - (WindowFullWidth-9));       //-되는경우 0으로보정
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (GetKeyState(VK_UP) & 0x8000)
        {
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



           
            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (GetKeyState(VK_RIGHT) & 0x8000)
        {
            ME->direction = 0;
            if ((ME->position.right) < MapMaxSize.x)     //캐릭최소위치
            {
                ME->position.left += ME->speed;
                ME->position.right += ME->speed;

                if ((ME->position.right) > MapMaxSize.x)
                {
                    ME->position.right = MapMaxSize.x;
                    ME->position.left = ME->position.right - ME->size.x;
                }



            }

            if (maincamera.pos.x < (MapMaxSize.x - (WindowFullWidth )))     //카메라 최소위치
            {
               
                if (ME->position.left > (MapMinSize.x + (WindowFullWidth/2)))
                {
                    maincamera.pos.x += ME->speed;
                }
                if (maincamera.pos.x > (MapMaxSize.x - (WindowFullWidth+9)))
                {
                    maincamera.pos.x = (MapMaxSize.x - (WindowFullWidth-9));       //-되는경우 0으로보정  //이런이유가 대체뭐야
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (GetKeyState(VK_DOWN) & 0x8000)
        {
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

            if (maincamera.pos.y < (MapMaxSize.y - (WindowFullHeight)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
            {

                if (ME->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
                {
                    maincamera.pos.y += ME->speed;
                }
                if (maincamera.pos.y > (MapMaxSize.y - (WindowFullHeight+9)))
                {
                    maincamera.pos.y = (MapMaxSize.y - (WindowFullHeight-9));       //-되는경우 0으로보정
                }
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
        if (GetKeyState(VK_LEFT) & 0x8000)
        {
            ME->direction = 1;
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

            InvalidateRect(hWnd, NULL, TRUE);
        }


        break;

    case WM_LBUTTONDOWN:

        mouseposx = LOWORD(lParam);
        mouseposy = HIWORD(lParam);

        //gamestate = 2;
                             //left right쪽은 mouseposx
              //me는 내마우스 items는 스타트이미지
        
        if (gamestate == 0)
        {

                                 
            if (mouseposx > teamplayimg_pos.left && mouseposx < teamplayimg_pos.right)
            {

                if (mouseposy > teamplayimg_pos.top && mouseposy < teamplayimg_pos.bottom)
                {
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


                    
                    //char형 보내기 성공 ㅇㅋ
                    //연결하고 안녕하세요를 보냈다
                    //char* buf = (char*)"안녕하세요s";       //대화s
                                                            //recv는 쓰레드에서실행
                    char* buf = (char*)"C"; //커넥트완료라는뜻
                    retval = send(sock, buf, strlen(buf), 0);

                   


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

        if (gamestate == 1)
        {


            if (mouseposx > readyimg_pos.left && mouseposx < readyimg_pos.right)
            {

                if (mouseposy > readyimg_pos.top && mouseposy < readyimg_pos.bottom)
                {
                    //준비하기 누른경우니까
                


                    char* buf = (char*)"R"; //레디 오케이라는뜻
                    retval = send(sock, buf, strlen(buf), 0);

                     //보내지말고 받기만해볼까?
                    //보내는건되는데 왜 받는게안되지?

                    /*
                    retval = recv(sock, buf, BUFSIZE, 0);    //길이가 반환됨.
                    if (retval == SOCKET_ERROR) {
                        //err_display((char*)"recv()");
                        break;
                    }
                    else if (retval == 0)
                        break;
                    
                    if (retval > 1)
                    {
                        gamestate = 2;
                        //뭔가값을받았다는거
                    }

                    buf[retval] = '\0';
                    if (buf[retval - 1] == 'R')
                    {
                        gamestate = 2;

                    }
                      */
                  
                }
            }
        }


    
       
        break;

    case WM_PAINT:
        //그려주는데  또머있냐..? 그려주는부분에서 차이있긴함 maincamera.x 이 2000이고 ENEMY가 2400이면  ENEMY - MAINCAMERA 이런식으로 처리한다.
        hdc = BeginPaint(hWnd, &ps);
        memdc = CreateCompatibleDC(hdc);        //더블버퍼링




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
            soloplayimg_pos.right = 600+xsize;
            soloplayimg_pos.bottom = 300+ysize;



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

        DeleteDC(memdc); //더블버퍼링
        EndPaint(hWnd, &ps);
        break;
    case WM_TIMER:

        //데이터를 계속받는다는식으로 말해야하나?


        if (gamestate == 1)
        {


          

          /*
            retval = recv(sock, buf, BUFSIZE, 0);
            buf[retval] = '\0';
            if (buf[retval - 1] == 'R')

            {
                exit(1);
            }


            */
            //*받는상태여야함

            /*
            retval = recv(server_sock, buf, BUFSIZE, 0);    //길이가 반환됨.
            if (retval == SOCKET_ERROR) {
                break;
            }
            else if (retval == 0)
                break;



            buf[retval] = '\0';
              */
        }
        //
        nowitemgauge += TIMERSEC;
        ME->bulletcreatetime += TIMERSEC;
        //8초마다 아이템생성
            if(nowitemgauge >= Itemcreatetime)
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
        //맨처음에 장애물생성
        if (obstaclecreate == true)
        {
            Obstacle* obstacle = new Obstacle;


            obstacle->obstacletype = 4;
            obstacle->position.top = 500;
            obstacle->position.left = 300;
            obstacle->position.right = 500;
            obstacle->position.bottom = 1300;
            Obstacles[obstaclenum] = obstacle;
            obstaclenum++;

            obstaclecreate = false;

        }


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
                               
                                delete ENEMY;
                                ENEMY = NULL;


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
            if ( ( Bullets[i]->survivaltime <= 0  )&& (Bullets[i]->exist==true))
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
                      
                        ME->position.top = -3000;
                            ME->position.left = -3100;
                            ME->position.right = -3000;
                            ME->position.bottom = -3100;
                            //추후 서버와통신시 바꿈
                                //ME = NULL;

                                //패배처리
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