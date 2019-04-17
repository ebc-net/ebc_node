#lib
QT += quick
CONFIG += c++11
CONFIG += staticlib
TARGET = ebcNet
TEMPLATE = lib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += ON_QT  #区分linux上的服务器程序与QT上的程序
DEFINES += ebcNet_LIBRARY

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/msg.pb.cpp \
    src/nodeid.cpp \
    src/node.cpp \
    src/bucket.cpp \
    src/netengine.cpp \
    src/msgpack.cpp \
    src/utils.cpp \
    src/search.cpp \
    src/ebcMP2PNetWorkAPI.cpp

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



win32{
PRE_TARGETDEPS += $$PWD/lib/win/libudt.a
PRE_TARGETDEPS += $$PWD/lib/win/libprotobuf-lite.a
PRE_TARGETDEPS += $$PWD/lib/win/libQsLog.a

    LIBS += -L$$PWD/lib/win -lprotobuf-lite -ludt -lQsLog
    LIBS += -lws2_32 -lwsock32
}

macx{
    LIBS += -L$$PWD/lib/mac -lprotobuf-lite -ludt -lQsLog
}
ios{
    LIBS += -L$$PWD/lib/ios -lprotobuf-lite -ludt -lQsLog
}

unix{
    ANDROID_EXTRA_LIBS = $$PWD/lib/android/libprotobuf-lite.so $$PWD/lib/android/libudt.so $$PWD/lib/android/libQsLog.so
    LIBS += -L$$PWD/lib/android -lprotobuf-lite -ludt -lQsLog
}

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

HEADERS += \
    include/msg.pb.h \
    include/node.h \
    include/udt.h \
    include/sockaddr.h \
    include/bucket.h \
    include/netengine.h \
    include/utils.h \
    include/msgpack.h \
    include/QsLog.h \
    include/nodeid.h \
    include/search.h \
    include/ebcMP2PNetWorkAPI.h




#QT += quick
#CONFIG += c++11
##CONFIG += staticlib
##TARGET = ebcNet
##TEMPLATE = lib

## The following define makes your compiler emit warnings if you use
## any feature of Qt which as been marked deprecated (the exact warnings
## depend on your compiler). Please consult the documentation of the
## deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += ON_QT  #区分linux上的服务器程序与QT上的程序
##DEFINES += ebcNet_LIBRARY

## You can also make your code fail to compile if you use deprecated APIs.
## In order to do so, uncomment the following line.
## You can also select to disable deprecated APIs only up to a certain version of Qt.
##DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#SOURCES += \
#    src/msg.pb.cpp \
#    src/nodeid.cpp \
#    src/node.cpp \
#    src/bucket.cpp \
#    src/netengine.cpp \
#    src/msgpack.cpp \
#    src/utils.cpp \
#    src/search.cpp \
#    src/ebcMP2PNetWorkAPI.cpp \
#    src/main.cpp \
#    src/logsignal.cpp
## Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target



#win32{
#PRE_TARGETDEPS += $$PWD/lib/win/libudt.a
#PRE_TARGETDEPS += $$PWD/lib/win/libprotobuf-lite.a
#PRE_TARGETDEPS += $$PWD/lib/win/libebcCryptoLib.a
#PRE_TARGETDEPS += $$PWD/lib/win/libQsLog.a

#    LIBS += -L$$PWD/lib/win -lprotobuf-lite -ludt -lebcCryptoLib -lQsLog
#    LIBS += -lws2_32 -lwsock32
#}

#macx{
#    LIBS += -L$$PWD/lib/mac -lprotobuf-lite -ludt -lebcCryptoLib -lQsLog
#}
#ios{
#    LIBS += -L$$PWD/lib/ios -lprotobuf-lite -ludt -lebcCryptoLib -lQsLog
#}

#unix{
#    ANDROID_EXTRA_LIBS = $$PWD/lib/android/libprotobuf-lite.so $$PWD/lib/android/libudt.so  $$PWD/lib/android/libebcCryptoLib.so $$PWD/lib/android/libQsLog.so
#    LIBS += -L$$PWD/lib/android -lprotobuf-lite -ludt -lebcCryptoLib -lQsLog
#}

#INCLUDEPATH += $$PWD/include
#DEPENDPATH += $$PWD/include

#HEADERS += \
#    include/msg.pb.h \
#    include/node.h \
#    include/udt.h \
#    include/sockaddr.h \
#    include/bucket.h \
#    include/netengine.h \
#    include/utils.h \
#    include/msgpack.h \
#    include/ebcCryptoLib.h \
#    include/QsLog.h \
#    include/nodeid.h \
#    include/search.h \
#    include/ebcMP2PNetWorkAPI.h \
#    include/logsignal.h

#RESOURCES += \
#   qml.qrc

#DISTFILES +=
