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

#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QQmlEngine>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrentRun>

#include "geomaps/GeoMapProvider.h"
#include "geomaps/MBTILES.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Navigator.h"


GeoMaps::GeoMapProvider::GeoMapProvider(QObject *parent)
    : GlobalObject(parent)
{
    _combinedGeoJSON_ = emptyGeoJSON();

    QFile geoJSONCacheFile(geoJSONCache);
    geoJSONCacheFile.open(QFile::ReadOnly);
    _combinedGeoJSON_ = geoJSONCacheFile.readAll();
    geoJSONCacheFile.close();

    _tileServer.listen(QHostAddress(QStringLiteral("127.0.0.1")));
}

void GeoMaps::GeoMapProvider::deferredInitialization()
{
    connect(GlobalObject::dataManager()->aviationMaps(), &DataManagement::Downloadable_Abstract::fileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);
    connect(GlobalObject::dataManager()->baseMaps(), &DataManagement::Downloadable_Abstract::fileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);
    connect(GlobalObject::dataManager()->baseMaps(), &DataManagement::Downloadable_Abstract::filesChanged, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);
    connect(GlobalObject::dataManager()->terrainMaps(), &DataManagement::Downloadable_Abstract::fileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);
    connect(GlobalObject::globalSettings(), &GlobalSettings::airspaceAltitudeLimitChanged, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);
    connect(GlobalObject::globalSettings(), &GlobalSettings::hideGlidingSectorsChanged, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);
    connect(GlobalObject::globalSettings(), &GlobalSettings::hillshadingChanged, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);

    _aviationDataCacheTimer.setSingleShot(true);
    _aviationDataCacheTimer.setInterval(3s);
    connect(&_aviationDataCacheTimer, &QTimer::timeout, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);

    onAviationMapsChanged();
    onMBTILESChanged();

    GlobalObject::dataManager()->aviationMaps()->killFileContentChanged_delayed();
    GlobalObject::dataManager()->baseMaps()->killFileContentChanged_delayed();
    GlobalObject::dataManager()->terrainMaps()->killFileContentChanged_delayed();
}


//
// Getter Methods
//

auto GeoMaps::GeoMapProvider::copyrightNotice() -> QString
{
    QString result;
    if (GlobalObject::dataManager()->aviationMaps()->hasFile())
    {
        result += "<h4>"+tr("Aviation maps")+"</h4>";
        result += QStringLiteral("<a href='https://openAIP.net'>© openAIP</a><br><a href='https://openflightmaps.org'>© open flightmaps</a>");
    }

    foreach(auto baseMapX, GlobalObject::dataManager()->baseMaps()->downloadables())
    {
        auto* baseMap = qobject_cast<DataManagement::Downloadable_SingleFile*>(baseMapX);
        if (baseMap == nullptr)
        {
            continue;
        }
        if (!baseMap->hasFile())
        {
            continue;
        }


        GeoMaps::MBTILES mbtiles(baseMap->fileName());

        auto name = baseMap->fileName().split(QStringLiteral("aviation_maps/")).last();
        result += ("<h4>"+tr("Basemap")+ " %1</h4>").arg(name);
        result += mbtiles.attribution();
    }

    return result;
}

auto GeoMaps::GeoMapProvider::geoJSON() -> QByteArray
{
    QMutexLocker lock(&_aviationDataMutex);
    return _combinedGeoJSON_;
}

auto GeoMaps::GeoMapProvider::styleFileURL() const -> QString
{
    if (_styleFile.isNull())
    {
        return QStringLiteral(":/flightMap/empty.json");
    }
    return "file://"+_styleFile->fileName();
}


//
// Methods
//

auto GeoMaps::GeoMapProvider::airspaces(const QGeoCoordinate& position) -> QVariantList
{
    // Lock data
    QMutexLocker lock(&_aviationDataMutex);

    QVector<Airspace> result;
    result.reserve(10);
    foreach(auto airspace, _airspaces_) {
        if (airspace.polygon().contains(position)) {
            result.append(airspace);
        }
    }

    // Sort airspaces according to lower boundary
    std::sort(result.begin(), result.end(), [](const Airspace& a, const Airspace& b) {return (a.estimatedLowerBoundMSL() > b.estimatedLowerBoundMSL()); });

    QVariantList final;
    foreach(auto airspace, result)
        final.append( QVariant::fromValue(airspace) );

    return final;
}

