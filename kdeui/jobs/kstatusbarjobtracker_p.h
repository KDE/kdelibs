/*  This file is part of the KDE project
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
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
    Private(QWidget *parent, bool withStopButton)
        : parent(parent)
        , currentProgressWidget(0)
        , showStopButton(withStopButton)
    { }

    ~Private() {
    }

    class ProgressWidget;

    QWidget *parent;
    QMap<KJob*, ProgressWidget*> progressWidget;
    ProgressWidget *currentProgressWidget;
    bool showStopButton;
};


class KStatusBarJobTracker::Private::ProgressWidget
    : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(KJob *job, KStatusBarJobTracker *object, QWidget *parent)
        : q(object), job(job), widget(0), progressBar(0), label(0), button(0),
          box(0), stack(0), /*totalSize(-1),*/ mode(NoInformation), beingDeleted(false)
    {
        init(job, parent);
    }

    ~ProgressWidget()
    {
        beingDeleted = true;
        delete widget;

        q->unregisterJob(job);
    }

    KStatusBarJobTracker *const q;
    KJob *const job;

    QWidget *widget;
    QProgressBar *progressBar;
    QLabel *label;
    KPushButton *button;
    QBoxLayout *box;
    QStackedWidget *stack;

    //qlonglong totalSize;

    StatusBarModes mode;
    bool beingDeleted;

    void init(KJob *job, QWidget *parent);

    void setMode(StatusBarModes newMode);

public Q_SLOTS:
    virtual void description(const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob::Unit unit, qulonglong amount);
    virtual void percent(unsigned long percent);
    virtual void speed(unsigned long value);
    virtual void slotClean();

private Q_SLOTS:
    void killJob();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
};


#endif // KSTATUSBARJOBTRACKER_P_H
