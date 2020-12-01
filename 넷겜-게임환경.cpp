
#define _CRT_SECURE_NO_WARNINGS         // 최신 VC++ 컴파일 시 경고 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include<time.h>//랜덤함수용
#define SERVERPORT 9000
#define BUFSIZE    512
#define TIMERSEC    80

#define MistakeWidth   9
#define MistakeHeight 32
#define WindowFullWidth 1000
#define WindowFullHeight 800


// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char* fmt, ...);
// 오류 출력 함수
void err_quit(char* msg);
void err_display(char* msg);
// 소켓 통신 스레드 함수
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);              //buf쓰레드생성

HINSTANCE hInst; // 인스턴스 핸들
HWND hEdit; // 편집 컨트롤
CRITICAL_SECTION cs; // 임계 영역


//클라이언트의 자료형들

//게임내 구조체선언


int recvn(SOCKET s, char* buf, int len, int flags) {
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR) return SOCKET_ERROR;
        else if (received == 0) break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

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




//서버통신에 쓰이는 변수들
int clientuser;

int P1_ready = 0;
int P2_ready = 0;
Player serverME;

int P1_Start = 0;
int P2_Start = 0;
//SOCKET client_sock;
SOCKET client_socks[2];
HANDLE Event[2];
//char buf[BUFSIZE + 1];
char* buf;

int sendtime;
int receivetime;

//서버에서 생성한 객체들을 클라이언트한테 보내는조건
int obstaclesend = 0;
int itemsend = 0;
int bulletsend = 0;

int gamestate = 0;

//게임내 전역변수선언

HDC hdc, memdc;
PAINTSTRUCT ps;





Bullet* Bullets[100];    //총알들의 모음
Obstacle* Obstacles[100]; //장애물들의 모음
Item* Items[100]; //아이템 들의 모음



//  static Player *ENEMY;
int objectnum;
int bulletnum;
int obstaclenum;
int itemnum;

Player* P1 = new Player;
Player* P2 = new Player;

Camera* P1_maincamera=new Camera;
Camera* P2_maincamera=new Camera;
Pos MapMaxSize;
Pos MapMinSize;

int Itemcreatetime = 8000;
int nowitemgauge = 0;
bool obstaclecreate = true;



//게임내 변수선언끝



