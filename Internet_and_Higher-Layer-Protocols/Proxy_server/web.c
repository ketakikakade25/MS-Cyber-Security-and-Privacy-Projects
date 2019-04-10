#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>

int error = 0;		//global variable for error identification
int flag = 0;
int init(int);		//function declaration for creating sockets (client and server)
void go(int);		//function declaration for processing requests (client and server)
void parseString(char[], char[], char[], int*);		//function declaration for parsing the URL requests
int connectRemoteServer(char[], char[],int);		//function declaration for server_connection to main server
void dnslookup(char[], char[]);		//function declaration for URL to IP conversion
void sendError(int);		//function declartion for error messages
int blockSites(char[]);		//function declaration for blocking the websites

int main(int argc,char* argv[]){
 	int listenfd;		//variable for storing return value of init function
 	int port = atoi(argv[1]);		//initializing port variable from command line argument given as port number
 	listenfd = init(port);		//calling the init function
 	go(listenfd);		//calling the go function
	return 0;
}

int init(int port){
	struct sockaddr_in serv_addr;		//structure variable for sockaddr_in structure
	int listenfd,checkbind;		//variables for storing return values of socket and bind functions
	listenfd = socket(AF_INET, SOCK_STREAM, 0);		//creating socket
	//checking for error in creating a new socket
	if(listenfd < 0) 
		printf("Error in creating a proxy socket\n");
	memset(&serv_addr, '0', sizeof(serv_addr));		//setting the memory size of proxy_serv_addr to zero
  	serv_addr.sin_family = AF_INET;		//assigning value of IPv4 or IPv6 format
  	serv_addr.sin_addr.s_addr = INADDR_ANY;		//to accept connection from any address
  	serv_addr.sin_port = htons(port);		//convert from host byte order to network byte order
	checkbind = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));		//binding server to the port
	if(checkbind < 0)		//check for the server is binded to the port
		printf("Error in binding the socket \n");
	return listenfd;
}

void go(int listenfd) {
	struct sockaddr_in cli_addr;		//structure variable for client address
	socklen_t clilen;
	int lcheck,valread,n,connfd;
	memset(&cli_addr, '0', sizeof(cli_addr));		//setting memory of client address to zero
	printf("CS656 project by group 3 (ag986@njit.edu)\n\n");
	while(1) {
		newreq:
			//listening on port
			lcheck = listen(listenfd, 50);
			if(lcheck < 0)
				printf("Error in listening on the port\n");
			//Accepting the request on port
			clilen = sizeof(cli_addr);
			connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &clilen);
			char temp1[256], temp2[4096],temp3[20],str[4096],temp4[50];
			char buf1[15];
      char buffer[65540],ip[32];
      int remfd=0,prtnm=0;
			bzero((char*)temp1,sizeof(temp1));
      bzero((char*)temp2,sizeof(temp2));
      bzero((char*)temp3,sizeof(temp3));
      bzero((char*)str,sizeof(str));
      bzero((char*)temp4,sizeof(temp4));
			bzero((char*)buffer, sizeof(buffer));
			valread = read(connfd,buffer,sizeof(buffer)-1);		//reading request from client into buffer
			if(valread < 0){
				printf("something went wrong with read()! %s\n", strerror(errno));
				error = 2;
				sendError(connfd);
				goto newreq;
			}
     	if(valread==0){
				close(connfd);
				goto newreq;
			}
			if(valread > 65535){
				error = 3;
				sendError(connfd);
				goto newreq;
			}
			sscanf(buffer,"%s %4096s %s",temp4,str,temp3);		//scanning sub parts of the requests to different variables
			if(strlen(str) > 4094){
				 error = 6;
				 sendError(connfd);
				 goto newreq;
			}
			if(((strncmp(temp4,"GET",3)==0))&&((strncmp(temp3,"HTTP/1.1",8)==0)||(strncmp(temp3,"HTTP/1.0",8)==0))){		//comparing strings for methods and protocols
				if(strncmp(str,"http://",7)==0){
					parseString(str,temp1,temp2,&prtnm);		//calling parseString function
					printf("LOG: request for(%s) processed\n",str);
					if(blockSites(temp1) == 1){
						error = 5;
						sendError(connfd);		//sending client socket status to send_error function to send respective error
						goto newreq;
					}
				}else{
					error = 1;
					sendError(connfd);
					goto newreq;
				}
			}else{
				if(strncmp(temp4,"POST",4)==0){
      	  error =4;
          sendError(connfd);
          goto newreq;
        }else{
        	close(connfd);
          goto newreq;
        }
				}
			dnslookup(ip,temp1);		//calling dnslookup function
			if(error != 0){		//checking for error
				sendError(connfd);
				goto newreq;
			}
			remfd = connectRemoteServer(temp1,ip,prtnm);		//connecting to server with the requested IP address
			if(error != 0){
				sendError(connfd);
				goto newreq;
			}
			bzero((char*)buffer, sizeof(buffer));
			if(temp2[0] == '\0'){		//if rest of the path is empty
				sprintf(buffer,"%s / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",temp4,temp3,temp1);		//creating requests without path
			}else{
				sprintf(buffer,"%s /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",temp4,temp2,temp3,temp1);		//creating requests with path
			}
      n = write(remfd,buffer,strlen(buffer));		//executing request on the server
			if(n<0){
				printf("Error writing to socket");
				error =2;
				sendError(connfd);
				goto newreq;
			}else{
				bzero((char*)buffer,sizeof(buffer));
				bzero((char*)buf1,sizeof(buf1));
				do{
					n=read(remfd,buffer,sizeof(buffer)-1);		//reading requested data from server
					if(n<0){
						printf("error in reading file");
						error =2;
						sendError(connfd);
						goto newreq;
					}
					sscanf(buffer,"%10s",buf1);
					if(strncmp(buf1,"HTTP",4) !=0 && flag == 0){		
						error =4;
						sendError(connfd);
						goto newreq;
					}else{
						if(write(connfd,buffer,n)<0){		//writing received data to client
							printf("Error writing to socket");
							error =2;
							sendError(connfd);
							goto newreq;
						}
					flag =1;
					}
					bzero((char*)buffer,sizeof(buffer));
					bzero((char*)buf1,sizeof(buf1));
				}while(n>0);
			}
			if(error != 0){
				sendError(connfd);
				goto newreq;
			}
			close(remfd);		//closing server connection
			close(connfd);		//closing client connection
			flag =0;
	}
}

