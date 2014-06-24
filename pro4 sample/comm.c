/*
 *    (C) Copyright 2010 VideoRay LLC.
 *    Author: Andy Goldstein <andy.goldstein@videoray.com>
 *
 *    This file is part of the VideoRay C sample.
 *
 *    This code is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *      This code is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with thise code.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "comm.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int  comm_open(const char *port, int baudrate) {
    struct termios tio;
    int fd;

    /* open the device to be non-blocking (read will return immediatly) */
    fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd <0) {
        return -1;
    }

    // this sample is just single threaded but this is left here for reference
    /* install the signal handler before making the device asynchronous
        saio.sa_handler = signal_handler;
        sigemptyset(&saio.sa_mask);
        saio.sa_flags = 0;
        saio.sa_restorer = NULL;
        sigaction(SIGIO,&saio,NULL);
    */

    /* set new port settings for canonical input processing */
    tio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_lflag =0;
    tio.c_cc[VMIN]=0;
    tio.c_cc[VTIME]=10;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&tio);

    /* allow the process to receive SIGIO */
    //fcntl(fd, F_SETOWN, getpid());
    /* Make the file descriptor asynchronous */
    //fcntl(fd, F_SETFL, FASYNC);

    return fd;
}


void comm_close(int fd) {
    close(fd);
}

int comm_write(int fd, char *buf, int cnt) {
    int c;
    c = write(fd,buf,cnt);
    tcdrain(fd);
    return c;
}

int  comm_read(int fd, char *buf, int max_cnt) {
	fd_set set;
    struct timeval timeout;
	int sel;
	
	FD_ZERO(&set); /* clear the set */
    FD_SET(fd, &set); /* add our file descriptor to the set */

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    sel = select(fd + 1, &set, NULL, NULL, &timeout);
	if (sel<=0) 
	  return -1;
	  
    return read(fd,buf,max_cnt);
}

