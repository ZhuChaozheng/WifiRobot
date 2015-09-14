#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/fb.h>

#define SIZE 640*960

#define BUFFER_COUNT 4

struct buffer{
	void *start;
	size_t length;
};

struct buffer framebuf[BUFFER_COUNT];
int buf_index;

char graphic_buff[SIZE];

int getGraphic(int fd);


static char *dev_name = "/dev/video0";
static int fd = -1;
struct buffer framebuf[BUFFER_COUNT];
static unsigned int n_buffers = 0;

FILE *file_fp;
static unsigned long file_length;
static unsigned char *file_name;
int buf_index = 0;


#if 0
int main(void)
{

	while (1)
	{	
		getGraphic();

//		LCDShow();
		sleep(2);
	}

	return 0;
}
#endif





int getGraphic(int fd)
{
	unsigned int i;
	int ret;

	printf("dbg: get graphic!\n\n\n\n");
#if 0//sleep(1);
	fd = open(dev_name, O_RDWR, 0);
	if (fd < 0)
	{
		printf("open %s failed\n", dev_name);
		perror(dev_name);
		return -1;
	}	
	printf("open camera 0v9650!\n");
#endif

	//Query Capability
	struct v4l2_capability cap;
	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0)
	{
		printf("VIDIOC_QUERYCAP failed(%d)\n", ret);
		return ret;
	}

	//Print capability informations
	/*printf("Capability information:\n");
	printf("driver: %s\n", cap.driver);	
	printf("card: %s\n", cap.card);
	printf("bus info: %s\n", cap.bus_info);
	printf("version: %x\n", cap.version);
	printf("capabilities: %x\n", cap.capabilities);*/

	//set stream Format
	struct v4l2_format fmt;
	int bpp;
	memset(&fmt, 0, sizeof (fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 320;
	fmt.fmt.pix.height = 240;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
	bpp = 16;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.bytesperline = fmt.fmt.pix.width * bpp / 8;
	fmt.fmt.pix.sizeimage = fmt.fmt.pix.width * fmt.fmt.pix.bytesperline;
	//**fmt.fmt.pix.priv = V4L2_FMT_OUT**
	fmt.fmt.pix.priv = 1;
	if ((ret = ioctl(fd, VIDIOC_S_FMT, &fmt)) < 0)
	{
		printf("VIDIOC_S_FMT failture (%d)\n", ret);
		return -1;
	}
	
	//get stream format
	ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
	if (ret < 0)
	{
		printf("VIDIOC_G_FMT failed (%d)\n", ret);
		return ret;
	}

	//print stream foramt
	//printf("Stream Format information: \n");
	//printf("type:   %d\n", fmt.type);
	//printf("width:  %d\n", fmt.fmt.pix.width);
	//printf("height: %d\n", fmt.fmt.pix.height);
	char fmtstr[8];
	memset(fmtstr, 0, 8);
	memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);

	/*printf("pixelformat: %s\n", fmtstr);
	printf("field: %d\n", fmt.fmt.pix.field);
	printf("bytesperline: %d\n", fmt.fmt.pix.bytesperline);
	printf("sizeimage: %d\n", fmt.fmt.pix.sizeimage);
	printf("colorspace: %d\n", fmt.fmt.pix.colorspace);
	printf("priv: %d\n", fmt.fmt.pix.priv);
	printf("raw_date: %s\n", fmt.fmt.raw_data);*/

	//request buffers
	struct v4l2_requestbuffers reqbuf;
	memset(&reqbuf, 0, sizeof (reqbuf));
	reqbuf.count = BUFFER_COUNT;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	if ((ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf)) < 0)
	{
		printf("VIDIOC_REQBUFS failture\n");
		return ret;
	}

	//query buffers
	struct v4l2_buffer buf;
	for (i = 0; i < BUFFER_COUNT; i++)
	{
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0)
		{
			printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
			return ret;
		}

		framebuf[i].length = buf.length;
		framebuf[i].start = (char *)mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if (framebuf[i].start == MAP_FAILED)
		{
			printf("mmap (%d) failed\n", i);
			return -1;
		}

		//printf("*************************\n");
		//Queen buffer
		ret = ioctl(fd, VIDIOC_QBUF, &buf);
		if (ret < 0)
		{
			printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
			return ret;
		}

		//printf("Frame buffer %d: address=0x%x, length=%d\n", i, (unsigned int)framebuf[i].start, framebuf[i].length);
	}

	
	//stream on
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if (ret < 0)
	{
		printf("VIDIOC_STREAMON failed (%d)\n", ret);
		return ret;
	}	

	//get frame
	ret = ioctl(fd, VIDIOC_DQBUF, &buf);
	if (ret < 0)
	{
		printf("VIDIOC_DQBUF failed\n"), ret;
		return ret;
	}

	/*file_fp = fopen("test", "w+");
	if (file_fp < 0)
	{
		printf("open frame data file failed\n");
		return -1;
	}

	fwrite(framebuf[buf.index].start, 1, buf.length, file_fp);
	fclose(file_fp);
	printf("Capture one frame saved in test\n");

	for (i = 0; i < BUFFER_COUNT; i++)
	{
		munmap(framebuf[i].start, framebuf[i].length);
	}*/

	//printf("\n\nbuf.lenght=%d\n\n", buf.length);	
	memcpy(graphic_buff, framebuf[buf.index].start, buf.length);
	
	for (i = 0; i < BUFFER_COUNT; i++)
	{
		munmap(framebuf[i].start, framebuf[i].length);
	}

#if 0
	for (i=0; i<5000; i++)
	{
		printf("%d ", graphic_buff[i]);
		if (i%50 == 0)
		{
			printf("\n");
		}
	}
#endif

//	close(fd);
	printf("Capture one frame saved in test\n");
	

	return 0;
}

