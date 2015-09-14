#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define dbg

#define BUFSZ 10
struct movement_info
{
  char car_forward;     //电机前
  char car_back;
  char car_left;
  char car_right;
  char car_stop;
  char steer_left;    //舵机正转反转
  char steer_right;
  char steer_stop;
};

//static struct movement_info car_movement;

struct qt_cmd
{
  struct movement_info car_movement;
  pthread_mutex_t     mutex;
  pthread_mutexattr_t  mutexattr;
};

struct qt_cmd old_cmd; //old control command in shm

extern char graphic_buff[320*480]; //图片存储区域，老代码所用，不必理会

byte convert_temp[8];
static int create_client_socket(struct sockaddr_in *server_addr, int portnumber,char *remote_ip);
static int create_server_socket(struct sockaddr_in *server_addr, int portnumber);
static void SendCmd(int cmd_sockfd, struct qt_cmd *cmd);
static void print_cmd_info(struct qt_cmd *info);

static int GetGraphic(int sockfd);
static void StructToByte(struct movement_info *movement);

int main(void)
{
	int shm_id;
	key_t key;
	struct qt_cmd *info_in;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int portnumber = 6543;  //通信端口
	int sin_size = 0;
	int sockfd;
	int newfd;

#if 1
	key = ftok("/app/sharememory", 0); //get sharememory key
	if (key == -1)
	{
		printf("parent ftok error\n");
	}	
	else
	{
		printf("parent ftok ok!\n");
	}

	shm_id = shmget(key, BUFSZ, IPC_CREAT | 666); //create sharememory
	if (shm_id < 0)
	{
	    printf("shmget failed!\n");
	    exit(1);
    }
 	
    printf("create a shared memory segment successfully: %d\n", shm_id);
#endif

	memset(&old_cmd, 0, sizeof(old_cmd)); 


/********************************************************************
 *			command server code
 * *****************************************************************/
 	info_in  = (struct qt_cmd *)shmat(shm_id, NULL, 0);//获取共享内存的指针

	sockfd = create_server_socket(&server_addr, portnumber); //create server's socket
	if (sockfd < 0)
	{
		printf("fail to create socket!\n");
		exit(1);
	}
/*  如果侦听到来自客户端的连接请求,则进行控制信息的发送处理*/
	
	sin_size = sizeof(struct sockaddr_in);
	if ((newfd=accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size)) == -1)
	{
		fprintf(stderr, "accept error:%s\n\a", strerror(errno));
		exit(1);
	}
	
/*****************************************************
	创建QT界面子进程并运行
*****************************************************/
	if (vfork() == 0) 
    {
        printf("fork robort_terminal!\n");
		
		execl("/app/WifiRobot", "/app/WifiRobot", "-qws", NULL);  //加载QT界面程序
		
	}
	
	sleep(1);

	
	for( ; ; )
	{
																			
		SendCmd(newfd, info_in);  //发送控制信息
	}

	close(sockfd);

	return 0;
}

/*******************************************************************
  打印共享内存的控制信息，调试时可以使用
*******************************************************************/
static void print_cmd_info(struct qt_cmd *info) //printf touch screnn information
{
  printf("forward=%d back=%d left=%d right=%d stop=%d\n",
  info->car_movement.car_forward, info->car_movement.car_back,
  info->car_movement.car_left, info->car_movement.car_right,
  info->car_movement.car_stop);

  printf("s_left=%d s_right=%d s_stop=%d\n",
  info->car_movement.steer_left, info->car_movement.steer_right, 
  info->car_movement.steer_stop);

}


/*******************************************************************
  创建服务器端的socket套接字，返回创建成功的套接字
*******************************************************************/
static int create_server_socket(struct sockaddr_in *server_addr, int portnumber)
{
	int sockfd;
	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Socket error:%s\n\a", strerror(errno));
		return -1;
	}
	printf("dbg: create socket!\n");

	bzero(server_addr, sizeof(struct sockaddr_in));
	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr->sin_port = htons(portnumber);

	/* 绑定IP和端口 */
	if (bind(sockfd, (struct sockaddr*)server_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("bind error:%s\n\a", strerror(errno));
		return -1;
	}
	printf("dbg: bind socket!\n");

	/* 侦听连接请求 */
    if (listen(sockfd, 20) == -1)
	{
		printf("listen error:%s\n\a", strerror(errno));
		return -1;
	}
	printf("dbg: listening...\n");

	return sockfd;
}

/*******************************************************************
  创建客户端的端的socket套接字，返回创建成功的套接字
*******************************************************************/
static int create_client_socket(struct sockaddr_in *server_addr, int portnumber,char *remote_ip)
{
	int sockfd;
	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Socket error\n");
		return -1;
	}
	printf("dbg: create socket!\n");
	
	bzero(server_addr, sizeof(struct sockaddr_in));
	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = inet_addr(remote_ip);
	server_addr->sin_port = htons(portnumber);
	printf("dbg, finish create socket!\n");
	return sockfd;
}
	

/*******************************************************************
                发送控制命令
*******************************************************************/
static void SendCmd(int cmd_sockfd, struct qt_cmd *cmd)
{
	/* 首先检查控制命令是否有变化，即是否有新的控制命令发出,若有，则发送*/
	if ((old_cmd.car_movement.car_forward!=cmd->car_movement.car_forward) ||
		(old_cmd.car_movement.car_back!=cmd->car_movement.car_back) ||
		(old_cmd.car_movement.car_left!=cmd->car_movement.car_left) ||
		(old_cmd.car_movement.car_right!=cmd->car_movement.car_right) ||
		(old_cmd.car_movement.steer_left!=cmd->car_movement.steer_left) ||
		(old_cmd.car_movement.steer_right!=cmd->car_movement.steer_right)||
		(old_cmd.car_movement.car_stop!=cmd->car_movement.car_stop) ||
		(old_cmd.car_movement.steer_stop!=cmd->car_movement.steer_stop))
	{
		StructToByte(&(cmd->car_movement);
		printf("new cmd!\n");
		send(cmd_sockfd,(char *)&move_info,sizeof(move_info),0);
//		send(cmd_sockfd,(char *) &cmd->car_movement,sizeof(cmd->car_movement),0);
//		write(cmd_sockfd,(char *) &cmd->car_movement, sizeof (cmd->car_movement));
		memcpy((char *)&old_cmd.car_movement, (char *)(&cmd->car_movement), sizeof(old_cmd));
	
		print_cmd_info(cmd); //打印调试
		print_cmd_info(&old_cmd);//打印调试
		//sleep(3);

		printf("\n\n");			
	}
	//memset((void *)&cmd->car_movement, 0, sizeof (cmd->car_movement));
}

void StructToByte(struct movement_info *movement)
{
	
	convert_temp[0] = movement->car_forward;
	convert_temp[1] = movement->car_back;
	convert_temp[2] = movement->car_left;
	convert_temp[3] = movement->car_right;
	convert_temp[4] = movement->car_stop;
	convert_temp[5] = movement->steer_left;
	convert_temp[6] = movement->steer_right;
	convert_temp[7] = movement->steer_stop;
	//return convert_temp;
}



int GetGraphic(int sockfd)
{
	ssize_t n;
	int recv_flag = 0;
	char *ptr = graphic_buff;

	printf("dbg:begin to receive graphic!\n");

	while ((n = read(sockfd, ptr, 320 * 480)) > 0)
	{
		printf("dbg: set rec flag!\n");
		ptr += n;
		recv_flag = 1;
	}
	printf("dbg:finish receive graphic!\n");
	return recv_flag;
}
