#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>

int main( int argc, char** argv)
{
	int fd;				// file descripter ( serial descripter )
        char buf[20] = "";              // message buffer
        char name[20] = "/dev/pts/";    // virtual port name

	struct termios newtio;		// value for setting terminal

	if ( argc != 2 )
	{
		printf("Usage : %s <portNum>\n", argv[0]);
		exit(1);
	}

	strcat( name, argv[1] );

//	fd = open( name, O_RDWR | O_NOCTTY | O_NONBLOCK );
	fd = open( name, O_RDWR | O_NOCTTY );
        // O_RDWR       : read and write mode
        // O_NOCTTY     : ?
        // O_NONBLOCK   : if no read data, no waiting

	if ( fd == -1 )
	{
		printf("Port number is incorrect.\n");
		exit(1);
	}

	memset( &newtio, 0, sizeof(newtio) );

	newtio.c_cflag = B115200;   // 115200 comm speed
	newtio.c_cflag |= CS8;      // 8bit data bit 
	newtio.c_cflag |= CLOCAL;   // use inner comm port 
	newtio.c_cflag |= CREAD;    // read and write
	newtio.c_iflag = 0;         // none parity 
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0; 
	newtio.c_cc[VMIN] = 1; 

	tcflush (fd, TCIFLUSH );
	tcsetattr(fd, TCSANOW, &newtio );
	// comm option setting

	while (1)
	{  
		scanf("%s", buf);
		write( fd, buf, 20);
		// write data to 'fd' port
	}

	close( fd);
	// close port

	return 0; 
}
