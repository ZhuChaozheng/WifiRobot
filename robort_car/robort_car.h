#ifndef _ROBORT_CAR_H
#define _ROBORT_CAR_H

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
#include <signal.h>
#include <fcntl.h>

#define BUFSZ 10
struct movement_info
{
	char car_forward;     //???
	char car_back;
	char car_left;
	char car_right;
	char car_stop;

	char steer_left;    //??????????
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

extern int getGraphic(int fd);
extern char graphic_buff[960*640];

#endif


