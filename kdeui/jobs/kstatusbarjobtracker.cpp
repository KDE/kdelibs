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
    : KAbstractWidgetJobTracker(parent), d(new Private(parent, button))
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
    d->currentProgressWidget = vi;

    d->progressWidget.insert(job, vi);
}

void KStatusBarJobTracker::unregisterJob(KJob *job)
{
    KAbstractWidgetJobTracker::unregisterJob(job);

    if (!d->progressWidget.contains(job))
        return;

    if (d->currentProgressWidget == d->progressWidget[job])
        d->currentProgressWidget = 0;

    if (!d->progressWidget[job]->beingDeleted)
        delete d->progressWidget[job];

    d->progressWidget.remove(job);
}

QWidget *KStatusBarJobTracker::widget(KJob *job)
{
    if (!d->progressWidget.contains(job)) {
        return 0;
    }

    return d->progressWidget[job];
}

void KStatusBarJobTracker::setStatusBarMode(StatusBarModes statusBarMode)
{
    if (!d->currentProgressWidget) {
        return;
    }

    d->currentProgressWidget->setMode(statusBarMode);
}

void KStatusBarJobTracker::description(KJob *job, const QString &title,
                                       const QPair<QString, QString> &field1,
                                       const QPair<QString, QString> &field2)
{
    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->description(title, field1, field2);
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

void KStatusBarJobTracker::Private::ProgressWidget::killJob()
{
    job->kill(KJob::EmitResult); // notify that the job has been killed
}

void KStatusBarJobTracker::Private::ProgressWidget::init(KJob *job, QWidget *parent)
{
    widget = new QWidget(parent);

    int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
    box = new QHBoxLayout(widget);
    box->setMargin(0);
    box->setSpacing(0);
    widget->setLayout(box);

    stack = new QStackedWidget(widget);
    box->addWidget(stack);

    if (q->d->showStopButton) {
        button = new KPushButton(i18n("Stop"), widget);
        box->addWidget(button);
        connect(button, SIGNAL(clicked(bool)),
                this, SLOT(killJob()));
    } else {
        button = 0;
    }

    progressBar = new QProgressBar(widget);
    progressBar->installEventFilter(this);
    progressBar->setMinimumWidth(w);
    stack->insertWidget(1, progressBar);

    label = new QLabel(widget);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->installEventFilter(this);
    label->setMinimumWidth(w);
    stack->insertWidget(2, label);
    setMinimumSize(sizeHint());

    setMode(KStatusBarJobTracker::LabelOnly);

    q->setAutoDelete(job, true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(widget);
    setLayout(layout);
}

void KStatusBarJobTracker::Private::ProgressWidget::setMode(StatusBarModes newMode)
{
    mode = newMode;

    if (newMode == KStatusBarJobTracker::NoInformation)
    {
        stack->hide();

        return;
    }

    if (newMode & KStatusBarJobTracker::LabelOnly)
    {
        stack->show();
        stack->setCurrentWidget(label);

        return; // TODO: we should make possible to show an informative label and the progress bar
    }

    if (newMode & KStatusBarJobTracker::ProgressOnly)
    {
        stack->show();
        stack->setCurrentWidget(progressBar);
    }
}

void KStatusBarJobTracker::Private::ProgressWidget::description(const QString &title,
                                                                const QPair<QString, QString> &field1,
                                                                const QPair<QString, QString> &field2)
{
    Q_UNUSED(field1);
    Q_UNUSED(field2);

    label->setText(title);
}

void KStatusBarJobTracker::Private::ProgressWidget::totalAmount(KJob::Unit unit, qulonglong amount)
{
    Q_UNUSED(unit);
    Q_UNUSED(amount);
#if 0 // currently unused
    if (unit==KJob::Bytes) {
        totalSize = amount;
    }
#endif
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

    setMode(KStatusBarJobTracker::NoInformation);
}

bool KStatusBarJobTracker::Private::ProgressWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==progressBar || obj==label) {

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);

            // TODO: we should make possible to show an informative label and the progress bar
            if (e->button() == Qt::LeftButton) {    // toggle view on left mouse button
                if (mode == KStatusBarJobTracker::LabelOnly) {
                    setMode(KStatusBarJobTracker::ProgressOnly);
                } else if (mode == KStatusBarJobTracker::ProgressOnly) {
                    setMode(KStatusBarJobTracker::LabelOnly);
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
