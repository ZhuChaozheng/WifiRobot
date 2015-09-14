#include "robort_car.h"


#define SERVER_IP  "192.168.1.119"  //服务器IP
#define SERVER_PORT  6543           //服务器和客户端通信的端口
void *GetCmd(void *);
struct movement_info  CharToStruct(unsigned char *temp);

static void print_cmd_info(struct qt_cmd *info);
static int create_client_socket(struct sockaddr_in *client_addr, int portnumber, char *remote_ip);
void sendGraphic(int sockfd);
static int create_server_socket(struct sockaddr_in *server_addr, int portnumber);

static void SigHandler(int iSignNum);
struct  qt_cmd  main_cmd;
static char sig_flag = 0;
	
struct movement_info *iic_data;
pid_t iic_pid = 0;
pthread_t tid;

int main(int argc, char *argv[])
{
	int sockfd;
	int newfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sin_size;
	int portnumber = 6543;  //通信端口
	int key;
	int shm_id;
	
	int ret;
	key = ftok("/app/iic_shm", 1); //get sharememory key
	if (key == -1)
	{
		printf("parent ftok error\n");
	}
	else
	{
		printf("parent iic_shm ftok ok!\n");
	}
	
	shm_id = shmget(key, BUFSZ, IPC_CREAT | 666); //create sharememory
	if (shm_id < 0)
	{
		 printf("shmget failed!\n");
	   	 exit(1);
	}
	printf("create a shared memory segment successfully: %d\n", shm_id);
	
	iic_data  = (struct movement_info *)shmat(shm_id, NULL, 0);
	
	signal(SIGUSR2, SigHandler); //register an user signal


/***********************************************************************
 *			IIC  code
 * ********************************************************************/
#if 1
	if ((iic_pid=fork()) == 0) //IIC child process
	{
		pid_t  iic_fd = -1;

		if ((iic_fd=open("/dev/gzsd6410_i2c", O_RDWR)) == -1)
		{
		    perror("open flash!\n");
		    exit(1);
		}

		key = ftok("/app/iic_shm", 1); //get sharememory key
		if (key == -1)
		{
			printf("child ftok error\n");
		}
		else
		{
			printf("child iic_shm ftok ok!\n");
		}

		shm_id = shmget(key, BUFSZ, IPC_CREAT | 666); //create sharememory
		if (shm_id < 0)
		{
			 printf("shmget failed!\n");
		}

		printf("create a shared memory segment successfully: %d\n", shm_id);
	
		iic_data  = (struct movement_info *)shmat(shm_id, NULL, 0);

		for (; ;)
		{
			if (sig_flag == 1)  //判断是否有新数据发来，为1则是
			{
				sig_flag = 0;
				
				memcpy((char*)&main_cmd.car_movement, (char *)iic_data, sizeof(struct movement_info)); //从共享内存中拷备数据
				printf("IN iic code\n");
				print_cmd_info(&main_cmd); //调试打印接收信息

				/////////////////////////////////////////
				/*add the IIC code!*/
				///////////////////////////////////////
				printf("size of movement_info:%d\n",sizeof(struct movement_info));
				if (write(iic_fd, (char *)iic_data ,sizeof(struct movement_info) ) != sizeof(struct movement_info)) //通过st_i2c驱动写控制变量到单片机
				{
					printf("dbg: send cmd to iic failed!\n");								}
			}		
		}

	}
#endif

//	sleep(10); //wait for server establish

	/*sockfd = create_client_socket(&server_addr, SERVER_PORT, SERVER_IP);
	if (sockfd < 0)
	{
		printf("fail to create socket!\n");
		exit(1);
	}*/
//	printf("dbg: creat socket!\n");

/************************
	if (connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1)
	{
		printf("fail to connect server!\n");
		exit(1);
	}
*********************************/
/*
	do 
	{
		printf("start to connect\n");
		ret = connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr));
		sleep(5);
		
	}while(ret == -1);
	printf("connect successfully!\n");
*/
	int on = 1;
	sockfd = create_server_socket(&server_addr, portnumber); //create server's socket
	if (sockfd < 0)
	{
		printf("fail to create socket!\n");
		exit(1);
	}

	sin_size = sizeof(struct sockaddr_in);
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
/*	if ((newfd=accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size)) == -1)
	{
		fprintf(stderr, "accept error:%s\n\a", strerror(errno));
		exit(1);
	}
	printf("-----------connected---------------\n");
	for (; ;)
	{
		if (GetCmd(newfd) == 1)
		{
			print_cmd_info(&main_cmd);
			
			memcpy((char *)iic_data, (char *)(&main_cmd.car_movement), sizeof(struct movement_info));
			
			kill(iic_pid, SIGUSR2);

		}
	}
*/
	for(;;)
	{
		sin_size = sizeof(struct sockaddr_in);
		if ((newfd=accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size)) == -1)
		{
			fprintf(stderr, "accept error:%s\n\a", strerror(errno));
			exit(1);
		}
		else{
			printf("server:connected\n");
		}
	//create a new child thread
		// if(pthread_create(&tid,NULL,(void *)GetCmd,(void *)(&newfd)) != 0)
		if(pthread_create(&tid, NULL, (void *)GetCmd, (void *)(&newfd)) != 0)
	 	{
	 		printf("create pthread error");
			break;
	 	}
	}
	return 0;
	
}



