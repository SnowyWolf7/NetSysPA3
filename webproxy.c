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
#include <time.h>

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


/*Get the timout from the user and delete the cache values when the timeout expires*/
void processTimeout(int timeout){
    clock_t start = clock(), diff;
    while((clock() - start) < timeout){
        diff = clock() - start;
        if(diff >= timeout){
            printf("THE SET TIMEOUT HAS EXPIRED! DELETING CACHE CONTENTS!!\n");
            break;
        }
        else{
            continue;
        }
    }

    FILE *file = fopen("Host_IP_Address_Cache.txt", "w");
    fclose(file);
    
}

/*Find the length of the header so it can be added to the content length*/
int parseHeader(char* rbuf){
    int i,count = 0;
    int size = strlen(rbuf) - 1;
    while(i < size){
        if(rbuf[i] == '\r'){
            i+=1;
            count+=1;
            if(rbuf[i] == '\n'){
                i+=1;
                count+=1;
                if(rbuf[i] == '\r'){
                    i+=1;
                    count+=1;
                    if(rbuf[i] == '\n'){
                        count+=1;
                        break;
                    }
                }
            }
        }
        count+=1;
        i+=1;
    }

    printf("The header length is: %d\n",count);
    return count;

}



/*parse through the received information given by the server to find the length of the content it's going to write*/
int parseReceive(char* rbuf, char* rbuf_2){
    int i,j,charCount = 0;
    char cLength[MAXLINE];
    int size = strlen(rbuf) - 1;
    while(i < size){
        //printf("RBUF CHAR AT THIS STAGE IS: %c\n",rbuf[i]);
        if(rbuf[i] == 'L'){
            i+=1;
            if(rbuf[i] == 'e'){
                i+=1;
                if(rbuf[i] == 'n'){
                    i+=1;
                    if(rbuf[i] == 'g'){
                        i+=1;
                        if(rbuf[i] == 't'){
                            i+=1;
                            if(rbuf[i] == 'h'){
                                i+=3;
                                while(rbuf[i] != '\n'){
                                    cLength[j] = rbuf[i];
                                    printf("Getting the length of the content, ****char is%c\n",rbuf[i]);
                                    charCount+=1;
                                    i+=1;
                                    j+=1;
                                }
                            }
                        }
                    }
                }
            }
        }
        i+=1;
    }

    /*Populate rbuf_2 with only the content length given by the server*/
    int it = 0;
    strtok(cLength, "\n");
    int cLengthNum = atoi(cLength);
    printf("THE CONTENT LENGTH IS: %d\n",cLengthNum);
    return cLengthNum;
    // while(it <= cLengthNum){
    //     rbuf_2[it] = rbuf[it];
    //     it+=1;
    // }

    
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

/*Get the IP address from the requested hostname so that it can be stored in the cache for quicler lookups*/
void extractIP(struct hostent *server, char* host_name){
    
    char IP[MAXLINE];

    inet_ntop(AF_INET, server->h_addr, IP, sizeof(IP));
    printf("IP ADDRESS IS: %s\n",IP);

    char *filename = "Host_IP_Address_Cache.txt";
        
    FILE *file = fopen(filename, "a");

    fprintf(file, "%s ", host_name);
    fprintf(file, "%s\n", IP);
    fclose(file);
    bzero(IP, MAXLINE);
    

}

/*Check to see if the requested hostname resides in the cache, if so return the corresponding IP address*/
char *checkHostRequest(char* host_name){
    
    long int fsize = fileSize("Host_IP_Address_Cache.txt");
    int numBytes = 0;
    FILE *file = fopen("Host_IP_Address_Cache.txt", "r");
    int totalBytes = 0;
    char line[MAXLINE];
    char *ipAddr;
    
    
    // while(totalBytes < fsize){
    //         fgets(line,sizeof(line), file);
    //         totalBytes += sizeof(line);
    //         char * cHost = strtok_r(line," ", &ipAddr);
    //         if(strcmp(cHost,host_name) == 0){
    //             return(ipAddr);
    //         }
    //         bzero(line, sizeof(line));
            
    //     }

    /*While the total number of bytes read is less than the file size, keep writing the file contents*/
    while(fgets(line,sizeof(line), file) != NULL){
            char * cHost = strtok_r(line," ", &ipAddr);
            if(strcmp(cHost,host_name) == 0){
                return(ipAddr);
            }
            //bzero(line, sizeof(line));
            
        }


    fclose(file);
    return "No";

}

/*If the requested hostname is in the blacklist then 1 for true so a 403 Forbidden error can be thrown*/
int checkBlackList(struct hostent *server,char* host_name){
    
    long int fsize = fileSize("BlackList.txt");
    int numBytes = 0;
    FILE *file = fopen("BlackList.txt", "r");
    int totalBytes = 0;
    char line[MAXLINE];
    char* ipAddr;
    char IP[MAXLINE];
    
    inet_ntop(AF_INET, server->h_addr, IP, sizeof(IP));
    /*While the total number of bytes read is less than the file size, keep writing the file contents*/
    while(totalBytes < fsize){
            fgets(line,sizeof(line), file);
            totalBytes += sizeof(line);

            char * cHost = strtok(line,"\n");
            //ipAddr = strtok(line,"\n");
            if(strcmp(line,host_name) == 0 || strcmp(IP, line) == 0){
                return(1);
            }
            bzero(line, sizeof(line));
            
        }
    fclose(file);
    
    return 0;
}



/*Process the get request*/
int get(int connfd){
    
    size_t n; 
    char buf[MAXLINE];
    char requestType[MAXLINE];
    char requestURL[MAXLINE];
    char requestTail[MAXLINE];
    char requestPort[MAXLINE];
    bzero(buf, MAXLINE);
    bzero(requestType, MAXLINE);
    bzero(requestPort, MAXLINE);
    bzero(requestURL, MAXLINE);
    
    /*If the size of the received buffer is 0, return to keep the connection alive*/
    n = read(connfd, buf, MAXLINE);
    
    printf("OG buffer is: %s\n",buf);
    char buf_2[MAXLINE];
    strcpy(buf_2,buf);
    
    if(sizeof(buf) == 0){
        return 0;
    }
    
    /*Loop through and get the requested http path from the request*/
    int a = 0;
    
    //int buflength = strlen(buf) - 1;
    while( a <= 2){
        requestType[a] = buf[a];
        a++;
        }
    a = 4;
    int b = 0;
    int count = 0;
    
    
    /*If the request type is not GET, return*/
    if (strcmp(requestType, "GET") != 0){
        printf("HTTP 400 Bad Request\n");
        char msg[] = "HTTP 400 Bad Request";
        write(connfd, msg, sizeof(msg));
        
        return 0;
    }
    char *token = strtok(buf, "\n");
    count = 0;
    
    /*Get the hostname from the GET request*/
    while(count != 1) {
        //printf("TOKEN IS:********%s\n",token);
        token = strtok(NULL, "\n");
        
        count+=1;
    }
    
    //printf("ABOUT TO START PARSING Host_Name\n");
    int start = 6;
    int start2 = 0;
    char host_name[MAXLINE];
    
    while(start < strlen(token)){
        host_name[start2] = token[start];
        
        start+=1;
        start2+=1;
    }

    
    //strtok(requestURL, "\n");
    strtok(host_name, "\n");
    strtok(host_name, "\r");
    //printf("The hostname is: %s\n",requestURL);
    printf("Host_Name AT END: *****%s\n",host_name);
    struct hostent *server;
    


    
    /*Create the socket connection to the server*/
    int sockfd, portno, s;
    portno = atoi(requestPort);
    
    if(portno == 0){
        portno = 80;
    }
    
    printf("The port requested is: %d\n",portno);
    int serverlen;
    struct sockaddr_in serveraddr;
    
    /* gethostbyname: get the server's DNS entry */
    char *ipAddr = checkHostRequest(host_name);
    printf("ipAddr IS: %s\n",ipAddr);
    
    if(strcmp(ipAddr, "No") == 0){
        /*Host doesn't already exist*/
        printf("No such host exists in cache so lookup ip address\n");
        server = gethostbyname(host_name);
        
        
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host as %s\n", host_name);
            char msg[] = "HTTP 404 Not Found\n";
            write(connfd, msg, strlen(msg));
            //exit(0);
            return 0;
        }
        /*Check to see if hostname is blacklisted*/
        if(checkBlackList(server,host_name) == 1){
            printf("Requested Host is BlackListed\n");
            char msg[] = "ERROR 403 FORBIDDEN\n";
            write(connfd, msg, strlen(msg));
            return 0;
        }
        
        
        else{
            printf("Resolved Host_Name Successfully!\n");
            extractIP(server, host_name);
            /* socket: create the socket */
            printf("About to create socket to server\n");
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) 
                error("ERROR opening socket");
            
            /* build the server's Internet address */
            bzero((char *) &serveraddr, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
            serveraddr.sin_port = htons(portno);
            
            
        }
    }
    else{
        /*Host does exist in cache*/
        
        printf("Found the host in the cache, didn't need to call gethostbyname\n");
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) 
                error("ERROR opening socket");
            
            /* build the server's Internet address */
            bzero((char *) &serveraddr, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_addr.s_addr = inet_addr(ipAddr);
            serveraddr.sin_port = htons(portno);
            printf("ipAddr IN else IS: %s\n",ipAddr);
            
    }

    
    

    if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
                printf("Couldn't connect to server\n");
                return 1;
            }

    /*Send the request to the desired server on behalf of client*/
    printf("About to send data to server\n");
    printf("The buffer contains: %s\n",buf);
    serverlen = sizeof(serveraddr);
    
    // if (sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen) < 0) {
    //     error("ERROR in sendto");
    // }
    
    write(sockfd, buf_2, n);
    
    /*Receive data from server*/
    char rbuf[MAXLINE];
    char rbuf_2[MAXLINE];
    
    
    // char *fname = host_name;
    // strcat(fname,".txt");
    // if( access( fname, F_OK ) != -1 ) {
    //     // file DOES exist, so use cached info
    //     printf("The requested server is already in cache, read the file\n");
    //     long int fsize = fileSize(fname);
    //     int numBytes = 0;
    //     int file;
    //     int totalBytes = 0;
    //     file = open(host_name, O_RDONLY); 
    //     /*While the total number of bytes read is less than the file size, keep writing the file contents*/
    //     while(totalBytes < fsize){
    //         numBytes = read(file,rbuf,MAXLINE);
    //         totalBytes += numBytes;
    //         write(connfd, rbuf,numBytes);
    //         bzero(rbuf, MAXLINE);
    //     }
    //     close(file);


    //     } 
    // else {
        // file DOESN't exist, so request from server, cache the info, and send the information back to the user
    
        /*Create the file that will store the requested server information in a cache for future use*/
        
        char *filename = host_name;
        
        printf("FILENAME BEING CREATED IS: %s\n",filename);
        FILE *file = fopen(filename, "w");
        int bytesReceived = 0;
        
        /*While the amount of content received is less than the (content-length + header length) keep looping until all info is received*/
        while(1){
            
            printf("Trying to receive from the server.............\n");
            s = read(sockfd, rbuf, MAXLINE); 
            //s = recvfrom(sockfd, rbuf, MAXLINE, 0, &serveraddr, &serverlen);
            
            printf("THE rbuf IS: *****************\n%s\n",rbuf);
            printf("END OF RBUF*********************\n");
            
            int contentLength;
            int headerLength = parseHeader(rbuf);
            int header = 0;
            if(headerLength > 0){
                contentLength = parseReceive(rbuf, rbuf_2);
                header = 1;
            }
            
            /*write to the client during the loop*/
            int b = 0;
            b = write(connfd, rbuf,s);
            
            /*Write content to a corresponding file so it has it for future use*/
            fprintf(file, "%s", rbuf);
            bzero(rbuf, sizeof(rbuf));
            bzero(rbuf_2, sizeof(rbuf_2));
            printf("Wrote %d bytes to the server \n", b);
            printf("Received %d bytes from server\n",s);
            
            if(header == 1){
                
                bytesReceived += (s - headerLength);
            }
            else{
                
                bytesReceived += s;
            }
            
            printf("NUMBER OF BYTES Received: %d\n", bytesReceived);
            /*If the server sends 0 bytes or the number of total bytes received equals the amount the server said the proxy would receive, break loop*/
            if (bytesReceived >= contentLength){
                
                break;
            }
            if(s == 0){
                
                break;
            }
            
            
        }
        
        
        fclose(file);
        close(sockfd);
        bzero(buf, sizeof(buf));
        bzero(buf_2,sizeof(buf_2));
        bzero(rbuf, sizeof(rbuf));
        bzero(rbuf_2, sizeof(rbuf_2));
    //}//else statement bracket    
        
}


int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in), timeout;
    struct sockaddr_in clientaddr;
    pthread_t tid; 

    //if (argc != 2) {
    if (argc < 3) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);

    /*Get the timeout from the user and pass the value to the function to process the timeout*/
    if (argc == 3){
        timeout = atoi(argv[2]);
        //processTimeout(timeout);
    }



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