//send전용 쓰레드
DWORD WINAPI SendInfoToClients(LPVOID arg) {
    int retval;

    SOCKET client_sock = (SOCKET)arg;// socks[0]이던 socks[1]이든 받아서 사용

    int clientnum = clientuser;

    DisplayText((char*)"SEND쓰레드시작 번호 =%d \n", clientnum);

    if (clientnum == 1)
    {
        Event[0] = CreateEvent(0, FALSE, FALSE, 0);
    }

    if (clientnum == 2)
    {
        Event[1] = CreateEvent(0, FALSE, FALSE, 0);
    }

    while (1) {

       
        if (sendtime >= TIMERSEC)
        {

           
            if (obstaclesend == 1 && gamestate==2)
            {   //장애물 생성하라고하면 send에 장애물보내줌


                if (clientnum == 1)
                {

                    char* message = (char*)"O";         //"123U"

                    Obstacle* obtemp = new Obstacle;
                    obtemp->obstacletype = Obstacles[obstaclenum-1]->obstacletype;
                    obtemp->position = Obstacles[obstaclenum - 1]->position;
                    retval = send(client_socks[0], message, strlen(message), 0);
                    retval = send(client_socks[0], (char*)obtemp, sizeof(*obtemp), 0);

                    delete obtemp;

             
                }


                if (clientnum == 2)
                {

                    char* message = (char*)"O";         //"123U"

                    Obstacle* obtemp = new Obstacle;
                    obtemp->obstacletype = Obstacles[obstaclenum - 1]->obstacletype;
                    obtemp->position = Obstacles[obstaclenum - 1]->position;
                    retval = send(client_socks[1], message, strlen(message), 0);
                    retval = send(client_socks[1], (char*)obtemp, sizeof(*obtemp), 0);

                    delete obtemp;

                  

                }
                obstaclesend =0;     //둘다보내는데왜;;

            }

            if (itemsend == 1 && gamestate == 2)

            {

                if (clientnum == 1)
                {

                    char* message = (char*)"I";         //"123U"

                    Item* itemp = new Item;
                    itemp->itemtype=Items[itemnum-1]->itemtype; //0파워업 1스피드업
                    itemp->itempower = Items[itemnum - 1]->itempower;
                    itemp->size = Items[itemnum - 1]->size;
                    itemp->position = Items[itemnum - 1]->position;
                   
                   
                    
                    // itemp->obstacletype = Obstacles[obstaclenum - 1]->obstacletype;
                    //ibtemp->position = Obstacles[obstaclenum - 1]->position;
                    retval = send(client_socks[0], message, strlen(message), 0);
                    retval = send(client_socks[0], (char*)itemp, sizeof(*itemp), 0);

                    delete itemp;


                }


                if (clientnum == 2)
                {

                    char* message = (char*)"I";         //"123U"

                    Item* itemp = new Item;
                    itemp->itemtype = Items[itemnum - 1]->itemtype; //0파워업 1스피드업
                    itemp->itempower = Items[itemnum - 1]->itempower;
                    itemp->size = Items[itemnum - 1]->size;
                    itemp->position = Items[itemnum - 1]->position;



                    // itemp->obstacletype = Obstacles[obstaclenum - 1]->obstacletype;
                    //ibtemp->position = Obstacles[obstaclenum - 1]->position;
                    retval = send(client_socks[1], message, strlen(message), 0);
                    retval = send(client_socks[1], (char*)itemp, sizeof(*itemp), 0);

                    delete itemp;





                }

                itemsend = 0;

            }

            if (bulletsend == 1 && gamestate == 2)

            {

               
                if (clientnum == 1)
                {

                       

                    Bullet* btemp = new Bullet;
                    btemp->direction = Bullets[bulletnum-1]->direction;
                    btemp->power = Bullets[bulletnum-1]->power;
                    btemp->position = Bullets[bulletnum - 1]->position;

                 


                    // itemp->obstacletype = Obstacles[obstaclenum - 1]->obstacletype;
                    //ibtemp->position = Obstacles[obstaclenum - 1]->position;
                    char* message = (char*)"B";
                    retval = send(client_socks[0], message, strlen(message), 0);
                    retval = send(client_socks[0], (char*)btemp, sizeof(*btemp), 0);

                    delete btemp;

                  
                }

                if (clientnum == 2)
                {

                    char* message = (char*)"B";         //"123U"

                    Bullet* btemp = new Bullet;
                    btemp->direction = Bullets[bulletnum - 1]->direction;
                    btemp->power = Bullets[bulletnum - 1]->power;
                    btemp->position = Bullets[bulletnum - 1]->position;



                     // itemp->obstacletype = Obstacles[obstaclenum - 1]->obstacletype;
                     //ibtemp->position = Obstacles[obstaclenum - 1]->position;
                    retval = send(client_socks[1], message, strlen(message), 0);
                    retval = send(client_socks[1], (char*)btemp, sizeof(*btemp), 0);

                    delete btemp;


                }
              

                bulletsend = 0;

            }

            //레디누르는데왜 80이받아지냐고 개씨발
            //4초마다 sendmessage
            
            if (clientnum == 1)
            {

            char* message;
            message = (char*)"123456789";
            retval = send(client_socks[0], message,strlen(message), 0);



            message = (char*)"12345678";
            retval = send(client_socks[0], message, strlen(message), 0);



            message = (char*)"123U";         //"123U"

            retval = send(client_socks[0], message, strlen(message), 0);

           
                   //주기적으로 보내는곳
            
                retval = send(client_socks[0], (char*)P1, sizeof(*P1), 0);
                retval = send(client_socks[0], (char*)P2, sizeof(*P2), 0);
                retval = send(client_socks[0], (char*)P1_maincamera, sizeof(*P1_maincamera), 0);
                  //클라는 내위치만보냄
            }

            if (clientnum == 2)
            {
                char* message;
                message = (char*)"123456789";
                retval = send(client_socks[1], message, strlen(message), 0);



                message = (char*)"12345678";
                retval = send(client_socks[1], message, strlen(message), 0);



                message = (char*)"123U";         //"123U"

                retval = send(client_socks[1], message, strlen(message), 0);

                //값들갱신용 잘된다 ㅇㅋ
                //ME->

                retval = send(client_socks[1], (char*)P2, sizeof(*P2), 0);
                retval = send(client_socks[1], (char*)P1, sizeof(*P1), 0);
                retval = send(client_socks[1], (char*)P2_maincamera, sizeof(*P2_maincamera), 0);

            }

            //u후에 구조체보내주자.
      
           // DisplayText((char*)"보낸값 =%s \n", message);

            sendtime = 0;
            
        }

        //if ()둘다한테 보내줘야한다.

        if (P1_ready == 1 && P2_ready == 1)
        {
          
            //P1_READY P2_READY일때 두번보내주고 빠져나간다
            //시작할때 캐릭터위치랑 장애물도보내줘야함 즉 R을못받음왜?
            buf =(char*)"R";
          
            if (clientnum == 1)
            {
                retval = send(client_socks[0], buf, 1, 0);
                retval = send(client_socks[0], (char*)P1, sizeof(*P1), 0);
               
            }



            if (clientnum == 2)
            {
                char*buf2 = (char*)"R";
                retval = send(client_socks[1], buf2, 1, 0);
                retval = send(client_socks[1], (char*)P2, sizeof(*P2), 0);
                
            }


            Sleep(1);
            obstaclesend = 1;
            P1_ready = 0;
            P2_ready = 0;
            gamestate = 2;
        }
       

        if (clientnum == 1)
        {
            WaitForSingleObject(Event[0], 1);
        }
        if (clientnum == 2)
        {
            WaitForSingleObject(Event[1], 1);
        }
    }


}