static int create_server_socket(struct sockaddr_in *server_addr, int portnumber)
{
	int sockfd;
	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Socket error:%s\n\a", strerror(errno));
		return -1;
														}
		printf("dbg: create socket!\n");

		bzero(server_addr, sizeof(struct sockaddr_in));
		server_addr->sin_family = AF_INET;
		server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr->sin_port = htons(portnumber);
		if (bind(sockfd, (struct sockaddr*)server_addr, sizeof(struct sockaddr)) == -1)
		{
			printf("bind error:%s\n\a", strerror(errno));								return -1;
		}
		printf("dbg: bind socket!\n");

		if (listen(sockfd, 5) == -1)
		{
			printf("listen error:%s\n\a", strerror(errno));
			return -1;
		}
		printf("dbg: listening...\n");

		return sockfd;
}

/*****************************************************
	创建客户端的socket套接字
****************************************************/
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



static void print_cmd_info(struct qt_cmd *info) //printf touch screnn inform    ation
{
	printf("In print_cmd_info\n");	
	printf("forward=%d back=%d left=%d right=%d stop=%d\n", 
		info->car_movement.car_forward, info->car_movement.car_back, 
		info->car_movement.car_left,info->car_movement.car_right,
		info->car_movement.car_stop);

	printf("s_left=%d s_right=%d s_stop=%d\n", 
		info->car_movement.steer_left,info->car_movement.steer_right, 
		info->car_movement.steer_stop);

}

struct movement_info CharToStruct(unsigned char *temp)
{
	struct qt_cmd temp_cmd;
	temp_cmd.car_movement.car_forward = temp[0];
	temp_cmd.car_movement.car_back = temp[1];
	temp_cmd.car_movement.car_left = temp[2];
	temp_cmd.car_movement.car_right = temp[3];
	temp_cmd.car_movement.car_stop = temp[4];
	temp_cmd.car_movement.steer_left = temp[5];
	temp_cmd.car_movement.steer_right = temp[6];
	temp_cmd.car_movement.steer_stop = temp[7];
	return  temp_cmd.car_movement;
}
/*****************************************************
	从服务器端接收运动控制变量
****************************************************/
void *GetCmd(void *sockfd)
{
	struct  qt_cmd  temp_cmd;
	int newfd;
	newfd = (int)(*(int *)sockfd);
	static unsigned char temp[8];
	int n;
	int cmd_flag = 0;
	for(;;)
	{
		memset((void *)temp, 0,sizeof(temp));
		if ((n = read(newfd, (void *)temp, sizeof (temp)))
		< 0)
		{
//		printf("not receive data!\n");
			memset((void *)temp, 0, sizeof (temp));
			return 0;
		}
		//printf("number of recv:%d\n",n);
//	printf("dbg: check received data!\n");
		int i;
		for(i  = 0;i<8;i++)
		{
			temp[i] -= '0';
		//if(temp[i] != 0)
		//	printf("%d\n",temp[i]);
		}
		temp_cmd.car_movement = CharToStruct(temp);

		if ((temp_cmd.car_movement.car_forward != main_cmd.car_movement.car_forward)||
			(temp_cmd.car_movement.car_back != main_cmd.car_movement.car_back) || 
			(temp_cmd.car_movement.car_left != main_cmd.car_movement.car_left)||
			(temp_cmd.car_movement.car_right != main_cmd.car_movement.car_right) ||
			(temp_cmd.car_movement.car_stop != main_cmd.car_movement.car_stop)||
			(temp_cmd.car_movement.steer_left != main_cmd.car_movement.steer_left) || 
			(temp_cmd.car_movement.steer_right != main_cmd.car_movement.steer_right)||
			(temp_cmd.car_movement.steer_stop != main_cmd.car_movement.steer_stop))
		{
			printf("size to copy: %d\n",sizeof (temp_cmd.car_movement));		
			memcpy((void *)&main_cmd.car_movement, (void*)&temp_cmd.car_movement, sizeof (temp_cmd.car_movement));
			//cmd_flag = 1;
			print_cmd_info(&temp_cmd);
			print_cmd_info(&main_cmd);
			
			memcpy((char *)iic_data, (char *)(&main_cmd.car_movement), sizeof(struct movement_info));
			
			kill(iic_pid, SIGUSR2);
		}
	}
	free(sockfd);
	close(sockfd);
	pthread_exit(NULL);
	//return cmd_flag;
}


void sendGraphic(int sockfd)
{
	printf("dbg: begin to send graphic!\n");

	if (write(sockfd, graphic_buff, 320 * 480) < 0)
	{
		printf("dbg:send graphic fail!\n");
	}
}


/*****************************************************
	信号处理函数，当主进程通知IIC子进程从共享
	内存中读取控制变量
****************************************************/
static void SigHandler(int iSignNum)
{
    printf("Capture signal number:%d\n", iSignNum);
	printf("In sighand\n");
	//print_cmd_info(iic_data);
	
	sig_flag = 1;

}  
