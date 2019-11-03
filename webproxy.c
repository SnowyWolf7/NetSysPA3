/* 
 *Aaron Steiner PA3
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int open_listenfd(int port);
//void echo(int connfd);
void *thread(void *vargp);

/*Clear the buffer*/
void restoreBuf(char* c) 
{ 
    int i; 
    for (i = 0; i < MAXLINE; i++) 
        c[i] = '\0'; 
}



/*Process the get request*/
int get(int connfd){
    size_t n; 
    char buf[MAXLINE];
    char requestType[MAXLINE];
    char requestURL[MAXLINE];
    char requestTail[MAXLINE];

    /*If the size of the received buffer is 0, return to keep the connection alive*/
    n = read(connfd, buf, MAXLINE);
    if(strlen(buf) == 0){
        return 0;
    }

    /*Loop through and get the requested http path from the request*/
    int a = 0;
    int buflength = strlen(buf) - 1;
    while( a <= 2){
        requestType[a] = buf[a];
        a++;
        }
    a = 4;
    int b = 0;
    while(buf[a] != 'w'){
        a++;
    }


    while(buf[a] != ' ')
    {
        if(buf[a] != '/'){
            requestURL[b] = buf[a];
            a++;
            b++;
        }
        else{
            a++;
        }
    }
    a++;
     while(a <= buflength)
    {
        requestTail[b] = buf[a];
        a++;
    }

    if (strcmp(requestType, "GET") != 0){
        printf("HTTP 400 Bad Request\n");
        char msg[] = "HTTP 400 Bad Request";
        write(connfd, msg, strlen(msg));
        return 0;
    }
    strtok(requestURL, "\n");
    printf("The hostname is: %s\n",requestURL);
    struct hostent *server;
    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(requestURL);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", requestURL);
        char msg[] = "HTTP 404 Not Found\n";
        write(connfd, msg, strlen(msg));
        exit(0);
    }


    /*Create the socket connection to the server*/
    int sockfd, portno, s;
    portno = 80;
    int serverlen;
    struct sockaddr_in serveraddr;
    /* socket: create the socket */
    printf("About to create socket to server\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
      (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /*Send the request to the desired server on behalf of client*/
    printf("About to send data to server\n");
    serverlen = sizeof(serveraddr);
    s = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    printf("Size of message sent %d\n",s);
    if (n < 0) 
        error("ERROR in sendto");

    /*Receive data from server*/
    char rbuf[MAXLINE];
    restoreBuf(rbuf);
    while(1){

            n = recvfrom(sockfd, rbuf, MAXLINE, 0, &serveraddr, &serverlen);
            if (n < 0) 
                error("ERROR in recvfrom");

            printf("rbuf from server: %s\n",rbuf);
        }



}


int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);

    listenfd = open_listenfd(port);
    while (1) {
	   connfdp = malloc(sizeof(int));
	   *connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
	   pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* thread routine */
void * thread(void * vargp) 
{  
    int connfd = *((int *)vargp);

    pthread_detach(pthread_self());
    /*Call the get processing on connfd*/
    get(connfd);
    free(vargp);
    //echo(connfd);
    close(connfd);
    return NULL;
}



/* 
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure 
 */
int open_listenfd(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */


