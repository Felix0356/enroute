#
# Translations
#

# The Qt library translations are a mess. We try to find the files at various
# places in the file system, generate our own *.qm files with lrelease and then
# use the lconvert tool to concatenate the *.qm file with the Qt library
# translations. Resulting files are then called enroute_XX.qm where XX is the
# language code. The variable LANGS contains all language codes that will be
# considered.

find_path(Qt6_TRANSLATION_DIR qtbase_de.qm
    HINTS ${Qt6Core_DIR}/../../../translations
    HINTS /usr/share/qt5/translations
    HINTS /usr/share/qt/translations
    HINTS /usr/translations
    REQUIRED
    )
set(Qt6_LCONVERT_EXECUTABLE Qt6::lconvert)
set(Qt6_LRELEASE_EXECUTABLE Qt6::lrelease)

set(LANGS de fr it pl)
foreach(_lang ${LANGS})
    set(QT_QM_FILES ${Qt6_TRANSLATION_DIR}/qtbase_${_lang}.qm)
    if (EXISTS ${Qt6_TRANSLATION_DIR}/qtdeclarative_${_lang}.qm)
        list(APPEND QT_QM_FILES ${Qt6_TRANSLATION_DIR}/qtdeclarative_${_lang}.qm)
    endif()
    if (EXISTS ${Qt6_TRANSLATION_DIR}/qtlocation_${_lang}.qm)
        list(APPEND QT_QM_FILES ${Qt6_TRANSLATION_DIR}/qtlocation_${_lang}.qm)
    endif()
    add_custom_command(
        OUTPUT enroute_${_lang}.qm
        DEPENDS ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_${_lang}.ts
        COMMAND ${Qt6_LRELEASE_EXECUTABLE} ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_${_lang}.ts -qm enrouteOnly_${_lang}.qm
        COMMAND ${Qt6_LCONVERT_EXECUTABLE} enrouteOnly_${_lang}.qm ${QT_QM_FILES} -o enroute_${_lang}.qm
        COMMENT "Generating translations for language ${_lang}"
        )
endforeach()


#
# Configure files
#

configure_file(flightMap.qrc.in flightMap.qrc)
configure_file(${CMAKE_SOURCE_DIR}/generatedSources/flightMap/flightMap-fonts.qrc.in flightMapFonts.qrc)
configure_file(icons.qrc.in icons.qrc)
configure_file(qml.qrc.in qml.qrc)
configure_file(ressources.qrc.in ressources.qrc)


#
# Set up sources
#

