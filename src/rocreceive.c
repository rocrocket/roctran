#include "roctran.h"

/*global variable*/

//listen port
int g_listen_port=0;

//socket descriptor
int socket_fd;

//client number record	
int all_client_num=0;
/*===============*/

/*usage function*/
void receive_usage()
{
	time_t mytime;
        struct tm *mylocaltime;
	mytime=time(NULL);
        mylocaltime=localtime(&mytime);
	char usage_str[]="rocreceive - roctran tool\n\
	\r===\n\
	\rusage:\n\
	\r./rocreceive -p PORT\n\
	\r===\n";
	printf("%s%s%d\n",usage_str,"rocrocket@",mylocaltime->tm_year+1900);
	
}
/*==============*/

/*{signal catch*/
void finishup(int signo)
{
	close(socket_fd);
	printf("\n");
	printf("====STATISICS====\n");
	printf("client number:%d\n",all_client_num);
	printf("=====GOODBYE=====\n");
	exit(0);
}
/*============}*/

/*cope with options and parameters*/
int parseOptions(int argc,char *argv[])
{
        int _argc;
        char *_argv[MAX_NUM_OPTIONS];
        int opt;//opt:return value of function "pareseOptions"

        if((argc==1)||((argc==2)&&(argv[1][0]!='-'))){
                //read the specified file
                return(1);
        }else{  
                int i;
                _argc=(argc>=MAX_NUM_OPTIONS)?(MAX_NUM_OPTIONS-1):argc;
                for(i=0;i<_argc;++i){
                        _argv[i]=strdup(argv[i]);
                }
        }
        optarg=NULL;
        while((opt=getopt(_argc,_argv,"p:h"))!=EOF){
                switch(opt){
                        case 'p':
				g_listen_port=atoi(optarg);
                                printf("listen port:%d\n",g_listen_port);
                                break;
                        default:
				return(2);
                }
        }
        return(0);
}
/*================================*/

/*{send acknowledge to client*/
int send_ack(int socket_fd)
{
	char ack_str[]="ROCGOT";
	int write_ret;
	write_ret=write(socket_fd,ack_str,sizeof(ack_str));	
	if(write_ret==-1){             
		perror("write FATAL"); 
		return(3);
	}
	return(0);
}
/*==========================}*/

