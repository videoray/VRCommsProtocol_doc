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
 *    This code is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with thise code.  If not, see <http://www.gnu.org/licenses/>.
 */


/* This is a sample demonstation on how to use the raw PRO4 protocol to control the VideoRay ROV.
 * This is primarily intended to illustrate the communication protocol and is not meant to be usable for control as-is.
 * It is a single threaded application which blocks waiting for a keyboard command.  This of course is not so nice, since the 
 * ROV state will not update regularly.  
 * Upon reception of a command the ROV memory map is updated and a command is sent to the ROV.
 * A basic response is recived and parsed.
 * Some ROV parameters are output to the console.
 *
 * More normally there would be a separate thread handling the communications to the ROV.  
 */
 
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "comm.h"
#include <protocol_pro4.h>
#include <pro4_specific_response.h>


/* hardcode the port to use */
#define COMM_PORT "/dev/com6"

/* ROV is always Node ID 1 for now */
#define ROV_ID 0x1

/*forward declaration of simple non-buffered keyboard routine */
int ui_command(void);


/* In this sample we just update a portion of the ROV CSR memory map. 
   Specifically we just set the thrusters and the lights
   Note: We serialize this struct directly so it must be packed and byte aligned.
		 We are also assuming that the endian on the host and target are the same (little endian)
*/   
struct rov_command_t{
    int16_t thrust_target[3]; //port, starboard, vert
    uint8_t lights;
}  __attribute__((__packed__));

/* simple flag used to indicate a fully parsed response packet has been received.*/
char got_response=0;

int main(void) {
	struct rov_command_t rov_command;
	int comm_port_fd;
	int c;
	char done=0;
	int surge=0;
	int yaw=0;
	char rov_command_header_buffer[PROTOCOL_PRO4_HEADER_SIZE];
#define MAX_RESPONSE_SIZE 8 /* this sample only ever requests a specific packet, and that is 8 bytes long */	
	char rov_response_buffer[PROTOCOL_PRO4_HEADER_SIZE + MAX_RESPONSE_SIZE + 1];
	char payload_xsum;
	int cnt;
	int ti;
	char response_byte;
	
	/* init the command struct.  Thrusters go from -100 to 100, where negative is reverse.
	                             lights go from 0 to 100 (0 is off, 100 is full on).
	*/							 
	rov_command.thrust_target[0]=0;
	rov_command.thrust_target[1]=0;
	rov_command.thrust_target[2]=0;
	rov_command.lights=0;

	
	
	printf("Simple ROV sample\r\nArrow keys set horizontal propulsion (vert is always 0),space sets thrust to 0, 0-9 for lights\r\nq to exit\r\n\r\n");
	
	/* open comm port */
	if ((comm_port_fd=comm_open(COMM_PORT, 115200))==-1) {
	   printf("Could not open serial port %s\r\n",COMM_PORT);
	   return -1;
	}
	
	/* simple application loop */
	while (!done) {
		c=ui_command();
		if (c>='0' && c<='9') {
			rov_command.lights = (c-'0') * 10;
		}
		else
		switch (c) {
			case 'q':
				done = 1;
			break;
			case 'U' :
				if (surge<100)
					++surge;
			break;
			case 'D':
				if (surge>-100)
					--surge;
			break;
			case 'L':
				if (yaw>-100)
					--yaw;
			break;
			case 'R':
				if (yaw>100)
					++yaw;
			break;
			case ' ':
				//kill the thrusters
				surge=0;
				yaw=0;
			break;
		};
		
        //Map surge and yaw inputs to thruster values.
	    //This sort-of emulates a joystick with a circular mapping		
        rov_command.thrust_target[0]=surge;
        rov_command.thrust_target[1]=surge;
        rov_command.thrust_target[0]+=yaw;
        rov_command.thrust_target[1]-=yaw;
		
		//limit thrusters to allowable percentages.
		for (ti=0;ti<=1;ti++) {
			if (rov_command.thrust_target[ti]<-100)
			  rov_command.thrust_target[ti]=-100;
			if (rov_command.thrust_target[ti]>100)
			  rov_command.thrust_target[ti]=100;
		}
		
		//printf("S/Y: %d:%d  P/S: %d:%d\r\n",surge, yaw, rov_command.thrust_target[0], rov_command.thrust_target[1]);
		
		/* Send the command packet.
		 * We split the transmission into 3 parts: header, payload, and the payload xsum.
		 * This is mainly just for illustation.  The entire packet can be built inplace and sent as a single unit
		*/	 
		cnt = protocol_pro4_build_request_header(ROV_ID, 
												 RESPONSE_FULL_ATTITUDE_DEPTH,
												 0x0,
												 sizeof(rov_command),
												 rov_command_header_buffer);
																							 

		comm_write(comm_port_fd,rov_command_header_buffer,cnt);

		comm_write(comm_port_fd,(char*) &rov_command,sizeof(rov_command));

		payload_xsum = protocol_pro4_checksum((char*) &rov_command, sizeof(rov_command));
		
		got_response=0; /* clear received response flag, this will be set in our 
						   response handler so we know when a full response packet has been 
						   received 
						*/
		
		comm_write(comm_port_fd,&payload_xsum,1);
		
		/* Loop until we get the reponse packet or timeout.
		 * We are expecting the Full Attitude and Depth response device specific response
		 * The protocol_pro4_parse handles the parsing and distribution of the packet data to the appropriate 
		 * application specific handler (see below).  In a more typical application, this parse routine would be 
		 * run in reponse to a serial port interupt or simialr
		 */
		while((comm_read(comm_port_fd,&response_byte,1)!=-1)) {
			protocol_pro4_parse(response_byte,rov_response_buffer,sizeof(rov_response_buffer));
			if (got_response) 
			  break;
		}
		
	}
	
	
	comm_close(comm_port_fd);
	

	return 0;
}