set(SOURCES
    # Ressources
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc

    # Translations
    enroute_de.qm
    enroute_fr.qm
    enroute_it.qm
    enroute_pl.qm

    # Other files
    AndroidManifest.xml.in
    flightMap.qrc.in
    icons.qrc.in
    qml.qrc.in
    README.md
    ressources.qrc.in

    # Header files
    dataManagement/DataManager.h
    dataManagement/Downloadable_Abstract.h
    dataManagement/Downloadable_MultiFile.h
    dataManagement/Downloadable_SingleFile.h
    dataManagement/SSLErrorHandler.h
    dataManagement/UpdateNotifier.h
    DemoRunner.h
    geomaps/Airspace.h
    geomaps/CUP.h
    geomaps/GeoJSON.h
    geomaps/GeoJSONHandler.h
    geomaps/GeoMapProvider.h
    geomaps/GPX.h
    geomaps/MBTILES.h
    geomaps/TileHandler.h
    geomaps/TileServer.h
    geomaps/Waypoint.h
    geomaps/WaypointLibrary.h
    GlobalObject.h
    GlobalSettings.h
    Librarian.h
    navigation/Aircraft.h
    navigation/Clock.h
    navigation/FlightRoute.h
    navigation/Leg.h
    navigation/Navigator.h
    navigation/RemainingRouteInfo.h
    platform/FileExchange.h
    platform/FileExchange_Abstract.h
    platform/Notifier.h
    platform/Notifier_Abstract.h
    platform/PlatformAdaptor.h
    platform/PlatformAdaptor_Abstract.h
    platform/SafeInsets_Abstract.h
    positioning/Geoid.h
    positioning/PositionInfo.h
    positioning/PositionInfoSource_Abstract.h
    positioning/PositionInfoSource_Satellite.h
    positioning/PositionProvider.h
    traffic/FlarmnetDB.h
    traffic/PasswordDB.h
    traffic/TrafficDataSource_Abstract.h
    traffic/TrafficDataSource_AbstractSocket.h
    traffic/TrafficDataSource_File.h
    traffic/TrafficDataSource_Simulate.h
    traffic/TrafficDataSource_Tcp.h
    traffic/TrafficDataSource_Udp.h
    traffic/TrafficDataProvider.h
    traffic/TrafficFactor_Abstract.h
    traffic/TrafficFactor_DistanceOnly.h
    traffic/TrafficFactor_WithPosition.h
    traffic/Warning.h
    units/Angle.h
    units/Distance.h
    units/Speed.h
    units/Time.h
    units/Units.h
    units/Volume.h
    units/VolumeFlow.h
    weather/Decoder.h
    weather/METAR.h
    weather/Station.h
    weather/TAF.h
    weather/WeatherDataProvider.h
    weather/Wind.h

    # C++ files
    dataManagement/DataManager.cpp
    dataManagement/Downloadable_Abstract.cpp
    dataManagement/Downloadable_MultiFile.cpp
    dataManagement/Downloadable_SingleFile.cpp
    dataManagement/SSLErrorHandler.cpp
    dataManagement/UpdateNotifier.cpp
    DemoRunner.cpp
    geomaps/Airspace.cpp
    geomaps/CUP.cpp
    geomaps/GeoJSON.cpp
    geomaps/GeoJSONHandler.cpp
    geomaps/GeoMapProvider.cpp
    geomaps/GPX.cpp
    geomaps/MBTILES.cpp
    geomaps/TileHandler.cpp
    geomaps/TileServer.cpp
    geomaps/Waypoint.cpp
    geomaps/WaypointLibrary.cpp
    GlobalObject.cpp
    GlobalSettings.cpp
    Librarian.cpp
    main.cpp
    navigation/Aircraft.cpp
    navigation/Clock.cpp
    navigation/FlightRoute.cpp
    navigation/FlightRoute_GPX.cpp
    navigation/Leg.cpp
    navigation/Navigator.cpp
    navigation/RemainingRouteInfo.cpp
    platform/FileExchange_Abstract.cpp
    platform/Notifier_Abstract.cpp
    platform/PlatformAdaptor_Abstract.cpp
    platform/SafeInsets_Abstract.cpp
    positioning/Geoid.cpp
    positioning/PositionInfo.cpp
    positioning/PositionInfoSource_Abstract.cpp
    positioning/PositionInfoSource_Satellite.cpp
    positioning/PositionProvider.cpp
    traffic/FlarmnetDB.cpp
    traffic/PasswordDB.cpp
    traffic/TrafficDataSource_Abstract.cpp
    traffic/TrafficDataSource_Abstract_FLARM.cpp
    traffic/TrafficDataSource_Abstract_GDL90.cpp
    traffic/TrafficDataSource_Abstract_XGPS.cpp
    traffic/TrafficDataSource_AbstractSocket.cpp
    traffic/TrafficDataSource_File.cpp
    traffic/TrafficDataSource_Simulate.cpp
    traffic/TrafficDataSource_Tcp.cpp
    traffic/TrafficDataSource_Udp.cpp
    traffic/TrafficDataProvider.cpp
    traffic/TrafficFactor_Abstract.cpp
    traffic/TrafficFactor_DistanceOnly.cpp
    traffic/TrafficFactor_WithPosition.cpp
    traffic/Warning.cpp
    units/Angle.cpp
    units/Distance.cpp
    units/Speed.cpp
    units/Time.cpp
    units/Volume.cpp
    units/VolumeFlow.cpp
    weather/Decoder.cpp
    weather/METAR.cpp
    weather/Station.cpp
    weather/TAF.cpp
    weather/WeatherDataProvider.cpp
    weather/Wind.cpp

    ${HEADERS}
    )

#
# We use this macro here to avoid creating extremely large C++ files with binary content
#

qt_add_big_resources(SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/flightMapFonts.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc
    )


#
# Generate android executable
#

