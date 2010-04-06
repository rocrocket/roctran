#include "roctran.h"

/*global variable*/

//the file path which you want to send
char file_path[1023];

//the server IP address
char server_ip[20];

//the server port number
int server_port=0;

//socket initial file descriptor for client
int socket_fd;

//total send bytes
long long int total_send_bytes=0;

//total receive bytes
long long int total_recv_bytes=0;

//total file number
int total_file_number=0;

//total dir number
int total_dir_number=0;

//the consult message
struct consult_message send_consult={0,0,0,0,0,0,0,0,0,0,0,0};
uid_t file_owner=0;
gid_t file_group=0;
/*===============*/

/*usage function*/
void send_usage()
{
        time_t mytime;
        struct tm *mylocaltime;
        mytime=time(NULL);
        mylocaltime=localtime(&mytime);
	char usage_str[]="rocsend - roctran tool\n\
	\r===\n\
	\rusage:\n\
	\r./rocsend -f path/to/file -p server_port -i server_ip [-a] [-o] [-g]\n\
	\r-f:the local file path\n\
	\r-p:the server port\n\
	\r-i:the server IP or domain name\n\
	\r-a:keep the permission of the file\n\
	\r-o:keep the owner of file\n\
	\r-g:keep the group of file\n\
	\r===\n";
        printf("%s%s%d\n",usage_str,"rocrocket@",mylocaltime->tm_year+1900);
}
/*==============*/

/*{init env*/
int init_env()
{
	/*{initialize the consult message structure*/
	send_consult.owner_r=0;
	send_consult.owner_w=0;
	send_consult.owner_x=0;
	send_consult.group_r=0;
	send_consult.group_w=0;
	send_consult.group_x=0;
	send_consult.other_r=0;
	send_consult.other_w=0;
	send_consult.other_x=0;
	/*========================================}*/
	return(0);
}
/*========}*/

/*{signal catch*/
void finishup(int signo)
{
	sleep(1);
	close(socket_fd);		
	printf("\n");	
	if(signo!=0){
		printf("Receive Ctrl-C.\n");	
	}
	printf("====STATISTICS====\n");	
	printf("total file number:%d\n",total_file_number);	
	printf("total dir  number:%d\n",total_dir_number);	
	printf("total send bytes :%lld\n",total_send_bytes);	
	printf("total recv bytes :%lld\n",total_recv_bytes);	
	printf("=====GOODBYE!=====\n");	
	exit(0);
}
/*============}*/

/*cope with options and parameters*/
int parseOptions(int argc,char *argv[])
{
	int f_flag=0,p_flag=0,i_flag=0;
	int _argc;
	char *_argv[MAX_NUM_OPTIONS];
	int opt;//opt:return value of function "parseOptions"

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
	while((opt=getopt(_argc,_argv,"f:p:i:hoga"))!=EOF){
		switch(opt){
			//local file path
			case 'f':
				#ifdef DEBUG
				printf("-f:%s\n",optarg);
				#endif
				strcpy(file_path,optarg);
				f_flag=1;
				break;
			//server port
			case 'p':
				#ifdef DEBUG
				printf("-p:%s\n",optarg);
				#endif
				server_port=atoi(optarg);
				p_flag=1;
				break;
			//server ip
			case 'i':
				#ifdef DEBUG
				printf("-i:%s\n",optarg);
				#endif
				strcpy(server_ip,optarg);
				i_flag=1;
				break;
			//keep the owner of the file
			case 'o':
				send_consult.keep_owner=1;
				break;
			//keep the group of the file
			case 'g':
				send_consult.keep_group=1;
				break;
			//keep the permission of the file
			case 'a':
				send_consult.keep_permission=1;
				break;
			default:
				return(2);
		}
	}
	if((p_flag==0)||(i_flag==0)||(f_flag==0)){
		return(3);
	}
	return(0);
}
/*================================*/

