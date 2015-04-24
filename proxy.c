/*
Computer Networks - 5580
Web Proxy Server:
1. Blocks websites if found in blocked_sites.txt file
2. Blocks words that are found in blocked_words.txt from appearing in website
3. Cacheing of sites: Storing/Fetching

Prepared by:
Avinash Gogineni
Santosh Rayala
Arun Kumar Kalyankar
*/


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <dirent.h>
#include <ctype.h>
#include <memory.h>
#define _GNU_SOURCE

//Web Proxy Port
#define PROXYPORT 8888 //Port where Proxy server will be listening for clients connections
#define WEBSERVERPORT 80 //Default port where proxy server will send the requests to internet
#define BUFFSIZEONE 512
#define BUFFSIZETWO 512
#define BCKLOG 16

//Global Variables
char web_server[BUFFSIZETWO]={0};
char server_name[BUFFSIZETWO]={0};
char *cached_sites_dir="./cached_sites";
char *blockedsites="blocked_sites.txt";
char *blockedwords="blocked_words.txt";
struct sockaddr_in acc_addr, bind_addr;
int useSamePort = 1;
int blockHost = 0;
int establish_socket_connection();
void error (char *);
int hostIpConversion(char*, char*);
FILE *createCache(char*,char*,char*);
char *findInCache(char *, char *, char *);
//int blockSite( char*);
int checkSite( char*);
void getCacheInfo (char* , char*);
void *proxyHandler (void *);
static void _mkdir(const char*);
int toggle_msg = 0;

int main (int argc, char *argv[]) {
  bzero ((char *) &bind_addr, sizeof (bind_addr));
  bzero ((char *) &acc_addr, sizeof (acc_addr));
  int sockfd = -1;
  int newsockfd;
  pthread_t thread_id = 0;
  int socklen = 0;
  int flag = 1;
  
  //Socket creation
  sockfd = establish_socket_connection();
  while (1) {
      //accept connections from client
      socklen = sizeof (acc_addr);
      newsockfd = 	accept (sockfd, (struct sockaddr *) &acc_addr, (socklen_t *) & socklen);
      if (newsockfd < 0) {
		error ("\nError: Could not accept the connection.\n");
	  }
	   setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));	
	  //create a new thread and assign the incoming connection
      if (pthread_create (&thread_id, NULL, proxyHandler, (void *)(intptr_t)newsockfd) < 0) {
		error ("\nError: Thread creation failed.\n");
	  }
    }

  return 0;
}

//Create the socket connection, bind the port address and listen on the port
int establish_socket_connection() {
  bind_addr.sin_family = AF_INET; //Binding of port
  bind_addr.sin_port = htons (PROXYPORT);
  bind_addr.sin_addr.s_addr = INADDR_ANY;
  int sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // open socket
  if (sockfd < 0) {
	error ("\nError: Socket initialization failed.\n");
  }
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &useSamePort, sizeof(int));
  // bind proxy to specified address and port
  if (bind (sockfd, (struct sockaddr *) &bind_addr, sizeof (bind_addr)) < 0) {
    error ("\nError: Socket binding failed.\n");
  }
  //listen for incomming connections
  listen (sockfd, BCKLOG);
  return sockfd;
}


//Display error messages
void error (char *err_msg) {
  perror (err_msg);
  exit (0);
}

