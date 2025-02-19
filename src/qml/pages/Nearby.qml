/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Nearby Waypoints")
    focus: true

    header: StandardHeader {}

    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

        currentIndex: sv.currentIndex

        TabButton { text: qsTr("AD") }
        TabButton { text: qsTr("NAV") }
        TabButton { text: qsTr("REP") }
        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        clip: true
        currentIndex: bar.currentIndex

        Component {
            id: waypointDelegate

            Item {
                width: sv.width
                height: idel.height

                // Background color according to METAR/FAA flight category
                Rectangle {
                    anchors.fill: parent
                    color: model.modelData.hasMETAR ? model.modelData.weatherStation.metar.flightCategoryColor : "transparent"
                    opacity: 0.2
                }

                WordWrappingItemDelegate {
                    id: idel

                    width: sv.width

                    icon.source: model.modelData.icon

                    text: {
                        // Mention horizontal distance
                        Navigator.aircraft.horizontalDistanceUnit

                        var result = model.modelData.twoLineTitle

                        var wayTo  = Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), model.modelData.coordinate)
                        if (wayTo !== "")
                            result = result + "<br>" + wayTo

                        if (model.modelData.hasMETAR)
                            result = result + "<br>" + model.modelData.weatherStation.metar.summary
                        return result
                    }

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        waypointDescription.waypoint = model.modelData
                        waypointDescription.open()
                    }
                }
            }
        }

        ListView {
            id: adList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: adList.model = global.geoMapProvider().nearbyWaypoints(PositionProvider.lastValidCoordinate, "AD")

            Label {
                anchors.fill: parent
                anchors.topMargin: view.font.pixelSize*2
                visible: parent.count == 0

                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No aerodrome data available. Please make sure that an aviation map is installed.</p>")
            }
        }

        ListView {
            id: naList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: naList.model = global.geoMapProvider().nearbyWaypoints(PositionProvider.lastValidCoordinate, "NAV")

            Label {
                anchors.fill: parent
                anchors.topMargin: view.font.pixelSize*2
                visible: parent.count == 0

                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No navaid data available.</p>")
            }
        }

        ListView {
            id: rpList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: rpList.model = global.geoMapProvider().nearbyWaypoints(PositionProvider.lastValidCoordinate, "WP")
            
            Label {
                anchors.fill: parent
                anchors.topMargin: view.font.pixelSize*2
                visible: parent.count == 0

                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No reporting point data available.</p>")
            }
        }

    } // SwipeView

    WaypointDescription {
        id: waypointDescription
    }
} // Page
