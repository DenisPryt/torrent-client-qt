import QtQuick 2.4

import Torrent.TorrentModel 1.0

import Material 0.1
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1


ListView{
    id : listView

    spacing: Units.dp( 4 )
    clip: true

    model: torrentModel
    delegate: TorrentDelegate {
        id: torrentDelegate
    }

    Scrollbar{
        flickableItem: listView
    }

    ActionButton {
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: Units.dp(32)
        }

        iconName: "content/add"
        onClicked: addTorrentDialog.open()
    }

}
