import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.4


Window
{

    Connections
    {
        id: conn_id
        target: log
        onNewLog:
        {
            if(level==2)
            {
                output_text.text+='<br />'+'<font color="green">&nbsp;'+str+'</font>'
            }
            else if(level==3)
            {
                output_text.text+='<br />'+'<font color="#ff8000">&nbsp;'+str+'</font>'
            }
            else if(level==4)
            {
                output_text.text+='<br />'+'<font color="red">&nbsp;'+str+'</font>'
            }
        }
    }
    visible: true
    width: 700
    height: 480
    title: qsTr("ebc")
    color: "#C0C0C0"
    Rectangle
    {
        id: rectangle
        anchors.horizontalCenter: parent.horizontalCenter
        width:  parent.width
        height: parent.height*3/4
        color: "#C7EDCC"
        clip: true

        ScrollView
        {
            contentWidth: parent.width
            id: view
            anchors.fill: parent

            Text
            {
                id: output_text
                width:  parent.width
                height: parent.height
                anchors.top:parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                color:  "green"
                font.pixelSize: 13
                lineHeight:1.2
                wrapMode: Text.Wrap
            }
        }
    }
    TextField
    {
        id: node_id
        width: 330
        anchors.horizontalCenter: rectangle.horizontalCenter
        anchors.top: rectangle.bottom
        maximumLength: 48
    }

    TextField
    {
        id: data_stream
        width: 330
        anchors.horizontalCenter: rectangle.horizontalCenter
        anchors.bottom: start_btn.top
    }
    Text
    {
        id: text1
        height:node_id.height
        anchors.right: node_id.left
        anchors.top: rectangle.bottom
        text: qsTr("search node id:")
        font.pixelSize: 18
        color: "blue"
    }

    Text
    {
        id: text2
        height:node_id.height
        anchors.right: data_stream.left
        anchors.bottom: start_btn.top
        text: qsTr("send data stream:")
        font.pixelSize: 18
        color:"green"
    }
    Button
    {
        id: start_btn
        anchors.bottom: parent.bottom;
        anchors.horizontalCenter: parent.horizontalCenter;
        text: "start"
        onClicked:
        {
            console.log("start...")
            log.start()
        }
    }

    Button
    {
        id: search_btn
        anchors.bottom: parent.bottom
        anchors.right: start_btn.left
        text: "search"
        onClicked:
        {
            console.log("searching   "+node_id.text)
            log.searchId(node_id.text)
        }
    }
    Button
    {
        id: send_btn
        anchors.bottom: parent.bottom
        anchors.right: search_btn.left
        text: "send"
        onClicked:
        {
            console.log("send data stream to "+node_id.text)
            log.sendData(node_id.text,data_stream.text,data_stream.text)
        }
    }
    Button
    {
        id: clean_text_btn
        anchors.bottom: parent.bottom
        anchors.left: start_btn.right
        text: "clean text"
        onClicked:
        {
            output_text.text =""
        }
    }
    Button
    {
        id: selfid
        anchors.bottom: parent.bottom
        anchors.left: clean_text_btn.right
        text: "selfId"
        onClicked:
        {

            console.log("My ID ")
            log.printSelfId()
        }
    }
    Button
    {
        id: deletenode
        anchors.bottom: parent.bottom
        anchors.left: selfid.right
        text: "delete"
        onClicked:
        {
            console.log("delete   "+node_id.text)
            log.deleteNode(node_id.text)
        }
    }
    Button
    {
        id: clean_search_btn
        anchors.bottom: parent.bottom
        anchors.right: send_btn.left
        text: "cleanSearch"
        onClicked:
        {
            node_id.text = ""
            data_stream.text = ""
        }
    }
//    Button
//    {
//        id: egg
//        width: 6
//        height:6
//        anchors.bottom: parent.bottom
//        anchors.right: parent.right
//         text: ""
//         onClicked:
//         {
//             node_id.text = "*****  彩蛋彩蛋 (:D)   还有一个彩蛋哦   *******"
//         }
//    }
//    Button
//    {
//        id: egg2
//        width: 2
//        anchors.bottom: parent.bottom
//        anchors.right: start_btn.left
//         text: ""
//         onClicked:
//         {
//             node_id.text = "************    彩蛋中的彩蛋 (:D) 真棒   *****************"
//             data_stream.text = "************    彩蛋中的彩蛋 (:D) 真棒   *****************"
//         }
//    }
}

