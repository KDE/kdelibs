/*
   This file is part of the KDE project

   Copyright © 2007 Fredrik Höglund <fredrik@kde.org>

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

#ifndef DELEGATEANIMATIONHANDLER_P_H
#define DELEGATEANIMATIONHANDLER_P_H

#include <QBasicTimer>
#include <QMap>
#include <QLinkedList>
#include <QPersistentModelIndex>
#include <QStyle>
#include <QTimeLine>
#include <QTime>


class QAbstractItemView;

namespace KIO
{

struct CachedRendering
{
    CachedRendering(QStyle::State state, const QSize &size);
    bool checkValidity(QStyle::State current) const { return state == current; }

    QStyle::State state;
    QPixmap regular;
    QPixmap hover;
};


class AnimationState
{
public:
    ~AnimationState();
    qreal hoverProgress() const;
    CachedRendering *cachedRendering() const { return renderCache; }
    void setCachedRendering(CachedRendering *rendering) { renderCache = rendering; }

private:
    AnimationState(const QModelIndex &index);
    bool update();

    QPersistentModelIndex index;
    QTimeLine::Direction direction;
    bool animating;
    qreal progress;
    QTime time;
    QTime creationTime;
    CachedRendering *renderCache;

    friend class DelegateAnimationHandler;
};


class DelegateAnimationHandler : public QObject
{
    Q_OBJECT

    typedef QLinkedList<AnimationState*> AnimationList;
    typedef QMapIterator<const QAbstractItemView *, AnimationList*> AnimationListsIterator;
    typedef QMutableMapIterator<const QAbstractItemView *, AnimationList*> MutableAnimationListsIterator;

public:
    DelegateAnimationHandler(QObject *parent = 0);
    ~DelegateAnimationHandler();

    AnimationState *animationState(const QStyleOption &option, const QModelIndex &index, const QAbstractItemView *view);

private slots:
    void viewDeleted(QObject *view);

private:
    AnimationState *findAnimationState(const QAbstractItemView *view, const QModelIndex &index) const;
    void addAnimationState(AnimationState *state, const QAbstractItemView *view);
    void startAnimation(AnimationState *state);
    int runAnimations(AnimationList *list, const QAbstractItemView *view);
    void timerEvent(QTimerEvent *event);

private:
    QMap<const QAbstractItemView*, AnimationList*> animationLists;
    QTime fadeInAddTime;
    QBasicTimer timer;
};

}

#endif
