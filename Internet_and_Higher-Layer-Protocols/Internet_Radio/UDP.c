#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include<stdlib.h>
 int main (int argc, const char* argv[]) {
     struct sockaddr_in saddr;
     int fd, ret_val;
     struct hostent *host; 
     char *DATA_BUFFER;
     FILE *file_ptr;
     int i=0,n,port,s,cnt;   
     port = atoi(argv[2]);
     s = atoi(argv[4]);     //delay time between 2 packets
     cnt = atoi(argv[5]);   //packet size
     DATA_BUFFER = malloc(cnt * sizeof(char));  //allocating size of buffer according to the packet size we input
     file_ptr = fopen(argv[3],"rb");    //rb is for read bytes of data from a file
     if (file_ptr == NULL)
        printf("mp3 file not found");
     fd = socket(AF_INET, SOCK_DGRAM,0);
     if (fd == -1) {
         fprintf(stderr, "socket failed [%s]\n", strerror(errno));
         return -1;
     }
     saddr.sin_family = AF_INET;
     saddr.sin_port = htons(port);
     host = gethostbyname(argv[1]);
     saddr.sin_addr = *((struct in_addr *)host->h_addr);
     bzero(DATA_BUFFER, cnt);
     for (i=0;!feof(file_ptr);i++){
        n = fread(DATA_BUFFER, 1,cnt, file_ptr);    //this line will take cnt bytes from file_ptr and put into buffer, and it will make only 1 packet
        ret_val = sendto(fd,DATA_BUFFER,n, 0, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
        if (ret_val < 0) {
            printf("sendto() failed [%s]\n", strerror(errno));
        }
        bzero(DATA_BUFFER, cnt);
	    usleep(s);
    }
     fclose(file_ptr);
     close(fd);
     free(DATA_BUFFER);
     return 0;
 }