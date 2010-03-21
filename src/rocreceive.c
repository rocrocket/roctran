#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<errno.h>

#define BUFSIZ_ROC BUFSIZ
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
char file_path[200];
char *path_p;
file_path[0]='\0';
path_p=file_path;
path_p=strncat(path_p,local_path,49);
path_p=strncat(path_p,"/",10);
path_p=strncat(path_p,str_p,50);
/*=======================================*/

/*open local new file*/
FILE *fp;
fp=fopen(path_p,"w");
if(fp==NULL)
{
	perror("fopen FATAL");
	return(2);
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

/*repeat read remotely and write locally*/
size_t fwrite_ret;
ssize_t write_ret;
char file_buf[BUFSIZ_ROC];
int fclose_ret;
char ack_str[]="ROCGOT";
long long int _file_size=file_size;
long long int recv_content_size=0;
while(1){
	if(_file_size>0){
		//read content size
		read_ret=read(file_des,&recv_content_size,sizeof(recv_content_size));
		if(read_ret==-1){
			perror("read FATAL");
			return(9);
		}
		//read content
		read_ret=read(file_des,file_buf,recv_content_size);
		fwrite_ret=fwrite(file_buf,read_ret,1,fp);
		if(fwrite_ret==0){
			perror("fwrite FATAL");
			return(6);
		}
		_file_size-=read_ret;
		printf("read_ret=%d,_file_size=%d\n",read_ret,_file_size);
		//send ack
		write_ret=write(file_des,ack_str,sizeof(ack_str));
		if(write_ret==-1){
			perror("write FATAL");
			return(7);
		}

	}else if(_file_size==0){
		fclose_ret=fclose(fp);
		if(fclose_ret!=0)
		{
			perror("flcose FATAL");
			return(8);
		}else{
			#ifdef DEBUG
			printf("fclose success!\n");
			#endif
			break;
		}
	}else{
		printf("Error happened during transfer.");
		return(9);
	}
}
/*======================================*/
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
