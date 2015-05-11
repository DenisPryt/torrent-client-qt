import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import Torrent.TorrentModel 1.0

import Material 0.1
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1

ListItem.Subtitled {
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

            indeterminate: TorState != TorrentModel.Downloading &&
                           TorState != TorrentModel.Seeding &&
                           TorState != TorrentModel.Preparing &&
                           TorState != TorrentModel.Paused

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
                visible: !parent.indeterminate
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
