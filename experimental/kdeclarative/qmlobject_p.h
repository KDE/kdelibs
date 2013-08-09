/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef QMLOBJECT_P_H
#define QMLOBJECT_P_H

#include <QObject>
#include <QQmlIncubationController>

#include <QWindow>
#include <QAnimationDriver>
#include <QGuiApplication>
#include <QScreen>


class QQmlEngine;
class QQmlComponent;


class QmlObjectIncubationController : public QObject, public QQmlIncubationController
{
    Q_OBJECT

public:
    QmlObjectIncubationController(QObject *parent)
        : QObject(parent),
          QQmlIncubationController()
    {
        // Allow incubation for 1/3 of a frame.
        m_incubation_time = qMax(1, int(1000 / QGuiApplication::primaryScreen()->refreshRate()) / 3);
    }

protected:
    virtual bool event(QEvent *e)
    {
        if (e->type() == QEvent::User) {
            incubate();
            return true;
        }
        return QObject::event(e);
    }

public Q_SLOTS:
    void incubate()
    {
        if (incubatingObjectCount()) {
            incubateFor(m_incubation_time * 2);
            if (incubatingObjectCount()) {
                QCoreApplication::postEvent(this, new QEvent(QEvent::User));
            }
        }
    }

    void animationStopped() { incubate(); }

protected:
    virtual void incubatingObjectCountChanged(int count)
    {
        if (count) {
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
        }
    }
private:
    int m_incubation_time;
};


#endif // multiple inclusion guard
