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

#include "delegateanimationhandler_p.h"

#include <QListView>
#include <QAbstractItemView>
#include <QPersistentModelIndex>
#include <QTime>
#include <QDebug>

#include <cmath>

#include "delegateanimationhandler_p.moc"

namespace KIO
{

// Needed because state() is a protected method
class ProtectedAccessor : public QAbstractItemView
{
public:
    bool draggingState() const { return state() == DraggingState; }
};



// ---------------------------------------------------------------------------



CachedRendering::CachedRendering(QStyle::State state, const QSize &size)
    : state(state), regular(QPixmap(size)), hover(QPixmap(size))
{
    regular.fill(Qt::transparent);
    hover.fill(Qt::transparent);
}



// ---------------------------------------------------------------------------



AnimationState::AnimationState(const QModelIndex &index)
        : index(index), direction(QTimeLine::Forward),
          animating(false), progress(0.0), renderCache(NULL)
{
    creationTime.start();
}


AnimationState::~AnimationState()
{
    delete renderCache;
}


bool AnimationState::update()
{
    const qreal runtime = (direction == QTimeLine::Forward ? 150 : 250); // milliseconds
    const qreal increment = 1000. / runtime / 1000.;
    const qreal delta = increment * time.restart();

    if (direction == QTimeLine::Forward)
    {
        progress = qMin(qreal(1.0), progress + delta);
        animating = (progress < 1.0);
    }
    else
    {
        progress = qMax(qreal(0.0), progress - delta);
        animating = (progress > 0.0);
    }

    return !animating;
}


qreal AnimationState::hoverProgress() const
{
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
    return qRound(255.0 * std::sin(progress * M_PI_2)) / 255.0;
}



// ---------------------------------------------------------------------------



DelegateAnimationHandler::DelegateAnimationHandler(QObject *parent)
    : QObject(parent)
{
}

DelegateAnimationHandler::~DelegateAnimationHandler()
{
    timer.stop();

    QMapIterator<const QAbstractItemView*, AnimationList*> i(animationLists);
    while (i.hasNext()) {
        i.next();
        qDeleteAll(*i.value());
        delete i.value();
    }
    animationLists.clear();
}

AnimationState *DelegateAnimationHandler::animationState(const QStyleOption &option,
                                                         const QModelIndex &index,
                                                         const QAbstractItemView *view)
{
   // We can't do animations reliably when an item is being dragged, since that
    // item will be drawn in two locations at the same time and hovered in one and
    // not the other. We can't tell them apart because they both have the same index.
    if (!view || static_cast<const ProtectedAccessor*>(view)->draggingState())
        return NULL;

    AnimationState *state = findAnimationState(view, index);
    bool hover = option.state & QStyle::State_MouseOver;

    // If the cursor has entered an item
    if (!state && hover)
    {
        state = new AnimationState(index);
        addAnimationState(state, view);

        if (!fadeInAddTime.isValid() ||
            (fadeInAddTime.isValid() && fadeInAddTime.elapsed() > 300))
        {
            startAnimation(state);
        }
        else
        {
            state->animating = false;
            state->progress  = 1.0;
            state->direction = QTimeLine::Forward;
        }

        fadeInAddTime.restart();
    }
    else if (state)
    {
        // If the cursor has exited an item
        if (!hover && (!state->animating || state->direction == QTimeLine::Forward))
        {
            state->direction = QTimeLine::Backward;

            if (state->creationTime.elapsed() < 200)
                state->progress = 0.0;

            startAnimation(state);
        }
        else if (hover && state->direction == QTimeLine::Backward)
        {
            // This is needed to handle the case where an item is dragged within
            // the view, and dropped in a different location. State_MouseOver will
            // initially not be set causing a "hover out" animation to start.
            // This reverses the direction as soon as we see the bit being set.
            state->direction = QTimeLine::Forward;

            if (!state->animating)
                startAnimation(state);
        }
    }

    return state;
}


AnimationState *DelegateAnimationHandler::findAnimationState(const QAbstractItemView *view,
                                                             const QModelIndex &index) const
{
    // Try to find a list of animation states for the view
    AnimationList *list = animationLists.value(view);

    if (list)
    {
        foreach (AnimationState *state, *list)
            if (state->index == index)
                return state;
    }

    return NULL;
}


void DelegateAnimationHandler::addAnimationState(AnimationState *state, const QAbstractItemView *view)
{
    AnimationList *list = animationLists.value(view);

    // If this is the first time we've seen this view
    if (!list)
    {
        connect(view, SIGNAL(destroyed(QObject*)), SLOT(viewDeleted(QObject*)));

        list = new AnimationList;
        animationLists.insert(view, list);
    }

    list->append(state);
}


void DelegateAnimationHandler::startAnimation(AnimationState *state)
{
    state->time.start();
    state->animating = true;

    if (!timer.isActive())
        timer.start(1000 / 30, this); // 30 fps
}


int DelegateAnimationHandler::runAnimations(AnimationList *list, const QAbstractItemView *view)
{
    int activeAnimations = 0;
    QRegion region;

    QMutableLinkedListIterator<AnimationState*> i(*list);
    while (i.hasNext())
    {
        AnimationState *state = i.next();

        if (!state->animating)
            continue;

        // We need to make sure the index is still valid, since it could be removed
        // while the animation is running.
        if (state->index.isValid())
        {
            bool finished = state->update();
            region += view->visualRect(state->index);

            if (!finished)
            {
                activeAnimations++;
                continue;
            }
        }

        // If the direction is Forward, the state object needs to stick around
        // after the animation has finished, so we know that we've already done
        // a "hover in" for the index.
        if (state->direction == QTimeLine::Backward || !state->index.isValid())
        {
            delete state;
            i.remove();
        }
    }

    // Trigger a repaint of the animated indexes
    if (!region.isEmpty())
        const_cast<QAbstractItemView*>(view)->viewport()->update(region);

    return activeAnimations;
}


void DelegateAnimationHandler::viewDeleted(QObject *view)
{
    AnimationList *list = animationLists.take(static_cast<QAbstractItemView*>(view));
    qDeleteAll(*list);
    delete list;
}


void DelegateAnimationHandler::timerEvent(QTimerEvent *)
{
    int activeAnimations = 0;

    AnimationListsIterator i(animationLists);
    while (i.hasNext())
    {
        i.next();
        AnimationList *list = i.value();
        const QAbstractItemView *view = i.key();

        activeAnimations += runAnimations(list, view);
    }

    if (activeAnimations == 0 && timer.isActive())
        timer.stop();
}

}

