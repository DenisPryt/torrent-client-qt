import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import Torrent.RateController 1.0

import Material 0.1
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1

ColumnLayout{

    Label {
        text: "Numeric Value Label + Active Focus on Press"
        wrapMode: Text.WordWrap
        Layout.alignment:  Qt.AlignBottom
    }

    Slider {
        id: downloadSlider

        Layout.alignment: Qt.AlignCenter
        focus: true
        tickmarksEnabled: true
        numericValueLabel: true
        stepSize: 10
        minimumValue: 0
        maximumValue: 1000
        activeFocusOnPress: true

        onValueChanged: {
            RateController.downloadLimit = value * 1024
        }

        Component.onCompleted: {
            value = RateController.downloadLimit / 1024
        }
    }

    Label {
        text: "Numeric Value Label + Active Focus on Press"
        wrapMode: Text.WordWrap
        Layout.alignment:  Qt.AlignBottom
    }

    Slider {
        id: uploadSlider

        Layout.alignment: Qt.AlignCenter
        focus: true
        tickmarksEnabled: true
        numericValueLabel: true
        stepSize: 10
        minimumValue: 0
        maximumValue: 1000
        activeFocusOnPress: true

        onValueChanged: {
            RateController.uploadLimit = value * 1024
        }

        Component.onCompleted: {
            value = RateController.uploadLimit / 1024
        }
    }

}
