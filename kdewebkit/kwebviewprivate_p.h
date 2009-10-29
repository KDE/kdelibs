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

#ifndef KWEBVIEW_KWEBVIEWPRIVATE_P_H
#define KWEBVIEW_KWEBVIEWPRIVATE_P_H

#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtWebKit/QWebFrame>

#include <kurl.h>

template <class T>
class KWebViewPrivate
{
public:
    KWebViewPrivate(T *parent)
    : q(parent),
      keyboardModifiers(Qt::NoModifier) ,
      pressedButtons(Qt::NoButton)
    {
    }

    bool isExternalContentAllowed()
    {
        KWebPage *webPage = qobject_cast<KWebPage*>(q->page());
        if (webPage) {
            return webPage->isExternalContentAllowed();
        }

        return false;
    }

    void setAllowExternalContent(bool allow)
    {
        KWebPage *webPage = qobject_cast<KWebPage*>(q->page());
        if (webPage) {
            webPage->setAllowExternalContent(allow);
        }
    }

    bool wheelEvent(int delta)
    {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            const int numDegrees = delta / 8;
            const int numSteps = numDegrees / 15;
            q->setZoomFactor(q->zoomFactor() + numSteps * 0.1);
            return true;
        }

        return false;
    }

    bool mouseReleased(const QPoint &pos)
    {
        const QWebHitTestResult result = q->page()->mainFrame()->hitTestContent(pos);
        const QUrl url = result.linkUrl();

        if (url.isEmpty()) {
            return false;
        }

        if ((pressedButtons & Qt::MidButton) ||
            ((pressedButtons & Qt::LeftButton) && (keyboardModifiers & Qt::ControlModifier))) {
            emit q->openUrlInNewWindow(url);
            return true;
        }

        if ((pressedButtons & Qt::LeftButton) && (keyboardModifiers & Qt::ShiftModifier)) {
            emit q->saveUrl(url);
            return true;
        }

        return false;
    }

    void postMouseReleaseHandling()
    {
        if (q->page() && !q->page()->isModified() && (pressedButtons & Qt::MidButton)) {
            const QString clipboardText(QApplication::clipboard()->text(QClipboard::Selection));
            KUrl url(clipboardText);
            if (!url.isEmpty() && url.isValid() && clipboardText.contains('.')) { // contains '.' -> domain
                if (url.scheme().isEmpty()) {
                    url = "http://" + clipboardText;
                }
                emit q->openUrl(url);
            }
        }
    }

    T *q;
    Qt::KeyboardModifiers keyboardModifiers;
    Qt::MouseButtons pressedButtons;
};

#endif