//Create a directory path if not exist
static void _mkdir(const char *dir) {
        char tmp[256];
        char *p = NULL;
        size_t len;
        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

//Create cached_sites folder to store already requested websites
FILE * createCache(char *hostname, char *url, char *web_ip) {
	char url_file[512];
	char website_file[512];
	FILE *fp ;
	strcpy(url_file,cached_sites_dir);
	strcpy(url_file + strlen(url_file), "/");
	strcpy(url_file + strlen(url_file), web_ip);
	strcpy (website_file, url_file);
	strcpy(url_file + strlen(url_file), ".url");
	fp = fopen(url_file, "w");
	fprintf(fp, "%s\r\n%s", hostname, url);
	fclose(fp);
	fp = fopen(website_file, "w");
	return fp;
}

//Function to convert hostname into IP address.
//Handles both IPv4 and IPv6.
int hostIpConversion(char * hostname , char* ip) {
    struct hostent *host;
    struct in_addr **addr_list;
    int i;
    //return 1 ,if could not resolve hostname
    if ( (host = gethostbyname( hostname ) ) == NULL) {
        return 1;
    }
    //convert hostname into IP address
    addr_list = (struct in_addr **) host->h_addr_list; 
    for(i = 0; addr_list[i]!=NULL; i++) {
        strcpy(ip, inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return 1;
}

void getCacheInfo (char *file , char *etag)
{
	char line[512];
	char *ptr;
	FILE *fp;
	const char *key="Etag:";
	etag[0] = '\0';
	fp = fopen(file, "r");
	while(!feof(fp)) {
     if (fgets(line,512,fp)) {
	   if (line[strlen(line) - 1] == '\n' ) {
			line[strlen(line) - 1] = '\0';
	   }
	   if (!strlen(	etag) && (ptr=strstr(line, key))) {
		   strcpy(etag, ptr+strlen(key) ); //tag found in cache file
	   }	   
	   if (strlen(etag)) {
	       break;
	   }
     }
	}
   fclose(fp);
  return;
} 
     

//function to block the blocked words from the requested content 
int checkSite(char *buffer) {
	char blockedWord[512];
	char *matchFound;
	FILE *fp;
	fp = fopen(blockedwords, "r");
    int i;
	while(!feof(fp)) {
    if (fgets(blockedWord,512,fp)) {
	   matchFound = buffer;
	   if (blockedWord[strlen(blockedWord) - 1] == '\n' ) {
				blockedWord[strlen(blockedWord) - 1] = '\0';
	   }	
       while ((matchFound = strcasestr(matchFound, blockedWord))) {
			 if (!isalpha(matchFound[-1]) && !isalpha(matchFound[strlen(blockedWord)])) {
		       for (i=0; i<strlen(blockedWord); i++,matchFound++) {
		          *matchFound = '*';
			   }
		     } else {
		        matchFound += strlen(blockedWord);
			 }
		 }
    }
   }
   fclose(fp);
   return 0;
}

//try to find the requested page in cached_sites_dir folder before fetching from the server
char * findInCache(char *hostname, char *url, char *web_ip) {
  DIR *search_directory;
  FILE *fp ;
  char *file_name=NULL, *extention;
  char givenHost[512];
  char givenURL[512];
  char fname[512];
  search_directory = opendir(cached_sites_dir);
  struct dirent *dir_content;
  //read all the files in directory
  if (search_directory != NULL) {
      while ((dir_content = readdir (search_directory))) {
		 if (strstr(dir_content->d_name, ".url")) {
			 givenHost[0] = givenURL[0] = '\0';
		     strcpy(fname, cached_sites_dir);
		     strcat(fname, "/");
		     strcat(fname, dir_content->d_name);
		     fp = fopen(fname, "r");
		     if (fp) {
		       fscanf(fp, "%s\r\n%s", givenHost, givenURL);
		       fclose(fp);
		     }
		     if (!strcmp(givenHost, hostname)  && !strcmp(givenURL, url)) {
				  file_name = strdup(fname);
				  extention = strstr(file_name,".url");
				  if (extention) {
				    *extention = '\0';
				  }
				  break;   
			   }
		 }
	     
	  }   
      (void) closedir (search_directory);
      return file_name;
    }
	return NULL;
}

//proxy handler
void *proxyHandler (void *sockptr) {
  struct sockaddr_in host_addr = {0};
  int mysocket = 0;
  int socketOne = 0;
  int socketTwo = 0;
  char buffer[BUFFSIZEONE];
  char httpRequest[BUFFSIZEONE];
  char requestPartOne[BUFFSIZETWO];
  char requestPartTwo[BUFFSIZETWO];
  char requestPartThree[BUFFSIZETWO];
  int var_len = 0;
  FILE *fp;
  mysocket = (int)(intptr_t)sockptr; //socket creation
  bzero ((char *) buffer, BUFFSIZEONE);
  recv (mysocket, buffer, BUFFSIZEONE, 0); //read the request
  sscanf (buffer, "%s %s %s", requestPartOne, requestPartTwo, requestPartThree);
  if ((strncmp (requestPartOne, "GET", 3) == 0)) {
      char *host_name = requestPartTwo;
      char *url = requestPartTwo;
      char web_ip[BUFFSIZETWO] = {0};
	  char server_name[BUFFSIZETWO] = {0};
	  char *cache_name;
	  char cache_etag[BUFFSIZEONE] = {0};
      if (host_name[0] == '/') {
           host_name = &host_name[1];
	  }     
	  if (0 != hostIpConversion(host_name, web_ip)) {	
		  if (strlen(web_server) && strlen(server_name)) {
				host_name = server_name;
				strcpy(web_ip, web_server);
		  } else {	
		        printf ("\nError: Could not resolve host [%s].\n", host_name);
			    return NULL;
		  }
	  } else {
		 strcpy(web_server, web_ip);
         strcpy(server_name, host_name);
         url = "/";
      }
      host_addr.sin_port = htons (WEBSERVERPORT);
      host_addr.sin_family = AF_INET;
      host_addr.sin_addr.s_addr = inet_addr(web_ip);
      //check if the requested site is blocked or not
	  if (host_name) {
		  char fileline[512];
		  fp = fopen(blockedsites, "r");
		  while(!feof(fp)) {
			if (fgets(fileline,512,fp)) {
				if (strstr(fileline, host_name)) {
					//fclose(fp);
					blockHost = 1;
				}
			}
		  }
		  fclose(fp);
		}
      if (blockHost == 1) {  
		     printf ("\nRequested Website [%s] was blocked by Administrator.\nPlease contact Administrator to get it unblocked.\n", host_name);
		     sprintf(buffer, "\nAccess to this website is restricted.\nContact Administrator to request access.\n");
		     send (mysocket, buffer, strlen(buffer), 0);
			 close (mysocket);
			 blockHost = 0;
			 return NULL;
	   }
	  printf("\nRequest Received for\n--------------------\n%s", web_ip);
	  //open socket and connect
      socketTwo = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
      socketOne = connect (socketTwo, (struct sockaddr *) &host_addr, sizeof (struct sockaddr));
      if (socketOne < 0) {
			printf ("\nError: Could not connect to the server.\n");
			//sprintf(buffer, "%s %s %s\r\nhost: %s\r\n\r\n", "GET", url, "HTTP/1.1", host_name);
			return NULL;
      }
      httpRequest[0]='\0';
      // check if the requested website already exists in Cache
	  printf ("\nSearching the requested site in cached directory first."); 
      cache_name = findInCache(host_name, url, web_ip);
      if (cache_name) {
		  toggle_msg = 1;
        printf ("\nRequested site [%s] FOUND in cached directory [%s].\nVerifying with the server if the cached site is the latest.\n", web_ip, cache_name);
        getCacheInfo(cache_name, cache_etag);
        if (strlen(cache_etag))
          sprintf(httpRequest, "\r\nIf-None-Match: %s", cache_etag);
      } else {
		  printf ("\nRequested site [%s] NOT FOUND in cached directory [%s].\nRequest sent to server.\n", web_ip, cache_name);
	  }
	  //send request to server
	  sprintf(buffer, "%s %s %s\r\nhost: %s%s\r\n\r\n", "GET", url, "HTTP/1.1", host_name, httpRequest);
      var_len = send (socketTwo, buffer, strlen (buffer), 0);
      if (var_len <= 0)
      {
			printf ("\nError: Could not write to socket.\n");
			return NULL;
	  }
      bzero ((char *) buffer, BUFFSIZEONE);
      var_len = recv (socketTwo, buffer, BUFFSIZEONE, 0);
      if (cache_name && strstr(buffer, " 304 "))
      {
		//site not modified since last cached
		//read cache file
	    fp = fopen(cache_name, "r");
	    while(!feof(fp)) {
         if ((var_len = fread(buffer, 1, BUFFSIZEONE, fp))) {
			 //check the site content for explicit content
	         checkSite(buffer);
		     send (mysocket, buffer, var_len, MSG_DONTWAIT);
		     bzero(buffer, BUFFSIZEONE);
         }
		}
        fclose(fp); 
	  } else {
		  //cached content is not the latest
		fp = createCache(host_name,url, web_ip);
		//check the site content for explicit content
		checkSite(buffer);
		send (mysocket, buffer, var_len, MSG_DONTWAIT);
		fwrite(buffer, var_len, 1, fp);
	    do {
	      bzero ((char *) buffer, BUFFSIZEONE);
	      var_len = recv (socketTwo, buffer, BUFFSIZEONE, 0);
	      if (var_len>0) {
			  //check the site content for explicit content
			checkSite(buffer);
		    send (mysocket, buffer, var_len, MSG_DONTWAIT	);
		    fwrite(buffer, var_len, 1, fp);
		  }		    
	    } while (var_len > 0);
	    fclose(fp);
	  }
    }
  else
    {
      send (mysocket, "\nSorry. Only HTTP request with GET is accepted.\n", 19, 0);
    }
  printf("\n\n\nWaiting for new requests....\n");
  fflush(stdout);
  close (socketTwo);//close socket
  close (mysocket);//close socket
  return NULL;
}
