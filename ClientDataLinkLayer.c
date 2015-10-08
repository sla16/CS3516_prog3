#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>		/* File header */
#include "header.h"

extern uint16_t frame_seq_num;

/*
 * Constructs the frame from the packet
 *
 * @param {char *} photo_buffer The 1-256 byte chunk of the photo (from the Network Layer)
 * @param {int} photo_size The amount of bytes in this chunk
 * @param {int} packet_num The current packet
 */
void CreateFrame(char *photo_buffer, int photo_size, int packet_num)
{
	int bytes_used = 0, resend_frame = 0;
	int i = -1;
	struct frame Frame;
	memset(&Frame, 0, sizeof(Frame)); /* Zero out structure */

	while(photo_size > 0) {
		/* Construct the frame for transmission */
		if(resend_frame != 1) {
			i++;
			Frame.frame_type = '0';			/* 0 for frame, 1 for frame ack, 2 for packet ack */
			/* 1 for EOP, 0 otherwise */
			if(photo_size <= 0) 
				Frame.eop = 'y';
			else
				Frame.eop = 'n';
			strncpy(Frame.datafield, photo_buffer + bytes_used, 130);
			bytes_used = bytes_used + 130;
			photo_size = photo_size - 130;
		}
		fprintf(f, "Frame #%d of packet #%d sent\n", i, packet_num);
		/* To physical layer */
		if ((resend_frame = SendFrame(Frame, i)) == 1) {	
			/* Frame successfully sent, send next frame */
			frame_seq_num += 1;
			resend_frame = 0;
		} else if (resend_frame == 2) {
			/* Received network ack, proceed to next packet */
			break;
		} else {
			resend_frame = 1;
		}
		break;
	}
}