/*{send one file to server*/
int send_file(int socket_fd,char *dir_path,char *file_path)
{
	#ifdef DEBUG
		printf("go into send_file funtion.\n");
	#endif

	/*{send consult_message to server*/
	ssize_t write_ret;
	write_ret=write(socket_fd,&send_consult,sizeof(send_consult));
	if(write_ret==-1){
		perror("write FATAL");
		return(23);
	}
	/*==============================}*/

	/*{get local file size*/
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
                return(4);
        }else{
		/*{get file size*/
                file_size=file_stat.st_size;
                printf("file size:%lld\n",file_size);
		/*=============}*/
        }
	/*====================}*/

	/*{send file type to server*/
	int file_type=1;
        write_ret=write(socket_fd,&file_type,sizeof(file_type));
        if(write_ret==-1)
        {
                perror("write FATAL");
		return(5);
        }else{
		#ifdef DEBUG
		printf("send file type to server successfully.\n");
		#endif
		total_send_bytes+=write_ret;
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
                return(6);
        }else{
		#ifdef DEBUG
		printf("send file name successfully.\n");
		#endif
		total_send_bytes+=write_ret;
	}
        /*========================}*/

	/*{if need,send owner info to server*/
	if(send_consult.keep_owner==1){
		write_ret=write(socket_fd,&file_owner,sizeof(file_owner));
		if(write_ret==-1)
		{
			perror("write FATAL");
			return(28);
		}
	}
	/*=================================}*/

	/*{if need,send group info to server*/
	if(send_consult.keep_group==1){
		write_ret=write(socket_fd,&file_group,sizeof(file_group));
		if(write_ret==-1)
		{
			perror("write FATAL");
			return(29);
		}
	}
	/*=================================}*/

	/*{send file size to server*/
        write_ret=write(socket_fd,&file_size,sizeof(file_size));
        if(write_ret==-1)
        {
                perror("write FATAL");
                return(7);
        }else{
		#ifdef DEBUG
		printf("send file size successfully.\n");
		#endif
		total_send_bytes+=write_ret;
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
		return(8);
	}

	while(1){
		//read local file
		if(_file_size>BUFSIZ_ROC){
			send_content_size=BUFSIZ_ROC;
			fread_ret=fread(file_content,sizeof(file_content),1,fp);
		}else if(_file_size<=0){
			_file_size=0;
			fread_ret=0;
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
				return(9);
			}else{
				total_send_bytes+=write_ret;
			}
			//send content to remote server
			write_ret=write(socket_fd,file_content,send_content_size);
			if(write_ret==-1)
			{
				perror("write FATAL");
				return(10);
			}else{
				_file_size-=BUFSIZ_ROC;
				#ifdef DEBUG
				printf("send to server %d bytes.\n",write_ret);
				#endif
				total_send_bytes+=write_ret;
			}
			//recv ack
			read_ret=read(socket_fd,recv_ack,sizeof(recv_ack));
			if(read_ret==-1){
				perror("read FATAL");
				return(11);
			}else{
				total_recv_bytes+=read_ret;
				if(strcmp(recv_ack,ack_str)==0){
					#ifdef DEBUG
					printf("receive ROCGOT\n");
					#endif
				}else{
					printf("ERROR:do not receive ROCGOT\n");
					return(12);
				}
			}
		}else if(fread_ret==0){
			fclose(fp);
			printf("Has be end of the file\n");
			break;
			printf("I will break!\n");
		}
		printf("I will break!!\n");
	}
	/*===========================}*/
	printf("I will break!!!\n");
	#ifdef DEBUG
		printf("Go out of the send_file\n");
	#endif
	return(0);
}
/*=======================}*/

/*{send one directory to server*/
int send_one_dir(int socket_fd,char *base_dir_path,char *dir_path)
{
	/*{send consult_message to server*/
        ssize_t write_ret;
	write_ret=write(socket_fd,&send_consult,sizeof(send_consult));
	if(write_ret==-1){
		perror("write FATAL");
		return(33);
	}
	/*==============================}*/

        /*{send file type to server*/
	int file_type=2;//1:folder type
        write_ret=write(socket_fd,&file_type,sizeof(file_type));
        if(write_ret==-1)
        {
                perror("write FATAL");
                return(13);
        }else{
		total_send_bytes+=write_ret;
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
                return(14);
        }else{
		total_send_bytes+=write_ret;
	}
        /*========================}*/

	/*{if need,send owner info to server*/
	if(send_consult.keep_owner==1){
		write_ret=write(socket_fd,&file_owner,sizeof(file_owner));
		if(write_ret==-1)
		{
			perror("write FATAL");
			return(15);
		}
	}
	/*=================================}*/

	/*{if need,send group info to server*/
	if(send_consult.keep_group==1){
		write_ret=write(socket_fd,&file_group,sizeof(file_group));
		if(write_ret==-1)
		{
			perror("write FATAL");
			return(16);
		}
	}
	/*=================================}*/

	/*{recv ack*/
	char ack_str[]="ROCGOT",recv_ack[BUFSIZ_ROC];
	int read_ret;
	read_ret=read(socket_fd,recv_ack,sizeof(recv_ack));
	if(read_ret==-1){
		perror("read FATAL");
		return(17);
	}else{
		total_recv_bytes+=read_ret;
		if(strcmp(recv_ack,ack_str)==0){
			printf("receive ROCGOT\n");
			return(0);
		}else{
			printf("ERROR:do not receive ROCGOT\n");
			return(18);
		}
	}
	/*========}*/
}
/*============================}*/