if( ANDROID )
    list(APPEND SOURCES
        ${QTQUICKCOMPILEDRESOURCES}
        platform/FileExchange_Android.h
        platform/FileExchange_Android.cpp
        platform/Notifier_Android.h
        platform/Notifier_Android.cpp
        platform/PlatformAdaptor_Android.h
        platform/PlatformAdaptor_Android.cpp
        platform/SafeInsets_Android.h
        platform/SafeInsets_Android.cpp
        android/src/de/akaflieg_freiburg/enroute/IntentLauncher.java
        android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java
        android/src/de/akaflieg_freiburg/enroute/Notifier.java
        android/src/de/akaflieg_freiburg/enroute/ShareActivity.java
        android/src/de/akaflieg_freiburg/enroute/ShareUtils.java
        )

    # Configure files
    configure_file(AndroidManifest.xml.in android/AndroidManifest.xml)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/action/drawable-hdpi/ic_info_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_info.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/file/drawable-hdpi/ic_file_download_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_file_download.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/alert/drawable-hdpi/ic_error_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_error.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/alert/drawable-hdpi/ic_warning_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_warning.png COPYONLY)

    # Copy manual into Android assets directory
    file(COPY ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/docs/manual DESTINATION android/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/generatedSources/src/android DESTINATION .)
    file(COPY ${CMAKE_SOURCE_DIR}/src/android DESTINATION .)

    # The android target require a few additional variables
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android" CACHE INTERNAL "")

    # Compile definitions
    add_compile_definitions(MANUAL_LOCATION="/android_asset/manual")
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc)

    qt_add_executable(${PROJECT_NAME} ${SOURCES} ${ANDROID_EXTRA_SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Concurrent Qt6::Core Qt::CorePrivate Qt6::Positioning Qt6::Quick Qt6::Sql Qt6::Svg Qt6::WebView qhttpengine sunset)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${ANDROID_EXTRA_LIBS}"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android"
        QT_ANDROID_TARGET_SDK_VERSION 33
    )
endif()


#
# Generate unix executable
#

if (NOT ANDROID)
    list(APPEND SOURCES
        platform/FileExchange_Linux.h
        platform/FileExchange_Linux.cpp
        platform/Notifier_Linux.h
        platform/Notifier_Linux.cpp
        platform/PlatformAdaptor_Linux.h
        platform/PlatformAdaptor_Linux.cpp
        platform/SafeInsets_Desktop.h
        platform/SafeInsets_Desktop.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc
        )

    # Compile definitions
    add_compile_definitions(MANUAL_LOCATION="${CMAKE_INSTALL_FULL_DOCDIR}/manual")

    # Add executable and libraries
    qt_add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt6::Concurrent
        Qt6::Core
        Qt6::Core5Compat
        Qt6::DBus
        Qt6::Positioning
        Qt6::Quick
        Qt6::Sql
        Qt6::Svg
        Qt6::Widgets
        kdsingleapplication
        qhttpengine
        sunset)
    target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        <QObject>
        <QPointer>
    )

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/docs/manual DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/3rdParty/sunset/src
    ${CMAKE_SOURCE_DIR}/3rdParty/GSL/include
    geomaps
    navigation
    platform
    positioning
    traffic
    ui
    units
)

qt_add_qml_module(${PROJECT_NAME}
    URI akaflieg_freiburg.enroute
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
    qml/dialogs/CenteringDialog.qml
    qml/dialogs/LongTextDialog.qml
    qml/items/AutoSizingMenu.qml
    qml/items/DegreeInput.qml
    qml/items/ElevationInput.qml
    qml/items/FlightVector.qml
    qml/items/NavBar.qml
    qml/items/RemainingRouteBar.qml
    qml/items/WordWrappingCheckDelegate.qml
    qml/items/WordWrappingItemDelegate.qml
    qml/items/WordWrappingSwitchDelegate.qml
    qml/pages/SettingsPage.qml
    SOURCES
    ui/ScaleQuickItem.h
    ui/ScaleQuickItem.cpp
)


# Enforce C++17 and no extensions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)


#
# Generate documentation
#

configure_file(Doxyfile.in Doxyfile)
add_custom_target(enroute_doc COMMAND doxygen)
add_dependencies(fulldoc enroute_doc)
