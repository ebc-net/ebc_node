/*************************************************************************
  > File Name: udt.cpp
  > Author: hxe
  > Mail: nomail
  > Created Time: 2019年01月09日 星期三 09时01分42秒
 ************************************************************************/

#include<iostream>
#include <string.h>
#include "netengine.h"
#include "bucket.h"
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
#else
// run on linux
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using printNodeFun = std::function<void(int type)>;
printNodeFun printNode;

void signal_handler(int sig)
{
    switch(sig)
    {
    case SIGHUP:
        break;
    case SIGUSR1:
        printNode(0);
        break;
    case SIGUSR2:
        printNode(1);
        break;
    case SIGINT:
    case SIGTERM:
        QLOG_ERROR() << "ebc was killed!";
        exit(0);
        break;
    }
}

void setupSignals()
{
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, signal_handler); /* catch hangup signal */
    signal(SIGINT, signal_handler); /* catch interrupt signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */
    signal(SIGUSR1, signal_handler); /* catch kill signal */
    signal(SIGUSR2, signal_handler); /* catch kill signal */
}

void daemonize()
{
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    pid_t sid = setsid();
    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    setupSignals();
}


#endif

void getRandom(unsigned char *buf, int len)
{
    std::mt19937 rd(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<char> rb(0);
    for(int i = 0; i < len; i++)
    {
        buf[i] = rb(rd);
    }
}

using namespace NET;
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
    //logger.addDestination(fileDestination);
    logger.addDestination(functorDestination);

#else
    Cout::instance();
    QLOG_INFO() << "Program started ";
#endif
//    NET::NodeId id;
//    getRandom(id.data(), 24);
//    id.at(0) = 'e';
//    id.at(1) = 'b';
//    id.at(2) = 'c';
    ebcCryptoLib cl;
    NET::NodeId id;
    cl.randomNbytes(id.data(), ID_LENGTH);
    Sp<NET::Bucket> kad = std::make_shared <Bucket>(id);
    NET::NetEngine net(id, kad);
    //服务器或是客户机的K桶

#ifndef ON_QT
    if(argc >= 2 && !strcmp(argv[1], "-d"))
    {
        printNode = [&kad](int type)
        {
            kad->dump(type);
        };
        daemonize();
        net.startServer();

        while(1)
            sleep(30);
    }
    else
    {
        net.startServer();
        //net.startClient();
        std::thread cmd = std::thread([&net, &id, &cl, &kad]()
        {
            //接收用户输入命令线程
            QLOG_INFO() << "Usege: ";
            QLOG_INFO() << "0: Print all node info";
            QLOG_INFO() << "q: quit the app";

            char c = 0;
            while((c = getchar()) != 'q')
            {
                switch(c)
                {
                case '0':
                    kad->dump(0);
                    break;
                case '1':
                    kad->dump(1);
                    break;
                case '3':
                    cl.randomNbytes(id.data(), ID_LENGTH);
                    QLOG_INFO() << "search id = ";
                    id.printNodeId();
                    net.startSearch(id);
                    break;
                case '\n':
                    break;
                default:
                    QLOG_ERROR() << "invalid cmd!";
                    break;
                }
            }
        }
                                     );

        cmd.join();
    }
    return 0;
#else
    lg.passKad(kad, &net);
    net.startClient();
    std::thread recvThread = std::thread([&]()
    {
        std::string buf;
        while(1)
        {
            int ret = net.getUserDate(buf);
            if(ret)
            {

                std::cout << "main:ret = " << ret << "value = " << buf << std::endl;
                std::cout << "value len = " << buf.size() << std::endl;
//                  QLOG_WARN()<<"ret = "<<ret;
//                  QLOG_WARN()<<"main:value len = "<<buf.size();
            }
            Sleep(5000);
            buf.clear();

        }

    });
    recvThread.detach();

    return app.exec();
#endif

}
