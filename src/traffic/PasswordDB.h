/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QHash>
#include <QObject>

namespace Traffic {

#warning

class PasswordDB : public QObject {
    Q_OBJECT

public:
    PasswordDB(QObject* parent);

    ~PasswordDB();

    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

    const bool contains(const QString& key)
    {
        return m_passwordDB.contains(key);
    }

    const QString getPassword(const QString& key)
    {
        return m_passwordDB.value(key);
    }

    void removePassword(const QString& key);

    Q_INVOKABLE void setPassword(const QString& key, const QString& value);

    Q_INVOKABLE void clear();

signals:
    void emptyChanged();

private:
    void read();
    void save();

    QString passwordDBFileName {};

    QHash<QString, QString> m_passwordDB {};
};

}
