#ifdef ON_QT
#include "logsignal.h"
#include "QsLog.h"
#include"bucket.h"




void logSignal::start()
{
    kad->dump();
}

#endif
