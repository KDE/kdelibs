/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis
   Copyright (C) 2006 Urs Wolfer <uwolfer @ fwo.ch>

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
/**
 * KProgressBar -- a progress indicator widget for KDE.
 */

#include <QLayout>

#include "kprogressbar.h"

struct KProgressBar::KProgressBarPrivate
{
    KProgressBarPrivate()
        : mFormat("%p%")
    {
    }
    QString    mFormat;
};

KProgressBar::KProgressBar(QWidget *parent)
  : QProgressBar(parent),
    d(new KProgressBarPrivate)
{
    setValue(0);
}

KProgressBar::KProgressBar(int totalSteps, QWidget *parent)
  : QProgressBar(parent),
    d(new KProgressBarPrivate)
{
    setRange(0, totalSteps);
    setValue(0);
}

KProgressBar::~KProgressBar()
{
    delete d;
}

void KProgressBar::advance(int offset)
{
    setValue(value() + offset);
}

void KProgressBar::setFormat(const QString & format)
{
    d->mFormat = format;
    if (d->mFormat != "%p%")
        setAlignment(Qt::AlignHCenter);
}

QString KProgressBar::format() const
{
    return d->mFormat;
}

QString KProgressBar::text() const
{
    int totalSteps = maximum() - minimum();

    if (maximum() == 0 || value() < minimum()
            || (value() == INT_MIN && minimum() == INT_MIN)
            || totalSteps == 0)
        return QString();

    QString newString(d->mFormat);
    newString.replace(QLatin1String("%v"),
                      QString::number(value()));
    newString.replace(QLatin1String("%m"),
                      QString::number(totalSteps));

    int progress = value() - minimum();

    if (totalSteps > INT_MAX / 1000) {
        progress /= 1000;
        totalSteps /= 1000;
    }

    newString.replace(QLatin1String("%p"),
                      QString::number((progress * 100) / totalSteps));

    return tr("%1").arg(newString);
}

void KProgressBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kprogressbar.moc"
