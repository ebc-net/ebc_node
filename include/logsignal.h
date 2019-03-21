#ifdef ON_QT
#ifndef LOGSIGNAL_H
#define LOGSIGNAL_H

#include <QObject>
#include "bucket.h"
#include"search.h"
#include"netengine.h"
#include "utils.h"


class logSignal :public QObject
{
    Q_OBJECT

public:
    logSignal():kad(nullptr),eng(nullptr){}

    Q_INVOKABLE void start();
    Q_INVOKABLE void searchId(QString );

    void passKad(NET::Sp<NET::Bucket> B,NET::NetEngine *E){ kad = B;eng = E;}

signals:
    void newLog( QString str,int level);

private:
    NET::Sp<NET::Bucket> kad;
    NET::NetEngine *eng;


};

#endif // LOGSIGNAL_H

#endif
