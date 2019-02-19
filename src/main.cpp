/*************************************************************************
    > File Name: udt.cpp
    > Author: hxe
    > Mail: nomail
    > Created Time: 2019年01月09日 星期三 09时01分42秒
 ************************************************************************/

#include<iostream>
#include <string.h>
#include "netengine.h"
#include "ebcCryptoLib.h"

#include "QsLog.h"

#ifdef ON_QT
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include "logsignal.h"
#endif

#ifdef ON_QT
//create an object "lg",whose function send signal to qml to show the message on the screen
logSignal lg;
//send signal to qml
void logFunction(const QString &message, QsLogging::Level level)
{
    emit lg.newLog(message, level);
}

#endif


int main(int argc, char *argv[])
{
#ifdef ON_QT
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    //register "lg" in qml named "log"
    engine.rootContext()->setContextProperty("log", &lg);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;


    using namespace QsLogging;

    // 1. init the logging mechanism
    Logger& logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath(QDir(app.applicationDirPath()).filePath("log.txt"));

    // 2. add two destinations
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
      sLogPath, EnableLogRotation, MaxSizeBytes(512), MaxOldLogCount(2)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    DestinationPtr functorDestination(DestinationFactory::MakeFunctorDestination(&logFunction));
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);
    logger.addDestination(functorDestination);

#else
	Cout::instance();
    QLOG_INFO() << "Program started ";
#endif

    NET::NodeId id;
    ebcCryptoLib cl;
    cl.randomNbytes(id.data(), 20);
    NET::Node::printNodeId(id);
    NET::NetEngine net(id);


#ifndef ON_QT
    net.startServer();
    std::thread cmd= std::thread([&net]()
    {
        //接收用户输入命令线程
        QLOG_INFO()<<"Usege: ";
        QLOG_INFO()<<"0: Print all node info";
        QLOG_INFO()<<"q: quit the app";

        char c=0;
        while((c=getchar()) != 'q')
        {
            switch(c)
            {
            case '0':
                net.printNodesInfo();
                break;
            case '1':
                net.printNodesInfo(1);
                break;
            case '\n':
                break;
            default:
                QLOG_INFO()<<"Unkown cmd!";
                break;
            }
        }
    }
    );

    net.sendHello();

    cmd.join();
    return 0;
#else

    net.startClient();
    return app.exec();
#endif
}

