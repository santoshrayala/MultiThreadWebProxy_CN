#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<pthread.h>                //include this for threading
#include <netdb.h>
#include<sys/stat.h>

/*define the constants and global declarations*/
pthread_t threadid[1000]; 					//can multithread 1000 clients
#define BUFFERSIZE 8000
//#define PROXY_IP "127.0.0.1"  				//local host
#define PROXY_IP "129.120.151.94"
#define PROXY_PORT 8083              		 //the proxy will listen on port 8083

#define SERVERPORT        80				// the server listens on 8082
//#define IP_ADDRESS "127.0.0.1"				//local host
#define IP_ADDRESS "129.120.151.94"
#define BUFSIZE 8000
char *request="GET / HTTP/1.0\nhost: www.cnn.com\n";




/* define a function that will be run by all threads  ugu */
void* serveProxy(int  client_sockfd)
{
	  int blockedFd,RequestServer=0,dateFound=0;
	  int dir_fd,temp_fd,cache_fd,server_fd;
	  long len;
	  char reqBuffer[1024],btemp[1024];
	  int chunkRead;
	 struct addrinfo hints, *res, *p;
	 int status,i,endOfReq;
	 char ipstr[INET6_ADDRSTRLEN];
	 char ipstrPointer[INET6_ADDRSTRLEN+1];
     int j,sockfd,cpy;
     char buffer[BUFSIZE];
     char headbuffer[1024];
     int fileStart=0;
     char hostname[100];
     char *servername="www.stackoverflow.com";
     char *serverip="000.000.000.000";
     char httpAppend[17] =" HTTP/1.0\r\n\r\n";
     char baseDir[200]="/home/ag0620/workspace/Cache/Head/";
     char tempDir[200]="/home/ag0620/workspace/Cache/Head/Temp/";
     char cacheDir[200]="/home/ag0620/workspace/Cache/";
     int line_num = 1,headBufferInfo=0;
     int find_result = 0;
     char temp[512];
     char day1[2];
          char mon1[3];
          char yr1[4];
          char hr1[2];
          char min1[2];
          char sec1[2];

          char day2[2];
              char mon2[3];
               char yr2[4];
               char hr2[2];
               char min2[2];
               char sec2[2];

     //serveripn= serverip;
char newRequest[100];
int k=0;
static struct sockaddr_in serv_addr;  //define a variable of type sockaddr_in that stores the information of server adddress
 cpy = read(client_sockfd,buffer,BUFFERSIZE);//read the client request and store into buffer
 //printf("size of request=%d",&cpy);
 printf("inside buffer : %s",buffer);
 if(cpy > 0 && cpy < BUFFERSIZE)	 //return code is valid chars
 				buffer[cpy]=0;		// terminate the buffer
 			else buffer[0]=0;
 	for( i=0;i<cpy;i++)				// remove CF and LF characters
 			if(buffer[i] == '\r' || buffer[i] == '\n')
 				buffer[i]='*';
 	for( i=4;i<BUFFERSIZE;i++) { // null terminate after the second space to ignore extra stuff
 			if(buffer[i] == ' ') {
 				endOfReq=i;
 				buffer[i] = 0;
 				break;
 			}
 		}
 	printf("value in buffer2 :%s",buffer);
 	printf("End of req %d",endOfReq);
 	printf("last char in buffer %c",buffer[sizeof(buffer)-1]);
 //request = buffer;   //copy buffer into request variable



// Find the hostname and get the ip address
	for( i=5;i<BUFFERSIZE;i++) { // null terminate after the second space to ignore extra stuff
				if(buffer[i] =='/' ) {
					fileStart = i+1;
                                        hostname[i]='0';
					k=5;
					break;
				}
				else
				{
				hostname[k]=buffer[i];
				k=k+1;
				}
			}


newRequest[0]='H';
newRequest[1]='E';
newRequest[2]='A';
newRequest[3]='D';
newRequest[4]=' ';
newRequest[5]='/';
k=6;
printf("%s",newRequest);

for(i=fileStart;i<endOfReq;i++)
{

	newRequest[k]=buffer[i];
	k++;

}
//newRequest[k]='H';

for(i=0;i<16;i++)
{
	newRequest[k]=httpAppend[i];
	k++;
}


printf("value of k = %d",k);



printf("%s",newRequest);
request=newRequest;
servername =hostname;

/*

check if the hostname is in blocked list
 */


    if((blockedFd = open("blockedsites.txt",O_RDONLY)) == -1)
    {
	printf("Unable to open the file blockedSites");
	return NULL;
    }

    len = (long)lseek(blockedFd, (off_t)0, SEEK_END); /* lseek to the file end to find the length */
    		      (void)lseek(blockedFd, (off_t)0, SEEK_SET); /* lseek back to the file start ready for reading */

	//fseek(blockedFd,0,SEEK_SET);
    //while (	(cpy = read(blockedFd, temp, 512)) > 0 )
    //{


    	//printf(temp);
    //	printf("\n");
    	 if((strcmp("www.google.com",servername)) == 0)
    	            {
    		 (void)write(client_sockfd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>Restricted site</title>\n</head><body>\n<h1>Site restricted by admin</h1>\nThe requested resource is blocked by proxy.\n</body></html>\n",224);

    			printf("the requested site is in block list");
    			return NULL;
    	            }


    	//send the code in blocks
    //}
    /*
    		      			(void)write(clientSocket_fd,buffer,cpy);
        while(fgets(temp, 50, blockedFd) != NULL)
        {
            if((strcmp(temp, hostname)) == 0)
            {
		printf("the requested site is in block list");
		return NULL;
            }

        }
*/




     memset(&hints, 0, sizeof hints);
     hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
     hints.ai_socktype = SOCK_STREAM;

     if ((status = getaddrinfo(servername, NULL, &hints, &res)) != 0) {
         printf("error in getaddressinfo()");
		return NULL;

     }

     printf("IP addresses for %s:\n\n", servername);

     for(p = res;p != NULL; p = p->ai_next) {
         void *addr;
         char *ipver;

         // get the pointer to the address itself,
         // different fields in IPv4 and IPv6:
         if (p->ai_family == AF_INET) { // IPv4
             struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
             addr = &(ipv4->sin_addr);
             ipver = "IPv4";
         } else { // IPv6
             struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
             addr = &(ipv6->sin6_addr);
             ipver = "IPv6";
         }


         //serverip = addr;
         // convert the IP to a string and print it:
         inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        // inet_ntop(p->ai_family, addr, serverip, sizeof serverip);

         serverip=ipstr;

// printf("%s",serverip);

 break;
     }

     freeaddrinfo(res); // free the linked list
     strcat(baseDir,serverip);

     /*Create directory to store the pages*/
     printf("creating dir: %s",baseDir);
     mkdir(baseDir,0777);
/**/strcat(baseDir,"/index.html");
	dir_fd=open(baseDir, O_WRONLY | O_CREAT, 0644);

	strcat(cacheDir,serverip);

	     /*Create directory to store the pages*/
	     printf("creating dir: %s",cacheDir);
	     mkdir(cacheDir,0777);
	/**/strcat(cacheDir,"/index.html");




    strcat(tempDir,serverip);

    /*Create directory to store the pages*/
    printf("creating dir: %s",tempDir);
    mkdir(tempDir,0777);
/**/strcat(tempDir,"/index.html");
if((temp_fd=open(tempDir,O_RDONLY)) == -1)
	{
printf("cannot open tempDir");
temp_fd=open(tempDir, O_WRONLY | O_CREAT, 0644);
RequestServer = 1;
	}



	printf("%s",serverip);
	printf("client trying to connect to %s and port %d\n",serverip,SERVERPORT);

	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0)    //create a socket where proxy will establish communication with server
	printf("socket() failed");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(serverip);
	serv_addr.sin_port = htons(SERVERPORT);

	// Connect to the socket which is offered by the server
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)  // client makes a connection with server
		printf("connect() failed");


	bzero(reqBuffer,1024);
		strcat(reqBuffer, "HEAD ");
		           strcat(reqBuffer,"/");
		           strcat(reqBuffer, " HTTP/1.1\n");
		           strcat(reqBuffer, "host: ");
		           strcat(reqBuffer,servername);
		           strcat(reqBuffer, "\n\n");
		           printf("Request >>>>> %s\n\n",reqBuffer);
		           printf("request sent to %s :\n%s\n\n",servername, reqBuffer);
		           chunkRead = write(sockfd, reqBuffer, strlen(reqBuffer));
		           chunkRead = read(sockfd, buffer,1024);//send file

		        	       //(void)write(client_sockfd,buffer,chunkRead);
		        	   		(void)write(dir_fd,buffer,chunkRead);//write the buffered data into client_sockfd



		           for(i=0;i<400;i++)
		           {
		           	printf("%c",buffer[i]);
		           	if(buffer[i]=='L' && buffer[i+1]=='a' && buffer[i+2]=='s') //&& temp[i+1]=='a' && temp[i+1]=='s'
		           	{
		           		k= i+20;
		           		dateFound=1;
		           		printf("reached");
		           		break;
		           	}
		           }
		           if(dateFound>0)
		           {
       		           i=k;
       		           day1[0]=buffer[i];
       		           day1[1]=buffer[i+1];


       		           printf("day:%c%c",day1[0],day1[1]);

       		           mon1[0]=buffer[i+3];
       		           mon1[1]=buffer[i+4];
       		           mon1[2]=buffer[i+5];

       		           printf("mon:%c%c%c",mon1[0],mon1[1],mon1[2]);

       		           yr1[0]=buffer[i+7];
       		           yr1[1]=buffer[i+8];
       		           yr1[2]=buffer[i+9];
       		           yr1[3]=buffer[i+10];

       		           printf("Yr:%c%c%c%c",yr1[0],yr1[1],yr1[2],yr1[3]);

       		           hr1[0]=buffer[i+12];
       		           hr1[1]=buffer[i+13];


       		           printf("HR:%c%c",hr1[0],hr1[1]);

       		           min1[0]=buffer[i+15];
       		           min1[1]=buffer[i+16];

       		           printf("MIN:%c%c",min1[0],min1[1]);

       		           sec1[0]=buffer[i+18];
       		           sec1[1]=buffer[i+19];
       		           printf("sec:%c%c",sec1[0],sec1[1]);
		           }
		           else
		           {
		        	   printf("last modified not found");
		        	   RequestServer=1;
		           }


       		         while ((cpy = read(temp_fd, headbuffer, 1024)) > 0 )
       		         {
       		        	 printf("found something in headbuffer");
                             headBufferInfo=1;
       		         }
