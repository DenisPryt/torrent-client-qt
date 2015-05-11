import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import Torrent.TorrentModel 1.0
import Torrent.RateController 1.0

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

            property var sidebarContent: [
                "All", "Downloading", "Seeding", "Paused", "Speed settings"
            ]

            property string selectedSidebarContent: sidebarContent[0]

            Column {
                width: parent.width

                Repeater {
                    model: sidebar.sidebarContent
                    delegate: ListItem.Standard {
                        text: modelData
                        selected: modelData == sidebar.selectedSidebarContent
                        onClicked: sidebar.selectedSidebarContent = modelData
                    }
                }
            }
        }

        Loader{
            id: pageLoader

            anchors {
                margins: units.dp( 6 )
                left: sidebar.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }

            asynchronous: true

            source: sidebar.selectedSidebarContent === "Speed settings"
                    ? "SpeedLimit.qml" : "TorrentListView.qml"
        }

        AddTorrentDialog{
            id : addTorrentDialog
            onAccepted: torrentModel.addTorrent( torrentFileUrl, destinationFolder )
        }

        ColorPickerDialog {
            id: colorPicker
        }

    }
}
