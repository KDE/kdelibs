/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>

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

#include "kstatusbarjobtracker.h"
#include "kstatusbarjobtracker_p.h"

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QMouseEvent>

#include <kpushbutton.h>
#include <klocale.h>
#include <kglobal.h>

KStatusBarJobTracker::KStatusBarJobTracker(QWidget *parent, bool button)
    : KAbstractWidgetJobTracker(parent), d(new Private(parent))
{
}

KStatusBarJobTracker::~KStatusBarJobTracker()
{
    delete d;
}

void KStatusBarJobTracker::registerJob(KJob *job)
{
    KAbstractWidgetJobTracker::registerJob(job);

    if (d->progressWidget.contains(job)) {
        return;
    }

    Private::ProgressWidget *vi = new Private::ProgressWidget(job, this, d->parent);

    d->progressWidget.insert(job, vi);
}

QWidget *KStatusBarJobTracker::widget(KJob *job)
{
    if (!d->progressWidget.contains(job)) {
        return 0;
    }

    return d->progressWidget[job];
}

void KStatusBarJobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->totalAmount(unit, amount);
}

void KStatusBarJobTracker::percent(KJob *job, unsigned long percent)
{
    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->percent(percent);
}

void KStatusBarJobTracker::speed(KJob *job, unsigned long value)
{
    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->speed(value);
}

void KStatusBarJobTracker::slotClean(KJob *job)
{
    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->slotClean();
}

void KStatusBarJobTracker::Private::ProgressWidget::init(QWidget *parent)
{
    showButton = button;

    // only clean this dialog
    q->setAutoDelete(job, false);

    widget = new QWidget(parent);

    int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
    box = new QHBoxLayout(widget);
    box->setMargin(0);
    box->setSpacing(0);

    button = new KPushButton("X", widget);
    box->addWidget(button);
    stack = new QStackedWidget(widget);
    box->addWidget(stack);
    connect(button, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    progressBar = new QProgressBar(widget);
//    progressBar->setFrameStyle( QFrame::Box | QFrame::Raised );
//    progressBar->setLineWidth( 1 );
    progressBar->setBackgroundRole(QPalette::Window); // ### KDE4: still needed?
    progressBar->installEventFilter(this);
    progressBar->setMinimumWidth(w);
    stack->insertWidget(1, progressBar);

    label = new QLabel("", widget);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->installEventFilter(this);
    label->setMinimumWidth(w);
    stack->insertWidget(2, label);
    setMinimumSize(sizeHint());

    setMode(KStatusBarJobTracker::Private::ProgressWidget::None);
}

void KStatusBarJobTracker::Private::ProgressWidget::setMode(Mode newMode)
{
    switch (newMode)
    {
    case None:
        if (showButton) {
            button->hide();
        }
        stack->hide();
        break;

    case Label:
        if (showButton) {
            button->show();
        }
        stack->show();
        stack->setCurrentWidget(label);
        break;

    case Progress:
        if (showButton) {
            button->show();
        }
        stack->show();
        stack->setCurrentWidget(progressBar);
        break;
    }

    mode = newMode;
}

void KStatusBarJobTracker::Private::ProgressWidget::totalAmount(KJob::Unit unit, qulonglong amount)
{
    if (unit==KJob::Bytes) {
        totalSize = amount;
    }
}

void KStatusBarJobTracker::Private::ProgressWidget::percent(unsigned long percent)
{
    progressBar->setValue(percent);
}

void KStatusBarJobTracker::Private::ProgressWidget::speed(unsigned long value)
{
    if (value == 0 ) { // speed is measured in bytes-per-second
        label->setText(i18n(" Stalled "));
    } else {
        label->setText(i18n(" %1/s ", KGlobal::locale()->formatByteSize(value)));
    }
}

void KStatusBarJobTracker::Private::ProgressWidget::slotClean()
{
    // we don't want to delete this widget, only clean
    progressBar->setValue(0);
    label->clear();

    setMode(KStatusBarJobTracker::Private::ProgressWidget::None);
}

bool KStatusBarJobTracker::Private::ProgressWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==progressBar || obj==label) {

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);

            if (e->button() == Qt::LeftButton) {    // toggle view on left mouse button
                if (mode == KStatusBarJobTracker::Private::ProgressWidget::Label) {
                    setMode(KStatusBarJobTracker::Private::ProgressWidget::Progress);
                } else if (mode == KStatusBarJobTracker::Private::ProgressWidget::Progress) {
                    setMode(KStatusBarJobTracker::Private::ProgressWidget::Label);
                }
                return true;
            }
        }

        return false;
    }

    return QWidget::eventFilter(obj, event);
}

#include "kstatusbarjobtracker.moc"
#include "kstatusbarjobtracker_p.moc"
