#ifndef BUFFDOG_UTIL
#define BUFFDOG_UTIL

#define terminate(message) self_destruct(message, __LINE__, __FILE__)


void self_destruct(char *message, int line, const char* file);

#endif
