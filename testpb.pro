QT -= gui
QT += network


CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += ON_QT  #区分linux上的服务器程序与QT上的程序

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    msg.pb.cc \
    node.cpp \
    bucket.cpp \
    netengine.cpp \
    msgpack.cpp \
    utils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



win32{
    LIBS += -L$$PWD/lib/ -lprotobuf-lite -ludt -lebcCryptoLib
    LIBS += -lws2_32 -lwsock32
}

unix {
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
    include/ebcCryptoLib.h