/* Device/Applicaion specfic callbacks that are used by the pro4 protocol parsing routines.
 * These need to be instatiated, even if we only use a subset of the functions in practice.
 * In this application we accept all ID's but only really care about the device specific respone
 * from the ROV.
 *
 * When we get the expected resposne we just output it to stdout.  Also doing a very very basic pressure to depth conversion.
 */

void protocol_pro4_handle_response(uint8_t id, 
                                   uint8_t flags,
                                   int addr, 
                                   int len,
                                   uint8_t device_type,       
                                   char *buf) {
	float depth;
	got_response=1;			
	/* just map to the response stuct.  Again assuming no packing, alignment, endian issues.*/
	Response_Attitude *response = (Response_Attitude*) buf;

	/* attitude datum are in degrees, depth is sent as mbar)*/
	/* 1 decibar = 1.019716 m conversion 
	 * also subtract out a standard atmosphere.
       There are many better pressure->depth conversions like unesco
	*/
	depth = 1.0 / 101.9716 * (response->depth-1013);
	if (depth<0) depth=0;
	printf("Heading/pitch/roll/mbar/depth: %d %d %d %d %3.3f\r\n",
								 response->heading,
								 response->pitch,
								 response->roll,
								 response->depth,
								 depth);
}

char protocol_pro4_accept_id(uint8_t id, char isResponse) {
	return 1;
}

/* The reset of the protocol_pro4 functions are essentially do not care for this app */

void protocol_pro4_handle_custom_command_request(uint8_t id,
                                                 int len, 
                                                 char *buf) {
}
void protocol_pro4_handle_custom_command_response(uint8_t id,
                                                         int len,
                                                         uint8_t device_type,
                                                         char *buf) {													 
}
void protocol_pro4_respond_device_specific(uint8_t flags) {
}
void protocol_pro4_respond_csr_read(int addr, int len) {
}
void protocol_pro4_csr_write(int addr,  int len, char *buf) {
}
char protocol_pro4_relay_packet(char* data) {
	return 0;
}


/* Simple non-buffered keyboard UI.  
   This special cases the arrow keys and returns a valid single character.  Of couse this means that the stanard "U,D,L,R" 
   characters cannot be used for other (non-arrow) purposes.
*/
int ui_command(void) {
	/* set stdin to not buffer or echo */
	struct termios oldt,newt;
	int c;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	c = getchar();
	//Do some basic arrow key processing, return U,D,L,R depending upon the key
	if (c==0x1b) {
		c=getchar();
		if (c==0x5b) {
			c=getchar();
			switch (c) {
				case 0x41:
					c='U';
				break;
				case 0x42:
					c='D';
				break;
				case 0x43:
					c='R';
				break;
				case 0x44:
					c='L';
				break;
			}
		}
	}

	/* reset stdin */
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return c;
}
