import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import Torrent.RateController 1.0

import Material 0.1
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1

View{
    anchors.fill: parent
    Column{
        Label {
            text: "Download speed limit"
            wrapMode: Text.WordWrap
        }

        Slider {
            id: downloadSlider

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
            text: "Upload speed limit"
            wrapMode: Text.WordWrap
        }

        Slider {
            id: uploadSlider

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
}
