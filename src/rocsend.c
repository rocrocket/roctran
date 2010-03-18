#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<string.h>
#define DEBUG

int main(int argc,char *argv[])
{

/*socket initial file descriptor for client*/
int socket_fd;

/*socket initialize*/
socket_fd=socket(AF_INET,SOCK_STREAM,0);
if(socket_fd==-1)
{
	perror("socket FATAL");
	exit(1);
}
#ifdef DEBUG
	printf("socket init success\n");
#endif
/*=================*/

/*address setup*/
struct sockaddr_in client_addr;
struct in_addr inp;
char server_ip[]="127.0.0.1";
int inet_ret;
//clear the structure
memset(&client_addr,0,sizeof(struct sockaddr_in));
inet_ret=inet_aton(server_ip,&inp);
if(inet_ret==0)
{
	perror("inet_aton FATAL");
	exit(2);
}
printf("memset success!\n");
client_addr.sin_family=AF_INET;
client_addr.sin_port=htons(21212);
/*inet_aton function's return value is networking sequence,
 * so we do not need to htonl it.*/
client_addr.sin_addr.s_addr=inp.s_addr;
printf("%d\n",inp.s_addr);
printf("%d\n",client_addr.sin_addr.s_addr);
/*==========*/

/*connect operation*/
int conn_res;
conn_res=connect(socket_fd,(struct sockaddr *)&client_addr,sizeof(client_addr));
if(conn_res<0)
{
	printf("connect failed...\n");
}else{
	printf("connect success!\n");
}
/*=================*/

/*send string to server*/
ssize_t write_ret;
char send_string[]="roctran V0.0.1";
write_ret=write(socket_fd,send_string,sizeof(send_string));
if(write_ret==-1)
{
	perror("write FATAL");
	exit(3);
}else if(write_ret==0){
	printf("nothing send\n");
}else{
	printf("send success!\n");
}
/*===================*/

close(socket_fd);
return(0);
}
