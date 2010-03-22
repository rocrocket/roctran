#include "roctran.h"

/*global variable*/

//listen port
int g_listen_port=0;

/*===============*/

/*usage function*/
void usage()
{
	time_t mytime;
        struct tm *mylocaltime;
	mytime=time(NULL);
        mylocaltime=localtime(&mytime);
	char usage_str[]="rocreceive - roctran tool\n\
===\n\
usage:\n\
./rocreceive -p PORT\n\
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
                //read the specified file
                return(2);
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
                                printf("optarg:%d\n",g_listen_port);
                                break;
                        default:
				return(1);
                }
        }
        return(0);
}
/*================================*/


/*function:receiver one file from client
 * param1:file descriptor
 * param2:the local storing file path
*/
int receive_file(int socket_fd,char *local_path)
{
	/*read file name from client*/
	char file_name[1023],*str_p;
	str_p=file_name;
	ssize_t read_ret;
	read_ret=read(socket_fd,str_p,sizeof(file_name));
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
	read_ret=read(socket_fd,&file_size,sizeof(file_size));
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
			read_ret=read(socket_fd,&recv_content_size,sizeof(recv_content_size));
			if(read_ret==-1){
				perror("read FATAL");
				return(9);
			}
			//read content
			read_ret=read(socket_fd,file_buf,recv_content_size);
			fwrite_ret=fwrite(file_buf,read_ret,1,fp);
			if(fwrite_ret==0){
				perror("fwrite FATAL");
				return(6);
			}
			_file_size-=read_ret;
			//printf("read_ret=%d,_file_size=%d\n",read_ret,_file_size);
			//send ack
			write_ret=write(socket_fd,ack_str,sizeof(ack_str));
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
        /*cope with options and parameters*/
        int parse_ret;
        parse_ret=parseOptions(argc,argv);
        if(parse_ret!=0){
		usage();
                exit(7);
        }
        /*================================*/

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
	int all_client_num=0;
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
			receive_ret=receive_file(newsocket_fd,".");
			if(receive_ret!=0)
			{
				printf("receive_file failed.ret=%d\n",receive_ret);
				exit(5);
			}
			close(newsocket_fd);
			return(0);
		}
	}
	/*================*/

	/*close the fd*/
	close(socket_fd);
	/*============*/
	printf("client number:%d\n",all_client_num);
	printf("good bye!\n");
	return(0);
}
