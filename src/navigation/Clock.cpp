/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include "Clock.h"
#include "positioning/PositionProvider.h"

#include <QDate>
#include <QGuiApplication>
#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;


Navigation::Clock::Clock(QObject *parent) : GlobalObject(parent)
{
    // We need to update the time regularly. I do not use a simple timer here that emits "timeChanged" once per minute, because I
    // want the signal to be emitted right after the full minute. So, I use a timer that once a minute set a single-shot time
    // that is set to fire up 500ms after the full minute. This design will also work reliably if "timer" get out of sync,
    // for instance because the app was sleeping for a while.
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Clock::setSingleShotTimer);
    timer->setInterval(1min);
    timer->start();

    // Start the single shot timer once manually
    setSingleShotTimer();

    // There are a few other events where we want to update the clock
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &Clock::timeChanged);
}


auto Navigation::Clock::describeTimeDifference(const QDateTime& pointInTime) -> QString
{
    auto minutes = qRound( ((double)QDateTime::currentDateTime().secsTo(pointInTime))/60.0);

    bool past = minutes < 0;
    minutes = qAbs(minutes);

    if (minutes == 0) {
        return tr("just now");
    }

    auto hours = minutes/60;
    minutes = minutes%60;

    QString result;
    if (hours == 0) {
        result = QStringLiteral("%1 min").arg(minutes);
    } else {
        result = QStringLiteral("%1:%2 h").arg(hours).arg(minutes, 2, 10, QChar('0'));
    }

    if (past) {
        result = tr("%1 ago").arg(result);
    } else {
        result = tr("in %1").arg(result);
    }

    return result.simplified();
}


auto Navigation::Clock::describePointInTime(QDateTime pointInTime) -> QString
{
    pointInTime = pointInTime.toUTC();

    auto currentDateLocal = QDate::currentDate();
    auto pointInTimeDateLocal = pointInTime.toLocalTime().date();

    // Obtain current position
    auto dayDelta = currentDateLocal.daysTo(pointInTimeDateLocal);
    switch(dayDelta) {
    case -1:
        return tr("yesterday %1").arg(pointInTime.toString(QStringLiteral("H:mm")));
    case 0:
        return pointInTime.toString(QStringLiteral("H:mm"));
    case 1:
        return tr("tomorrow %1").arg(pointInTime.toString(QStringLiteral("H:mm")));
    default:
        return pointInTime.toString(QStringLiteral("d. MMM, H:mm"));
    }

    return pointInTime.toString(QStringLiteral("d. MMM, H:mm"));
}


void Navigation::Clock::setSingleShotTimer()
{
    QTime current = QDateTime::currentDateTime().time();
    int msecsToNextMinute = 60*1000 - (current.msecsSinceStartOfDay() % (60*1000));
    QTimer::singleShot(msecsToNextMinute+500, this, &Clock::timeChanged);
    if (current.msecsSinceStartOfDay() < 1000*60) {
        emit dateChanged();
    }
}


auto Navigation::Clock::timeAsUTCString() -> QString
{
    return QDateTime::currentDateTimeUtc().toString(QStringLiteral("H:mm"));
}