printf("DateFound :%d headBuffer : %d\n",dateFound,headBufferInfo);
       		         if(dateFound>0 && headBufferInfo>0)
       		         {
       		       for(i=0;i<412;i++)
       		       		           {
       		       		           //	printf("%c",buffer[i]);
       		       		           	if(headbuffer[i]=='L' && headbuffer[i+1]=='a' && headbuffer[i+2]=='s') //&& temp[i+1]=='a' && temp[i+1]=='s'
       		       		           	{
       		       		           		k= i+20;
       		       		           		printf("reached date in headbuffer");
       		       		           		break;
       		       		           	}
       		       		           }
       		       		          		           i=k;
       		       		          		           day2[0]=headbuffer[i];
       		       		          		           day2[1]=headbuffer[i+1];


       		       		          		           printf("day:%c%c",day2[0],day2[1]);

       		       		          		           mon2[0]=headbuffer[i+3];
       		       		          		           mon2[1]=headbuffer[i+4];
       		       		          		           mon2[2]=headbuffer[i+5];

       		       		          		           printf("mon:%c%c%c",mon2[0],mon2[1],mon2[2]);

       		       		          		           yr2[0]=headbuffer[i+7];
       		       		          		           yr2[1]=headbuffer[i+8];
       		       		          		           yr2[2]=headbuffer[i+9];
       		       		          		           yr2[3]=headbuffer[i+10];

       		       		          		           printf("Yr:%c%c%c%c",yr2[0],yr2[1],yr2[2],yr2[3]);

       		       		          		           hr2[0]=headbuffer[i+12];
       		       		          		           hr2[1]=headbuffer[i+13];


       		       		          		           printf("HR:%c%c",hr2[0],hr2[1]);

       		       		          		           min2[0]=headbuffer[i+15];
       		       		          		           min2[1]=headbuffer[i+16];

       		       		          		           printf("MIN:%c%c",min2[0],min2[1]);

       		       		          		           sec2[0]=headbuffer[i+18];
       		       		          		           sec2[1]=headbuffer[i+19];
       		       		          		           printf("sec:%c%c",sec2[0],sec2[1]);


if(yr2[0]==yr1[0] && yr2[1]==yr1[1] && yr2[2]==yr1[2] && yr2[3]==yr1[3])
{
RequestServer = 0;
}
else
{
RequestServer = 1;
}
       		         }




