/* CS656-003 Group M1 - Akin Raju (ar2286) / Aneri Oza (ao288) / Anjo Sam Thomas (at672)/ Molly Correia (mwc34) / Nikhil Lalkota (nl269)/ Ritisha Vijay Thakar (rt383)
web.c >> an HTTP proxy server*/
#include <sys/socket.h>							//For socket functions
#include <netinet/in.h> 						//Defines INET_ADDRSTRLEN macro as 16 & INET6_ADDRSTRLEN as 46
#include <sys/types.h>							
#include <arpa/inet.h>							//For inet functions
#include <stdio.h>
#include <string.h> 							//For memset()
#include <unistd.h> 							//For close()
#include <stdlib.h> 							//For exit()
#include <netdb.h>								//For getaddrinfo()
#include <ctype.h>								//For isdigit()
#include <sys/time.h>
#define QUEUE_SIZE 100
#define DEBUG 0
#define BUFF_SIZE 1100000

void print_err(char *message){	perror(message);exit(1);}
int send_data(int sock, char *buffer, int length,int option){	//sends 'length' bytes of buffer data via 'sock' connection.
	int sent=0, tot_sent=0, left=length;
	while (tot_sent<length){
		sent=send(sock,buffer+tot_sent,left,option);
		if(sent==-1){	break;	}									//send error		
		tot_sent=tot_sent+sent;
		left=left-sent;
	}
	if(sent==-1)return -1;
	else		return tot_sent;
}
int get_value(char *buffer, char *pre_variable, char *variable, int post_value,char *value ){//extract value referenced by pre_variabe+variable and puts in 'value'
	int i=0;
	char match_string[100]="";
	strcat( (strcat(match_string,pre_variable)) ,variable);
	char *buffer_ptr=strstr(buffer,match_string);
	if (buffer_ptr==NULL){strcpy(value,""); return 0;}
	buffer_ptr+=strlen(match_string);
	for(i=0; *buffer_ptr!=post_value && *buffer_ptr!='\0' ;i++,++buffer_ptr){
		value[i]=*buffer_ptr;
	}
	value[i]='\0';
	return i;
}
void doParse(char *recv_buf,char *send_buf,char *host,char *host_port,char *path,char *port_string){ //extracts hostip,port and path
	int get_len=get_value(recv_buf,"\r\n","Host: ",'\r',host);
	if(get_len==0)	get_len=get_value(recv_buf,"\r\n","host: ",'\r',host);
	if(get_len==0)	get_len=get_value(recv_buf,"\r\n","HOST: ",'\r',host);	
	get_value(recv_buf,"//",host,' ',path);
	strcpy(host_port,host);
	get_value(host,"",":",'\0',port_string);
	if (strcmp(port_string,"")==0){
		strcat(port_string,"80");				//sets port as 80 if not given
	}
	else{
		char *ptr=strstr(host,":");
		*ptr='\0';
	}
}
int client_receive(int conn,char *recv_buf, char *send_buf,int recv_buf_size){ //receives request from client
	int tot_len=0,tec=0,recvlen=0,flag=0,get_len=0,i=0,j=0;
	char new_buf[BUFF_SIZE]="",transfer_encoding[70]="",string[]="\r\n0\r\n\r\n",string2[]="0\r\n\r\n";
	while(1){
		if(DEBUG)printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Waiting for Client's-request.\n");
		if ( ( recvlen=recv(conn,recv_buf,recv_buf_size-1,0) ) <0 ){if(DEBUG)perror("Client receive error ");return -1;} 
		recv_buf[recvlen]='\0';							     							
		if(DEBUG) printf("Client's Request:\n%s\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Length rcvd from client: %d\n",recv_buf,recvlen);
		if (recvlen==0){ if(DEBUG)printf("\nCLIENT SENT 0\n");break;}

		if(tec==0){								//Checks for chunked transfer-encoding.
			get_len=get_value(recv_buf,"\r\n","Transfer-Encoding: ",'\r',transfer_encoding);
			if(get_len==0)	get_len=get_value(recv_buf,"\r\n","transfer-encoding: ",'\r',transfer_encoding);
			if(get_len==0)	get_len=get_value(recv_buf,"\r\n","TRANSFER-ENCODING: ",'\r',transfer_encoding);
			if(strstr(transfer_encoding,"Chunked")!=NULL||strstr(transfer_encoding,"chunked")!=NULL||strstr(transfer_encoding,"CHUNKED")!=NULL){tec=1;if(DEBUG)printf("\nTransfer-Encoding found\n");}
		}	
		if(tec>0){										//Handles chunked requests
			for(i=0,j=tot_len;i<recvlen;i++,j++){
				new_buf[j]=recv_buf[i];
			}
			new_buf[j]='\0';
			tot_len+=recvlen;
			if(tot_len>65535)break;
			if(recvlen==strlen(string2)&&strcmp(recv_buf,string2)==0){flag=1;}
			else if(recvlen>=strlen(string)){
				char *p=recv_buf+recvlen-strlen(string);
				if(strcmp(p,string)==0)flag=1;	
			}
			if(flag==1){	if(DEBUG)printf("\nChunk DONE!\n");	break;}
			else{ if(DEBUG)printf("\nChunk has not ended\n"); }			
		}
		if(tec==0){break;}
	}
	if(tec==1){
		for(i=0;i<tot_len;i++){
			recv_buf[i]=new_buf[i];
		}
		recv_buf[i]='\0';
		if(DEBUG)printf("TOTAL REQUEST:\n%s\nTOTAL LENGTH:%d\n",recv_buf,tot_len);
		return tot_len;
	}
	return recvlen;
}
void dns(char *host,char *host_ip,char *port_string){ 	//gets IP of host
	char iplist[100][INET_ADDRSTRLEN]={""},ip[INET_ADDRSTRLEN],valueBuff[100];
	struct addrinfo *address_list, hints;
	int i=0,j=0,getaddr_ret,position=0,sock=0,top=0;
	
	memset(&valueBuff,0,sizeof(valueBuff));
	memset(&hints, 0,sizeof(hints));			//Initialize structure with 0
	hints.ai_family = AF_INET;  				//Get IPv4 addresses
	hints.ai_socktype = SOCK_STREAM; 			//Get SOCK_STREAM socket type
	hints.ai_protocol = 6;						//Get TCP protocol type
	
	if ( (getaddr_ret=getaddrinfo(host,"http", &hints, &address_list) ) !=0){ 	//Returns the list of IP's for the host
		if(getaddr_ret==EAI_NONAME)	return;
	}
	for( ; address_list!=NULL ; address_list=address_list->ai_next,i++){		//Extracting each IP
		inet_ntop( address_list->ai_family, &((struct sockaddr_in *)address_list->ai_addr)->sin_addr,ip,INET_ADDRSTRLEN);
		//printf("IP: %s, Socktype: %d, Protocol: %d\n",ip,address_list->ai_socktype,address_list->ai_protocol);
		strcat(iplist[i],ip);
	}
	freeaddrinfo(address_list);
	//Choosing preferred IP 
	struct timeval start, end;
    long mtime=0;
	for(j=0;j<i;j++){
		struct sockaddr_in serv_addr;
		if( (sock=socket(AF_INET,SOCK_STREAM,0))<0){ if(DEBUG)perror("DNS socket create error");continue;}
		memset (&serv_addr , 0 , sizeof(serv_addr));	// Sets the structure with 0s.
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(atoi(port_string));	
		socklen_t slength=sizeof(serv_addr);		
		if(inet_pton(AF_INET,iplist[j],&serv_addr.sin_addr )<0){if(DEBUG) perror("inet_pton error"); continue;}
		gettimeofday(&start, NULL);
		if(connect(sock,(struct sockaddr *) &serv_addr,slength)<0){ if(DEBUG)perror("DNS connect error");continue;}
		gettimeofday(&end, NULL);	
		close(sock);	
		if(mtime>end.tv_usec - start.tv_usec || top==0){//Selecting IP taking the least time
			mtime=end.tv_usec - start.tv_usec;
			top=1;
			position=j;
		}	
	}
	if(top==1)strcpy(host_ip,iplist[position]);
}
int get_msg_size(char *recv_buf, int recvlen ){			//returns the size of the message body
	char *buffer_ptr=strstr(recv_buf,"\r\n\r\n");
	if (buffer_ptr==NULL) return recvlen;
	return recvlen-((buffer_ptr+4)-recv_buf);
}
//doHTTP forwards requests to server and relays the response to client
void doHTTP(int client_conn,char *recv_buf,char *send_buf,int recv_buf_size,int send_buf_size,char *host,char *host_port,char *host_ip,char *port_string,int c_recvlen){
	int server_conn,s_sentlen=0,s_recvlen=0,c_sentlen=0,content_len=0,tot_len=0,tec=0,get_len=0,flag=0;
	char content_len_str[50]="",transfer_encoding[70]="",string[]="\r\n0\r\n\r\n",string2[]="0\r\n\r\n",*m=NULL,match_string[300]="http://";
	struct sockaddr_in serveraddr;
	//Removing host_port from GET string:
    strcat(match_string,host_port);
    if((m=strstr(recv_buf,match_string))!=NULL){
        char *s=m+strlen(match_string);
        memmove(m, s,recv_buf+c_recvlen-s);
        c_recvlen=c_recvlen-strlen(match_string);
    }
    recv_buf[c_recvlen]='\0';
	//Connecting to server:
	memset (&serveraddr , 0 , sizeof(serveraddr));		// Sets the structure with 0s.
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(port_string));			
	if(inet_pton(AF_INET,host_ip,&serveraddr.sin_addr )<0){ if(DEBUG)perror("inet_pton error"); return;}
	if( (server_conn=socket(AF_INET,SOCK_STREAM,0))<0){if(DEBUG) perror("server socket create error");return;}
	socklen_t slength=sizeof(serveraddr);
	if(connect(server_conn,(struct sockaddr *) &serveraddr,slength)<0){if(DEBUG) perror("Server connect error");return;}
	//Connected to server; now forwarding the client's request to it.	
	s_sentlen=(send_data(server_conn,recv_buf,c_recvlen,MSG_NOSIGNAL));
	if(s_sentlen<0){if(DEBUG) perror("Server send_data error");close(server_conn);return;}
	if(DEBUG) printf("\nClient's request forwarded to server\n");
	memset (recv_buf , 0 , recv_buf_size);
	do{	//Receive and forward server's response to client:
		start:		
		if(DEBUG)printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Waiting for server-resp.\n");
		if ( ( s_recvlen=recv(server_conn,recv_buf,recv_buf_size-1,0) ) <0 ){ if(DEBUG)perror("Server receive error"); close(server_conn);return;}
		recv_buf[s_recvlen]='\0';
		if(DEBUG) printf("\nServer's Response:\n%s\n",recv_buf);
		if(DEBUG) printf("Server's Response-length rcvd: %d\n",s_recvlen);
		if (s_recvlen==0){ if(DEBUG)printf("\nSERVER SENT 0\n");break;}
		c_sentlen=(send_data(client_conn,recv_buf,s_recvlen,MSG_NOSIGNAL ));	//Forward response to client
		if(c_sentlen<0){if(DEBUG)perror("Client send_data error");close(server_conn);return;}
		if(DEBUG) printf("\nServer's Response forwarded to client\n");
		if(tec==0){		//Check if response has chunked Transfer-Encoding
			get_len=get_value(recv_buf,"\r\n","Transfer-Encoding: ",'\r',transfer_encoding);
			if(get_len==0)	get_len=get_value(recv_buf,"\r\n","transfer-encoding: ",'\r',transfer_encoding);
			if(get_len==0)	get_len=get_value(recv_buf,"\r\n","TRANSFER-ENCODING: ",'\r',transfer_encoding);
			if(strstr(transfer_encoding,"Chunked")!=NULL||strstr(transfer_encoding,"chunked")!=NULL||strstr(transfer_encoding,"CHUNKED")!=NULL){tec=1;if(DEBUG)printf("\nTransfer-Encoding found\n");}
		}
		if(tec>0){		//Response is chunked
			if(s_recvlen==strlen(string2)&&strcmp(recv_buf,string2)==0){flag=1;}	//Last chunk found
			else if(s_recvlen>=strlen(string)){
				char *p=recv_buf+s_recvlen-strlen(string);
				if(strcmp(p,string)==0)flag=1;										//Terminating chunk found
			}
			if(flag==1){	if(DEBUG)printf("\nChunk DONE!\n");	break;}
			else{ if(DEBUG)printf("\nChunk has not ended\n"); }			
			goto start;
		}
		//For Content-length processing
		get_len=get_value(recv_buf,"\r\n","Content-Length: ",'\r',content_len_str);
		if(get_len==0)get_len=get_value(recv_buf,"\r\n","content-length: ",'\r',content_len_str);
		if(strcmp(content_len_str,"")!=0){					//Buffer contains header with new content-length value
			content_len=atoi(content_len_str);		
			if(DEBUG)printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>NEW CONT SIZE: %d,LAST MSG SIZE: %d\n",content_len,tot_len);	
			tot_len=0;
		}
		int msg_size=get_msg_size(recv_buf,s_recvlen);		//Extracts message body length
		tot_len+=msg_size;
		if(DEBUG)printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CURRENT CONT SIZE: %d\n",msg_size);	
	}while(tot_len<content_len);							//Loop until the whole content is received
	if(DEBUG)printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TOT CONT SIZE:%d\n",tot_len);	
	close(server_conn);
}		
int block(char *host, char *list){							//Checks if host is in blocklist
	FILE *bl;    char *hostp=NULL,a[255]="",domain[255]="",*dp=NULL;
	if((bl= fopen(list, "r"))==NULL){printf("'blocklist.txt' not found. Exiting...\n"); exit(1);} 
    strcpy(a,host);
    hostp=a;
	if(strncmp(hostp,"www.",4)==0){	hostp=hostp+4;}
    while(!feof(bl)){
        fgets(domain, sizeof(domain), bl);
        dp=domain; 
		char *p=NULL;
		while(dp[0]==' ')++dp;
		if(strncmp(dp,"http://",7)==0){dp=dp+7;}
		if(strncmp(dp,"http:/",6)==0){dp=dp+6;}
		if(strncmp(dp,"http:",5)==0){dp=dp+5;}
		if(strncmp(dp,"www.",4)==0){dp=dp+4;}
		if((p=strchr(dp,'/'))!=NULL ||(p=strchr(dp,'\r'))!=NULL || (p=strchr(dp,'\n'))!=NULL)*p='\0';
		while(dp[strlen(dp)-1]==' ')dp[strlen(dp)-1]='\0';
        if(strcasecmp(dp,hostp)==0){fclose(bl);return 1;}
	}
	fclose(bl);
    return 0;
}
void send_client(int conn,char *send_buf,int send_buf_size,char *msg1,char*msg2){
	snprintf(send_buf,send_buf_size,"HTTP/1.1 %s\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n<html>%s</html>",msg1,(int)(13+strlen(msg2)),msg2);
	if((send_data(conn,send_buf,strlen(send_buf),MSG_NOSIGNAL)) <0)if(DEBUG)perror("Error sending error-data to client");close(conn);
}
int main(int argc, char **argv){
	int listener, conn, proxy_port, sockoptval=1;
	FILE *bl;
	struct sockaddr_in proxyaddr, clientaddr;
	if (argc!=3){printf("Please pass the proxy port and black-list path as arguments.\nUsage: %s <port number> <black_list.txt>\nExiting...\n",argv[0]);  exit(1);}
	if( (proxy_port=atoi(argv[1])) > 65535 || proxy_port==0){printf("Invalid port entered.\nExiting\n");  exit(1);}
	if((bl= fopen(argv[2], "r"))==NULL){
		printf("'%s' not found. Exiting...\n",argv[2]); exit(1);} 
	fclose(bl);
	//Server Socket is created. AF_INET is the address family IPV4, SOCK_STREAM is for TCP connection, Last field represents IP protocol(0)   
	if ( (listener=socket(AF_INET, SOCK_STREAM,0)) < 0 )   print_err("Listen error");
	//Setting SO_REUSEADDR option in case of restarting server on same port.
	if ( setsockopt(listener,SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int)) <0 ) print_err("setsockopt() error");
	memset (&proxyaddr , 0 , sizeof(proxyaddr));		// Sets the structure with 0s.
	proxyaddr.sin_family = AF_INET;
	proxyaddr.sin_addr.s_addr = htonl(INADDR_ANY);		// Accepts connection through any of the server IPs 
	proxyaddr.sin_port = htons(proxy_port);				// The port number passed as argument is used to listen for connection.
	// Bind assigns the address and port to the socket.
	if ( bind(listener, (struct sockaddr *)&proxyaddr , sizeof(proxyaddr)) <0 )   print_err("Bind error");
	// Listens on the port for client to connect. QUEUE_SIZE is the max number of clients that can be queued before their connection gets refused.
	if ( listen(listener,QUEUE_SIZE)<0 )	   print_err("Listen error"); 
	printf("Proxy server group M1 (ao288@njit.edu) listening on port (%d)\n",proxy_port);
	while(1){		//Loop for each client connection
		if(DEBUG)printf("\n-----------------------------------------------------------------------------------------------------------------------------\n");
		char recv_buf[BUFF_SIZE]="",send_buf[BUFF_SIZE]="REQ ",get_string[BUFF_SIZE]="",host[500]="",path[1000]="",port_string[6]="",host_ip[INET_ADDRSTRLEN]="",host_port[500]="";
	
		memset (&clientaddr,0,sizeof(clientaddr));
		socklen_t clength=sizeof(clientaddr);
		// Establishes a connection from the client to conn.
		if ( (conn=accept( listener, (struct sockaddr *) &clientaddr , &clength )) <0 )   print_err("Connect error"); 
		
		int recvlen=client_receive(conn,recv_buf,send_buf,sizeof(recv_buf));
		if(recvlen>65535) {							//Request size>65535 not allowed 
			send_client(conn,send_buf,sizeof(send_buf),"413 Request Entity Too Large","Error: 413 Request Entity Too Large"); continue;
		}
		if(recvlen==-1){close(conn); continue;}		//Receive error
		get_value(recv_buf,"GET ","",' ',get_string);
		if(strcmp(get_string,"")==0) {				//GET method not found
			send_client(conn,send_buf,sizeof(send_buf),"501 Not Implemented","Error: Method not supported."); continue;
		}
		printf("REQUEST: %s\n",get_string);
		doParse(recv_buf,send_buf,host,host_port,path,port_string);
		if(strcmp(host,"")==0 || strlen(host)>253) {//Host header not found or host length greater than max allowed
			send_client(conn,send_buf,sizeof(send_buf),"400 Bad Request","Error: 400 Bad Request."); continue;
		}
		if(block(host,argv[2])==1){					//Request should be blocked
			send_client(conn,send_buf,sizeof(send_buf),"403 Forbidden","Error: 403 Forbidden - Website Blocked"); continue;
		}
		dns(host,host_ip,port_string);
		if(strcmp(host_ip,"")==0) {					//No Ip found for host
			send_client(conn,send_buf,sizeof(send_buf),"404 Not Found","Error: 404 Not Found"); continue;
		}
		doHTTP(conn,recv_buf,send_buf,sizeof(recv_buf),sizeof(send_buf),host,host_port,host_ip,port_string,recvlen);
		close(conn); 								//The client connection is closed 
	}
}