/*function:receiver one file from client
 * param1:file descriptor
 * param2:the local storing file path
 * return value:
 * 	 0:transfer correct and go on.
 * 	88:transfer finish.
*/
int receive_file(int socket_fd,char *local_path)
{
	ssize_t read_ret;

	/*{read a consult message from client*/	
	struct consult_message receive_consult;
	read_ret=read(socket_fd,&receive_consult,sizeof(receive_consult));
	if(read_ret==-1){
		perror("read FATAL");
		return(4);
	}else{
		#ifdef DEBUG
			printf("%u %u %u, %u %u %u %u %u %u %u %u %u\n",receive_consult.keep_owner,receive_consult.keep_group,receive_consult.keep_permission,receive_consult.owner_r,receive_consult.owner_w,receive_consult.owner_x,receive_consult.group_r,receive_consult.group_w,receive_consult.group_x,receive_consult.other_r,receive_consult.other_w,receive_consult.other_x);
		#endif
	}
	/*===================================}*/

	/*{read file type from client*/
	int file_type=99;
        /* 1:regular file
	 * 2:folder file
	 * 3:soft link file
	 * 4:pipe file
	 * 88:transfer finish
	 * 99:unknown type*/

	read_ret=read(socket_fd,&file_type,sizeof(file_type));	
	if(read_ret==-1)
	{
		perror("read FATAL");
		return(5);
	}else{
		#ifdef DEBUG
		printf("file type:%d\n",file_type);
		#endif

		if(file_type==88){
			//send ack
                        int send_ack_ret;              
                        send_ack_ret=send_ack(socket_fd);
                        if(send_ack_ret!=0){           
                                printf("send_ack FATAL");      
                                return(6);                     
                        }
			/*transfer finish*/
			return(88);	
		}
	}
	/*==========================}*/
	
	/*read file name from client*/
	char file_name[1023],*str_p;
	str_p=file_name;
	read_ret=read(socket_fd,str_p,sizeof(file_name));
	if(read_ret==-1)
	{
		perror("read FATAL");
		return(7);
	}else{
		#ifdef DEBUG
		printf("file name:%s\n",str_p);
		#endif
	}
	/*==========================*/

	/*concatenate two part into one file path*/
	char file_path[1023];
	char *path_p;
	file_path[0]='\0';
	path_p=file_path;
	path_p=strncat(path_p,local_path,300);
	path_p=strncat(path_p,"/",10);
	path_p=strncat(path_p,str_p,300);
	/*=======================================*/

	/*{if need,read owner info from client*/
	uid_t file_owner_id=0;
	if(receive_consult.keep_owner==1){
		read_ret=read(socket_fd,&file_owner_id,sizeof(file_owner_id));
		if(read_ret==-1){
			perror("read FATAL");
			return(8);
		}else{
			#ifdef DEBUG
				printf("file_owner_id=%u\n",file_owner_id);	
			#endif
		}
	}
	/*===================================}*/

	/*{if need,read owner info from client*/
	gid_t file_group_id;
	if(receive_consult.keep_group==1){
		read_ret=read(socket_fd,&file_group_id,sizeof(file_group_id));
		if(read_ret==-1){
			perror("read FATAL");
			return(9);
		}else{
			#ifdef DEBUG
				printf("file_group_id=%u\n",file_group_id);	
			#endif
		}
	}
	/*===================================}*/

	/*{if need,process permission*/
	mode_t permission_mode=0;
	#ifdef DEBUG
		printf("1:permission_mode=%o\n",permission_mode);	
	#endif

	//initial permission_mode
	if(file_type==1){
		permission_mode=S_IFREG;
		#ifdef DEBUG
			printf("2:permission_mode=%o\n",permission_mode);	
		#endif
	}else if(file_type==2){
		permission_mode=S_IFDIR;
		#ifdef DEBUG
			printf("3:permission_mode=%o\n",permission_mode);	
		#endif
	}

	if(receive_consult.keep_permission==1){
		#ifdef DEBUG
			printf("%d %d %d, %d %d %d %d %d %d %d %d %d\n",receive_consult.keep_owner,receive_consult.keep_group,receive_consult.keep_permission,receive_consult.owner_r,receive_consult.owner_w,receive_consult.owner_x,receive_consult.group_r,receive_consult.group_w,receive_consult.group_x,receive_consult.other_r,receive_consult.other_w,receive_consult.other_x);
		#endif
		//owner permission
		if(receive_consult.owner_r==1){
			permission_mode|=S_IRUSR;
			#ifdef DEBUG
				printf("4:permission_mode=%o\n",permission_mode);	
			#endif
		}
		if(receive_consult.owner_w==1){
			permission_mode|=S_IWUSR;
			#ifdef DEBUG
				printf("5:permission_mode=%o\n",permission_mode);	
			#endif
		}
		if(receive_consult.owner_x==1){
			permission_mode|=S_IXUSR;
			#ifdef DEBUG
				printf("6:permission_mode=%o\n",permission_mode);	
			#endif
		}
			
		//group permission
		if(receive_consult.group_r==1){
			permission_mode|=S_IRGRP;
			#ifdef DEBUG
				printf("7:permission_mode=%o\n",permission_mode);	
			#endif
		}
		if(receive_consult.group_w==1){
			permission_mode|=S_IWGRP;
			#ifdef DEBUG
				printf("8:permission_mode=%o\n",permission_mode);	
			#endif
		}
		if(receive_consult.group_x==1){
			permission_mode|=S_IXGRP;
			#ifdef DEBUG
				printf("9:permission_mode=%o\n",permission_mode);	
			#endif
		}

		//other permission
		if(receive_consult.other_r==1){
			permission_mode|=S_IROTH;
			#ifdef DEBUG
				printf("10:permission_mode=%o\n",permission_mode);	
			#endif
		}
		if(receive_consult.other_w==1){
			permission_mode|=S_IWOTH;
			#ifdef DEBUG
				printf("11:permission_mode=%o\n",permission_mode);	
			#endif
		}
		if(receive_consult.other_x==1){
			permission_mode|=S_IXOTH;
			#ifdef DEBUG
				printf("12:permission_mode=%o\n",permission_mode);	
			#endif
		}
	}else{
		permission_mode=S_IRUSR | S_IWUSR | S_IXUSR;
	}
	/*========================================}*/

	/*{when file type is folder*/
	if(file_type==2){
		int mkdir_ret;
		mkdir_ret=mkdir(file_path,permission_mode);
		if(mkdir_ret==-1){
			if(errno!=EEXIST){
				perror("mkdir FATAL");
				return(10);	
			}
		}
		
		//set owner and grouper
		int chown_ret;
		if(receive_consult.keep_owner==1){
			chown_ret=chown(file_path,file_owner_id,-1);
			if(chown_ret==-1){
				perror("chown FATAL");
				return(11); 
			}
		}
		if(receive_consult.keep_group==1){
			chown_ret=chown(file_path,-1,file_group_id);
			if(chown_ret==-1){
				perror("chown FATAL");
				return(12); 
			}
		}

		//send_ack
		int send_ack_ret;              
		send_ack_ret=send_ack(socket_fd);
		if(send_ack_ret!=0){           
			printf("send_ack FATAL");      
			return(13);
		}else{
			return(0);
		}
		
	}
	/*========================}*/

	/*open local new file*/
	FILE *fp;
	#ifdef DEBUG
	printf("path_p=%s\n",path_p);
	#endif
	fp=fopen(path_p,"w");
	if(fp==NULL){
		perror("fopen FATAL");
		return(14);
	}
	/*===================*/

	/*read file size from client*/
	long long int file_size;
	read_ret=read(socket_fd,&file_size,sizeof(file_size));
	if(read_ret==-1)
	{
		perror("read FATAL");
		return(15);
	}else{
		#ifdef DEBUG
		printf("file size:%lld\n",file_size);
		#endif
	}
	/*==========================*/

	/*{repeat read remotely and write locally*/
	size_t fwrite_ret;
	ssize_t write_ret;
	char file_buf[BUFSIZ_ROC];
	int fclose_ret;
	long long int _file_size=file_size;
	long long int recv_content_size=0;
	while(1){
		if(_file_size>0){
			//read content size
			read_ret=read(socket_fd,&recv_content_size,sizeof(recv_content_size));
			if(read_ret==-1){
				perror("read FATAL");
				return(16);
			}else{
				#ifdef DEBUG
				printf("recv_content_size=%d\n",recv_content_size);
				#endif
			}
			//read content
			read_ret=read(socket_fd,file_buf,recv_content_size);
			fwrite_ret=fwrite(file_buf,read_ret,1,fp);
			if(fwrite_ret==0){
				perror("fwrite FATAL");
				return(17);
			}else{
				#ifdef DEBUG
				printf("fwrite locally successfully\n");
				#endif
			}
			_file_size-=read_ret;
			//send ack
			int send_ack_ret;
			send_ack_ret=send_ack(socket_fd);
			if(send_ack_ret!=0){
				printf("send_ack FATAL");
				return(18);
			}else{
				#ifdef DEBUG
				printf("send ack\n");
				#endif	
			}
		}else if(_file_size==0){
			fclose_ret=fclose(fp);
			if(fclose_ret!=0)
			{
				perror("flcose FATAL");
				return(19);
			}else{
				#ifdef DEBUG
				printf("fclose success!\n");
				#endif
				break;
			}
		}else{
			printf("Error happened during transfer.");
			return(20);
		}
	}
	/*======================================}*/

	/*{if need,set the permission of the file*/
	int chmod_ret;
	if(receive_consult.keep_permission==1){
		chmod_ret=chmod(file_path,permission_mode);
		if(chmod_ret==-1){
			perror("chmod FATAL");
			return(21);
		}else{
			#ifdef DEBUG
				printf("permission_mode=%d\n",permission_mode);
			#endif
		}
	}
	/*======================================}*/

	/*{set owner and grouper*/
	int chown_ret;
	if(receive_consult.keep_owner==1){
		chown_ret=chown(file_path,file_owner_id,-1);
		if(chown_ret==-1){
			perror("chown WARNING");
		}else{
			#ifdef DEBUG
				printf("chown owner to %d success.\n",file_owner_id);
			#endif
		}
	}
	if(receive_consult.keep_group==1){
		chown_ret=chown(file_path,-1,file_group_id);
		if(chown_ret==-1){
			perror("chown WARNING");
		}else{
			#ifdef DEBUG
				printf("chown group to %d success.\n",file_group_id);
			#endif
		}
	}
	/*=====================}*/

	return(0);
}

