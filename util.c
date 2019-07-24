#include <signal.h>
#include <stdio.h>

void self_destruct(char *message, int line_number, char *file_name) {
	printf("[%s:%d] %s\n", file_name, line_number, message);
	raise(SIGINT);
}