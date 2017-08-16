#ifndef  _LOGGER_H
#define  _LOGGER_H

#include <syslog.h>

void logger_reload();
void logger(int level, const char *fmt, ...) ;
void DumpMsg(unsigned char *buffer, int len);

#endif
