#ifndef QSLOGDISABLEFORTHISFILE_H
#define QSLOGDISABLEFORTHISFILE_H

#include <QtDebug>
// When included AFTER QsLog.h, this file will disable logging in that C++ file. When included
// before, it will lead to compiler warnings or errors about macro redefinitions.

#undef QLOG_TRACE
#undef QLOG_DEBUG
#undef QLOG_INFO
#undef QLOG_WARN
#undef QLOG_ERROR
#undef QLOG_FATAL

#define B_RED(str)      "\033[1;31m" str "\033[0m"
#define B_GREEN(str)    "\033[1;32m" str "\033[0m"
#define B_YELLOW(str)   "\033[1;33m" str "\033[0m"
#define B_BLUE(str)     "\033[1;34m" str "\033[0m"
#define B_MAGENTA(str)  "\033[1;35m" str "\033[0m"
#define B_CYAN(str)     "\033[1;36m" str "\033[0m"
#define B_WHITE(str)    "\033[1;37m" str "\033[0m"

#define QLOG_TRACE() if (1) {} else qDebug()
#define QLOG_DEBUG() if (1) {} else qDebug()
#define QLOG_INFO()  if (1) {} else qDebug()
#define QLOG_WARN()  if (1) {} else qDebug()
#define QLOG_ERROR() if (1) {} else qDebug()
#define QLOG_FATAL() if (1) {} else qDebug()

#endif // QSLOGDISABLEFORTHISFILE_H
