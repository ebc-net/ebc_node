// Copyright (c) 2013, Razvan Petru
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSLOG_H
#define QSLOG_H

#ifdef ON_QT
#include "QsLogLevel.h"
#include "QsLogDest.h"
#include <QDebug>
#include <QString>

#define QS_LOG_VERSION "2.0b3"

namespace QsLogging
{
class Destination;
class LoggerImpl; // d pointer

class QSLOG_SHARED_OBJECT Logger
{
public:
    static Logger& instance();
    static void destroyInstance();
    static Level levelFromLogMessage(const QString& logMessage, bool* conversionSucceeded = 0);

    ~Logger();

    //! Adds a log message destination. Don't add null destinations.
    void addDestination(DestinationPtr destination);
    //! Logging at a level < 'newLevel' will be ignored
    void setLoggingLevel(Level newLevel);
    //! The default level is INFO
    Level loggingLevel() const;
    //! Set to false to disable timestamp inclusion in log messages
    void setIncludeTimestamp(bool e);
    //! Default value is true.
    bool includeTimestamp() const;
    //! Set to false to disable log level inclusion in log messages
    void setIncludeLogLevel(bool l);
    //! Default value is true.
    bool includeLogLevel() const;

    //! The helper forwards the streaming to QDebug and builds the final
    //! log message.
    class QSLOG_SHARED_OBJECT Helper
    {
    public:
        explicit Helper(Level logLevel) :
            level(logLevel),
            qtDebug(&buffer)
        {}
        ~Helper();
        QDebug& stream(){ return qtDebug; }

    private:
        void writeToLog();

        Level level;
        QString buffer;
        QDebug qtDebug;
	};

private:
    Logger();
    Logger(const Logger&);            // not available
    Logger& operator=(const Logger&); // not available

    void enqueueWrite(const QString& message, Level level);
    void write(const QString& message, Level level);

    LoggerImpl* d;

    friend class LogWriterRunnable;
};

} // end namespace

//! Logging macros: define QS_LOG_LINE_NUMBERS to get the file and line number
//! in the log output.
#define QLOG_INFO()\
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream()<< __LINE__
#define QLOG_WARN()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream()<< __LINE__
#define QLOG_ERROR() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream()<< __LINE__


#ifdef QS_LOG_DISABLE
#include "QsLogDisableForThisFile.h"
#endif

#else
//////////////////////////////////////ON LINUX
#include <stdio.h>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <mutex>


class Cout
{
public:
    Cout();
    ~Cout();

    static Cout* instance();
    void writeFile(std::string);
    class logBuff
    {
        public:
            #define RESET   "\033[0m"
			#define RED     "\033[1m\033[31m"      /* Red */
            #define GREEN   "\033[1m\033[32m"      /* Green */
            #define YELLOW  "\033[1m\033[33m"      /* Yellow */

            typedef enum _LEVEL
            {
                INFO = 0,
                WARNING,
                ERROR,
            }LEVEL;

            explicit logBuff(LEVEL l, class Cout* c):level(l),pout(c)
            {
                if(INFO == level )
                  sbuf += "[INFO ] ";
                else if(WARNING == level)
                  sbuf += "[WARN ] ";
                else if(ERROR == level)
                  sbuf += "[ERROR] ";

                std::stringstream ss;
                auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                ss<<std::put_time(std::localtime(&t), "%m-%d %T");
                sbuf += ss.str()+" ";
            }

            logBuff& operator<<(bool value){sbuf += value?"true":"false"; return* this;}
            logBuff& operator<<(float value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(double value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(int value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(unsigned int value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(short value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(unsigned short value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(long value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(unsigned long value){sbuf += std::to_string(value); return* this;}
            logBuff& operator<<(const char* str){sbuf += str; return* this;}


            ~logBuff()
            {
                sbuf += "\n";
                if(INFO == level )
                  printf(GREEN "%s" RESET, sbuf.c_str());
                else if(WARNING == level)
                  printf(YELLOW "%s" RESET, sbuf.c_str());
                else if(ERROR == level)
                  printf(RED "%s" RESET, sbuf.c_str());

                pout->writeFile(sbuf);
            }

        private:
            std::string sbuf;
            LEVEL level;
            class Cout *pout;
    };

private:
    std::ofstream logFile;
    std::mutex flock;
};

#define QLOG_INFO() \
	    Cout::logBuff(Cout::logBuff::LEVEL::INFO, Cout::instance())

#define QLOG_WARN() \
	    Cout::logBuff(Cout::logBuff::LEVEL::WARNING, Cout::instance())

#define QLOG_ERROR() \
	    Cout::logBuff(Cout::logBuff::LEVEL::ERROR, Cout::instance())

#endif //ON_QT

#endif // QSLOG_H
