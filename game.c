#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <fcntl.h>
#include "othello.h"
#define SERV_PORT 6666
static int width;
static int height;
static int cx = 3;
static int cy = 3;
int player;
int enemy;
int direction[8] = {0};
bool check(int y,int x);
void cal_snd(int y, int x);
bool other_no_move = 0;
void gameover();
bool checkall() {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            move(height/2,width/2);
            //printw("%d,%d",i,j);
            if(check(i,j))
            {
                memset(direction, 0, sizeof(direction));
                return 1;
            }
            
            memset(direction, 0, sizeof(direction));
        }
    }
//    return 0;

    return 0;
}
int
main(int argc, char* argv[])
{
/*  
    if(strcmp(argv[1],"-s")==0) 
    {
        printw("server\n");
        int sockfd,newsockfd,clilen, childpid;
        struct sockaddr_in cli_addr, serv_addr;
        struct hostent *he;
        int myserv_port=SERV_PORT;
        if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
            fprintf(stderr,"server:can't open stream socket");
        bzero((char*)&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        myserv_port = atoi(argv[2]);
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_addr.sin_port=htons(myserv_port);
        if(bind(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr))<0)
            fprintf(stderr,"server:can't bind local address");
        listen(sockfd,0);
    }
    else
        printw("client\n");
*/
    initscr();          // start curses mode 
    getmaxyx(stdscr, height, width);// get screen size

    cbreak();           // disable buffering
                    // - use raw() to disable Ctrl-Z and Ctrl-C as well,
    halfdelay(1);           // non-blocking getch after n * 1/10 seconds
    noecho();           // disable echo
    keypad(stdscr, TRUE);       // enable function keys and arrow keys
    curs_set(0);            // hide the cursor

    init_colors();

restart:
    clear();
    cx = cy = 3;
    init_board();
    draw_board();
    draw_cursor(cx, cy, 1);
    draw_score();
    refresh();

/* can print here
    move(height-2, 0);  
    if(strcmp(argv[1],"-s")==0) 
    {
        printw("server\n");
        int sockfd,newsockfd,clilen, childpid;
        struct sockaddr_in cli_addr, serv_addr;
        struct hostent *he;
        int myserv_port=SERV_PORT;
    }
    else
        printw("client\n");
*/
    int w_fd;//s:newsockfd, c:client_fd
    int turn; //your turn:1, otherwise:0
    //server socket's
    int sockfd,clilen, childpid;
    struct sockaddr_in cli_addr, serv_addr;
    struct hostent *he;
    int myserv_port;
    //
    //client socket's
    int    server_fd;
    struct sockaddr_in client_sin;
    int serv_port;
    struct hostent *serv_he;
    //
    move(height-5, 0);  
    if(strcmp(argv[1],"-s")==0) 
    {
        //printw("server\n");
        turn = 1;
        player = PLAYER1;
        enemy = PLAYER2;
        /*int sockfd,newsockfd,clilen, childpid;
        struct sockaddr_in cli_addr, serv_addr;
        struct hostent *he;
        int myserv_port;*/
        if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
            printw("server:can't open stream socket");
        bzero((char*)&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        myserv_port = atoi(argv[2]);
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_addr.sin_port=htons(myserv_port);
        if(bind(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr))<0)
            printw("server:can't bind local address");
        printw("before listen\n");
        listen(sockfd,0);
        printw("after listen\n");
        w_fd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
        printw("after accept newsock=%d\n",w_fd);
    }
    else
    {
        //printw("client\n");
        turn = 0;
        player = PLAYER2;
        enemy = PLAYER1;
        if((serv_he=gethostbyname(argv[2])) == NULL)
        {
            printw("Usage : client <server ip> <port> <testfile>");
            //exit(1);
        }
        serv_port = (u_short)atoi(argv[3]);
        w_fd = socket(AF_INET,SOCK_STREAM,0);
        bzero(&client_sin,sizeof(client_sin));
        client_sin.sin_family = AF_INET;
        client_sin.sin_addr = *((struct in_addr *)serv_he->h_addr); 
        client_sin.sin_port = htons(serv_port);
        if(connect(w_fd,(struct sockaddr *)&client_sin,sizeof(client_sin)) == -1)
        {
            printw("connect error");
        }
    }
    attron(A_BOLD);
    move(height-1, 0);  printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
    attroff(A_BOLD);


    while(true) {           // main loop
        if(turn == 1)
        {
            
            char msg[10];
            int ch = getch();
        
            int moved = 0;
            if(checkall()==0) {//
                turn = 0;
                sprintf(msg,"-1,-1,");
                write(w_fd,msg,sizeof(msg));
                if(other_no_move)
                {    gameover();
                    sleep(5);
                     break;
                }
                else
                {   
                    other_no_move = 0;
                    continue;
                }
            }
            move(height/2+2,width/2);
            //printw("after checkall");
            switch(ch) {
            case ' ':
            case 0x0d:
            case 0x0a:
            case KEY_ENTER:
                if(check(cy,cx)) {
                    board[cy][cx] = player;
                    cal_snd(cy,cx);
                    char tmp[5];
                    sprintf(msg,"%d,%d,",cx,cy);
                    //printw("msg:%s",msg);
                    write(w_fd,msg,sizeof(msg));
                    draw_cursor(cx, cy, 1);
                    draw_score();
                    refresh();
                    turn = 0;
                }
                break;
            case 'q':
            case 'Q':
                sprintf(msg,"-2,-2i,");
                write(w_fd,msg,sizeof(msg));
                goto quit;
                break;
            case 'r':
            case 'R':
                goto restart;
                break;
            case 'k':
            case KEY_UP:
                draw_cursor(cx, cy, 0);
                cy = (cy-1+BOARDSZ) % BOARDSZ;
                draw_cursor(cx, cy, 1);
                moved++;
                break;
            case 'j':
            case KEY_DOWN:
                draw_cursor(cx, cy, 0);
                cy = (cy+1) % BOARDSZ;
                draw_cursor(cx, cy, 1);
                moved++;
                break;
            case 'h':
            case KEY_LEFT:
                draw_cursor(cx, cy, 0);
                cx = (cx-1+BOARDSZ) % BOARDSZ;
                draw_cursor(cx, cy, 1);
                moved++;
                break;
            case 'l':
            case KEY_RIGHT:
                draw_cursor(cx, cy, 0);
                cx = (cx+1) % BOARDSZ;
                draw_cursor(cx, cy, 1);
                moved++;
                break;
            }

            if(moved) {
                refresh();
                moved = 0;
            }

            napms(1);       // sleep for 1ms
        }//end of (if turn==1)
        else 
        {
            char msg[10];
            read(w_fd,msg,sizeof(msg));
            move(height-10, 0); 
            //printw("msg:%s\n",msg);
            char *sx = strtok(msg,",\n");
            char *sy = strtok(NULL,",\n");
            if (atoi(sy)==-2) {
                goto quit;
            }
            if(atoi(sy)!=-1) { 
                //printw("!!!!!!!!!!!%s%s",sx,sy);
                board[atoi(sy)][atoi(sx)] = enemy;
                cal_rcv(atoi(sy),atoi(sx));
                draw_cursor(atoi(sx), atoi(sy), 0);
                //draw_box(atoi(sx),atoi(sy),colorborder,0);
                //refresh();
                //draw_board();
                draw_score();
                refresh();
                //draw_cursor(atoi(sx), atoi(sy), 0);
            }
            else //
                other_no_move = 1;
            turn = 1;

        }//end of else turn
    }

quit:
    endwin();           // end curses mode

    return 0;
}
bool check(int y,int x) {
    //int direction[8] = {0};
    int ckx,cky;
    bool bcheck = 0;
    if(board[y][x]!=0)//already has disc
        return 0;

    if(x-1 >= 0)
        if(board[y][x-1] == enemy) {
            //direction[0] = 1;
            cky = y;
            ckx = x-2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                if(board[cky][ckx] == player)
                {
                    direction[0] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                ckx = ckx-1;
            }

        }
    if(x-1>=0 && y-1>=0)
        if(board[y-1][x-1] == enemy) {
            //direction[1] = 1;
            cky = y-2;
            ckx = x-2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                if(board[cky][ckx] == player)
                {
                    direction[1] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                ckx = ckx-1;
                cky = cky-1;
            }

        }
    if(y-1>=0)
        if(board[y-1][x] == enemy) {
            //direction[2] = 1;
            cky = y-2;
            ckx = x;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == player)
                {
                    direction[2] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                cky = cky-1;
            }

        }
    if(x+1<8 && y-1>=0)
        if(board[y-1][x+1] == enemy) {
            //direction[3] = 1;
            cky = y-2;
            ckx = x+2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == player)
                {
                    direction[3] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                cky = cky-1;
                ckx = ckx+1;
            }

        }
    move(height-5, 0);

    if(x+1<8)
        if(board[y][x+1] == enemy) {
            //direction[4] = 1;
            cky = y;
            ckx = x+2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {

                
                if(board[cky][ckx] == player)
                {   
                    direction[4] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                //cky = cky-1;
                ckx = ckx+1;
            }

        }
    if(x+1<8 && y+1<8)
        if(board[y+1][x+1] == enemy) {
            //direction[5] = 1;
            cky = y+2;
            ckx = x+2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == player)
                {
                    direction[5] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                cky = cky+1;
                ckx = ckx+1;
            }

        }
    if(y+1<8)
        if(board[y+1][x] == enemy) {
            //direction[6] = 1;
            cky = y+2;
            ckx = x;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == player)
                {
                    direction[6] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                cky = cky+1;
                ckx = ckx;
            }

        }
    if(x-1>=0 && y+1<8)
        if(board[y+1][x-1] == enemy) {
            //direction[7] = 1;
            cky = y+2;
            ckx = x-2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == player)
                {
                    direction[7] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == enemy)
                    ;
                else
                    break;
                cky = cky+1;
                ckx = ckx-1;
            }

        }
    return bcheck;
    /*for(int idx = 0; idx < 8; idx++)
    {
        if(direction[idx]) {
        while
            switch(idx) {
                case 0:
                    x = x-1;
                    
            }
        }
    }*/
}
bool check_rcv(int y,int x) {
    //int direction[8] = {0};
    int ckx,cky;
    bool bcheck = 0;
    /*if(board[y][x]!=0)//already has disc ddf
        return 0;*/
    //printw("in check_rcv dir 0");

    if(x-1 >= 0)
        if(board[y][x-1] == player) {
            //direction[0] = 1;
            //printw("in check_rcv dir 0");
            cky = y;
            ckx = x-2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                if(board[cky][ckx] == enemy)
                {
                    direction[0] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                ckx = ckx-1;
            }

        }
    if(x-1>=0 && y-1>=0)
        if(board[y-1][x-1] == player) {
            //direction[1] = 1;
            cky = y-2;
            ckx = x-2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                if(board[cky][ckx] == enemy)
                {
                    direction[1] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                ckx = ckx-1;
                cky = cky-1;
            }

        }
    if(y-1>=0)
        if(board[y-1][x] == player) {
            //direction[2] = 1;
            cky = y-2;
            ckx = x;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == enemy)
                {
                    direction[2] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                cky = cky-1;
            }

        }
    if(x+1<8 && y-1>=0)
        if(board[y-1][x+1] == player) {
            //direction[3] = 1;
            cky = y-2;
            ckx = x+2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == enemy)
                {
                    direction[3] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                cky = cky-1;
                ckx = ckx+1;
            }

        }
    move(height-5, 0);

    if(x+1<8)
        if(board[y][x+1] == player) {
            //direction[4] = 1;
            cky = y;
            ckx = x+2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {

                
                if(board[cky][ckx] == enemy)
                {   
                    direction[4] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                //cky = cky-1;
                ckx = ckx+1;
            }

        }
    if(x+1<8 && y+1<8)
        if(board[y+1][x+1] == player) {
            //direction[5] = 1;
            cky = y+2;
            ckx = x+2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == enemy)
                {
                    direction[5] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                cky = cky+1;
                ckx = ckx+1;
            }

        }
    if(y+1<8)
        if(board[y+1][x] == player) {
            //direction[6] = 1;
            cky = y+2;
            ckx = x;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == enemy)
                {
                    direction[6] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                cky = cky+1;
                ckx = ckx;
            }

        }
    if(x-1>=0 && y+1<8)
        if(board[y+1][x-1] == player) {
            //direction[7] = 1;
            cky = y+2;
            ckx = x-2;
            while(ckx>=0&&cky>=0&&ckx<8&&cky<8)
            {
                
                if(board[cky][ckx] == enemy)
                {
                    direction[7] = 1;
                    bcheck = 1;
                    break;
                }
                else if(board[cky][ckx] == player)
                    ;
                else
                    break;
                cky = cky+1;
                ckx = ckx-1;
            }

        }
    return bcheck;
    /*for(int idx = 0; idx < 8; idx++)
    {
        if(direction[idx]) {
        while
            switch(idx) {
                case 0:
                    x = x-1;
                    
            }
        }
    }*/
}
void cal_rcv(int y, int x) {
    move(height-10,0);
    check_rcv(y,x); //just find directions
    int caly = y;//+m
    int calx = x;//+n
    int m,n;
    move(height-8,0);
    //printw("in call snd:%d,%d,%d,%d,%d,%d,%d,%d",direction[0],direction[1],direction[2],direction[3],direction[4],direction[5],direction[6],direction[7]);
    for(int i = 0; i < 8; i++) {
        caly = y;
        calx = x;
        if(direction[i] == 1)
        {   
            //printw("in direction %d",i);
            switch(i) {
                case 0:
                    m = 0;
                    n = -1;
                    break;
                case 1:
                    m = -1;
                    n = -1;
                    break;

                case 2:
                    m = -1;
                    n = 0;
                    break;
                case 3:
                    m = -1;
                    n = 1;
                    break;
                case 4:
                    m = 0;
                    n = 1;
                    break;
                case 5:
                    m = 1;
                    n = 1;
                    break;
                case 6:
                    m = 1;
                    n = 0;
                    break;
                case 7:
                    m = 1;
                    n = -1;
                    break;
            }
            caly = caly + m;
            calx = calx + n;
            while(board[caly][calx]!=enemy) {//dff
                board[caly][calx] = enemy;//dff
                caly = caly + m;
                calx = calx + n;
            }
        }
    }//end for
    draw_board();

    memset(direction, 0, sizeof(direction));
}
void cal_snd(int y, int x) {
    int caly = y;//+m
    int calx = x;//+n
    int m,n;
    move(height-8,0);
    //printw("in call snd:%d",direction[4]);
    for(int i = 0; i < 8; i++) {
        caly = y;
        calx = x;
        if(direction[i] == 1)
        {   
            //printw("in direction %d",i);
            switch(i) {
                case 0:
                    m = 0;
                    n = -1;
                    break;
                case 1:
                    m = -1;
                    n = -1;
                    break;

                case 2:
                    m = -1;
                    n = 0;
                    break;
                case 3:
                    m = -1;
                    n = 1;
                    break;
                case 4:
                    m = 0;
                    n = 1;
                    break;
                case 5:
                    m = 1;
                    n = 1;
                    break;
                case 6:
                    m = 1;
                    n = 0;
                    break;
                case 7:
                    m = 1;
                    n = -1;
                    break;
            }
            caly = caly + m;
            calx = calx + n;
            while(board[caly][calx]!=player) {
                board[caly][calx] = player;
                caly = caly + m;
                calx = calx + n;
            }
        }
    }//end for
    draw_board();

    memset(direction, 0, sizeof(direction));
}
void gameover() {
    int i, j;
    int me = 0, other = 0;
    for(i = 0; i < BOARDSZ; i++) {
        for(j = 0; j < BOARDSZ; j++) {
            if(board[i][j] == player) me++;
            if(board[i][j] == enemy) other++;
        }
    }
    
    move(height/2,width/2);
    //printw("in gameover");
    if(me > other)
        printw("I WIN");
    else if(me < other)
        printw("LOSE");
    else
        printw("EVEN");
    refresh();
    fflush(stdout);
    //sleep(10);
    //exit(0);
}
