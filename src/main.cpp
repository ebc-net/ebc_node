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
#include "bucket.h"

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
	switch(sig) {
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
			QLOG_ERROR()<<"ebc was killed!";
			exit(0);
			break;
	}
}

void setupSignals()
{
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
	signal(SIGINT,signal_handler); /* catch interrupt signal */
	signal(SIGTERM,signal_handler); /* catch kill signal */
}

void daemonize()
{
	pid_t pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS);

	umask(0);

	pid_t sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	setupSignals();
}


#endif

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
	logger.addDestination(fileDestination);
	logger.addDestination(functorDestination);

#else
	Cout::instance();
	QLOG_INFO() << "Program started ";
#endif
    NET::NodeId id;
    ebcCryptoLib cl;
    cl.randomNbytes(id.data(), ID_LENGTH);
    id.printNodeId();
    NET::Bucket kad(id);
    NET::NetEngine net(id,kad);
    //服务器或是客户机的K桶

#if 1
#ifndef ON_QT
	if(argc >= 2 && !strcmp(argv[1], "-d"))	
	{
        printNode = [&kad](int type)
		{
			if(type == 0)
                kad.dump();
//				net.printNodesInfo();
//			else if(type == 1)
//				net.printNodesInfo(1);

		};
		daemonize();

		while(1)
		  sleep(30);
	}
	else
	{
		net.startClient();
        std::thread cmd= std::thread([&kad]()
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
                    kad.dump();
					break;
					case '1':
                    kad.dump();;
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

		cmd.join();
	}
	return 0;
#else
    net.startClient();
    return app.exec();
#endif
#endif
}

