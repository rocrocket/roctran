#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/stat.h>
#include<libgen.h>
#include<time.h>
#include<dirent.h>
#include<signal.h>

#define BUFSIZ_ROC 4000
#define MAX_NUM_OPTIONS 48
#define DEBUG

/*{structure define*/

/*the consult message*/
struct consult_message{
	//keep the owner name of the file or dir
	unsigned keep_owner:1;
	//keep the group name of the file or dir
	unsigned keep_group:1;
	//keep the permission of the file or dir
	unsigned keep_permission:1;
	//owner read permission of file or dir
	unsigned owner_r:1;
	//owner write permission of file or dir
	unsigned owner_w:1;
	//owner execute permission of file or dir
	unsigned owner_x:1;
	//group read permission of file or dir
	unsigned group_r:1;
	//group write permission of file or dir
	unsigned group_w:1;
	//group execute permission of file or dir
	unsigned group_x:1;
	//other read permission of file or dir
	unsigned other_r:1;
	//other write permission of file or dir
	unsigned other_w:1;
	//other execute permission of file or dir
	unsigned other_x:1;
};
/*==================*/

/*================}*/
