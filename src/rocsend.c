#include "roctran.h"

/*global variable*/
//the file path which you want to send
char file_path[1023];
/*===============*/

/*usage function*/
void usage()
{
        time_t mytime;
        struct tm *mylocaltime;
        mytime=time(NULL);
        mylocaltime=localtime(&mytime);
	char usage_str[]="rocsend - roctran tool\r\n\
===\n\
usage:\n\
./rocsend -f path/to/file\n\
===\n";
        printf("%s%s%d\n",usage_str,"rocrocket@",mylocaltime->tm_year+1900);
}
/*==============*/

/*cope with options and parameters*/
int parseOptions(int argc,char *argv[])
{
	int _argc;
	char *_argv[MAX_NUM_OPTIONS];
	int opt;//opt:return value of function "pareseOptions"

	if((argc==1)||((argc==2)&&(argv[1][0]!='-'))){
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
				usage();
		}
	}
	return(0);
}
/*================================*/

/*{send one file to server*/
int send_file(int socket_fd,char *dir_path,char *file_path)
{
	/*{get locale file size*/
	struct stat file_stat;
	int stat_ret;
	long long int file_size;

        char absolute_path[1023];      
        char *abs_p=absolute_path;     
        absolute_path[0]='\0';
        abs_p=strncat(absolute_path,dir_path,300);
        abs_p=strncat(absolute_path,"/",10);
        abs_p=strncat(absolute_path,file_path,300);
        printf("abs_p=%s\n",abs_p);

        stat_ret=stat(abs_p,&file_stat);
        if(stat_ret==-1)
        {
                perror("stat FATAL");
                return(2);
        }else{
		/*{get file size*/
                file_size=file_stat.st_size;
                printf("file size:%lld\n",file_size);
		/*=============}*/
        }
	/*====================}*/

	/*{send file type to server*/
	int file_type=1;
        ssize_t write_ret;
        write_ret=write(socket_fd,&file_type,sizeof(file_type));
        if(write_ret==-1)
        {
                perror("write FATAL");
		return(3);
        }
	/*========================}*/

	/*{send file name to server*/
	char send_file_name[1023],*file_name_p;
	strcpy(send_file_name,file_path);
	file_name_p=send_file_name;
	printf("send_file_name:%s\n",file_name_p);

        write_ret=write(socket_fd,file_name_p,sizeof(send_file_name));
        if(write_ret==-1)
        {
                perror("write FATAL");
                return(1);
        }else{
		#ifdef DEBUG
		printf("send file name successfully.\n");
		#endif
	}
        /*========================}*/

	/*{send file size to server*/
        write_ret=write(socket_fd,&file_size,sizeof(file_size));
        if(write_ret==-1)
        {
                perror("write FATAL");
                exit(8);
        }else{
		#ifdef DEBUG
		printf("send file size successfully.\n");
		#endif
	}
	/*========================}*/

	/*{read file content to local buffer*/
	FILE *fp;
	char file_content[BUFSIZ_ROC];
	size_t fread_ret;
	ssize_t read_ret;
	long long int _file_size=file_size;
	long long int send_content_size=0;
	char ack_str[]="ROCGOT",recv_ack[BUFSIZ_ROC];

	printf("fopen file_path=%s\n",file_path);
	fp=fopen(abs_p,"r");
	if(fp==NULL)
	{
		perror("fopen FATAL");
		return(4);
	}

	while(1){
		//read local file
		if(_file_size>BUFSIZ_ROC){
			send_content_size=BUFSIZ_ROC;
			fread_ret=fread(file_content,sizeof(file_content),1,fp);
		}else{
			send_content_size=_file_size;
			fread_ret=fread(file_content,_file_size,1,fp);
		}
		if(fread_ret>0){
			//send content size to remote server
			write_ret=write(socket_fd,&send_content_size,sizeof(send_content_size));
			if(write_ret==-1)
			{
				perror("write FATAL");
				return(5);
			}
			//send content to remote server
			write_ret=write(socket_fd,file_content,send_content_size);
			if(write_ret==-1)
			{
				perror("write FATAL");
				return(6);
			}else{
				_file_size-=BUFSIZ_ROC;
				printf("send to server %d bytes.\n",write_ret);
			}
			//recv ack
			read_ret=read(socket_fd,recv_ack,sizeof(recv_ack));
			if(read_ret==-1){
				perror("read FATAL");
				return(7);
			}else{
				if(strcmp(recv_ack,ack_str)==0){
					printf("receive ROCGOT\n");
				}else{
					printf("ERROR:do not receive ROCGOT\n");
					return(8);
				}
			}
		}else if(fread_ret==0){
			printf("Has be end of the file\n");
			fclose(fp);
			break;
		}
	}
	/*===========================}*/

}
/*=======================}*/

/*{send one directory to server*/
int send_one_dir(int socket_fd,char *base_dir_path,char *dir_path)
{
        /*{send file type to server*/
	int file_type=2;//1:folder type
        ssize_t write_ret;
        write_ret=write(socket_fd,&file_type,sizeof(file_type));
        if(write_ret==-1)
        {
                perror("write FATAL");
                return(3);
        }
        /*========================}*/

        /*{send dir name to server*/
        printf("dir_path:%s\n",dir_path);
                        
        char send_dir_name[1023],*dir_name_p;
        strcpy(send_dir_name,dir_path);
        dir_name_p=send_dir_name;
        printf("send_file_name:%s\n",dir_name_p);
        write_ret=write(socket_fd,dir_name_p,sizeof(send_dir_name));
        if(write_ret==-1)
        {
                perror("write FATAL");
                return(1);
        }
        /*========================}*/

	/*{recv ack*/
	char ack_str[]="ROCGOT",recv_ack[BUFSIZ_ROC];
	int read_ret;
	read_ret=read(socket_fd,recv_ack,sizeof(recv_ack));
	if(read_ret==-1){
		perror("read FATAL");
		return(7);
	}else{
		if(strcmp(recv_ack,ack_str)==0){
			printf("receive ROCGOT\n");
			return(0);
		}else{
			printf("ERROR:do not receive ROCGOT\n");
			return(8);
		}
	}
	/*========}*/
}
/*============================}*/

