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

#include "kgesturemap.h"

#include <kapplication.h>
#include <kaction.h>
#include <QtGui/QActionEvent>

#include <kglobal.h>

#include <kdebug.h>

/*
 This is a class for internal use by the KDE libraries only. This class
 may change or go away without notice so don't try to use it in non-kdelibs
 code.
 */

class KGestureMapContainer {
public:
    KGestureMap gestureMap;
};


K_GLOBAL_STATIC(KGestureMapContainer, g_instance)


KGestureMap::~KGestureMap()
{
}


KGestureMap *KGestureMap::self()
{
    return &g_instance->gestureMap;
}


KGestureMap::KGestureMap()
{
    m_gestureTimeout.setSingleShot(true);
    connect(&m_gestureTimeout, SIGNAL(timeout()), this, SLOT(stopAcquisition()));
    //It would be nice to install the filter on demand. Unfortunately,
    //undesired behavior might result due to changing invocation
    //orders of different event filters.
    if (qApp)
        qApp->installEventFilter(this);
}


void KGestureMap::addGesture(const KShapeGesture &gesture, KAction *act)
{
    if (!gesture.isValid() || !act)
        return;
    kDebug(283) << "KGestureMap::addGesture(KShapeGesture ...)";
    if (!m_shapeGestures.contains(gesture))
        m_shapeGestures.insert(gesture, act);
    else
        kDebug(283) << "Tried to register an action for a gesture already taken";
}


void KGestureMap::addGesture(const KRockerGesture &gesture, KAction *act)
{
    if (!gesture.isValid() || !act)
        return;
    kDebug(283) << "KGestureMap::addGesture(KRockerGesture ...)";
    if (!m_rockerGestures.contains(gesture))
        m_rockerGestures.insert(gesture, act);
    else
        kDebug(283) << "Tried to register an action for a gesture already taken";
}


void KGestureMap::removeGesture(const KShapeGesture &gesture, KAction *act)
{
    if (!gesture.isValid())
        return;
    kDebug(283) << "KGestureMap::removeGesture(KShapeGesture ...)";
    KAction *oldAct = m_shapeGestures.value(gesture);
    if (oldAct == act || !act /*wildcard*/)
        m_shapeGestures.remove(gesture);
}


void KGestureMap::removeGesture(const KRockerGesture &gesture, KAction *act)
{
    if (!gesture.isValid())
        return;
    kDebug(283) << "KGestureMap::removeGesture(KRockerGesture ...)";
    KAction *oldAct = m_rockerGestures.value(gesture);
    if (oldAct == act || !act /*wildcard*/)
        m_rockerGestures.remove(gesture);
}


KAction *KGestureMap::findAction(const KShapeGesture &gesture) const
{
    return m_shapeGestures.value(gesture);
}


KAction *KGestureMap::findAction(const KRockerGesture &gesture) const
{
    return m_rockerGestures.value(gesture);
}


void KGestureMap::installEventFilterOnMe(KApplication *app)
{
    app->installEventFilter(this);
}


inline int KGestureMap::bitCount(int n)
{
    int count = 0;
    while (n) {
        n &= (n - 1);
        count++;
    }
    return count;
}


void KGestureMap::handleAction(KAction *kact)
{
    if (!kact)
        return;
    kDebug(283) << "handleAction";
    //TODO: only activate in the action's context, just like keyboard shortcuts
    kact->trigger();
    return;
}


void KGestureMap::matchShapeGesture()
{
    //TODO: tune and tweak until satisfied with result :)
    m_shapeGesture.setShape(m_points);
    float dist, minDist = 20.0;
    KAction *bestMatch = 0;

    for (QHash<KShapeGesture, KAction *>::const_iterator it = m_shapeGestures.constBegin();
        it != m_shapeGestures.constEnd(); ++it) {
        dist = m_shapeGesture.distance(it.key(), 1000.0);
        if (dist < minDist) {
            minDist = dist;
            bestMatch = it.value();
        }
    }
    handleAction(bestMatch);
}


//slot
void KGestureMap::stopAcquisition()
{
    m_gestureTimeout.stop();
    m_acquiring = false;
}


//TODO: Probably kwin, kded and others should not have a gesture map.
//Maybe making them friends and providing a private "die()" function would work.
/*
 * Act on rocker gestures immediately and collect movement data for evaluation.
 * The decision when to consume and when to relay an event is quite tricky.
 * I decided to only consume clicks that belong to completed rocker gestures.
 * A user might e.g. go back in a browser several times using rocker gestures,
 * thus changing what's under the cursor every time. This might lead to
 * unintended clicks on links where there was free space before.
 */

bool KGestureMap::eventFilter(QObject *obj, QEvent *e)
{
    //disable until it does not interfere with other input any more
    return false;
    Q_UNUSED(obj);
    int type = e->type();

    //catch right-clicks disguised as context menu events. if we ignore a
    //context menu event caused by a right-click, it should get resent
    //as a right-click event, according to documentation.
    //### this is preliminary
    if (type == QEvent::ContextMenu) {
        QContextMenuEvent *cme = static_cast<QContextMenuEvent *>(e);
        if (cme->reason() == QContextMenuEvent::Mouse) {
            cme->ignore();
            return true;
        }
        return false;
    }

    if (type < QEvent::MouseButtonPress || type > QEvent::MouseMove)
        return false;

    QMouseEvent *me = static_cast<QMouseEvent *>(e);
    if (type == QEvent::MouseButtonPress) {
        int nButtonsDown = bitCount(me->buttons());
        kDebug(283) << "number of buttons down:" << nButtonsDown;

        //right button down starts gesture acquisition
        if (nButtonsDown == 1 && me->button() == Qt::RightButton) {
            //"startAcquisition()"
            m_acquiring = true;
            m_gestureTimeout.start(4000);
            kDebug(283) << "========================";
            m_points.clear();
            m_points.append(me->pos());
            return true;
        } else if (nButtonsDown != 2)
            return false;

        //rocker gestures. do not trigger any movement gestures from now on.
        stopAcquisition();
        int buttonHeld = me->buttons() ^ me->button();
        m_rockerGesture.setButtons(static_cast<Qt::MouseButton>(buttonHeld), me->button());
        KAction *match = m_rockerGestures.value(m_rockerGesture);
        if (!match)
            return false;
        handleAction(match);
        return true;
    }

    if (m_acquiring) {
        if (type == QEvent::MouseMove) {
            m_points.append(me->pos());
            //abort to avoid using too much memory. 1010 points should be enough
            //for everyone! :)
            //next reallocation of m_points would happen at 1012 items
            if (m_points.size() > 1010)
                stopAcquisition();
            return true;
        } else if (type == QEvent::MouseButtonRelease && me->button() == Qt::RightButton) {
            stopAcquisition();

            //TODO: pre-selection of gestures by length (optimization), if necessary
            //possibly apply other heuristics
            //then try all remaining gestures for sufficiently small distance
            int dist = 0;
            for (int i = 1; i < m_points.size(); i++) {
                dist += (m_points[i] - m_points[i-1]).manhattanLength();
                if (dist > 40) {
                    matchShapeGesture();
                    return true;
                }
                //this was probably a small glitch while right-clicking if we get here.
                //TODO: open the context menu or do whatever happens on right-click (how?)
            }
            return false;
        }
    }
    return false;
}

#include "kgesturemap.moc"
