/* This file is part of the KDE libraries
    Copyright (C) 2006,2007 Andreas Hartmetz (ahartmetz@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KGESTUREMAP_H
#define KGESTUREMAP_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QTimer>
#include <QPolygon>

#include "kgesture.h"

class KApplication;
class KAction;
class QEvent;

class KGestureMap : public QObject
{
    Q_OBJECT
public:
    static KGestureMap *self();

    virtual bool eventFilter(QObject *obj, QEvent *e);
    void addGesture(const KShapeGesture &gesture, KAction *kact);
    void addGesture(const KRockerGesture &gesture, KAction *kact);
    void removeGesture(const KShapeGesture &gesture, KAction *kact);
    void removeGesture(const KRockerGesture &gesture, KAction *kact);
    KAction *findAction(const KShapeGesture &gesture) const;
    KAction *findAction(const KRockerGesture &gesture) const;

private Q_SLOTS:
    void stopAcquisition();

private:
    friend class KGestureMapContainer;
    KGestureMap();
    virtual ~KGestureMap();

    friend class KApplicationPrivate;
    //intended to be used at application initialization
    void installEventFilterOnMe(KApplication *app);

    inline int bitCount(int n);
    void handleAction(KAction *kact);
    void matchShapeGesture();

    //this is an internal class so don't bother with a d-pointer
    QHash<KShapeGesture, KAction *> m_shapeGestures;
    QHash<KRockerGesture, KAction *> m_rockerGestures;
    QPolygon m_points;
    QTimer m_gestureTimeout;
    bool m_acquiring;

    KShapeGesture m_shapeGesture;
    KRockerGesture m_rockerGesture;
};

#endif //KGESTUREMAP_H