/*{send finish signal*/
int send_finish_signal(int socket_fd)
{
	printf("sending finish signal");
	int file_type=88;
	int write_ret;

	/*send file type to server*/
	write_ret=write(socket_fd,&file_type,sizeof(file_type)); 
	if(write_ret==-1)
	{
		perror("write FATAL");
		return(2);
	}

	/*{recv ack*/
	char ack_str[]="ROCGOT",recv_ack[BUFSIZ_ROC];
	int read_ret;
	read_ret=read(socket_fd,recv_ack,sizeof(recv_ack));
	if(read_ret==-1){
		perror("read FATAL");
		return(3);
	}else{
		if(strcmp(recv_ack,ack_str)==0){
                        printf("receive ROCGOT\n");    
                        return(0);                     
                }else{
                        printf("ERROR:do not receive ROCGOT\n");
                        return(8);                     
                }
        }
	/*========}*/
}
/*==================}*/

/*{send file or folder*/
int send_dir(int socket_fd,char *dir_path,char *file_path)
{
	DIR *dp;
	struct dirent *dirp;

	struct stat file_stat;
	int stat_ret;
	int file_type=99;
	/* 1:regular file
	 * 2:folder file
	 * 3:soft link file
	 * 4:pipe file
	 * 88:transfer finish
	 * 99:unknown type*/
	long long int file_size;

	char absolute_path[1023];
	char *abs_p=absolute_path;
	absolute_path[0]='\0';
	abs_p=strncat(absolute_path,dir_path,300);
	abs_p=strncat(absolute_path,"/",10);
	abs_p=strncat(absolute_path,file_path,300);
	printf("abs_p=%s\n",abs_p);

	stat_ret=stat(abs_p,&file_stat);
	if(stat_ret==-1){
		perror("stat FATAL");
		return(2);
	}else{
                /*{get file type*/
                if(S_ISREG(file_stat.st_mode)){
                        file_type=1;
                }else if(S_ISDIR(file_stat.st_mode)){
                        file_type=2;
                }else if(S_ISLNK(file_stat.st_mode)){
                        file_type=3;
                }else if(S_ISFIFO(file_stat.st_mode)){
                        file_type=4;
                }else{
                        file_type=99;
                }
                printf("file type:%d\n",file_type);
                /*=============}*/	
	}

	/*recursive send file and folder to server*/
	if(file_type==2){
		int send_one_dir_ret;
		send_one_dir_ret=send_one_dir(socket_fd,dir_path,file_path);
		if(send_one_dir_ret!=0){
			printf("send_one_dir FATAL\n");
			return(3);
		}
		if((dp=opendir(abs_p))==NULL){
			perror("opendir error");
			return(4);
		}			
		while((dirp=readdir(dp))!=NULL){
			if((strcmp(dirp->d_name,".")==0)||(strcmp(dirp->d_name,"..")==0)){
				continue;	
			}else{
				char sub_file_path[1023];
				char *sub_p=sub_file_path;
				sub_file_path[0]='\0';
				int send_dir_ret;
				sub_p=strncat(sub_p,file_path,300);
				sub_p=strncat(sub_p,"/",10);
				sub_p=strncat(sub_p,dirp->d_name,300);

				#ifdef DEBUG
				printf("sub_p=%s\n",sub_p);
				#endif

				/*{recursion*/
				send_dir_ret=send_dir(socket_fd,dir_path,sub_file_path);
				if(send_dir_ret!=0){
					printf("send_dir FATAL\n");	
					return(4);
				}
				/*=========}*/
			}
		}
	}else if(file_type==1){
		send_file(socket_fd,dir_path,file_path);
		return(0);
	}
}

int main(int argc,char *argv[])
{
	/*cope with options and parameters*/
	int parse_ret;
	parse_ret=parseOptions(argc,argv);
	if(parse_ret!=0){
		usage();
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
	client_addr.sin_family=AF_INET;
	client_addr.sin_port=htons(21212);
	/*inet_aton function's return value is networking sequence,
	 * so we do not need to htonl it.*/
	client_addr.sin_addr.s_addr=inp.s_addr;
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

	/*{get dir_path and file_path*/
	char send_dir_path[1023],send_file_path[1023];
	char *dir_path_p,*file_path_p;
	strcpy(send_file_path,file_path);
	file_path_p=basename(send_file_path);
	strcpy(send_dir_path,file_path);
	dir_path_p=dirname(send_dir_path);
	/*==========================}*/

	printf("send_dir_path:%s\n",dir_path_p);
	printf("send_file_path:%s\n",file_path_p);

	/*{send file or folder to server*/
	int send_dir_ret;
	send_dir_ret=send_dir(socket_fd,dir_path_p,file_path_p);
	if(send_dir_ret!=0){
		printf("send_dir FATAL\n");
		exit(3);
	}
	/*=============================}*/

	/*{send finish signal to server*/
	int send_finish_signal_ret;
	send_finish_signal_ret=send_finish_signal(socket_fd);
	if(send_finish_signal_ret!=0){
		printf("send_finish_signal FATAL\n");
		exit(4);
	}
	/*============================}*/

	close(socket_fd);
	printf("Goodbye!\n");
	return(0);
}
