/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2007 Trolltech ASA
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2008 Laurent Montel <montel@kde.org>
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2009 Dawit Alemayehu <adawit @ kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "kwebview.h"
#include "kwebpage.h"
#include "kwebview_p.h"

#include <QtGui/QMouseEvent>


KWebView::KWebView(QWidget *parent, bool createCustomPage)
         :QWebView(parent), d(new KWebViewPrivate<KWebView>(this))
{
    if (createCustomPage)
        setPage(new KWebPage(this));
}

KWebView::~KWebView()
{
    delete d;
}

bool KWebView::isExternalContentAllowed() const
{
    return d->isExternalContentAllowed();
}

void KWebView::setAllowExternalContent(bool allow)
{
    d->setAllowExternalContent(allow);
}

void KWebView::wheelEvent(QWheelEvent *event)
{
    if (d->wheelEvent(event->delta())) {
        event->accept();
    } else {
        QWebView::wheelEvent(event);
    }
}


void KWebView::mousePressEvent(QMouseEvent *event)
{
    d->pressedButtons = event->buttons();
    d->keyboardModifiers = event->modifiers();
    QWebView::mousePressEvent(event);
}

void KWebView::mouseReleaseEvent(QMouseEvent *event)
{    
    if (d->mouseReleased(event->pos()) || d->handleUrlPasteFromClipboard(event)) {
        event->accept();
        return;
    }

    QWebView::mouseReleaseEvent(event);
}

#include "kwebview.moc"
