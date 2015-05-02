import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

FileDialog{
    id : root
    title: "Please choose a file"
    nameFilters: [ "Torrent files (*.torrent)", "All files (*)" ]
    onRejected: {
        console.log("Canceled")
    }
}

