#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

int serialport_open(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    fd = open(serialport, O_RDWR | O_NOCTTY);
    
    if (fd == -1)  {
        perror("serialport_init: Unable to open port ");
        return -1;
    }
    
		/* get current serial port settings */
		tcgetattr(fd, &toptions);
		/* set 9600 baud both ways */
		cfsetispeed(&toptions, B9600);
		cfsetospeed(&toptions, B9600);
		/* 8 bits, no parity, no stop bits */
		toptions.c_cflag &= ~PARENB;
		toptions.c_cflag &= ~CSTOPB;
		toptions.c_cflag &= ~CSIZE;
		toptions.c_cflag |= CS8;
		/* Canonical mode */
		toptions.c_lflag |= ICANON;
		/* commit the serial port settings */
		tcsetattr(fd, TCSANOW, &toptions);

    return fd;
}