void parseString(char str[], char temp1[], char temp2[], int *prtnm){
	char new[4096];
	bzero((char*)new,sizeof(new));
	//getting the port number
	int i =0, j,s=0,prt=0;
	*prtnm = 80;
	char portnum[20];
	for (s=7;s<strlen(str);s++){
		if(str[s]==':'){
			s=0;
			bzero((char*)portnum,sizeof(portnum));
			prt=1;
			break;
		}
	}
	while(str[i] != '\0'){
		if(str[i] == '/'){
			i++;
      if(str[i]=='/'){
     	j=0;
      memset(new, '0', sizeof(new));
			memset(portnum,'0',sizeof(portnum));
      i++;
      if(prt!=1){
       	while(str[i]!='/'){
       		new[j] = str[i];
       		i++;
       		j++;
        }
			}else{
				while(str[i]!=':'){
					new[j]=str[i];
					i++;
					j++;
				}
				i++;
				while(str[i]!='/'){
					portnum[s]=str[i];
					s++;
					i++;
				}
				portnum[s]='\0';
        *prtnm=atoi(portnum);
				}
        new[j] = '\0';
        i--;
        strcpy(temp1,new);
        }else{
        	j=0;
          memset(new, '0', sizeof(new));
          while(str[i] != '\0'){
           	new[j++] = str[i++];
          }
          new[j] = '\0';
          strcpy(temp2,new);
          }
		}
        i++;
	}
}

int connectRemoteServer(char temp1[], char ip[], int prt){
	struct sockaddr_in remt_addr;
	int remfd,conn;
	bzero((char*)&remt_addr,sizeof(remt_addr));
	remt_addr.sin_family = AF_INET;
	remt_addr.sin_port = htons(prt);
	remt_addr.sin_addr.s_addr = inet_addr(ip);
	remfd = socket(AF_INET, SOCK_STREAM, 0);		//creating sever socket
    if(remfd < 0)
    	printf("Error in creating server socket\n");
    conn = connect(remfd,(struct sockaddr*) &remt_addr,sizeof(remt_addr));		//connecting to server
    if(conn < 0){
    	printf("Server is not ready (%s)\n",strerror(errno));
		error = 1;
    }
    return remfd;
}

void dnslookup(char ip[], char temp1[]){
	struct addrinfo address,*result = NULL,*p;
  struct sockaddr_in *h;
	int rec;
  memset(&address,0,sizeof(address));
  address.ai_family=AF_INET;
  address.ai_socktype=SOCK_STREAM;
  if((rec=getaddrinfo(temp1,NULL,&address,&result))!=0){		//fetching IP address from domain name
  printf("Error in retrieving IP address\n");
  error = 1;
 	}else{
		//assigning IP address to ip variable
  	for(p=result;p!=NULL;p=p->ai_next){
  		if(p->ai_family==AF_INET){
  			h=(struct sockaddr_in *) p->ai_addr;
  			strcpy(ip,inet_ntoa(h->sin_addr));
     	}
   	}
   	freeaddrinfo(result);		//freeing the memory for result
   	}
}

int blockSites(char site[]){
	if((strstr(site,"torrentz.eu") != NULL) || (strstr(site,"makemoney.com") != NULL) || (strstr(site,"lottoforever.com") != NULL)){
		return 1;
	}
	return 0;
}

//error messages for different types of errors
void sendError(int connfd){
	int n;
	char msg[1024];
	memset(&msg,0,sizeof(msg));
	switch(error){
	case 1:	strcpy(msg, "HTTP/1.1 400 Bad request\n""Content-Type: text/html\n""\n""<h1>400 Bad Request<h1>");
			break;
	case 2:	strcpy(msg, "HTTP/1.1 500 Error\n""Content-Type: text/html\n""\n""<h1>500 Internal server error<h1>");
			break;
	case 3: strcpy(msg, "HTTP/1.1 413 Error\n""Content-Type: text/html\n""\n""<h1>413 Payload Too Large<h1>");
		 	break;
	case 4: strcpy(msg, "HTTP/1.1 501 Error\n""Content-Type: text/html\n""\n""<h1>501 Not Implemented<h1>");
			break;
	case 5: strcpy(msg, "HTTP/1.1 401 Error\n""Content-Type: text/html\n""\n""<h1>401 Unauthorized<h1>");
        	break;
	case 6: strcpy(msg, "HTTP/1.1 414 Error\n""Content-Type: text/html\n""\n""<h1>414 URI Too Long<h1>");
            break;
	}
	n = write(connfd,msg,strlen(msg));
	if(n<0) printf("Error writing to socket");
	error = 0;
	close(connfd);
}