
#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include<time.h>//�����Լ���
#define SERVERPORT 9000
#define BUFSIZE    512
#define TIMERSEC    80

#define MistakeWidth   9
#define MistakeHeight 32
#define WindowFullWidth 1000
#define WindowFullHeight 800


// ������ ���ν���
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char* fmt, ...);
// ���� ��� �Լ�
void err_quit(char* msg);
void err_display(char* msg);
// ���� ��� ������ �Լ�
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);              //buf���������

HINSTANCE hInst; // �ν��Ͻ� �ڵ�
HWND hEdit; // ���� ��Ʈ��
CRITICAL_SECTION cs; // �Ӱ� ����


//Ŭ���̾�Ʈ�� �ڷ�����

//���ӳ� ����ü����


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
        direction = 0;           //0������ 1����
        hp = 100;
        speed = 10;
        power = 10;
        powertype = 1;
        bulletcreatetime = TIMERSEC;

    }

};




//������ſ� ���̴� ������
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

//�������� ������ ��ü���� Ŭ���̾�Ʈ���� ����������
int obstaclesend = 0;
int itemsend = 0;
int bulletsend = 0;

int gamestate = 0;

//���ӳ� ������������

HDC hdc, memdc;
PAINTSTRUCT ps;





Bullet* Bullets[100];    //�Ѿ˵��� ����
Obstacle* Obstacles[100]; //��ֹ����� ����
Item* Items[100]; //������ ���� ����



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



//���ӳ� ��������



