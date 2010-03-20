#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<errno.h>
#include<netinet/in.h>
#include<string.h>
#include<libgen.h>

#define MAX_NUM_OPTIONS 48
#define DEBUG

/*global variable*/
//the file path which you want to send
char file_path[1023];
/*===============*/

/*cope with options and parameters*/
int parseOptions(int argc,char *argv[])
{
int _argc;
char *_argv[MAX_NUM_OPTIONS];
int opt;//opt:return value of function "pareseOptions"

if((argc==2)&&(argv[1][0]!='-')){
        //read the specified file
        printf("Wrong command format.\n");
	return(1);
}else{
	int i;
	_argc=(argc>=MAX_NUM_OPTIONS)?(MAX_NUM_OPTIONS-1):argc;
	for(i=0;i<_argc;++i){
		_argv[i]=strdup(argv[i]);
	}
}
optarg=NULL;
while((opt=getopt(_argc,_argv,"f:h"))!=EOF){
	switch(opt){
		case 'f':
			printf("optarg:%s\n",optarg);
			strcpy(file_path,optarg);
			break;
		default:
			printf("Help information");
	}
}
return(0);
}
/*================================*/

int main(int argc,char *argv[])
{
/*cope with options and parameters*/
int parse_ret;
parse_ret=parseOptions(argc,argv);
if(parse_ret!=0){
	printf("parseOptions return %d\n",parse_ret);
	exit(7);
}
/*================================*/

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
#ifdef DEBUG
printf("%d\n",inp.s_addr);
printf("%d\n",client_addr.sin_addr.s_addr);
#endif
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

/*send file name to server*/
printf("file_path:%s\n",file_path);

char send_file_name[1023],*file_name_p;
ssize_t write_ret;
strcpy(send_file_name,file_path);
file_name_p=basename(send_file_name);
printf("send_file_name:%s\n",file_name_p);
write_ret=write(socket_fd,file_name_p,sizeof(send_file_name));
if(write_ret==-1)
{
	perror("write FATAL");
	exit(7);
}
/*========================*/

/*get the file size*/
struct stat file_stat;
int stat_ret;
long long int file_size;
stat_ret=stat(file_path,&file_stat);
if(stat_ret==-1)
{
	perror("stat FATAL");
	exit(3);
}else{
	file_size=file_stat.st_size;
	printf("file size:%lld\n",file_size);	
}
/*=================*/

/*send the file size to server*/
write_ret=write(socket_fd,&file_size,sizeof(file_size));
if(write_ret==-1)
{
	perror("write FATAL");
	exit(8);
}
printf("file size has sent to server\n");
/*============================*/

/*read file content to local buffer*/
FILE *fp;
fp=fopen(file_path,"rb");
if(fp==NULL)
{
	perror("fopen FATAL");
	exit(4);
}

char *file_content;
file_content=(char *)malloc(file_size);
if(file_content==NULL)
{
	perror("malloc FATAL");
	exit(5);
}

size_t fread_ret;
fread_ret=fread(file_content,file_size,1,fp);
if(fread_ret==0)
{
	perror("fread FATAL");
	exit(6);
}else{
	printf("fread success\n");
}
/*===========================*/

/*send file content to server*/
write_ret=write(socket_fd,file_content,file_size);
if(write_ret==-1)
{
	perror("write FATAL");
	exit(7);
}else{
	printf("write content to server success\n");
}
/*===========================*/

fclose(fp);
close(socket_fd);
printf("Goodbye!\n");
return(0);
}
