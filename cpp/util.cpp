#include <cstdio>
#include <cstdlib>
#include "device.h"

void self_destruct(char *message, int line_number, const char* file_name) {
	printf("[%s:%d] %s\n", file_name, line_number, message);
	close_device();
	exit(1);
}