printf("RequestServer:%d",RequestServer);


close(sockfd);


if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0)    //create a socket where proxy will establish communication with server
printf("socket() failed");

serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = inet_addr(serverip);
serv_addr.sin_port = htons(SERVERPORT);

// Connect to the socket which is offered by the server
if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)  // client makes a connection with server
	printf("connect() failed");


if(RequestServer>0)
{
	 if(( cache_fd=open(cacheDir, O_WRONLY | O_CREAT, 0644)) == -1) { //open the file for reading
				           					  printf("unable to open cache file to write the new page");

				           		}
	bzero(reqBuffer,1024);
			strcat(reqBuffer, "GET ");
			           strcat(reqBuffer,"/");
			           strcat(reqBuffer, " HTTP/1.1\n");
			           strcat(reqBuffer, "host: ");
			           strcat(reqBuffer,servername);
			           strcat(reqBuffer, "\n\n");
			           printf("Request >>>>> %s\n\n",reqBuffer);
			           printf("request sent to %s :\n%s\n\n",servername, reqBuffer);
			           chunkRead = write(sockfd, reqBuffer, strlen(reqBuffer));

			           while ((chunkRead = read(sockfd, buffer,1024))!= (size_t)NULL)//send file
			           {
			        	   printf("INSIDE THE SERVVER READ");

			        	   strncpy(btemp,buffer, 1024);
			        	   btemp[strlen(buffer)-1]='\0';
			        	   p = strstr(btemp,"news");

			        	  	       if(p!=NULL){
			        	  	    	   printf("profane word %c%c",p[0],p[1]);
			        	  	       }
			        	   (void)write(client_sockfd,buffer,chunkRead);
			        	   (void)write(cache_fd,buffer,chunkRead);//write the buffered data into client_sockfd
			           }


}


