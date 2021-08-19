#include <wiiuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "easy-serial.h"

#define die(error...) do { \
	fprintf(stderr, error); \
	putc('\n', stderr); \
	exit(EXIT_FAILURE); \
} while (0)

#define ONE_WIIMOTE 1
#define CONNECT_TIMEOUT 5
#define SHAKE_THRESHOLD 22.5

int main(int argc, char **argv) {
	// Connect to serial
	int serial;
	if (argc >= 2) {
		int serial = serialport_open(argv[1], 9600);
		if (serial == -1) {
			die("Couldn't open serial port.");
		}
	}
	else {
		serial = -1;
	}

	// Connect to the wiimote
	wiimote **wiimotes = wiiuse_init(ONE_WIIMOTE);
	{
		// Find wiimotes in discovery mode
		int found_wiimotes = wiiuse_find(wiimotes, ONE_WIIMOTE, CONNECT_TIMEOUT);
		if (found_wiimotes == 0) {
			die("Couldn't find any wiimotes in discovery mode.");
		}

		// Connect to discovered wiimote
		int connected_wiimotes = wiiuse_connect(wiimotes, ONE_WIIMOTE);
		if (connected_wiimotes == 0) {
			die("Couldn't connect to the discovered wiimote.");
		}
	}

	// Initialize the wiimote
	wiiuse_set_leds(wiimotes[0], 0b01100000);
	wiiuse_motion_sensing(wiimotes[0], 1);

	// Event loop
	float roll_buffer[10] = { 0 };
	uint16_t prev_buttons = 0;
	while (WIIMOTE_IS_CONNECTED(wiimotes[0])) {
		if (wiiuse_poll(wiimotes, ONE_WIIMOTE)) {
			if (wiimotes[0]->event == WIIUSE_EVENT) {
				uint16_t buttons = wiimotes[0]->btns;
				memmove(roll_buffer + 1, roll_buffer, sizeof(float) * 9);
				roll_buffer[0] = wiimotes[0]->orient.roll;
				if (fabs(roll_buffer[0] - roll_buffer[9]) >= SHAKE_THRESHOLD) {
					buttons |= (1 << 15);
				}
				if (prev_buttons != buttons) {
					if (serial != -1) {
						if (write(serial, &buttons, 2) != 2) {
							perror("write");
						}
					}
				}
				prev_buttons = buttons;
			}
		}
	}

	// Cleanup
	if (serial != -1) {
		close(serial);
	}

	return 0;
}