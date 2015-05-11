import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import Qt.labs.settings 1.0

import Material 0.1
import Material.Extras 0.1

Dialog{
    id: root

    title: "Add new torrent"
    positiveButtonText: "Done"

    property alias torrentFileUrl       : torrentFileDialog.fileUrl
    property alias destinationFolder    : destFolderDialog.folder

    Column{
        TextField {
            text: torrentFileDialog.fileUrl
            placeholderText: "Torrent file"
            floatingLabel: true

            input.readOnly: true

            MouseArea{
                anchors.fill: parent
                onClicked: torrentFileDialog.open()
            }

            anchors.horizontalCenter: parent.horizontalCenter
        }

        TextField {
            text: destFolderDialog.folder
            placeholderText: "Destination folder"
            floatingLabel: true

            input.readOnly: true

            MouseArea{
                anchors.fill: parent
                onClicked: destFolderDialog.open()
            }

            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Settings{
        category: "LastFolders"
        property alias torrentFile    : torrentFileDialog.folder
        property alias destination    : destFolderDialog.folder
    }

    FileDialog{
        id : destFolderDialog

        title: "Please choose a destination folder"

        selectFolder: true
        selectExisting: false
        selectMultiple: false
    }

    FileDialog{
        id : torrentFileDialog

        title: "Please choose a file"
        nameFilters: [ "Torrent files (*.torrent)", "All files (*)" ]
    }

}
