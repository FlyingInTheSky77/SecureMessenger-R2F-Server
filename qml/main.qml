import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import io.qt.BackEnd 1.0

Window
{
    visible: true
    width: 700
    minimumWidth: 500
    height: 450
    minimumHeight: 200
    title: qsTr( "Road2Future Messanger - Server" )
    color: "#CED0D4"

    BackEnd
    {
        id: backend
        onSmbConnected_signal:
        {
            ti.append( addMsg( qsTr( "Somebody has connected" ) ) );
        }
        onSmbDisconnected_signal:
        {
            ti.append( addMsg( qsTr( "Somebody has disconnected" ) ) );
        }
        onNewMessage_signal:
        {
            ti.append( addMsg( message ) );
        }
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        RowLayout
        {
            anchors.horizontalCenter: parent.horizontalCenter
            BetterButton
            {
                id: btn_start
                anchors.left: parent.left
                text: qsTr( "Start server" )
                color: enabled ? this.down ? "#78C37F" : "#87DB8D" : "gray"
                onClicked:
                {
                    ti.append( addMsg( backend.startClicked() ) );
                    this.enabled = false;
                }
            }
            BetterButton
            {
                enabled: !btn_start.enabled
                anchors.right: parent.right
                text: qsTr( "Stop server" )
                color: enabled ? this.down ? "#DB7A74" : "#FF7E79" : "gray"                
                onClicked:
                {
                    ti.append( addMsg( backend.stopClicked() ) );
                    btn_start.enabled = true;
                }
            }
        }
        LayoutSection
        {
            Layout.fillHeight: true
            ScrollView
            {
                id: scrollView
                anchors.fill: parent
                TextArea
                {
                    id: ti
                    readOnly: true
                    selectByMouse : true
                    font.pixelSize: 14
                    wrapMode: TextInput.WordWrap
                }
            }
        }
        BetterButton
        {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr( "Test connection" )
            color: this.down ? "#6FA3D2" : "#7DB7E9"
            border.color: "#6FA3D2"
            onClicked:
            {
                ti.append( addMsg( backend.testConnectionClicked() ) );
            }
        }
    }

    Component.onCompleted:
    {
        ti.text = addMsg( qsTr( "R2F-Messanger-Server started\n- - - - - -" ) );
    }

    function addMsg( someText )
    {
        return "[" + currentTime() + "] " + someText;
    }

    function currentTime()
    {
        var now = new Date();
        var nowString = ( "0" + now.getHours() ).slice( -2 ) + ":"
                + ( "0" + now.getMinutes() ).slice( -2 ) + ":"
                + ( "0" + now.getSeconds() ).slice( -2 );
        return nowString;
    }
}