auto GeoMaps::GeoMapProvider::closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition) -> Waypoint
{
    position.setAltitude(qQNaN());

    Waypoint result;
    const auto wpts = waypoints();
    for(const auto& wp : wpts) {
        if (!wp.isValid()) {
            continue;
        }
        if (!result.isValid()) {
            result = wp;
        }
        if (position.distanceTo(wp.coordinate()) < position.distanceTo(result.coordinate())) {
            result = wp;
        }
    }

    const auto wpLibrary = GlobalObject::waypointLibrary()->waypoints();
    for(const auto& wp : wpLibrary) {
        if (!wp.isValid()) {
            continue;
        }
        if (!result.isValid()) {
            result = wp;
        }
        if (position.distanceTo(wp.coordinate()) < position.distanceTo(result.coordinate())) {
            result = wp;
        }
    }

    for(auto& wp : GlobalObject::navigator()->flightRoute()->midFieldWaypoints() ) {
        if (!wp.isValid()) {
            continue;
        }
        if (position.distanceTo(wp.coordinate()) < position.distanceTo(result.coordinate())) {
            result = wp;
        }
    }

    if (position.distanceTo(result.coordinate()) > position.distanceTo(distPosition)) {
        position.setAltitude( terrainElevationAMSL(position).toM() );
        return {position};
    }

    return result;
}

auto GeoMaps::GeoMapProvider::terrainElevationAMSL(const QGeoCoordinate& coordinate) -> Units::Distance
{
    int zoomMin = 6;
    int zoomMax = 10;

    for(int zoom = zoomMax; zoom >= zoomMin; zoom--)
    {
        auto tilex = (coordinate.longitude()+180.0)/360.0 * (1<<zoom);
        auto tiley = (1.0 - asinh(tan(qDegreesToRadians(coordinate.latitude())))/M_PI)/2.0 * (1<<zoom);
        auto intraTileX = qRound(255.0*(tilex-floor(tilex)));
        auto intraTileY = qRound(255.0*(tiley-floor(tiley)));

        qint64 keyA = qFloor(tilex)&0xFFFF;
        qint64 keyB = qFloor(tiley)&0xFFFF;
        qint64 key = (keyA<<32) + (keyB<<16) + zoom;

        if (terrainTileCache.contains(key))
        {
            auto* tileImg = terrainTileCache.object(key);
            if (tileImg->isNull())
            {
                continue;
            }
            auto pix = tileImg->pixel(intraTileX, intraTileY);
            double elevation = (qRed(pix)*256.0 + qGreen(pix) + qBlue(pix)/256.0) - 32768.0;
            return Units::Distance::fromM(elevation);
        }
    }


    foreach(auto mbtPtr, m_terrainMapTiles)
    {
        if (mbtPtr.isNull())
        {
            continue;
        }

        for(int zoom = zoomMax; zoom >= zoomMin; zoom--)
        {
            auto tilex = (coordinate.longitude()+180.0)/360.0 * (1<<zoom);
            auto tiley = (1.0 - asinh(tan(qDegreesToRadians(coordinate.latitude())))/M_PI)/2.0 * (1<<zoom);
            auto intraTileX = qRound(255.0*(tilex-floor(tilex)));
            auto intraTileY = qRound(255.0*(tiley-floor(tiley)));

            qint64 keyA = qFloor(tilex)&0xFFFF;
            qint64 keyB = qFloor(tiley)&0xFFFF;
            qint64 key = (keyA<<32) + (keyB<<16) + zoom;

            auto tileData = mbtPtr->tile(zoom, qFloor(tilex), qFloor(tiley));
            if (!tileData.isEmpty())
            {
                auto* tileImg = new QImage();
                tileImg->loadFromData(tileData);
                if (tileImg->isNull())
                {
                    delete tileImg;
                    continue;
                }
                terrainTileCache.insert(key,tileImg);

                auto pix = tileImg->pixel(intraTileX, intraTileY);
                double elevation = (qRed(pix)*256.0 + qGreen(pix) + qBlue(pix)/256.0) - 32768.0;
                return Units::Distance::fromM(elevation);
            }
        }
    }
    return {};
}

