#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<errno.h>

#define DEBUG

/*function:receiver one file from client
 * param1:file descriptor
 * param2:the local storing path
*/
int receive_file(int file_des,char *local_path)
{
/*read file name from client*/
char file_name[1023],*str_p;
str_p=file_name;
ssize_t read_ret;
read_ret=read(file_des,str_p,sizeof(file_name));
if(read_ret==-1)
{
	perror("read FATAL");
	return(1);
}else{
	#ifdef DEBUG
	printf("filename:%s\n",str_p);
	#endif
}
/*==========================*/

/*concatenate two part into one file path*/
printf("string operation start.\n");
char file_path[200];
char *path_p;
file_path[0]='\0';
path_p=file_path;
#ifdef DEBUG
printf("path_p1:%s\n",path_p);
#endif

path_p=strncat(path_p,local_path,49);
#ifdef DEBUG
printf("path_p2:%s\n",path_p);
#endif

path_p=strncat(path_p,"/",10);
#ifdef DEBUG
printf("path_p3:%s\n",path_p);
#endif

path_p=strncat(path_p,str_p,50);
#ifdef DEBUG
printf("path_p4:%s\n",path_p);
#endif
/*=======================================*/

/*open local new file*/
FILE *fp;
fp=fopen(path_p,"wb");
if(fp==NULL)
{
	perror("fopen FATAL");
	return(2);
}else{
	#ifdef DEBUG
	printf("fopen success!\n");
	#endif
}
/*===================*/

/*read file size from client*/
long long int file_size;
read_ret=read(file_des,&file_size,sizeof(file_size));
if(read_ret==-1)
{
	perror("read FATAL");
	return(3);
}else{
	#ifdef DEBUG
	printf("file size:%lld\n",file_size);
	#endif
}
/*==========================*/

/*read file content from client*/
char *file_buf;
file_buf=(char *)malloc(file_size);
if(file_buf==NULL)
{
	perror("malloc FATAL");
	return(4);
}else{
	#ifdef DEBUG
	printf("malloc success!\n");
	#endif
}
read_ret=read(file_des,file_buf,file_size);
if(read_ret==-1)
{
	perror("read FATAL");
	return(5);
}else{
	#ifdef DEBUG
	//printf("file content:%s\n",file_buf);
	#endif
}
/*=============================*/

/*write to local file*/
size_t fwrite_ret;
fwrite_ret=fwrite(file_buf,file_size,1,fp);
if(fwrite_ret==0)
{
	perror("fwrite FATAL");
	return(6);
}
/*===================*/

/*close local file*/
int fclose_ret;
fclose_ret=fclose(fp);
if(fclose_ret!=0)
{
	perror("flcose FATAL");
	return(7);
}else{
	#ifdef DEBUG
	printf("fclose success!\n");
	#endif
}
/*================*/
return(0);
}

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
//set socket address
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

/*receive file*/
int receive_ret;
receive_ret=receive_file(newsocket_fd,".");
if(receive_ret!=0)
{
	printf("receive_file failed.ret=%d\n",receive_ret);
	exit(5);
}
/*============*/

/*close the fd*/
close(newsocket_fd);
close(socket_fd);
/*============*/
printf("good bye!\n");
return(0);
}
