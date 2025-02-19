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

#pragma once

#include "GlobalObject.h"

namespace Platform {

/*! \brief Interface to platform-specific file exchange functionality
 *
 * This pure virtual class is an interface to file exchange functionality that
 * need platform-specific code to operate. The files FileExchange_XXX.(h|cpp)
 * implement a child class FileExchange that contains the actual implementation.
 *
 * If supported by the platform, child classes need to react to requests by the
 * platform to open a file (e.g. a GeoJSON file containing a flight route). Once
 * a request is received, the method processFileRequest() should be called.
 */

class FileExchange_Abstract : public GlobalObject
{
    Q_OBJECT

public:
    /*! \brief Functions and types of a file that this app handles */
    enum FileFunction
      {
        UnknownFunction, /*< Unknown file */
        FlightRouteOrWaypointLibrary, /*< File contains a flight route or a waypoint library. */
        FlightRoute, /*< File contains a flight route. */
        VectorMap, /*< File contains a vector map. */
        RasterMap, /*< File contains a raster map. */
        WaypointLibrary /*< Waypoint library in CUP or GeoJSON format */
      };
    Q_ENUM(FileFunction)


    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit FileExchange_Abstract(QObject *parent = nullptr);

    ~FileExchange_Abstract() override = default;


    //
    // Methods
    //

    /*! \brief Import content from file
     *
     * On desktop systems, this method is supposed to open a file dialog to
     * import a file. On mobile systems, this method is supposed to do nothing.
     *
     */
    Q_INVOKABLE virtual void importContent() = 0;

    /*! \brief Share content
     *
     * On desktop systems, this method is supposed to show a file dialog to save
     * the file. On mobile devices, this method is supposed to open a dialog
     * that allows to chose the method to send this file (e-mail, dropbox,
     * signal chat, …)
     *
     * @param content File content
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without
     * suffix of path. This can be used, e.g. as the name of the attachment when
     * sending files by e-mail.
     *
     * @returns Empty string on success, the string "abort" on abort, and a
     * translated error message otherwise
     */
    Q_INVOKABLE virtual QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

    /*! \brief View content
     *
     * This method is supposed open the content in an appropriate app.  Example:
     * if the content is GeoJSON, the content might be opened in Google Earth,
     * or in a mobile mapping application.
     *
     * @param content content text
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "FlightRoute-%1.geojson".
     *
     * @returns Empty string on success, a translated error message otherwise
     */
    Q_INVOKABLE virtual QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

public slots:
    /*! \brief Determine file function and emit openFileRequest()
     *
     * This helper function is called by platform-dependent code whenever the
     * app is asked to open a file.  It will look at the file, determine the
     * file function and emit the signal openFileRequest() as appropriate.
     *
     * @param path File name
     */
    virtual void processFileOpenRequest(const QString& path);

    /*! \brief Determine file function and emit openFileRequest()
     *
     * Overloaded function for convenience
     *
     * @param path QByteArray containing an UTF8-Encoded strong
     */
    void processFileOpenRequest(const QByteArray& path);

signals:
    /*! \brief Emitted when platform asks this app to open a file
     *
     * This signal is emitted whenever the platform-dependent code receives
     * information that enroute is requested to open a file.
     *
     * @param fileName Path of the file on the local file system
     *
     * @param fileFunction Function and file type.
     */
    void openFileRequest(QString fileName, Platform::FileExchange_Abstract::FileFunction fileFunction);

private:
    Q_DISABLE_COPY_MOVE(FileExchange_Abstract)
};

} // namespace Platform
