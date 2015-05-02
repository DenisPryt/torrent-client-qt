import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import Torrent.TorrentModel 1.0

import Material 0.1
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1

ApplicationWindow {
    id: root

    visible: true
    clientSideDecorations: false
    theme {
        accentColor: "#009688"
    }

    property var speed2Name: function( bytesPerSecond ){
        if ( bytesPerSecond < 1000 )
            return bytesPerSecond + " B";
        if ( bytesPerSecond < 1024000 )
            return Math.round( bytesPerSecond / 1024 * 100 ) / 100 + " Kb"
        if ( bytesPerSecond < 1048576000 )
            return Math.round( bytesPerSecond / 1048576 * 100 ) / 100 + " Mb"
        else
            return Math.round( bytesPerSecond / 1073741824 * 100 ) / 100 + " Gb"
    }

    property var sidebarContent: [
        "All", "Downloading", "Seeding", "Paused", "Speed settings"
    ]

    property string selectedSidebarContent: sidebarContent[0]

    initialPage: Page {
        title: "Torrent Client Qt"

        actionBar.maxActionCount: 4
        actions: [
            Action{
                id : addTorrentAction
                name: "New Torrent"
                iconName: "action/note_add"
                onTriggered: addTorrentDialog.open()
            },

            Action {
                iconName: "av/play_arrow"
                name: "Play"
                onTriggered: {
                    for ( var i = 0; i < torrentModel.count; ++i ){
                        var res = torrentModel.data( i, TorrentModel.TorState );
                        if ( res == TorrentModel.Paused )
                            torrentModel.setPause( i, false )
                    }
                }
            },

            Action {
                iconName: "av/pause"
                name: "Pause"
                onTriggered: {
                    for ( var i = 0; i < torrentModel.count; ++i ){
                        var res = torrentModel.data( i, TorrentModel.TorState );
                        if ( res == TorrentModel.Downloading || res == TorrentModel.Seeding )
                            torrentModel.setPause( i, true )
                    }
                }
            },

            Action {
                iconName: "image/color_lens"
                name: "Color"
                onTriggered: colorPicker.show()
            },

            Action {
                iconName: "action/language"
                name: "Language"
            },

            Action {
                iconName: "action/settings"
                name: "Settings"
            }
        ]

        Sidebar {
            id: sidebar

            Column {
                width: parent.width

                Repeater {
                    model: root.sidebarContent
                    delegate: ListItem.Standard {
                        text: modelData
                        selected: modelData == selectedSidebarContent
                        onClicked: selectedSidebarContent = modelData
                    }
                }
            }
        }
        ListView{
            id : listView

            spacing: units.dp( 4 )
            anchors {
                margins: units.dp( 6 )
                left: sidebar.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            clip: true

            model: torrentModel
            delegate: ListItem.Subtitled {
                id: torrentDelegate

                width: listView.width

                text: TorName

                content : Item{
                    id: torSpeed
                    RowLayout{
                        Icon{
                            color: TorState == TorrentModel.Downloading
                                   ? theme.accentColor : Palette.colors['grey']['500']
                            size: units.dp( 15 )
                            name: "file/file_download"
                        }
                        Label{
                            Layout.fillWidth: true
                            Layout.minimumWidth: units.dp(48)
                            text: speed2Name( TorDownSpeed )
                        }
                        Icon{
                            color: TorState == TorrentModel.Downloading ||
                                   TorState == TorrentModel.Seeding
                                   ? theme.accentColor : Palette.colors['grey']['500']
                            size: units.dp( 15 )
                            name: "file/file_upload"
                        }
                        Label{
                            text: speed2Name( TorUpSpeed )
                        }
                    }
                }

                secondaryItem: Item{
                    id: progressLoader
                    anchors{
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }

                    height: torrentDelegate.height - units.dp( 16 )
                    width: height

                    Icon{
                        color: theme.primaryColor
                        name: "action/done"
                        size: parent.height
                        visible: progressCircle.value == progressCircle.maximumValue
                    }

                    ProgressCircle {
                        id: progressCircle

                        anchors.fill: parent

                        indeterminate: false
                        minimumValue: 0
                        maximumValue: 100

                        visible: progressCircle.value < progressCircle.maximumValue
                        color: TorState == TorrentModel.Preparing ? Palette.colors['yellow']['700']
                                                                  : Theme.primaryColor
                        value: TorProgress < 0 ? 0 : TorProgress
                        Behavior on value{
                            NumberAnimation { duration: 100 }
                        }

                        Label {
                            color: TorState == TorrentModel.Preparing ? Palette.colors['yellow']['900']
                                                                      : Theme.primaryColor
                            style: 'subheading'
                            anchors.centerIn: parent
                            text: Math.round(TorProgress) + "%"
                        }
                    }
                }

                action: Icon {
                    id: playPauseIcon
                    anchors.centerIn: parent

                    name: TorState == TorrentModel.Paused ? "av/play_arrow" : "av/pause"
                    size: torrentDelegate.height - units.dp( 32 )

                    MouseArea{
                        id: playPauseArea
                        anchors.fill: parent
                        onClicked: {
                            if ( TorState == TorrentModel.Downloading || TorState == TorrentModel.Seeding )
                                torrentModel.setPause( index, true )
                            else if ( TorState == TorrentModel.Paused )
                                torrentModel.setPause( index, false )
                        }
                    }
                }
            }

            AddTorrentDialog{
                id : addTorrentDialog
                onAccepted: torrentModel.addTorrent( fileUrl, "d:\\testTorrents" )
            }

            Dialog {
                id: colorPicker
                title: "Pick color"

                positiveButtonText: "Done"

                MenuField {
                    id: selection
                    model: ["Primary color", "Accent color", "Background color"]
                    width: units.dp(160)
                }

                Grid {
                    columns: 7
                    spacing: units.dp(8)

                    Repeater {
                        model: [
                            "red", "pink", "purple", "deepPurple", "indigo",
                            "blue", "lightBlue", "cyan", "teal", "green",
                            "lightGreen", "lime", "yellow", "amber", "orange",
                            "deepOrange", "grey", "blueGrey", "brown", "black",
                            "white"
                        ]

                        Rectangle {
                            width: units.dp(30)
                            height: units.dp(30)
                            radius: units.dp(2)
                            color: Palette.colors[modelData]["500"]
                            border.width: modelData === "white" ? units.dp(2) : 0
                            border.color: Theme.alpha("#000", 0.26)

                            Ink {
                                anchors.fill: parent

                                onPressed: {
                                    switch(selection.selectedIndex) {
                                    case 0:
                                        theme.primaryColor = parent.color
                                        break;
                                    case 1:
                                        theme.accentColor = parent.color
                                        break;
                                    case 2:
                                        theme.backgroundColor = parent.color
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                onRejected: {
                    // TODO set default colors again but we currently don't know what that is
                }
            }

        }
    }
}
