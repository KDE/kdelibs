/*  This file is part of the KDE project
    Copyright (C) 2007 Rafael Fernández López <ereslibre@gmail.com>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef KSTATUSBARJOBTRACKER_P_H
#define KSTATUSBARJOBTRACKER_P_H

#include <QWidget>
#include <QMap>
#include <QTime>
#include <QBoxLayout>
#include <QStackedWidget>

#include <kdebug.h>
#include <kurl.h>

class KPushButton;
class QCheckBox;
class KLineEdit;
class QLabel;
class QProgressBar;


class KStatusBarJobTracker::Private
{
public:
    Private(QWidget *parent)
        : parent(parent) { }

    ~Private() {
    }

    class ProgressWidget;

    QWidget *parent;
    QMap<KJob*, ProgressWidget*> progressWidget;
};


class KStatusBarJobTracker::Private::ProgressWidget
    : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(KJob *job, KStatusBarJobTracker *object, QWidget *parent)
        : q(object), job(job), widget(0), progressBar(0), label(0), button(0),
          box(0), stack(0), totalSize(0), mode(None), showButton(false)
    {
        init(parent);
    }

    ~ProgressWidget()
    {
    }

    KStatusBarJobTracker *const q;
    KJob *const job;

    QWidget *widget;
    QProgressBar *progressBar;
    QLabel *label;
    KPushButton *button;
    QBoxLayout *box;
    QStackedWidget *stack;

    qulonglong totalSize;

    enum Mode { None, Label, Progress };
    Mode mode;

    bool showButton;

    void init(QWidget *parent);

    void setMode(Mode newMode);

public Q_SLOTS:
    virtual void totalAmount(KJob::Unit unit, qulonglong amount);
    virtual void percent(unsigned long percent);
    virtual void speed(unsigned long value);
    virtual void slotClean();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
};


#endif // KSTATUSBARJOBTRACKER_P_H