//send���� ������
DWORD WINAPI SendInfoToClients(LPVOID arg) {
    int retval;

    SOCKET client_sock = (SOCKET)arg;// socks[0]�̴� socks[1]�̵� �޾Ƽ� ���

    int clientnum = clientuser;

    DisplayText((char*)"SEND��������� ��ȣ =%d \n", clientnum);

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
            {   //��ֹ� �����϶���ϸ� send�� ��ֹ�������


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
                obstaclesend =0;     //�Ѵٺ����µ���;;

            }

            if (itemsend == 1 && gamestate == 2)

            {

                if (clientnum == 1)
                {

                    char* message = (char*)"I";         //"123U"

                    Item* itemp = new Item;
                    itemp->itemtype=Items[itemnum-1]->itemtype; //0�Ŀ��� 1���ǵ��
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
                    itemp->itemtype = Items[itemnum - 1]->itemtype; //0�Ŀ��� 1���ǵ��
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

            //���𴩸��µ��� 80�̹޾����İ� ������
            //4�ʸ��� sendmessage
            
            if (clientnum == 1)
            {

            char* message;
            message = (char*)"123456789";
            retval = send(client_socks[0], message,strlen(message), 0);



            message = (char*)"12345678";
            retval = send(client_socks[0], message, strlen(message), 0);



            message = (char*)"123U";         //"123U"

            retval = send(client_socks[0], message, strlen(message), 0);

           
                   //�ֱ������� �����°�
            
                retval = send(client_socks[0], (char*)P1, sizeof(*P1), 0);
                retval = send(client_socks[0], (char*)P2, sizeof(*P2), 0);
                retval = send(client_socks[0], (char*)P1_maincamera, sizeof(*P1_maincamera), 0);
                  //Ŭ��� ����ġ������
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

                //���鰻�ſ� �ߵȴ� ����
                //ME->

                retval = send(client_socks[1], (char*)P2, sizeof(*P2), 0);
                retval = send(client_socks[1], (char*)P1, sizeof(*P1), 0);
                retval = send(client_socks[1], (char*)P2_maincamera, sizeof(*P2_maincamera), 0);

            }

            //u�Ŀ� ����ü��������.
      
           // DisplayText((char*)"������ =%s \n", message);

            sendtime = 0;
            
        }

        //if ()�Ѵ����� ��������Ѵ�.

        if (P1_ready == 1 && P2_ready == 1)
        {
          
            //P1_READY P2_READY�϶� �ι������ְ� ����������
            //�����Ҷ� ĳ������ġ�� ��ֹ������������ �� R����������?
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





// �̸� �ٲ���� recv���뾲����.
DWORD WINAPI ProcessClient(LPVOID arg)
{

    //w���������� 
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;



    char buf[BUFSIZE + 1]; //recv�ҋ� �迭ũ�Ⱑ �������־���Ѵ� ��?
    

    int strlen;

    // Ŭ���̾�Ʈ ���� ���
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    int count = 0;
    int thisclientnum = 0;


    //������recv�����̴�.
    while (1) {
        // ������ �ޱ�

    
        retval = recv(client_sock, buf, 1, 0);    //���̰� ��ȯ��.
       // retval = recvn(client_sock, buf, sizeof(buf), 0);
        if (retval == SOCKET_ERROR) {
            err_display((char*)"recv()");
            break;
        }
        else if (retval == 0)
            break;

        buf[retval] = '\0'; //�״����� '\0'���� ���������� ����
       
        //���� buf�� ���ڿ����
       //��ºκ�  DisplayText((char*)"[TCP/%s:%d] %s ����ũ�� %d ���������� %d\r\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf,retval,retval);


        //������ó��
        if (buf[retval - 1] == 'C')
        {


            DisplayText((char*)"Ŭ���̾�Ʈ ����Ϸ�");

            thisclientnum = clientuser;

            DisplayText((char*)"��Ī�� ������ %d", clientuser);

        }

        if (buf[retval - 1] == 'R')
        {


            DisplayText((char*)"Ŭ���̾�Ʈ ����Ϸ�");
            if (thisclientnum == 1)
            {
                P1_ready = 1;



                //1�Ƿ����Ѱ�� ���ߴ� ġ�� �ϴ� �Ѿ�Բ�

            }
            if (thisclientnum == 2)
            {
                P2_ready = 1;

            }

            DisplayText((char*)"������ ������ȣ %d", thisclientnum);


            if (P1_ready == 1 && P2_ready == 1)
            {
                // DisplayText((char*)"��ΰ� �����Ͽ� ���ӽ��� %d", thisclientnum);

                P1_Start = 1;
                P2_Start = 1;



            }


           




        }


        if (buf[0] == 'U') //12345678 ���� 1�ΰ�� 
        {

            if (thisclientnum == 1)
            {
                //�����ذ� U�ΰ�� �������°� ������

                            //8�ʸ��� Ŭ�󿡼� �������� u��������

                Player* ptemp = new Player;
                retval = recv(client_socks[0], (char*)ptemp, sizeof(*ptemp), 0);
                //�����Ѱ���
                       // ENEMY=PTEMP �̰Ų��̳�?
                P1->hp = ptemp->hp;      //�����͸���������

                P1->position = ptemp->position;
                P1->direction = ptemp->direction;
             
           //��ºκ�     DisplayText((char*)"ME��ġ =%d \n", P1->position.left);
                //�޾����� ǥ��
                delete ptemp;



            }


            if (thisclientnum == 2)
            {
                //�����ذ� U�ΰ�� �������°� ������

                            //8�ʸ��� Ŭ�󿡼� �������� u��������

                Player* ptemp = new Player;
                retval = recv(client_socks[1], (char*)ptemp, sizeof(*ptemp), 0);
                //�����Ѱ���
                       // ENEMY=PTEMP �̰Ų��̳�?
                P2->hp = ptemp->hp;      //�����͸���������

                P2->position = ptemp->position;
                P2->direction = ptemp->direction;
             //   DisplayText((char*)"ME��ġ =%d \n", P2->position.left);
                //�޾����� ǥ��
                delete ptemp;



            }


        }

        if (buf[0] == 'B')    //���۽� ���� �Ѿ˻���
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

                   //�Ѿ˻����� ������ Ŭ�����׺���
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

                //�Ѿ˻����� ������ Ŭ�����׺���
            }

        }

        if (buf[0] == 'w')    //���۽� ����
        {

            if (thisclientnum == 1)
            {
                if (P1->position.top > MapMinSize.y)     //ĳ���ּ���ġ
                {
                    P1->position.top -= P1->speed;
                    P1->position.bottom -= P1->speed;

                    if (P1->position.top < MapMinSize.y)
                    {
                        P1->position.top = MapMinSize.y;
                        P1->position.bottom = P1->position.top + P1->size.y;
                    }

                    if (P1_maincamera->pos.y > MapMinSize.y)     //ī�޶� �ּ���ġ
                    {

                        if (P1->position.top < (MapMaxSize.y - (WindowFullHeight / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                        {
                            P1_maincamera->pos.y -= P1->speed;
                        }

                        if (P1_maincamera->pos.y < MapMinSize.y)
                        {
                            P1_maincamera->pos.y = MapMinSize.y;       //-�Ǵ°�� 0���κ���
                        }
                    }



                }
               
            }


            if (thisclientnum == 2)
            {
                if (P2->position.top > MapMinSize.y)     //ĳ���ּ���ġ
                {
                   
                    P2->position.top -= P2->speed;
                    P2->position.bottom -= P2->speed;

                    if (P2->position.top < MapMinSize.y)
                    {
                        P2->position.top = MapMinSize.y;
                        P2->position.bottom = P2->position.top + P2->size.y;
                    }

                    if (P2_maincamera->pos.y > MapMinSize.y)     //ī�޶� �ּ���ġ
                    {

                        if (P2->position.top < (MapMaxSize.y - (WindowFullHeight / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                        {
                            P2_maincamera->pos.y -= P2->speed;
                        }

                        if (P2_maincamera->pos.y < MapMinSize.y)
                        {
                            P2_maincamera->pos.y = MapMinSize.y;       //-�Ǵ°�� 0���κ���
                        }
                    }


                }
               
            }

          



        }

        if (buf[0] == 's')    //���۽� ����
        {
            //DisplayText((char*)"������ =%s \n", buf);
            if (thisclientnum == 1)
            {
                if ((P1->position.bottom) < MapMaxSize.y)     //ĳ���ּ���ġ
                {
                    P1->position.top += P1->speed;
                    P1->position.bottom += P1->speed;

                    if ((P1->position.bottom) > MapMaxSize.y)
                    {
                        P1->position.bottom = MapMaxSize.y;
                        P1->position.top = P1->position.bottom - P1->size.y;
                    }

                    
                    if (P1_maincamera->pos.y < (MapMaxSize.y - (WindowFullHeight)))     //ī�޶� �ּ���ġ             //8�� ������ ������� 1300�̶�ġ���¦���ڸ�
                    {

                        if (P1->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
                        {
                            P1_maincamera->pos.y += P1->speed;
                        }
                        if (P1_maincamera->pos.y > (MapMaxSize.y - (WindowFullHeight + 9)))
                        {
                            P1_maincamera->pos.y = (MapMaxSize.y - (WindowFullHeight - 9));       //-�Ǵ°�� 0���κ���
                        }
                    }

                }
                
            
            }



            if (thisclientnum == 2)
            {
                if ((P2->position.bottom) < MapMaxSize.y)     //ĳ���ּ���ġ
                {
                    P2->position.top += P2->speed;
                    P2->position.bottom += P2->speed;

                    if ((P2->position.bottom) > MapMaxSize.y)
                    {
                        P2->position.bottom = MapMaxSize.y;
                        P2->position.top = P2->position.bottom - P2->size.y;
                    }
                    
                    if (P2_maincamera->pos.y < (MapMaxSize.y - (WindowFullHeight)))     //ī�޶� �ּ���ġ             //8�� ������ ������� 1300�̶�ġ���¦���ڸ�
                    {

                        if (P2->position.top > (MapMinSize.y + (WindowFullHeight / 2)))
                        {
                            P2_maincamera->pos.y += P2->speed;
                        }
                        if (P2_maincamera->pos.y > (MapMaxSize.y - (WindowFullHeight + 9)))
                        {
                            P2_maincamera->pos.y = (MapMaxSize.y - (WindowFullHeight - 9));       //-�Ǵ°�� 0���κ���
                        }
                    }



                }

                

            }

        }


        if (buf[0] == 'M')    //���۽� ����     LEFT
        {
            if (thisclientnum == 1)
            {
                P1->direction = 1;
                //top�̾ƴ϶� left
                if (P1->position.left > MapMinSize.x)     //ĳ���ּ���ġ
                {
                    P1->position.left -= P1->speed;
                    P1->position.right -= P1->speed;

                    if (P1->position.left < MapMinSize.x)
                    {
                        P1->position.left = MapMinSize.x;
                        P1->position.right = P1->position.left + P1->size.x;
                    }


                    if (P1_maincamera->pos.x > MapMinSize.x)     //ī�޶� �ּ���ġ
                    {

                        if (P1->position.right < (MapMaxSize.x - (WindowFullWidth / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                        {
                            P1_maincamera->pos.x -= P1->speed;
                        }

                        if (P1_maincamera->pos.x < MapMinSize.x)
                        {
                            P1_maincamera->pos.x = MapMinSize.x;       //-�Ǵ°�� 0���κ���
                        }
                    }


                }

            }


            if (thisclientnum == 2)
            {
                P2->direction = 1;
                //top�̾ƴ϶� left
                if (P2->position.left > MapMinSize.x)     //ĳ���ּ���ġ
                {
                    P2->position.left -= P2->speed;
                    P2->position.right -= P2->speed;

                    if (P2->position.left < MapMinSize.x)
                    {
                        P2->position.left = MapMinSize.x;
                        P2->position.right = P2->position.left + P2->size.x;
                    }

                    if (P2_maincamera->pos.x > MapMinSize.x)     //ī�޶� �ּ���ġ
                    {

                        if (P2->position.right < (MapMaxSize.x - (WindowFullWidth / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                        {
                            P2_maincamera->pos.x -= P2->speed;
                        }

                        if (P2_maincamera->pos.x < MapMinSize.x)
                        {
                            P2_maincamera->pos.x = MapMinSize.x;       //-�Ǵ°�� 0���κ���
                        }
                    }

                }

            }
        }

        if (buf[0] == 'N')    //���۽� ���� RIGHT
            {
            if (thisclientnum == 1)
            {
                P1->direction = 0;
                if ((P1->position.right) < MapMaxSize.x)     //�Ϲ�����Ȳ ����Ű�� �����������̵�
                {
                    P1->position.left += P1->speed;
                    P1->position.right += P1->speed;

                    if ((P1->position.right) > MapMaxSize.x)  //���� ��������ϸ� ����
                    {
                        P1->position.right = MapMaxSize.x;
                        P1->position.left = P1->position.right - P1->size.x;
                    }



                    if (P1_maincamera->pos.x < (MapMaxSize.x - (WindowFullWidth)))     //ī�޶� �ּ���ġ             //8�� ������ ������� 1300�̶�ġ���¦���ڸ�
                    {

                        if (P1->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                        {
                            P1_maincamera->pos.x += P1->speed;
                        }
                        if (P1_maincamera->pos.x > (MapMaxSize.x - (WindowFullWidth + 9)))
                        {
                            P1_maincamera->pos.x = (MapMaxSize.x - (WindowFullWidth - 9));       //-�Ǵ°�� 0���κ���
                        }
                    }
                }
            }

            if (thisclientnum == 2)
            {
                P2->direction = 0;
                if ((P2->position.right) < MapMaxSize.x)     //�Ϲ�����Ȳ ����Ű�� �����������̵�
                {
                    P2->position.left += P2->speed;
                    P2->position.right += P2->speed;

                    if ((P2->position.right) > MapMaxSize.x)  //���� ��������ϸ� ����
                    {
                        P2->position.right = MapMaxSize.x;
                        P2->position.left = P2->position.right - P2->size.x;
                    }


                    if (P2_maincamera->pos.x < (MapMaxSize.x - (WindowFullWidth)))     //ī�޶� �ּ���ġ             //8�� ������ ������� 1300�̶�ġ���¦���ڸ�
                    {

                        if (P2->position.left > (MapMinSize.x + (WindowFullWidth / 2)))//��ĳ���Ͱ� �ʹ����Ѿ�� ī�޶� ���̿�������
                        {
                            P2_maincamera->pos.x += P2->speed;
                        }
                        if (P2_maincamera->pos.x > (MapMaxSize.x - (WindowFullWidth + 9)))
                        {
                            P2_maincamera->pos.x = (MapMaxSize.x - (WindowFullWidth - 9));       //-�Ǵ°�� 0���κ���
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
    DisplayText((char*)"[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}









int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;
    InitializeCriticalSection(&cs);

    // ������ Ŭ���� ���
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

    // ������ ����
    HWND hWnd = CreateWindow("MyWndClass", "TCP ����", WS_OVERLAPPEDWINDOW,
        0, 0, 600, 200, NULL, NULL, hInstance, NULL);
    if (hWnd == NULL) return 1;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // ���� ��� ������ ����
    CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

    // �޽��� ����
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DeleteCriticalSection(&cs);
    return msg.wParam;
}

// ������ ���ν���
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



        //����ī�޶� �÷��̾�γ�����
        P1_maincamera->pos.x = 0;
        P1_maincamera->pos.y = 0;
        P2_maincamera->pos.x = 0;
        P2_maincamera->pos.y = 0;
        MapMinSize.x = 10;
        MapMinSize.y = 0;
        MapMaxSize.x = 1300;
        MapMaxSize.y = 1300;

        SetTimer(hWnd, 1, TIMERSEC, NULL); //Ÿ�̸ӽ���
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

                if (itemnum < 8)      //�ϴ� 8����������
                {
                    Item* item = new Item;
                    item->itemtype = rand() % 2; //0�Ŀ��� 1���ǵ��
                    item->itempower = 5;
                    item->size.x = 50;
                    item->size.y = 50;
                    //��ġ����
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

// ���� ��Ʈ�� ��� �Լ�
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

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// TCP ���� ���� �κ�
DWORD WINAPI ServerMain(LPVOID arg)              //servermain ���⼭ send,recv�� ��������.
{
    int retval;

    // ���� �ʱ�ȭ
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

    // ������ ��ſ� ����� ����
  //  SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;
    HANDLE sendhThread;
    while (1) {

        //�ο��޾Ƽ� ��� ���� //accept���Ŀ� ������+1�����ش�

        // accept()
        addrlen = sizeof(clientaddr);
        client_socks[clientuser] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_socks[clientuser] == INVALID_SOCKET) {
            err_display((char*)"accept()");
            break;
        }




        // ������ Ŭ���̾�Ʈ ���� ���
        DisplayText((char*)"\r\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ������ ���� processclient    recv�� send����������
        hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_socks[clientuser], 0, NULL); //recv�� ���� send1�ʻ��� ����
        sendhThread = CreateThread(NULL, 0, SendInfoToClients, (LPVOID)client_socks[clientuser], 0, NULL); //recv�� ���� send1�ʻ��� ����

        //������ �������÷�����.

        //���� ���ً����� �����ΰ�?
        clientuser += 1;


        if (hThread == NULL) { closesocket(client_socks[clientuser]); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}
