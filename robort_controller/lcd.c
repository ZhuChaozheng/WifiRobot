#include "lcd.h"


void LCDShow()
{
	int spi_fd;
	int lcd_fd;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	int screensize;
	int location;
	char *fbp;
	int i, j;
	int count = 0;


	lcd_fd = open("/dev/fb0", O_RDWR|O_NONBLOCK);
	if (lcd_fd < 0)
	{
		printf("open lcd failture\n");
		exit(1);
	}
	printf("open lcd!\n");	

	if (ioctl(lcd_fd, FBIOGET_FSCREENINFO, &finfo))
	{
		printf("FBIOGET_FSCREEN error\n");
		exit(1);
	}

	if (ioctl(lcd_fd, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("FBIOGET_VSCREEN error\n");
		exit(1);
	}
	printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
	if (vinfo.bits_per_pixel != 16)
	{
		printf("Error: not supported bits_per_pixel, it only supports 16 bit color!\n");
		exit(1);
	}

	/*printf("xres: %d\n", vinfo.xres);
	printf("yres: %d\n", vinfo.yres);
	printf("bits_per_pixel: %d\n", vinfo.bits_per_pixel);
	printf("line_length: %d\n", finfo.line_length);	*/

	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if ((int)fbp == -1)
	{
		printf("lcd mmap failture\n");
		exit(1);
	}
	
//	printf("%d\n", (int)fbp);

//	fread(buffer,  960*640, 1, file_fd);

//	printf("LCD Show\n");

	for (i = 10; i < 250; i++)
	{
		for (j = 160; j < 480; j++)
		{
			int r, g, b;
			int y, u, v;
			location = (j  + i *480 )*2;
			
			*(fbp + location) = graphic_buff[count];	
			*(fbp + location+1) = graphic_buff[count + 1];	
			count = count + 2;
		}
	}

	munmap(fbp, screensize);

	close(lcd_fd);
}




