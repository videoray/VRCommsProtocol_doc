#ifndef __COMM_H__
#define __COMM_H__

/*
 *    (C) Copyright 2010 VideoRay LLC.
 *    Author: Andy Goldstein <andy.goldstein@videoray.com>
 *
 *	  This file is part of the VideoRay C sample.
 *
 *    This code is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This code is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with thise code.  If not, see <http://www.gnu.org/licenses/>.
 */
 
 
/* 
   Very simple helper functions for basic serial port interaction.
   can be used for some portibillity to devices which do not support posix, termios, etc.
 */
int  comm_open(const char *port, int baudrate);
void comm_close(int fd);
int  comm_write(int fd, char *buf, int cnt);
int  comm_read(int fd, char *buf, int max_cnt);

#endif
