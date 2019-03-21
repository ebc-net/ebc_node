#ifdef ON_QT
#include "logsignal.h"
#include "QsLog.h"
#include"bucket.h"
#include"netengine.h"




void logSignal::start()
{
    kad->dump(0);
}

void logSignal::searchId(std::string &id)
{
    eng->startSearch(id);
}

#endif