/*{send finish signal*/
int send_finish_signal(int socket_fd)
{
	printf("sending finish signal\n");
	int file_type=88;
	ssize_t write_ret;

	/*{send consult_message to server*/
	write_ret=write(socket_fd,&send_consult,sizeof(send_consult));
	if(write_ret==-1){
		perror("write FATAL");
		return(23);
	}
	/*==============================}*/

	/*send file type to server*/
	write_ret=write(socket_fd,&file_type,sizeof(file_type)); 
	if(write_ret==-1)
	{
		perror("write FATAL");
		return(19);
	}else{
		total_send_bytes+=write_ret;
	}

	/*{recv ack*/
	char ack_str[]="ROCGOT",recv_ack[BUFSIZ_ROC];
	int read_ret;
	read_ret=read(socket_fd,recv_ack,sizeof(recv_ack));
	if(read_ret==-1){
		perror("read FATAL");
		return(20);
	}else{
		total_recv_bytes+=read_ret;
		if(strcmp(recv_ack,ack_str)==0){
                        printf("receive ROCGOT\n");    
                        return(0);                     
                }else{
                        printf("ERROR:do not receive ROCGOT\n");
                        return(21);                     
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

	int init_ret;
	init_ret=init_env();
	if(init_ret!=0){
		printf("init_env FATAL\n");
		return(40);
	}

	stat_ret=stat(abs_p,&file_stat);
	if(stat_ret==-1){
		perror("stat FATAL");
		return(22);
	}else{
		mode_t per_mode=file_stat.st_mode;
                /*{get file type*/
                if(S_ISREG(per_mode)){
                        file_type=1;
                }else if(S_ISDIR(per_mode)){
                        file_type=2;
                }else if(S_ISLNK(per_mode)){
                        file_type=3;
                }else if(S_ISFIFO(per_mode)){
                        file_type=4;
                }else{
                        file_type=99;
                }
                printf("file type:%d\n",file_type);
                /*=============}*/	

		/*{get the owner and group*/
		if(send_consult.keep_owner==1)
			file_owner=file_stat.st_uid;		
		if(send_consult.keep_group==1)
			file_group=file_stat.st_gid;		
		/*=======================}*/

		/*{get the permission*/
		if(send_consult.keep_permission==1){
			//user permission
			if(per_mode & S_IRUSR)
				send_consult.owner_r=1;
			if(per_mode & S_IWUSR)
				send_consult.owner_w=1;
			if(per_mode & S_IXUSR)
				send_consult.owner_x=1;

			//group permission
			if(per_mode & S_IRGRP)
				send_consult.group_r=1;
			if(per_mode & S_IWGRP)
				send_consult.group_w=1;
			if(per_mode & S_IXGRP)
				send_consult.group_x=1;
			
			//other permission
			if(per_mode & S_IROTH)	
				send_consult.other_r=1;
			if(per_mode & S_IWOTH)
				send_consult.other_w=1;
			if(per_mode & S_IXOTH)
				send_consult.other_x=1;
		}
		/*=======================}*/
                #ifdef DEBUG
                        printf("%u %u %u, %u %u %u %u %u %u %u %u %u\n",send_consult.keep_owner,send_consult.keep_group,send_consult.keep_permission,send_consult.owner_r,send_consult.owner_w,send_consult.owner_x,send_consult.group_r,send_consult.group_w,send_consult.group_x,send_consult.other_r,send_consult.other_w,send_consult.other_x);
                #endif
	}

	/*recursive send file and folder to server*/
	if(file_type==2){
		int send_one_dir_ret;
		send_one_dir_ret=send_one_dir(socket_fd,dir_path,file_path);
		if(send_one_dir_ret!=0){
			printf("send_one_dir FATAL\n");
			return(24);
		}else{
			total_dir_number+=1;
		}
		if((dp=opendir(abs_p))==NULL){
			perror("opendir error");
			return(25);
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
					return(26);
				}
				/*=========}*/
			}
		}
	}else if(file_type==1){
		#ifdef DEBUG
			printf("going to call send_file\n");
		#endif
		int send_file_ret;
		send_file_ret=send_file(socket_fd,dir_path,file_path);
		if(send_file_ret!=0){
			printf("send_file return value:%d\n",send_file_ret);
			return(27);
		}else{
			#ifdef DEBUG
				printf("send_file successfully\n");
			#endif
			total_file_number+=1;
		}
		return(0);
	}
	/*you can extend file type here.*/
}

int main(int argc,char *argv[])
{
	signal(SIGINT,finishup);	

	/*cope with options and parameters*/
	int parse_ret;
	parse_ret=parseOptions(argc,argv);
	if(parse_ret!=0){
		send_usage();
		exit(7);
	}
	/*================================*/


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
	}else{
		#ifdef DEBUG
			printf("send_dir return 0\n");
		#endif
	}
	/*=============================}*/

	/*{send finish signal to server*/
	int send_finish_signal_ret;
	send_finish_signal_ret=send_finish_signal(socket_fd);
	if(send_finish_signal_ret!=0){
		printf("send_finish_signal FATAL\n");
		exit(4);
	}else{
		#ifdef DEBUG
			printf("send_finish_signal return 0\n");
		#endif
	}
	/*============================}*/

	finishup(0);
	return(0);
}
