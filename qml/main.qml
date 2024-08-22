import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import io.qt.BackEnd 1.0
import io.qt.TcpDumpManager 1.0

Window
{
    id: main_window
    visible: true
    width: 700
    minimumWidth: 500
    height: 450
    minimumHeight: 200
    title: qsTr( "\"Road-2-Future\" Secure messenger  |  Server" )
    color: "#CED0D4"

    BackEnd
    {
        id: backend
        onSmbConnected_signal:
        {
            showNotification( qsTr( "Somebody has connected" ) );
        }
        onSmbDisconnected_signal:
        {
            showNotification( qsTr( "Somebody has disconnected" ) );
        }
        onNewMessage_signal:
        {
            showNotification( message );
        }
    }


    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter
            BetterButton
            {
                id: btn_start
                text: qsTr( "Start server" )
                color: enabled ? this.down ? "#78C37F" : "#87DB8D" : "gray"
                onClicked:
                {
                    showNotification( backend.startClicked() )
                    this.enabled = false
                    btn_anylyzer_start.enabled = true
                    btn_stop.enabled = true
                }
            }
            BetterButton
            {
                id: btn_stop
                enabled: !btn_start.enabled
                text: qsTr( "Stop server" )
                color: enabled ? this.down ? "#DB7A74" : "#FF7E79" : "gray"                
                onClicked:
                {
                    showNotification( backend.stopClicked() )
                    btn_start.enabled = true
                    btn_anylyzer_start.enabled = false
                    btn_anylyzer_stop.enabled = false
                    this.enabled = false
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
                    id: textArea
                    readOnly: true
                    selectByMouse : true
                    font.pixelSize: 14
                    wrapMode: TextInput.WordWrap
                }
            }
        }
        BetterButton
        {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr( "Server status" )
            color: this.down ? "#6FA3D2" : "#7DB7E9"
            border.color: "#6FA3D2"
            onClicked:
            {
                showNotification( backend.showServerStatusClicked() );
            }
        }
        TcpDumpManager
        {
                id: tcpDumpManager
                onTcppackage_signal:
                {
                    showTCPpackege( new_package );
                }
                onClassifyPacket_signal:
                {
                    showNotification ( classify_new_package );
                }
        }

        // Network Analyzer
        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter
            BetterButton
            {
                id: btn_anylyzer_start
                enabled: false
                Layout.alignment: Qt.AlignHCenter
                text: qsTr( "START Analyzer" )
                color: enabled ? this.down ? "#78C37F" : "#87DB8D" : "gray"
                border.color: "#6FA3D2"
                onClicked:
                {
                     this.enabled = false
                     btn_anylyzer_stop.enabled = true
                     main_window.height += 220
                     analysisTextSection.height = 200
                     analysisTextSection.visible = true
                     analysisTextSection.state = "visibleState"
                     btn_stop.enabled = false

                     tcpDumpManager.startTcpDump()
                }
            }
            BetterButton
            {
                id: btn_anylyzer_stop
                enabled: false
                Layout.alignment: Qt.AlignHCenter
                text: qsTr( "STOP Analyzer" )
                color: enabled ? this.down ? "#DB7A74" : "#FF7E79" : "gray"
                border.color: "#6FA3D2"
                onClicked:
                {
                    btn_anylyzer_start.enabled = true
                    this.enabled = false
                    main_window.height -= 220
                    analysisTextSection.height = 0
                    analysisTextSection.visible = false
                    btn_stop.enabled = true
                }
            }
        }
        LayoutSection {
            id: analysisTextSection
            width: parent.width
            height: 0
            visible: false
            ScrollView {
                anchors.fill: parent
                TextArea {
                    id: analysisTextArea
                    text: qsTr( "Raw tcp-packets:" )
                    wrapMode: TextArea.Wrap
                    readOnly: true
                    selectByMouse : true
                    font.pixelSize: 14
                }
            }
         }
    }

    Component.onCompleted:
    {
        showNotification( qsTr( "R2F-Messenger-Server program started" ) );
        btn_anylyzer_start.enabled = false;
        analysisTextSection.height = 0;
    }

    function showNotification( notification ) {
        textArea.append( addTimeToNotification( notification ) );
        // to always show the last notification in case TextArea is filled:
        textArea.cursorPosition = textArea.length;  // automatic ScrollDown
    }

    function showTCPpackege( new_package ) {
        analysisTextArea.append( new_package );
    }

    function addTimeToNotification( notification )
    {
        return "[" + currentTime() + "] " + notification;
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
