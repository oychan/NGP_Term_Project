#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#define SERVERIP   "127.0.0.1"          //�ǽ��ҽ� ������ ip�Է����ָ��
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
//������ �̸�



//���������� ����


//������Ű��� ������
HANDLE hThread_1, hThread_2;
SOCKET sock;
int retval;
char buf[BUFSIZE + 1];
//char* buf;          �����ʿ�?


int J_send_key;
char* J_send_char;



int gamestate = 0;

int sendtime;
int receivetime;



TCHAR* str;


int pnum;
HANDLE Event;








//���ӳ� ����������


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
        direction = 0;           //0������ 1����
        hp = 100;
        speed = 10;
        power = 10;
        powertype = 1;
        bulletcreatetime = TIMERSEC;

    }

};






//���ӳ� ����������

HDC hdc, memdc;
PAINTSTRUCT ps;





Bullet* Bullets[100];    //�Ѿ˵��� ����
Obstacle* Obstacles[100]; //��ֹ����� ����
Item* Items[100]; //������ ���� ����
Player* Objects[100];    //������Ʈ���� ����




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





//������Ž���
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

    //�� �����ش�.
    J_send_char = (char*)"zero";
    while (1) {

        // if 

        if (sendtime >= TIMERSEC)
        {
            //8�ʸ��� u�����ְ� ������Ʈ�ڷẸ����
            // n�ʸ��� �����ִ°� send  recv�� u�״������޴½� ����
            J_send_char = (char*)"U";  //���ں����°� ��������?  // char�������� 1[ �̷����ؼ� ���ڸ� [�ΰ��     
            retval = send(sock, J_send_char, strlen(J_send_char), 0);
            J_send_char = (char*)"zero";

            //��ġ������ٲ۴ٰ� �������Ŀ� �Ѵ�
            retval = send(server_sock, (char*)ME, sizeof(*ME), 0);   //me�ϱ� ��¥��ũ��

            sendtime = 0;
        }

        if (J_send_key == 1) //����Ű�� �����ִ°�� ����Ű������
        {

            if (J_send_key == 1)
            {
                J_send_char = (char*)"C";  //���ں����°� ��������?  // char�������� 1[ �̷����ؼ� ���ڸ� [�ΰ��                 
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

        if (J_send_char != "zero")   //�������� ����ɰ�� send������
        {
            //r��������Ȯ��

            str = TEXT(J_send_char);

            retval = send(server_sock, J_send_char, strlen(J_send_char), 0); // char��������.

            J_send_char = (char*)"zero";
        }

    }

    closesocket(server_sock);

    return 0;
}




//recv������
DWORD WINAPI PositionRecv(LPVOID arg)     //���޴´�.
{
    int retval;

    char buf[BUFSIZE + 1]; //�̰ɾ��ϸ� ���� �ֱ׷���..;;

    //�����޾ƺ���

    SOCKET client_sock = (SOCKET)arg;


    while (1) {

        // retval = recv(client_sock, buf, BUFSIZE, 0);    //���̰� ��ȯ��.
        retval = recv(client_sock, buf, 1, 0);                         //�ϳ����޾Ƽ��׷���? �����̷ι޾ƺ���?
       // retval = recv(client_sock, buf, 1, 0); //�ϳ����޾��ش�.
        if (retval == SOCKET_ERROR) {
            break;
        }
        else if (retval == 0)
            break;

        //  buf[retval] = '\0';        //�������°��� ������������ '\0'���ش�.
                   //recv buf �Ǻ����̻��ؼ����̴°ǰ�?

            //
        if (retval != 0) //���������������.
        {


            int len = strlen(buf);

            if (buf[0] == 'R')
            {
                // exit(1);
                gamestate = 2;
                Player* ptemp = new Player;
                retval = recv(client_sock, (char*)ptemp, sizeof(*ptemp), 0);
                //�����Ѱ���

                ME->hp = ptemp->hp;      //�����͸���������
                ME->position = ptemp->position;


                delete ptemp;


            }


            if (buf[0] == 'O')
            {

                //��ֹ� ����Ű�ΰ�� ��ֹ�����


                Obstacle* obstacle = new Obstacle;
                Obstacle* otemp = new Obstacle;
                retval = recv(client_sock, (char*)otemp, sizeof(*otemp), 0);
                //�����Ѱ���

                obstacle->obstacletype = otemp->obstacletype;
                obstacle->position = otemp->position;

                Obstacles[obstaclenum] = obstacle;

                obstaclenum++;


                delete otemp;
                //  ME->hp = ptemp->hp;      //�����͸���������
                //  ME->position = ptemp->position;


                //  delete ptemp;


            }


            if (buf[0] == 'I')
            {

                //������ ����Ű�ΰ�� �����ۻ���


                Item* item = new Item;
                Item* itemp = new Item;
                retval = recv(client_sock, (char*)itemp, sizeof(*itemp), 0);
                //�����Ѱ���


                item->itemtype = itemp->itemtype; //0�Ŀ��� 1���ǵ��
                item->itempower = itemp->itempower;
                item->size = itemp->size;
                item->position = itemp->position;


                Items[itemnum] = item;

                itemnum++;


                delete itemp;
                //  ME->hp = ptemp->hp;      //�����͸���������
                //  ME->position = ptemp->position;


                //  delete ptemp;


            }


            if (buf[0] == 'B')
            {

                //�ҷ�������� �ҷ�����


                Bullet* bullet = new Bullet;
                Bullet* btemp = new Bullet;
                retval = recv(client_sock, (char*)btemp, sizeof(*btemp), 0);
                //�����Ѱ���


                bullet->direction = btemp->direction; //0�Ŀ��� 1���ǵ��
                bullet->power = btemp->power;
                bullet->position = btemp->position;
                //  item->position = ibemp->position;


                Bullets[bulletnum] = bullet;

                bulletnum++;


                delete btemp;


            }



            //        //buf[0]=='P' �ΰ���ε� ��
            if (buf[0] == 'U') //12345678 ���� 1�ΰ�� 
            {
                // wsprintf(str, TEXT((char*)"����buf�� %d�Դϴ�."), 100 );
               //  str = TEXT((char*)"hiruhiru %d");
                pnum = buf[0];                //����ü buf���� R�� �ֵ��İ� ���ذ��Ȱ���

                Player* ptemp = new Player;
                retval = recv(client_sock, (char*)ptemp, sizeof(*ptemp), 0);
                //�����Ѱ���


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

            SetEvent(Event);    //  ����ƴϸ� �׹ؿ�

        }


    }

    closesocket(client_sock);

    return 0;
}






//

// 0Ÿ��Ʋȭ�� 1���ӽ��۴����� ���ӷε�â  2���ӽ����÷���


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//Player ����ü
// ��ġ4����, ����Ÿ��,�����۰���,

//�������� winmain


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    static char szAppName[] = "Midterm";
    static char szTitle[] = "2020-2 networkgame";
    MSG  msg;
    HWND  hWnd;                 //������â����Ÿ� �̺κм���
    WNDCLASS wc;              //������â����Ÿ��̺κм���






    //�̰Ծ����� ǥ�þ���       �ϰ������ �ٸ��ݹ��Լ����ϰ�����ſ��µ�;





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


    RegisterClass(&wc);  //�̰Ծ����� ǥ�þ���


    hWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, 0, WindowFullWidth + MistakeWidth, WindowFullHeight + MistakeHeight, NULL, NULL, hInstance, NULL);
    //�����츸���.
    //+9+32��
      //��ũ��
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






//SPACE������ �ҷ�����
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    //�̹����� ��ǥ������
    //���Ӹ�� w������ ���Ӹ��0
    switch (uMsg)
    {

    case WM_CREATE:




        //����Ʈ�� ME
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

        //����ġ�� 200
        maincamera.pos.x = 0;
        maincamera.pos.y = 0;

        MapMinSize.x = 10;
        MapMinSize.y = 0;


        //�������̶� ������Ʈ �ٷλ������� ��
        //
        MapMaxSize.x = 1300;
        MapMaxSize.y = 1300;
        SetTimer(hWnd, 1, TIMERSEC, NULL);




        break;
    case WM_CHAR:



        if (gamestate == 2) {      //���ӻ���2�϶��� �ش��̵�.




            if (wParam == 'a' || wParam == 'A')
            {
                //�Ѿ˻���

                if (ME->bulletcreatetime >= 800)   //�ҷ����簡 1�ʿ� 3ȸ�� �ִ�� ����
                {
                    //�Ѿ˻������Ǵ�
                    ME->bulletcreatetime = 0;

                    J_send_char = (char*)"B";
                    //�Ѿ˻���

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

            //�ӽ÷� Ű������ �ƹ��������� ����

            /*
            if (ME->position.top > MapMinSize.y)     //ĳ���ּ���ġ
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


                 //�������ú���
                 /*
                 if (maincamera.pos.y > MapMinSize.y)     //ī�޶� �ּ���ġ
                 {

                     if (ME->position.top < (MapMaxSize.y - (WindowFullHeight / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                     {
                         maincamera.pos.y -= ME->speed;
                     }

                     if (maincamera.pos.y < MapMinSize.y)
                     {
                         maincamera.pos.y = MapMinSize.y;       //-�Ǵ°�� 0���κ���
                     }
                 }

                   */

                   //  InvalidateRect(hWnd, NULL, TRUE);
        }
        if (wParam == VK_DOWN)
        {

            J_send_char = (char*)"s";

            //   ��������ó���ϸ鼭 �����
/*
if ((ME->position.bottom) < MapMaxSize.y)     //ĳ���ּ���ġ
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
        if (maincamera.pos.y < (MapMaxSize.y - (WindowFullHeight)))     //ī�޶� �ּ���ġ             //8�� ������ ������� 1300�̶�ġ���¦���ڸ�
        {

            if (ME->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
            {
                maincamera.pos.y += ME->speed;
            }
            if (maincamera.pos.y > (MapMaxSize.y - (WindowFullHeight + 9)))
            {
                maincamera.pos.y = (MapMaxSize.y - (WindowFullHeight - 9));       //-�Ǵ°�� 0���κ���
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
            //top�̾ƴ϶� left
            if (ME->position.left > MapMinSize.x)     //ĳ���ּ���ġ
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
                  if (maincamera.pos.x > MapMinSize.x)     //ī�޶� �ּ���ġ
                  {

                      if (ME->position.right < (MapMaxSize.x - (WindowFullWidth / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                      {
                          maincamera.pos.x -= ME->speed;
                      }

                      if (maincamera.pos.x < MapMinSize.x)
                      {
                          maincamera.pos.x = MapMinSize.x;       //-�Ǵ°�� 0���κ���
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
            if ((ME->position.right) < MapMaxSize.x)     //�Ϲ�����Ȳ ����Ű�� �����������̵�
            {
                ME->position.left += ME->speed;
                ME->position.right += ME->speed;

                if ((ME->position.right) > MapMaxSize.x)  //���� ��������ϸ� ����
                {
                    ME->position.right = MapMaxSize.x;
                    ME->position.left = ME->position.right - ME->size.x;
                }



            }
                */

                /*
                if (maincamera.pos.x < (MapMaxSize.x - (WindowFullWidth)))     //ī�޶� �ּ���ġ             //8�� ������ ������� 1300�̶�ġ���¦���ڸ�
                {

                    if (ME->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                    {
                        maincamera.pos.x += ME->speed;
                    }
                    if (maincamera.pos.x > (MapMaxSize.x - (WindowFullWidth + 9)))
                    {
                        maincamera.pos.x = (MapMaxSize.x - (WindowFullWidth - 9));       //-�Ǵ°�� 0���κ���
                    }
                }
                  */
                  // InvalidateRect(hWnd, NULL, TRUE);

        }


        break;

    case WM_LBUTTONDOWN:

        mouseposx = LOWORD(lParam);
        mouseposy = HIWORD(lParam);



        if (gamestate == 0)        //Ÿ��Ʋȭ�� ȥ���ϱ�,�����ϱⰡ�ִ�.
        {


            if (mouseposx > teamplayimg_pos.left && mouseposx < teamplayimg_pos.right)
            {

                if (mouseposy > teamplayimg_pos.top && mouseposy < teamplayimg_pos.bottom)
                {

                    //������Ž���
                    //���÷��� �ϱ� �������� ������ ��Ž���.
                    gamestate = 1;            //�������â�� ��������

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

                    hThread_1 = CreateThread(NULL, 0, KeySend, (LPVOID)sock, 0, NULL);                    //sned������.

                    hThread_2 = CreateThread(NULL, 0, PositionRecv, (LPVOID)sock, 0, NULL);   //recv������



                    J_send_key = 1; //Ű������

                    //����ü��������
                     /*
                     Player me;
                     retval = send(sock, (char*)&me, sizeof(me), 0);

                     printf((char*)"[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\r\n", retval);

                       //�����ͱ���
                    //retval = send(sock, (char*)ME, sizeof(*ME), 0);        //���� �����Ͱ������⼺��
                    */

                }
            }





            if (mouseposx > soloplayimg_pos.left && mouseposx < soloplayimg_pos.right)
            {

                if (mouseposy > soloplayimg_pos.top && mouseposy < soloplayimg_pos.bottom)
                {
                    gamestate = 2;            //�������â�� ��������

                }
            }


        }

        if (gamestate == 1)        //�����ϱ⴩�� ä�ô��â.
        {


            if (mouseposx > readyimg_pos.left && mouseposx < readyimg_pos.right)
            {

                if (mouseposy > readyimg_pos.top && mouseposy < readyimg_pos.bottom)
                {



                    J_send_char = (char*)"R";

                    // J_send_key = 1; //Ű������

                }
            }
        }




        break;

    case WM_PAINT:
        //�׷��ִµ�  �Ǹ��ֳ�..? �׷��ִºκп��� �����ֱ��� maincamera.x �� 2000�̰� ENEMY�� 2400�̸�  ENEMY - MAINCAMERA �̷������� ó���Ѵ�.
        hdc = BeginPaint(hWnd, &ps);
        memdc = CreateCompatibleDC(hdc);        //������۸�

       // WaitForSingleObject(Event,20);

      //  TCHAR str2[128];
      //  wsprintf(str2, TEXT((char*)"����item������ %d�Դϴ�. \n"), itemnum);
     //   TextOut(hdc, 100, 100, str2, lstrlen(str2));

       // TextOut(hdc, 400, 100, str, lstrlen(str));

        if (gamestate == 0) {               //����Ÿ��Ʋȭ���ΰ�� ���


            startimg = (HBITMAP)LoadImage(NULL, TEXT("startsolo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            OldBitmap = (HBITMAP)SelectObject(memdc, startimg);



            // ��Ʈ�� ������ �˾Ƴ��� getobject

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



            // ��Ʈ�� ������ �˾Ƴ��� getobject

            GetObject(startimg, sizeof(BITMAP), &BIT);
            xsize = BIT.bmWidth;
            ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 500, xsize, ysize, memdc, 0, 0, SRCCOPY);

            teamplayimg_pos.top = 500;
            teamplayimg_pos.left = 600;
            teamplayimg_pos.right = 600 + xsize;
            teamplayimg_pos.bottom = 500 + ysize;



            SelectObject(memdc, OldBitmap); // hImage ������ �����ϱ� ���� hOldBitmap�� �����Ѵ�
            DeleteObject(startimg); // ���� ������ ��Ʈ���� �����Ѵ�


            /*
            HBITMAP startimg2 = (HBITMAP)LoadImage(NULL, TEXT("startsolo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);


            GetObject(startimg2, sizeof(BITMAP), &BIT);

            xsize = BIT.bmWidth;
            ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 600, xsize, ysize, memdc, 0, 0, SRCCOPY);

            // if ���� �� ���̰��ȿ� ���콺Ŭ���Ѱ��


            SelectObject(memdc, OldBitmap); // hImage ������ �����ϱ� ���� hOldBitmap�� �����Ѵ�
            DeleteObject(startimg2); // ���� ������ ��Ʈ���� �����Ѵ�

              */
        }

        if (gamestate == 1) {               //����Ÿ��Ʋȭ���ΰ�� ���


            HBITMAP readyimg = (HBITMAP)LoadImage(NULL, TEXT("ready.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            OldBitmap = (HBITMAP)SelectObject(memdc, readyimg);



            // ��Ʈ�� ������ �˾Ƴ��� getobject

            GetObject(readyimg, sizeof(BITMAP), &BIT);
            int xsize = BIT.bmWidth;
            int ysize = BIT.bmHeight;

            BitBlt(hdc, 600, 300, xsize, ysize, memdc, 0, 0, SRCCOPY);

            readyimg_pos.top = 300;
            readyimg_pos.left = 600;
            readyimg_pos.right = 600 + xsize;
            readyimg_pos.bottom = 300 + ysize;






            SelectObject(memdc, OldBitmap); // hImage ������ �����ϱ� ���� hOldBitmap�� �����Ѵ�
            DeleteObject(readyimg); // ���� ������ ��Ʈ���� �����Ѵ�



        }


        if (gamestate == 2) {      //�����÷��� �̹���

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
        if (gamestate == 3) //�¸��й�
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


            // ��Ʈ�� ������ �˾Ƴ��� getobject

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
              //10���� ������Ը�����


            SelectObject(memdc, OldBitmap); // hImage ������ �����ϱ� ���� hOldBitmap�� �����Ѵ�
            DeleteObject(startimg); // ���� ������ ��Ʈ���� �����Ѵ�



        }


        WaitForSingleObject(Event, 1);

        DeleteDC(memdc); //������۸�
        EndPaint(hWnd, &ps);
        break;
    case WM_TIMER:



        //�����͸� ��ӹ޴´ٴ½����� ���ؾ��ϳ�?


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
            //8�ʸ��� �����ۻ���

            /*
            if (nowitemgauge >= Itemcreatetime)
            {
                nowitemgauge = 0;

                srand(time(NULL));

                if (itemnum < 8)      //�ϴ� 8����������
                {
                    Item* item = new Item;
                    item->itemtype = rand() % 2; //0�Ŀ��� 1���ǵ��
                    item->itempower = 5;
                    item->size.x = 50;
                    item->size.y = 50;
                    //��ġ����
                    item->position.top = rand() % 800 + 100;
                    item->position.left = rand() % 800 + 100;
                    item->position.right = item->position.left + item->size.x;
                    item->position.bottom = item->position.top + item->size.y;
                    Items[itemnum] = item;
                    itemnum++;

                }


            }
            */



            //Ű�����°�� �ؼ�


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
                //0.1�ʴ� 100�������
                Bullets[i]->survivaltime -= TIMERSEC;

                //�浹ó�� bullet�� object�� �ൿ�ϴ°��.

                             //���� objectnum�ؼ� ��������. �ϴ� enemy���� ��
             //�浹ó�� ����

                if (ENEMY != NULL)
                {
                    //�Ѿ��浹ó������

                    if ((ENEMY->position.left < Bullets[i]->position.left && Bullets[i]->position.left < ENEMY->position.right) || (ENEMY->position.left < Bullets[i]->position.right && Bullets[i]->position.right < ENEMY->position.right))
                    {

                        if ((ENEMY->position.top < Bullets[i]->position.top && Bullets[i]->position.top < ENEMY->position.bottom) || (ENEMY->position.top < Bullets[i]->position.bottom && Bullets[i]->position.bottom < ENEMY->position.bottom)) //�񱳰��� bullet�̵Ǿ����.
                        {




                            if (ENEMY != NULL)
                            {
                                ENEMY->hp -= Bullets[i]->power;
                                if (ENEMY->hp <= 0)
                                {

                                    gamestate = 3;
                                    //�׾���ó��
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

                                //��ĭ�������ش�.


                            }

                        }

                    }

                }
                //��ĳ���Ϳ� �Ѿ� �浹(���̺���)


                if (ME != NULL)
                {
                    //�Ѿ��浹ó������

                    if ((ME->position.left < Bullets[i]->position.left && Bullets[i]->position.left < ME->position.right) || (ME->position.left < Bullets[i]->position.right && Bullets[i]->position.right < ME->position.right))
                    {

                        if ((ME->position.top < Bullets[i]->position.top && Bullets[i]->position.top < ME->position.bottom) || (ME->position.top < Bullets[i]->position.bottom && Bullets[i]->position.bottom < ME->position.bottom)) //�񱳰��� bullet�̵Ǿ����.
                        {




                            if (ME != NULL)
                            {
                                ME->hp -= Bullets[i]->power;
                                if (ME->hp <= 0)
                                {
                                    //���� �й�
                                    gamestate = 3;
                                    //�׾���ó��
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

                                //��ĭ�������ش�.


                            }

                        }

                    }

                }





                //�����ð� ������ ���� ���������� ���������ٵ� �� ���������� �������ε� �ֲ��̳�?

                //���������� ���ѹ�??
                if ((Bullets[i]->survivaltime <= 0) && (Bullets[i]->exist == true))
                {

                    delete Bullets[i];
                    bulletnum -= 1;

                    for (int j = 0; j < bulletnum; j++)
                    {
                        Bullets[j] = Bullets[j + 1];
                        //��ĭ�������ش�.


                    }

                }



            }

            //������ ���� �ǽð�ó������
            for (int i = 0; i < itemnum; i++)
            {

                //�浹ó�� ����

                if (ME != NULL)
                {
                    //�������浹ó������

                    if ((ME->position.left < Items[i]->position.left && Items[i]->position.left < ME->position.right) || (ME->position.left < Items[i]->position.right && Items[i]->position.right < ME->position.right))
                    {

                        if ((ME->position.top < Items[i]->position.top && Items[i]->position.top < ME->position.bottom) || (ME->position.top < Items[i]->position.bottom && Items[i]->position.bottom < ME->position.bottom)) //�񱳰��� bullet�̵Ǿ����.
                        {




                            if (ME != NULL)
                            {
                                //�浹�϶� ��������. �����ۻ������  �� ��ġ���°Ի���

                                if (Items[i]->itemtype == 0)  //�Ŀ���
                                {
                                    ME->power += Items[i]->itempower;
                                }


                                if (Items[i]->itemtype == 1)//���ǵ��
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

                            //���� �浹��(�Ѿ�,������ ����)
                            Items[i]->exist = false;
                            delete Items[i];
                            // bulletnum -= 1;
                            itemnum -= 1;
                            for (int j = 0; j < itemnum; j++)
                            {

                                Items[j] = Items[j + 1];

                                //��ĭ�������ش�.


                            }

                        }

                    }

                }



                //���Ӹ��ƴ϶� ENEMY�� ó������
                if (ENEMY != NULL)
                {
                    //�������浹ó������

                    if ((ENEMY->position.left < Items[i]->position.left && Items[i]->position.left < ENEMY->position.right) || (ENEMY->position.left < Items[i]->position.right && Items[i]->position.right < ENEMY->position.right))
                    {

                        if ((ENEMY->position.top < Items[i]->position.top && Items[i]->position.top < ENEMY->position.bottom) || (ENEMY->position.top < Items[i]->position.bottom && Items[i]->position.bottom < ENEMY->position.bottom)) //�񱳰��� bullet�̵Ǿ����.
                        {




                            if (ENEMY != NULL)
                            {
                                //�浹�϶� ��������. �����ۻ������  �� ��ġ���°Ի���

                                if (Items[i]->itemtype == 0)  //�Ŀ���
                                {
                                    //������ ���Ŭ�󿡼� ó����
                                    //ME->power += Items[i]->itempower;
                                }


                                if (Items[i]->itemtype == 1)//���ǵ��
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

                            //���� �浹��(�Ѿ�,������ ����)
                            Items[i]->exist = false;
                            delete Items[i];
                            // bulletnum -= 1;
                            itemnum -= 1;
                            for (int j = 0; j < itemnum; j++)
                            {

                                Items[j] = Items[j + 1];

                                //��ĭ�������ش�.


                            }

                        }

                    }

                }



            }

            //��ֹ� ���� �ǽð�ó������
            for (int i = 0; i < obstaclenum; i++)
            {

                //�浹ó�� ����

                if (ME != NULL)
                {

                    //�̵�ä�� �ڷΰ����� ����
                    if ((Obstacles[i]->position.left < ME->position.left && ME->position.left < Obstacles[i]->position.right) || (Obstacles[i]->position.left < ME->position.right && ME->position.right < Obstacles[i]->position.right))
                    {

                        if ((Obstacles[i]->position.top < ME->position.top && ME->position.top < Obstacles[i]->position.bottom) || (Obstacles[i]->position.top < ME->position.bottom && ME->position.bottom < Obstacles[i]->position.bottom)) //�񱳰��� bullet�̵Ǿ����.
                        {

                            //�ִ���ó�����ȳ���??�����̸ӿ�
                         //������� me�����ش�

                            gamestate = 3;
                            ME->hp = -999;
                          
                        
                        }

                    }

                }


                //��ֹ��� �� �浹ó��

                if (ENEMY != NULL)
                {

                    //�̵�ä�� �ڷΰ����� ����
                    if ((Obstacles[i]->position.left < ENEMY->position.left && ENEMY->position.left < Obstacles[i]->position.right) || (Obstacles[i]->position.left < ENEMY->position.right && ENEMY->position.right < Obstacles[i]->position.right))
                    {

                        if ((Obstacles[i]->position.top < ENEMY->position.top && ENEMY->position.top < Obstacles[i]->position.bottom) || (Obstacles[i]->position.top < ENEMY->position.bottom && ENEMY->position.bottom < Obstacles[i]->position.bottom)) //�񱳰��� bullet�̵Ǿ����.
                        {

                            //�ִ���ó�����ȳ���??�����̸ӿ�
                         //������� me�����ش�

                            gamestate = 3;
                            ENEMY->hp = -999;

                            //���й�ó��
                            //���� ��������Ž� �ٲ�
                                //ME = NULL;

                                //�й�ó��
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
    //wm timer���� ��Ʈ��ũ��ɼ���.
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}