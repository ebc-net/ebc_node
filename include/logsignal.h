#ifndef LOGSIGNAL_H
#define LOGSIGNAL_H

#include <QObject>


class logSignal :public QObject
{
    Q_OBJECT

public:
    logSignal();
    Q_INVOKABLE void start();
signals:
    void newLog( QString str,int level);


};

#endif // LOGSIGNAL_H
