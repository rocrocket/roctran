#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<errno.h>

int main(int argc,char *argv[])
{
int socket_fd;
socket_fd=socket(AF_INET,SOCK_STREAM,0);
if(socket_fd==-1)
{
	perror("socket FATAL");
	exit(1);
}
#ifdef DEBUG
        printf("socket init success\n");
#endif

/*address setup*/
struct sockaddr_in server_addr;
struct in_addr inp;
int inet_ret;
//clear the structure
memset(&server_addr,0,sizeof(struct sockaddr_in));
server_addr.sin_family=AF_INET;
server_addr.sin_port=htons(21212);
server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
/*==========*/

/*bind operation*/
int bind_ret;
bind_ret=bind(socket_fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
if(bind_ret!=0)
{
	perror("bind FATAL");
	exit(2);
}
#ifdef DEBUG
	printf("bind success!\n");
#endif
/*==============*/

/*listen operation*/
int listen_ret;
listen_ret=listen(socket_fd,10);
if(listen_ret!=0)
{
	perror("listen FATAL");
	exit(3);
}
/*================*/

/*accept operation*/
int newsocket_fd;
newsocket_fd=accept(socket_fd,NULL,NULL);
if(newsocket_fd==-1)
{
	perror("accept FATAL");
	exit(4);
}
printf("One child call me!\n");
/*================*/

/*read from a client*/
ssize_t read_ret;
int receive_len;
char receive_string[100];
char *str_p=receive_string;

//read_ret=read(newsocket_fd,&receive_len,sizeof(reiceive_len));	
read_ret=read(newsocket_fd,&receive_string,99);	
if(read_ret==-1)
{
	perror("read FATAL");
	exit(4);
}else{
	printf("I receive!\n");
	printf("RECV:%s\n",receive_string);
}
/*================*/

return(0);
}