else
{
	if(( cache_fd = open(cacheDir,O_RDONLY)) == -1) { //open the file for reading
					  printf("unable to open cache file");

		}
		else
		{
		len = (long)lseek(cache_fd, (off_t)0, SEEK_END);  //lseek to the file end to find the length
			      (void)lseek(cache_fd, (off_t)0, SEEK_SET); // lseek back to the file start ready for reading

			(void)write(client_sockfd,buffer,strlen(buffer));
			while (	(cpy = read(cache_fd, buffer, BUFFERSIZE)) > 0 ) { //send the code in blocks
				(void)write(client_sockfd,buffer,cpy);
			}
		}

}



/*


	printf("Send bytes=%d %s\n",strlen(request), request);
	write(sockfd, request, strlen(request));  //send the request to the server

	while( (i=read(sockfd,buffer,BUFSIZE)) > 0)
	{//read the response by server and copy into buffer
		(void)write(client_sockfd,buffer,i);
		(void)write(dir_fd,buffer,i);//write the buffered data into client_sockfd
	}

*/
	return NULL;
}



//The main() function will acts as a  server listening to client requests.
//The serveRequest() acts as a client that gets the response from server . The response will be forwarded to actual client
void main ()
{
	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int bytesReceived = 0;
	char message[BUFFERSIZE];


	server_sockfd = socket (AF_INET, SOCK_STREAM, 0);// create a socket

	// assign an address (name) to the socket
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(PROXY_IP);
	server_address.sin_port = htons(PROXY_PORT);
	server_len = sizeof (server_address);
	bind (server_sockfd, (struct sockaddr *) &server_address, server_len);

	// create a connection queue and wait for clients to arrive
	int clientCount =0;
while(1)
{

     clientCount++;
     int err;
	listen (server_sockfd, 5);

	client_len = sizeof (client_address);
	client_sockfd = accept (server_sockfd, (struct sockaddr *) &client_address, &client_len);

    //Create a thread for each client
	err = pthread_create(&(threadid[clientCount]), NULL, &serveProxy, client_sockfd);
if(err!=0)
	printf("failed to create thread %d\n", clientCount);
else
	printf("created thread %d\n", clientCount);

	//serveProxy(client_sockfd);
}
}

