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


/*Find the file size and return it*/
long int fileSize(char filename[]) 
{ 
    //printf("In fileSize function\n");
    FILE* file = fopen(filename, "r"); 
  
    if (file == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(file, 0L, SEEK_END); 
   
    long int number = ftell(file); 
   
    fclose(file); 
    
    return number; 
}

/*Process the get request*/
int get(int connfd){
    size_t n; 
    char buf[MAXLINE];
    char rString[MAXLINE];
    char Default[14];
    char fstring[MAXLINE];
    
    int numBytes = 0;
    restoreBuf(buf);
    restoreBuf(rString);
    restoreBuf(fstring);
    char* httpmsg;
    /*If the size of the received buffer is 0, return to keep the connection alive*/
    n = read(connfd, buf, MAXLINE);
    if(strlen(buf) == 0){
        return 0;
    }
    /*Loop through and get the filepath from the request*/
    int a = 0;
    int b = 4;
    int buflength = strlen(buf) - 1;
    char filepath[buflength - 4];
    bzero(filepath, sizeof(filepath));
    while( a <= buflength - 4){
        filepath[a] = buf[b];
        a++;
        b++;
        }
    //printf("filepath: %s\n", filepath);
    strtok(filepath, "\n");

    /*create a path for when the get request is the default localhost:<port #> */
    for(int i=0; i < 14; i++){
        Default[i] = buf[i];
    }

    /*if the default directory is requested show default index.html*/
    if(strcmp(Default, "GET / HTTP/1.1") == 0){
        
        //int numBytes = 0;
        /*Get the size of the file*/
        long int fsize = fileSize("index.html");
        
        /*Open the requested file*/
        int file;
        file = open("index.html", O_RDONLY); 
        
        
        char sBuf[MAXLINE];
        /*Read the file and set the number of bytes read to numBytes*/
        numBytes = read(file,sBuf,MAXLINE);

        /*This is the first segment of the default request*/
        httpmsg="HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length:";

        /*Convert the long int file size to a string*/
        char sizeString[4];
        sprintf(sizeString, "%ld", fsize);
        
        /*Concatenate all needed pieces of the string to rString*/
        strcat(rString, httpmsg);
        
        strcat(rString,sizeString);
        
        strcat(rString,sBuf);
        
        /*Copy rString to buf and write buf back to the connection*/
        strcpy(buf,rString);
        write(connfd, buf,sizeof(rString));
        
    }

    else{
        /*extract directory from the filepath*/
        char dir[50];
        strcpy(dir, "");

        int i = 1;
        while(1){
            if(filepath[i] == ' '){
                break;
            }
            dir[i-1] = filepath[i];
            i++;
        }
        
        

        /*get file extension*/
        int q = 1;
        char* ext;
        while(q <= 50){
            if(dir[q] == '.'){
                if(dir[q+1] == 'h'){
                    ext = "text/html";
                }
                else if(dir[q+1] == 't'){
                    ext = "text/plain";
                }
                else if(dir[q+1] == 'p'){
                    ext = "image/png";
                }
                else if(dir[q+1] == 'g'){
                    ext = "image/gif";
                }
                else if(dir[q+1] == 'j'){
                    if(dir[q+2] == 's'){
                        ext = "application/javascript";
                    }
                    else{
                        ext = "image/jpg";
                    }
                }
                else if(dir[q+1] == 'c'){
                    ext = "text/css";
                }
            }
            q++;
        }
        
        long int fsize = fileSize(dir);
        int numBytes = 0;
        int file;
        /*If the size returns -1 that means that file doesn't exist, write the 500 internal server error back*/
        if(fsize == -1){
            printf("HTTP/1.1 500 Internal Server Error\n");
            char msg[] = "HTTP/1.1 500 Internal Server Error";
            write(connfd, msg, strlen(msg));
            return 0;
            
        }
        else{

            file = open(dir, O_RDONLY); 
            /*Get the various pieces of information required for the header and concatenate them onto rString*/
            httpmsg="HTTP/1.1 200 Document Follows\r\nContent-Type:";
            char extmsg[] = "\r\nContent-Length:";
            restoreBuf(rString);
            strcat(rString,httpmsg);
            strcat(rString,ext);
            
            strcat(rString,extmsg);
            char sizeString[MAXLINE];
            strtok(sizeString, "\n");
            
            sprintf(sizeString, "%ld", fsize);
            
            strcat(rString,sizeString);
            strcat(rString,"\r\n\r\n");
            //strcat(rString,fstring);
            restoreBuf(buf);
            strcpy(buf,rString);
            printf("%s\n",buf);
            /*Write the header information back before writing the file information*/
            int error = write(connfd, buf,strlen(rString));
            int totalBytes = 0;
            /*While the total number of bytes read is less than the file size, keep writing the file contents*/
            while(totalBytes < fsize){
                numBytes = read(file,rString,MAXLINE);
                totalBytes += numBytes;
                write(connfd, rString,numBytes);
            }
            
            close(file);
        }
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