auto GeoMaps::GeoMapProvider::emptyGeoJSON() -> QByteArray
{
    QJsonObject resultObject;
    resultObject.insert(QStringLiteral("type"), "FeatureCollection");
    resultObject.insert(QStringLiteral("features"), QJsonArray());
    QJsonDocument geoDoc(resultObject);
    return geoDoc.toJson(QJsonDocument::JsonFormat::Compact);
}

auto GeoMaps::GeoMapProvider::filteredWaypoints(const QString &filter) -> QVector<GeoMaps::Waypoint>
{

    QStringList filterWords;
    foreach(auto word, filter.simplified().split(' ', Qt::SkipEmptyParts)) {
        QString simplifiedWord = GlobalObject::librarian()->simplifySpecialChars(word);
        if (simplifiedWord.isEmpty()) {
            continue;
        }
        filterWords.append(simplifiedWord);
    }

    QVector<GeoMaps::Waypoint> result;

    const auto wps = waypoints();
    for(const auto& wp : wps) {
        if (!wp.isValid()) {
            continue;
        }
        bool allWordsFound = true;
        foreach(auto word, filterWords) {
            QString fullName = GlobalObject::librarian()->simplifySpecialChars(wp.name());
            if (!fullName.contains(word, Qt::CaseInsensitive) && !wp.ICAOCode().contains(word, Qt::CaseInsensitive)) {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound) {
            result.append( wp );
        }
    }

    const auto wpsLib = GlobalObject::waypointLibrary()->waypoints();
    for(const auto& wp : wpsLib) {
        if (!wp.isValid()) {
            continue;
        }
        bool allWordsFound = true;
        foreach(auto word, filterWords) {
            QString fullName = GlobalObject::librarian()->simplifySpecialChars(wp.name());
            if (!fullName.contains(word, Qt::CaseInsensitive) && !wp.ICAOCode().contains(word, Qt::CaseInsensitive)) {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound) {
            result.append( wp );
        }
    }

    std::sort(result.begin(), result.end(), [](const Waypoint& a, const Waypoint& b) {return a.name() < b.name(); });

    return result;
}

auto GeoMaps::GeoMapProvider::findByID(const QString &id) -> Waypoint
{
    auto wps = waypoints();

    foreach(auto wp, wps) {
        if (!wp.isValid()) {
            continue;
        }
        if (wp.ICAOCode() == id) {
            return wp;
        }
    }
    return {};
}

auto GeoMaps::GeoMapProvider::nearbyWaypoints(const QGeoCoordinate& position, const QString& type) -> QList<GeoMaps::Waypoint>
{
    auto wps = waypoints();

    QVector<Waypoint> tWps;
    foreach(auto wp, wps) {
        if (!wp.isValid()) {
            continue;
        }
        if (wp.type() != type) {
            continue;
        }
        tWps.append(wp);
    }

    std::sort(tWps.begin(), tWps.end(), [position](const Waypoint &a, const Waypoint &b) {return position.distanceTo(a.coordinate()) < position.distanceTo(b.coordinate()); });

    return tWps.mid(0,20);
}

auto GeoMaps::GeoMapProvider::waypoints() -> QVector<Waypoint>
{
    QMutexLocker locker(&_aviationDataMutex);
    return _waypoints_;
}


//
// Private Methods and Slots
//

void GeoMaps::GeoMapProvider::onAviationMapsChanged()
{
    // Paranoid safety checks
    if (_aviationDataCacheFuture.isRunning()) {
        _aviationDataCacheTimer.start();
        return;
    }

    //
    // Generate new GeoJSON array and new list of waypoints
    //
    QStringList JSONFileNames;
    foreach(auto geoMapPtrX, GlobalObject::dataManager()->aviationMaps()->downloadables()) {
        auto *geoMapPtr = qobject_cast<DataManagement::Downloadable_SingleFile*>(geoMapPtrX);
        if (geoMapPtr == nullptr)
        {
            continue;
        }
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(u".geojson", Qt::CaseInsensitive)) {
            continue;
        }
        if (!geoMapPtr->hasFile()) {
            continue;
        }
        JSONFileNames += geoMapPtr->fileName();
    }

    _aviationDataCacheFuture = QtConcurrent::run(&GeoMaps::GeoMapProvider::fillAviationDataCache, this, JSONFileNames, GlobalObject::globalSettings()->airspaceAltitudeLimit(), GlobalObject::globalSettings()->hideGlidingSectors());
}

void GeoMaps::GeoMapProvider::onMBTILESChanged()
{
    terrainTileCache.clear();

    qDeleteAll(m_baseMapRasterTiles);
    m_baseMapRasterTiles.clear();
    foreach(auto downloadableX, GlobalObject::dataManager()->baseMapsRaster()->downloadables())
    {
        auto *downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(downloadableX);
        if (downloadable == nullptr)
        {
            continue;
        }
        if (!downloadable->hasFile())
        {
            continue;
        }

        m_baseMapRasterTiles.append(new GeoMaps::MBTILES(downloadable->fileName(), this));
    }
    qDeleteAll(m_baseMapVectorTiles);
    m_baseMapVectorTiles.clear();
    foreach(auto downloadableX, GlobalObject::dataManager()->baseMapsVector()->downloadables())
    {
        auto *downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(downloadableX);
        if (downloadable == nullptr)
        {
            continue;
        }
        if (!downloadable->hasFile())
        {
            continue;
        }

        m_baseMapVectorTiles.append(new GeoMaps::MBTILES(downloadable->fileName(), this));
    }
    emit baseMapTilesChanged();

    qDeleteAll(m_terrainMapTiles);
    m_terrainMapTiles.clear();
    foreach(auto downloadableX, GlobalObject::dataManager()->terrainMaps()->downloadables())
    {
        auto *downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(downloadableX);
        if (downloadable == nullptr)
        {
            continue;
        }
        if (!downloadable->hasFile())
        {
            continue;
        }

        m_terrainMapTiles.append(new GeoMaps::MBTILES(downloadable->fileName(), this));
    }
    emit terrainMapTilesChanged();

    // Delete old style file, stop serving tiles
    delete _styleFile;
    _tileServer.removeMbtilesFileSet(_currentBaseMapPath);
    _tileServer.removeMbtilesFileSet(_currentTerrainMapPath);
    _currentBaseMapPath = QString::number(QRandomGenerator::global()->bounded(static_cast<quint32>(1000000000)));
    _currentTerrainMapPath = QString::number(QRandomGenerator::global()->bounded(static_cast<quint32>(1000000000)));

    QFile file;
    if (GlobalObject::dataManager()->baseMaps()->hasFile())
    {
        // Serve new tile set under new name
        if (!m_baseMapRasterTiles.isEmpty())
        {
            _tileServer.addMbtilesFileSet(m_baseMapRasterTiles, _currentBaseMapPath);
            file.setFileName(QStringLiteral(":/flightMap/mapstyle-raster.json"));
        }
        else
        {
            _tileServer.addMbtilesFileSet(m_baseMapVectorTiles, _currentBaseMapPath);
            file.setFileName(QStringLiteral(":/flightMap/osm-liberty.json"));
        }
    }
    else
    {
        file.setFileName(QStringLiteral(":/flightMap/empty.json"));
    }
    _tileServer.addMbtilesFileSet(m_terrainMapTiles, _currentTerrainMapPath);

    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    data.replace("%URL%", (_tileServer.serverUrl()+"/"+_currentBaseMapPath).toLatin1());
    if (GlobalObject::globalSettings()->hillshading())
    {
        data.replace("%URLT%", (_tileServer.serverUrl()+"/"+_currentTerrainMapPath).toLatin1());
    }
    else
    {
        data.replace("%URLT%", (_tileServer.serverUrl()).toLatin1());
    }
    data.replace("%URL2%", _tileServer.serverUrl().toLatin1());
    _styleFile = new QTemporaryFile(this);
    _styleFile->open();
    _styleFile->write(data);
    _styleFile->close();

    emit styleFileURLChanged();
}

void GeoMaps::GeoMapProvider::fillAviationDataCache(QStringList JSONFileNames, Units::Distance airspaceAltitudeLimit, bool hideGlidingSectors)
{
    // Avoid rounding errors
    airspaceAltitudeLimit = airspaceAltitudeLimit-Units::Distance::fromFT(1);

    // Ensure that order is the same every time
    JSONFileNames.sort();

    //
    // Generate new GeoJSON array and new list of waypoints
    //

    // First, create a vector of JSON objects.
    // We use a QSet to keep track of objects that have already been added in order to avoid duplicated entries.
    // The vector is used to ensure that the order of the objects remains identical during runs.
    // A QSet seems to have some built-in randomness and does not do that.
    QVector<QJsonObject> objectVector;
    {
        QSet<QJsonObject> objectSet;
        foreach(auto JSONFileName, JSONFileNames) {
            // Read the lock file
            QLockFile lockFile(JSONFileName+".lock");
            lockFile.lock();
            QFile file(JSONFileName);
            file.open(QIODevice::ReadOnly);
            auto document = QJsonDocument::fromJson(file.readAll());
            file.close();
            lockFile.unlock();

            foreach(auto value, document.object()[QStringLiteral("features")].toArray())
            {
                auto object = value.toObject();
                if (objectSet.contains(object))
                {
                    continue;
                }
                objectVector += object;
                objectSet += object;
            }
        }
    }

    // Create vectors of airspaces and waypoints
    QVector<Airspace> newAirspaces;
    QVector<Waypoint> newWaypoints;
    foreach(auto object, objectVector) {
        // Check if the current object is a waypoint. If so, add it to the list of waypoints.
        Waypoint wp(object);
        if (wp.isValid()) {
            newWaypoints.append(wp);
            continue;
        }

        // Check if the current object is an airspace. If so, add it to the list of airspaces.
        Airspace as(object);
        if (as.isValid()) {
            newAirspaces.append(as);
            continue;
        }
    }

    // Then, create a new JSONArray of features and a new list of waypoints
    QJsonArray newFeatures;
    foreach(auto object, objectVector) {
        // Ignore all objects that are airspaces and that begin above the airspaceAltitudeLimit.
        Airspace airspaceTest(object);
        if (airspaceAltitudeLimit.isFinite() && (airspaceTest.estimatedLowerBoundMSL() > airspaceAltitudeLimit)) {
            continue;
        }

        // If 'hideGlidingSector' is set, ignore all objects that are airspaces
        // and that are gliding sectors
        if (hideGlidingSectors) {
            Airspace airspaceTest(object);
            if (airspaceTest.CAT() == QLatin1String("GLD")) {
                continue;
            }
        }

        newFeatures += object;
    }

    QByteArray newGeoJSON;
    {
        QJsonObject resultObject;
        resultObject.insert(QStringLiteral("type"), "FeatureCollection");
        resultObject.insert(QStringLiteral("features"), newFeatures);
        QJsonDocument geoDoc(resultObject);
        newGeoJSON = geoDoc.toJson();
    }
    auto _geoJSONChanged = (newGeoJSON != _combinedGeoJSON_);
    auto _waypointsChanged = (newWaypoints != _waypoints_);

    // Sort waypoints by name
    std::sort(newWaypoints.begin(), newWaypoints.end(), [](const Waypoint &a, const Waypoint &b) {return a.name() < b.name(); });

    _aviationDataMutex.lock();
    _airspaces_ = newAirspaces;
    if (_waypointsChanged)
    {
        _waypoints_ = newWaypoints;
    }
    if (_geoJSONChanged)
    {
        _combinedGeoJSON_ = newGeoJSON;
        QFile geoJSONCacheFile(geoJSONCache);
        geoJSONCacheFile.open(QFile::WriteOnly);
        geoJSONCacheFile.write(_combinedGeoJSON_);
        geoJSONCacheFile.close();
    }
    _aviationDataMutex.unlock();

    if (_waypointsChanged)
    {
        emit waypointsChanged();
    }
    if (_geoJSONChanged)
    {
        emit geoJSONChanged();
    }

}
