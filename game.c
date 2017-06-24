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
	initscr();			// start curses mode 
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
					// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);			// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

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
        printw("server\n");
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
        printw("client\n");
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
	move(height-1, 0);	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);


	while(true) {			// main loop
        if(turn == 1)
        {
            char msg[10];
		    int ch = getch();
        
            int moved = 0;

            switch(ch) {
            case ' ':
            case 0x0d:
            case 0x0a:
            case KEY_ENTER:
                board[cy][cx] = player;
                char tmp[5];
                sprintf(msg,"%d,%d,",cx,cy);
                //printw("msg:%s",msg);
                write(w_fd,msg,sizeof(msg));
                draw_cursor(cx, cy, 1);
                draw_score();
                refresh();
                turn = 0;
                break;
            case 'q':
            case 'Q':
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

            napms(1);		// sleep for 1ms
        }//end of (if turn==1)
        else 
        {
            char msg[10];
            read(w_fd,msg,sizeof(msg));
            move(height-10, 0);	
            //printw("msg:%s\n",msg);
            char *sx = strtok(msg,",\n");
            char *sy = strtok(NULL,",\n");
            
            //printw("!!!!!!!!!!!%s%s",sx,sy);
            board[atoi(sy)][atoi(sx)] = enemy;
            draw_cursor(atoi(sx), atoi(sy), 0);
            //draw_box(atoi(sx),atoi(sy),colorborder,0);
            //refresh();
            //draw_board();
            draw_score();
            refresh();
            //draw_cursor(atoi(sx), atoi(sy), 0);
            turn = 1;

        }//end of else turn
	}

quit:
	endwin();			// end curses mode

	return 0;
}
