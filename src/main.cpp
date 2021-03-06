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

#ifdef ON_QT
#include <QCoreApplication>
#endif


int main(int argc, char *argv[])
{
#ifdef ON_QT
    QCoreApplication a(argc, argv);
#endif
    NET::NodeId id;

    ebcCryptoLib cl;
    cl.randomNbytes(id.data(), 20);
    NET::Node::printNodeId(id);

    NET::NetEngine net(id);
    net.startClient();


    std::thread cmd= std::thread([&net]()
    {
        //接收用户输入命令线程
        std::cout<<"Usege: "<<std::endl;
        std::cout<<"0: Print all node info"<<std::endl;
        std::cout<<"q: quit the app"<<std::endl;

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
                std::cout<<"Unkown cmd!"<<std::endl;
                break;
            }
        }
    }
    );

    net.sendHello();

    cmd.join();
    return 0;
}

