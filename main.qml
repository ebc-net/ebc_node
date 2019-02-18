import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.4


Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("ebc")
    color: "#C0C0C0"

    Connections {
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

    Rectangle
    {
        id: rectangle
        anchors.horizontalCenter: parent.horizontalCenter
        width:  parent.width*2/3
        height: parent.height*4/5
        color: "#C7EDCC"
        clip: true

        ScrollView
        {
           contentWidth: parent.width
            id: view
            anchors.fill: parent



            Text {
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
}

