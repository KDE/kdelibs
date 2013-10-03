// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>
   Copyright (C) 2008 Norbert Frese <nf2@scheinwelt.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkowner.h"
#include "kbookmarkdialog.h"

class KBookmarkOwner::FutureBookmarkPrivate : public QSharedData
{
public:
    QString title;
    QUrl url;
    QString icon;
};

KBookmarkOwner::FutureBookmark::FutureBookmark(const QString &title, const QUrl &url, const QString &icon)
    : d(new FutureBookmarkPrivate)
{
    d->title = title;
    d->url = url;
    d->icon = icon;
}

KBookmarkOwner::FutureBookmark::FutureBookmark(const FutureBookmark &other)
    : d(other.d)
{
}

KBookmarkOwner::FutureBookmark& KBookmarkOwner::FutureBookmark::operator=(const FutureBookmark &other)
{
    d = other.d;
    return *this;
}

KBookmarkOwner::FutureBookmark::~FutureBookmark()
{
}

QString KBookmarkOwner::FutureBookmark::title() const
{
    return d->title;
}

QUrl KBookmarkOwner::FutureBookmark::url() const
{
    return d->url;
}

QString KBookmarkOwner::FutureBookmark::icon() const
{
    return d->icon;
}

bool KBookmarkOwner::enableOption(BookmarkOption action) const
{
    if(action == ShowAddBookmark)
        return true;
    if(action == ShowEditBookmark)
        return true;
    return false;
}

KBookmarkDialog * KBookmarkOwner::bookmarkDialog(KBookmarkManager * mgr, QWidget * parent)
{
    return new KBookmarkDialog(mgr, parent);
}

void KBookmarkOwner::openFolderinTabs(const KBookmarkGroup &)
{

}

void KBookmarkOwner::openInNewTab(const KBookmark &bm)
{
    // fallback
    openBookmark(bm, Qt::LeftButton, Qt::NoModifier);
}

void KBookmarkOwner::openInNewWindow(const KBookmark &bm)
{
    // fallback
    openBookmark(bm, Qt::LeftButton, Qt::NoModifier);
}

