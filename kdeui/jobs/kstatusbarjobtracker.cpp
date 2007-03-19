/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
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

#include "kstatusbarjobtracker.h"

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QMouseEvent>

#include <klocale.h>

class KStatusBarJobTracker::Private
{
public:
    Private() : widget(0), progressBar(0), label(0), button(0),
                box(0), stack(0), totalSize(0),
                mode(None), showButton(false) { }

    QWidget *widget;
    QProgressBar *progressBar;
    QLabel *label;
    QPushButton *button;
    QBoxLayout *box;
    QStackedWidget *stack;

    qulonglong totalSize;

    enum Mode { None, Label, Progress };
    Mode mode;

    bool showButton;

    void setMode(Mode newMode);
};


KStatusBarJobTracker::KStatusBarJobTracker(QWidget *parent, bool button)
    : KAbstractWidgetJobTracker(parent), d(new Private)
{
    d->showButton = button;

    // only clean this dialog
    setAutoDelete(false);

    d->widget = new QWidget(parent);

    int w = d->widget->fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
    d->box = new QHBoxLayout(d->widget);
    d->box->setMargin(0);
    d->box->setSpacing(0);

    d->button = new QPushButton("X", d->widget);
    d->box->addWidget(d->button);
    d->stack = new QStackedWidget(d->widget);
    d->box->addWidget(d->stack);
    connect(d->button, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    d->progressBar = new QProgressBar(d->widget);
//    d->progressBar->setFrameStyle( QFrame::Box | QFrame::Raised );
//    d->progressBar->setLineWidth( 1 );
    d->progressBar->setBackgroundRole(QPalette::Window); // ### KDE4: still needed?
    d->progressBar->installEventFilter(this);
    d->progressBar->setMinimumWidth(w);
    d->stack->insertWidget(1, d->progressBar);

    d->label = new QLabel("", d->widget);
    d->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->label->installEventFilter(this);
    d->label->setMinimumWidth(w);
    d->stack->insertWidget(2, d->label);
    d->widget->setMinimumSize(d->widget->sizeHint());

    d->setMode(KStatusBarJobTracker::Private::None);
}

KStatusBarJobTracker::~KStatusBarJobTracker()
{
    delete d->widget;
    delete d;
}

void KStatusBarJobTracker::registerJob(KJob *job)
{
    KAbstractWidgetJobTracker::registerJob(job);
    d->setMode(KStatusBarJobTracker::Private::Progress);
}

QWidget *KStatusBarJobTracker::widget()
{
    return d->widget;
}

bool KStatusBarJobTracker::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==d->progressBar || obj==d->label) {

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);

            if (e->button() == Qt::LeftButton) {    // toggle view on left mouse button
                if (d->mode == KStatusBarJobTracker::Private::Label) {
                    d->setMode(KStatusBarJobTracker::Private::Progress);
                } else if (d->mode == KStatusBarJobTracker::Private::Progress) {
                    d->setMode(KStatusBarJobTracker::Private::Label);
                }
                return true;
            }
        }

        return false;
    }

    return KAbstractWidgetJobTracker::eventFilter(obj, event);
}

void KStatusBarJobTracker::totalAmount(KJob */*job*/, KJob::Unit unit, qulonglong amount)
{
    if (unit==KJob::Bytes) {
        d->totalSize = amount;
    }
}

void KStatusBarJobTracker::percent(KJob */*job*/, unsigned long percent)
{
    d->progressBar->setValue(percent);
}

void KStatusBarJobTracker::speed(KJob */*job*/, unsigned long value)
{
    if (value == 0 ) { // speed is measured in bytes-per-second
        d->label->setText(i18n(" Stalled "));
    } else {
        d->label->setText(i18n(" %1/s ", KGlobal::locale()->formatByteSize(value)));
    }
}

void KStatusBarJobTracker::slotClean()
{
    // we don't want to delete this widget, only clean
    d->progressBar->setValue(0);
    d->label->clear();

    d->setMode(KStatusBarJobTracker::Private::None);
}

void KStatusBarJobTracker::Private::setMode(Mode newMode)
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

#include "kstatusbarjobtracker.moc"
