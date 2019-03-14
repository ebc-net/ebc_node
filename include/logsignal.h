#ifdef ON_QT
#ifndef LOGSIGNAL_H
#define LOGSIGNAL_H

#include <QObject>
#include "bucket.h"


class logSignal :public QObject
{
    Q_OBJECT

public:
    logSignal():kad(nullptr){}
    Q_INVOKABLE void start();
    void passKad(NET::Bucket* B){ kad = B;}

signals:
    void newLog( QString str,int level);

private:
    NET::Bucket *kad;


};

#endif // LOGSIGNAL_H

#endif