// 이름 바꿔야함 recv전용쓰레드.
DWORD WINAPI ProcessClient(LPVOID arg)
{

    //w누를때마다 
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;



    char buf[BUFSIZE + 1]; //recv할떈 배열크기가 정해져있어야한다 왜?
    

    int strlen;

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    int count = 0;
    int thisclientnum = 0;


    //데이터recv전용이다.
    while (1) {
        // 데이터 받기

    
        retval = recv(client_sock, buf, 1, 0);    //길이가 반환됨.
       // retval = recvn(client_sock, buf, sizeof(buf), 0);
        if (retval == SOCKET_ERROR) {
            err_display((char*)"recv()");
            break;
        }
        else if (retval == 0)
            break;

        buf[retval] = '\0'; //그다음값 '\0'으로 끝을보여줌 ㅇㅋ
       
        //받은 buf형 문자열출력
       //출력부분  DisplayText((char*)"[TCP/%s:%d] %s 받은크기 %d 받은값변수 %d\r\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf,retval,retval);


        //받은값처리
        if (buf[retval - 1] == 'C')
        {


            DisplayText((char*)"클라이언트 연결완료");

            thisclientnum = clientuser;

            DisplayText((char*)"매칭된 유저수 %d", clientuser);

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

                P1_Start = 1;
                P2_Start = 1;



            }


           




        }


        if (buf[0] == 'U') //12345678 에서 1인경우 
        {

            if (thisclientnum == 1)
            {
                //보내준게 U인경우 절차에맞게 보내줌

                            //8초마다 클라에서 서버한테 u를보내줌

                Player* ptemp = new Player;
                retval = recv(client_socks[0], (char*)ptemp, sizeof(*ptemp), 0);
                //값만넘겨줌
                       // ENEMY=PTEMP 이거꼬이나?
                P1->hp = ptemp->hp;      //데이터만고쳐주자

                P1->position = ptemp->position;
                P1->direction = ptemp->direction;
             
           //출력부분     DisplayText((char*)"ME위치 =%d \n", P1->position.left);
                //받아진값 표시
                delete ptemp;



            }


            if (thisclientnum == 2)
            {
                //보내준게 U인경우 절차에맞게 보내줌

                            //8초마다 클라에서 서버한테 u를보내줌

                Player* ptemp = new Player;
                retval = recv(client_socks[1], (char*)ptemp, sizeof(*ptemp), 0);
                //값만넘겨줌
                       // ENEMY=PTEMP 이거꼬이나?
                P2->hp = ptemp->hp;      //데이터만고쳐주자

                P2->position = ptemp->position;
                P2->direction = ptemp->direction;
             //   DisplayText((char*)"ME위치 =%d \n", P2->position.left);
                //받아진값 표시
                delete ptemp;



            }


        }

        if (buf[0] == 'B')    //조작시 행위 총알생성
        {
            if (thisclientnum == 1)
            {
                Bullet* bullet = new Bullet;
                bullet->direction = P1->direction;
                bullet->power = P1->power;
                if (P1->direction == 0)
                {

                    bullet->position.top = P1->position.top + 30;
                    bullet->position.left = P1->position.left + 100;
                    bullet->position.right = P1->position.left + 150;
                    bullet->position.bottom = P1->position.bottom - 30;
                }

                if (P1->direction == 1)
                {
                    bullet->position.top = P1->position.top + 30;
                    bullet->position.left = P1->position.right - 150;
                    bullet->position.right = P1->position.right - 100;
                    bullet->position.bottom = P1->position.bottom - 30;



                }
                Bullets[bulletnum] = bullet;
                bulletnum++;

                bulletsend = 1;

                   //총알생성후 생성을 클라한테보냄
            }

            if (thisclientnum == 2)
            {
                Bullet* bullet = new Bullet;
                bullet->direction = P2->direction;
                bullet->power = P2->power;
                if (P2->direction == 0)
                {

                    bullet->position.top = P2->position.top + 30;
                    bullet->position.left = P2->position.left + 100;
                    bullet->position.right = P2->position.left + 150;
                    bullet->position.bottom = P2->position.bottom - 30;
                }

                if (P2->direction == 1)
                {
                    bullet->position.top = P2->position.top + 30;
                    bullet->position.left = P2->position.right - 150;
                    bullet->position.right = P2->position.right - 100;
                    bullet->position.bottom = P2->position.bottom - 30;



                }
                Bullets[bulletnum] = bullet;
                bulletnum++;

                bulletsend = 1;

                //총알생성후 생성을 클라한테보냄
            }

        }

        if (buf[0] == 'w')    //조작시 행위
        {

            if (thisclientnum == 1)
            {
                if (P1->position.top > MapMinSize.y)     //캐릭최소위치
                {
                    P1->position.top -= P1->speed;
                    P1->position.bottom -= P1->speed;

                    if (P1->position.top < MapMinSize.y)
                    {
                        P1->position.top = MapMinSize.y;
                        P1->position.bottom = P1->position.top + P1->size.y;
                    }

                    if (P1_maincamera->pos.y > MapMinSize.y)     //카메라 최소위치
                    {

                        if (P1->position.top < (MapMaxSize.y - (WindowFullHeight / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                        {
                            P1_maincamera->pos.y -= P1->speed;
                        }

                        if (P1_maincamera->pos.y < MapMinSize.y)
                        {
                            P1_maincamera->pos.y = MapMinSize.y;       //-되는경우 0으로보정
                        }
                    }



                }
               
            }


            if (thisclientnum == 2)
            {
                if (P2->position.top > MapMinSize.y)     //캐릭최소위치
                {
                   
                    P2->position.top -= P2->speed;
                    P2->position.bottom -= P2->speed;

                    if (P2->position.top < MapMinSize.y)
                    {
                        P2->position.top = MapMinSize.y;
                        P2->position.bottom = P2->position.top + P2->size.y;
                    }

                    if (P2_maincamera->pos.y > MapMinSize.y)     //카메라 최소위치
                    {

                        if (P2->position.top < (MapMaxSize.y - (WindowFullHeight / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                        {
                            P2_maincamera->pos.y -= P2->speed;
                        }

                        if (P2_maincamera->pos.y < MapMinSize.y)
                        {
                            P2_maincamera->pos.y = MapMinSize.y;       //-되는경우 0으로보정
                        }
                    }


                }
               
            }

          



        }

        if (buf[0] == 's')    //조작시 행위
        {
            //DisplayText((char*)"받은값 =%s \n", buf);
            if (thisclientnum == 1)
            {
                if ((P1->position.bottom) < MapMaxSize.y)     //캐릭최소위치
                {
                    P1->position.top += P1->speed;
                    P1->position.bottom += P1->speed;

                    if ((P1->position.bottom) > MapMaxSize.y)
                    {
                        P1->position.bottom = MapMaxSize.y;
                        P1->position.top = P1->position.bottom - P1->size.y;
                    }

                    
                    if (P1_maincamera->pos.y < (MapMaxSize.y - (WindowFullHeight)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
                    {

                        if (P1->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
                        {
                            P1_maincamera->pos.y += P1->speed;
                        }
                        if (P1_maincamera->pos.y > (MapMaxSize.y - (WindowFullHeight + 9)))
                        {
                            P1_maincamera->pos.y = (MapMaxSize.y - (WindowFullHeight - 9));       //-되는경우 0으로보정
                        }
                    }

                }
                
            
            }



            if (thisclientnum == 2)
            {
                if ((P2->position.bottom) < MapMaxSize.y)     //캐릭최소위치
                {
                    P2->position.top += P2->speed;
                    P2->position.bottom += P2->speed;

                    if ((P2->position.bottom) > MapMaxSize.y)
                    {
                        P2->position.bottom = MapMaxSize.y;
                        P2->position.top = P2->position.bottom - P2->size.y;
                    }
                    
                    if (P2_maincamera->pos.y < (MapMaxSize.y - (WindowFullHeight)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
                    {

                        if (P2->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
                        {
                            P2_maincamera->pos.y += P2->speed;
                        }
                        if (P2_maincamera->pos.y > (MapMaxSize.y - (WindowFullHeight + 9)))
                        {
                            P2_maincamera->pos.y = (MapMaxSize.y - (WindowFullHeight - 9));       //-되는경우 0으로보정
                        }
                    }



                }

                

            }

        }


        if (buf[0] == 'M')    //조작시 행위     LEFT
        {
            if (thisclientnum == 1)
            {
                P1->direction = 1;
                //top이아니라 left
                if (P1->position.left > MapMinSize.x)     //캐릭최소위치
                {
                    P1->position.left -= P1->speed;
                    P1->position.right -= P1->speed;

                    if (P1->position.left < MapMinSize.x)
                    {
                        P1->position.left = MapMinSize.x;
                        P1->position.right = P1->position.left + P1->size.x;
                    }


                    if (P1_maincamera->pos.x > MapMinSize.x)     //카메라 최소위치
                    {

                        if (P1->position.right < (MapMaxSize.x - (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                        {
                            P1_maincamera->pos.x -= P1->speed;
                        }

                        if (P1_maincamera->pos.x < MapMinSize.x)
                        {
                            P1_maincamera->pos.x = MapMinSize.x;       //-되는경우 0으로보정
                        }
                    }


                }

            }


            if (thisclientnum == 2)
            {
                P2->direction = 1;
                //top이아니라 left
                if (P2->position.left > MapMinSize.x)     //캐릭최소위치
                {
                    P2->position.left -= P2->speed;
                    P2->position.right -= P2->speed;

                    if (P2->position.left < MapMinSize.x)
                    {
                        P2->position.left = MapMinSize.x;
                        P2->position.right = P2->position.left + P2->size.x;
                    }

                    if (P2_maincamera->pos.x > MapMinSize.x)     //카메라 최소위치
                    {

                        if (P2->position.right < (MapMaxSize.x - (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                        {
                            P2_maincamera->pos.x -= P2->speed;
                        }

                        if (P2_maincamera->pos.x < MapMinSize.x)
                        {
                            P2_maincamera->pos.x = MapMinSize.x;       //-되는경우 0으로보정
                        }
                    }

                }

            }
        }

        if (buf[0] == 'N')    //조작시 행위 RIGHT
            {
            if (thisclientnum == 1)
            {
                P1->direction = 0;
                if ((P1->position.right) < MapMaxSize.x)     //일반적상황 오른키시 오른쪽으로이동
                {
                    P1->position.left += P1->speed;
                    P1->position.right += P1->speed;

                    if ((P1->position.right) > MapMaxSize.x)  //맵을 벗어나려고하면 보정
                    {
                        P1->position.right = MapMaxSize.x;
                        P1->position.left = P1->position.right - P1->size.x;
                    }



                    if (P1_maincamera->pos.x < (MapMaxSize.x - (WindowFullWidth)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
                    {

                        if (P1->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                        {
                            P1_maincamera->pos.x += P1->speed;
                        }
                        if (P1_maincamera->pos.x > (MapMaxSize.x - (WindowFullWidth + 9)))
                        {
                            P1_maincamera->pos.x = (MapMaxSize.x - (WindowFullWidth - 9));       //-되는경우 0으로보정
                        }
                    }
                }
            }

            if (thisclientnum == 2)
            {
                P2->direction = 0;
                if ((P2->position.right) < MapMaxSize.x)     //일반적상황 오른키시 오른쪽으로이동
                {
                    P2->position.left += P2->speed;
                    P2->position.right += P2->speed;

                    if ((P2->position.right) > MapMaxSize.x)  //맵을 벗어나려고하면 보정
                    {
                        P2->position.right = MapMaxSize.x;
                        P2->position.left = P2->position.right - P2->size.x;
                    }


                    if (P2_maincamera->pos.x < (MapMaxSize.x - (WindowFullWidth)))     //카메라 최소위치             //8은 보정값 예를들어 1300이라치면살짝모자름
                    {

                        if (P2->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//내캐릭터가 맵반을넘어가야 카메라도 같이움직여줌
                        {
                            P2_maincamera->pos.x += P2->speed;
                        }
                        if (P2_maincamera->pos.x > (MapMaxSize.x - (WindowFullWidth + 9)))
                        {
                            P2_maincamera->pos.x = (MapMaxSize.x - (WindowFullWidth - 9));       //-되는경우 0으로보정
                        }
                    }
                }
            }

            }
        if (thisclientnum == 1)
        {
            SetEvent(Event[0]);
        }
        if (thisclientnum == 2)
        {
            SetEvent(Event[1]);
        }
        //    receivetime = 0;
       // }

    
}


    // closesocket()
    closesocket(client_sock);
    DisplayText((char*)"[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\r\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}









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

        P1->position.top = 100;
        P1->position.left = 100;
        P1->position.right = 200;
        P1->position.bottom = 200;
        P1->size.x = 100;
        P1->size.y = 100;
        //
        P2->position.top = 500;
        P2->position.left = 800;
        P2->position.right = 900;
        P2->position.bottom = 600;



        //메인카메라 플레이어별로나누자
        P1_maincamera->pos.x = 0;
        P1_maincamera->pos.y = 0;
        P2_maincamera->pos.x = 0;
        P2_maincamera->pos.y = 0;
        MapMinSize.x = 10;
        MapMinSize.y = 0;
        MapMaxSize.x = 1300;
        MapMaxSize.y = 1300;

        SetTimer(hWnd, 1, TIMERSEC, NULL); //타이머실행
        break;
    case WM_SIZE:
        MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
    case WM_SETFOCUS:
        SetFocus(hEdit);
        break;
    case WM_TIMER:
        nowitemgauge += TIMERSEC;
        sendtime += TIMERSEC;   //
        receivetime += TIMERSEC;

        if (gamestate == 2)
        {
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
                    item->position.top = rand() % 800 + 400;
                    item->position.left = rand() % 800 + 400;
                    item->position.right = item->position.left + item->size.x;
                    item->position.bottom = item->position.top + item->size.y;
                    Items[itemnum] = item;
                    itemnum++;

                    itemsend = 1;
                }


            }

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
             //   obstaclesend = 1;

            }

        }
        
        

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
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
DWORD WINAPI ServerMain(LPVOID arg)              //servermain 여기서 send,recv를 생성해줌.
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
  //  SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;
    HANDLE sendhThread;
    while (1) {

        //인원받아서 사용 ㅇㅋ //accept한후에 유저수+1씩해준다

        // accept()
        addrlen = sizeof(clientaddr);
        client_socks[clientuser] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_socks[clientuser] == INVALID_SOCKET) {
            err_display((char*)"accept()");
            break;
        }




        // 접속한 클라이언트 정보 출력
        DisplayText((char*)"\r\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\r\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성 processclient    recv랑 send생성해주자
        hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_socks[clientuser], 0, NULL); //recv쪽 이제 send1쪽생성 ㅇㅋ
        sendhThread = CreateThread(NULL, 0, SendInfoToClients, (LPVOID)client_socks[clientuser], 0, NULL); //recv쪽 이제 send1쪽생성 ㅇㅋ

        //받으면 유저수늘려주자.

        //여기 세줄떄문에 고장인가?
        clientuser += 1;


        if (hThread == NULL) { closesocket(client_socks[clientuser]); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}