int main(int argc,char *argv[])
{
	signal(SIGINT,finishup);

        /*cope with options and parameters*/
        int parse_ret;
        parse_ret=parseOptions(argc,argv);
        if(parse_ret!=0){
		receive_usage();
                exit(7);
        }
        /*================================*/

	socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd==-1)
	{
		perror("socket FATAL");
		exit(1);
	}
	#ifdef DEBUG
		printf("socket init success\n");
	#endif

	/*{set socket option*/
	int opt=1;
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	/*=================}*/

	/*address setup*/
	struct sockaddr_in server_addr;
	struct in_addr inp;
	int inet_ret;
	//clear the structure
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	//set socket address
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(g_listen_port);
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
	pid_t pid;
	while(1){
		newsocket_fd=accept(socket_fd,NULL,NULL);
		if(newsocket_fd==-1)
		{
			perror("accept FATAL");
			exit(4);
		}else{
			all_client_num++;
		}
		pid=fork();
		if(pid==-1){ //error
			perror("fork FATAL");
			exit(5);
		}else if(pid>0){ //parent process
			close(newsocket_fd);
		}else if(pid==0){ //child process
			close(socket_fd);
			int receive_ret;
			while(1){
				receive_ret=receive_file(newsocket_fd,".");
				if(receive_ret==88){
					/*88:finish transfer*/
					break;
				}else if(receive_ret!=0){
					printf("receive_file failed.ret=%d\n",receive_ret);
					exit(5);
				}
			}
			close(newsocket_fd);
			return(0);
		}
	}
	/*================*/
}
