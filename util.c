#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "device.h"

void self_destruct(char *message, int line_number, char *file_name) {
	printf("[%s:%d] %s\n", file_name, line_number, message);
	raise(SIGINT);
}

void signal_handler(int signal_number) {
	if (signal_number == SIGINT) {
		printf("\nreceived SIGINT, stopping\n");
		close_fbfd();
		exit(0);
	}
}

void set_up_signal_handling() {
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		printf("\ncouldn't install SIGINT handler\n");
		exit(1);
	}
}
