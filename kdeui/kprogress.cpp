/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
/**
 * KProgress -- a progress indicator widget for KDE.
 */

#include <stdlib.h>
#include <limits.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstyle.h>
#include <qtimer.h>

#include "kprogress.h"

#include <kapplication.h>
#include <kwin.h>

KProgress::KProgress(QWidget *parent, const char *name, WFlags f)
  : QProgressBar(parent, name, f),
    mFormat("%p%")
{
    setProgress(0);
}

KProgress::KProgress(int totalSteps, QWidget *parent, const char *name, WFlags f)
  : QProgressBar(totalSteps, parent, name, f),
    mFormat("%p%")
{
    setProgress(0);
}

KProgress::~KProgress()
{
}

void KProgress::advance(int offset)
{
    setProgress(progress() + offset);
}

void KProgress::setTotalSteps(int totalSteps)
{
    QProgressBar::setTotalSteps(totalSteps);

    if (totalSteps)
    {
        emit percentageChanged((progress() * 100) / totalSteps);
    }
}

void KProgress::setProgress(int progress)
{
    QProgressBar::setProgress(progress);

    if (totalSteps())
    {
        emit percentageChanged((progress * 100) / totalSteps());
    }
}

void KProgress::setValue(int progress)
{
    setProgress(progress); 
}

void KProgress::setRange(int /*min*/, int max)
{
    setTotalSteps(max);
}

int KProgress::maxValue()
{
    return totalSteps();
}

void KProgress::setTextEnabled(bool enable)
{
    setPercentageVisible(enable);
}

bool KProgress::textEnabled() const
{
    return percentageVisible();
}

void KProgress::setFormat(const QString & format)
{
    mFormat = format;
    if (mFormat != "%p%")
        setCenterIndicator(true);
}

QString KProgress::format() const
{
    return mFormat;
}

int KProgress::value() const
{
    return progress();
}

bool KProgress::setIndicator(QString &indicator, int progress, int totalSteps)
{
    if (!totalSteps)
        return false;
    QString newString(mFormat);
    newString.replace(QString::fromLatin1("%v"),
                      QString::number(progress));
    newString.replace(QString::fromLatin1("%m"),
                      QString::number(totalSteps));

    if (totalSteps > INT_MAX / 1000) {
        progress /= 1000;
        totalSteps /= 1000;
    }

    newString.replace(QString::fromLatin1("%p"),
                      QString::number((progress * 100) / totalSteps)); 

    if (newString != indicator)
    {
        indicator = newString;
        return true;
    }

    return false;
}

class KProgressDialog::KProgressDialogPrivate
{
    public:
        KProgressDialogPrivate() :
            AutoClose(true),
            AutoReset(false),
            Cancelled(false),
            AllowCancel(true),
            Shown(false),
            MinDuration(2000) { }

        bool       AutoClose;
        bool       AutoReset;
        bool       Cancelled;
        bool       AllowCancel;
        bool       Shown;
        int        MinDuration;
        QString    CancelText;
        QLabel*    Label;
        KProgress* ProgressBar;
        QTimer*    ShowTimer;
};

/*
 * KProgressDialog implementation
 */
KProgressDialog::KProgressDialog(QWidget* parent, const char* name,
                                 const QString& caption, const QString& text,
                                 bool modal)
    : KDialogBase(KDialogBase::Plain, caption, KDialogBase::Cancel,
                  KDialogBase::Cancel, parent, name, modal),
      d( new KProgressDialogPrivate )
{
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
    d->ShowTimer = new QTimer(this);
    
    showButton(KDialogBase::Close, false);
    d->CancelText = actionButton(KDialogBase::Cancel)->text();

    QFrame* mainWidget = plainPage();
    QVBoxLayout* layout = new QVBoxLayout(mainWidget, 10);

    d->Label = new QLabel(text, mainWidget);
    layout->addWidget(d->Label);

    d->ProgressBar = new KProgress(mainWidget);
    layout->addWidget(d->ProgressBar);

    connect(d->ProgressBar, SIGNAL(percentageChanged(int)),
            this, SLOT(slotAutoActions(int)));
    connect(d->ShowTimer, SIGNAL(timeout()), this, SLOT(slotAutoShow()));
    d->ShowTimer->start(d->MinDuration, true);
}

KProgressDialog::~KProgressDialog()
{
    delete d;
}

void KProgressDialog::slotAutoShow()
{
    if (d->Shown || d->Cancelled)
    {
        return;
    }

    show();
    kapp->processEvents();
    d->Shown = true;
}

void KProgressDialog::slotCancel()
{
    d->Cancelled = true;

    if (d->AllowCancel)
    {
        KDialogBase::slotCancel();
    }
}

bool KProgressDialog::wasCancelled()
{
    return d->Cancelled;
}

bool KProgressDialog::wasCancelled() const
{
    return d->Cancelled;
}

void KProgressDialog::setMinimumDuration(int ms)
{
    d->MinDuration = ms;
    if (!d->Shown)
    {
        d->ShowTimer->stop();
        d->ShowTimer->start(d->MinDuration, true);
    }
}

int KProgressDialog::minimumDuration()
{
    return d->MinDuration;
}

int KProgressDialog::minimumDuration() const
{
    return d->MinDuration;
}

void KProgressDialog::setAllowCancel(bool allowCancel)
{
    d->AllowCancel = allowCancel;
    showCancelButton(allowCancel);
}

bool KProgressDialog::allowCancel()
{
    return d->AllowCancel;
}

bool KProgressDialog::allowCancel() const
{
    return d->AllowCancel;
}

KProgress* KProgressDialog::progressBar()
{
    return d->ProgressBar;
}

const KProgress* KProgressDialog::progressBar() const
{
    return d->ProgressBar;
}

void KProgressDialog::setLabel(const QString& text)
{
    d->Label->setText(text);
}

QString KProgressDialog::labelText()
{
    return d->Label->text();
}

QString KProgressDialog::labelText() const
{
    return d->Label->text();
}

void KProgressDialog::showCancelButton(bool show)
{
    showButtonCancel(show);
}

bool KProgressDialog::autoClose()
{
    return d->AutoClose;
}

void KProgressDialog::setAutoClose(bool autoClose)
{
    d->AutoClose = autoClose;
}

bool KProgressDialog::autoReset()
{
    return d->AutoReset;
}

bool KProgressDialog::autoReset() const
{
    return d->AutoReset;
}

void KProgressDialog::setAutoReset(bool autoReset)
{
    d->AutoReset = autoReset;
}

void KProgressDialog::setButtonText(const QString& text)
{
    d->CancelText = text;
    setButtonCancelText(d->CancelText);
}

QString KProgressDialog::buttonText()
{
    return d->CancelText;
}

QString KProgressDialog::buttonText() const
{
    return d->CancelText;
}

void KProgressDialog::slotAutoActions(int percentage)
{
    if (percentage < 100)
    {
        setButtonCancelText(d->CancelText);
    }
    else
    {
        if (d->AutoReset)
        {
            d->ProgressBar->setProgress(0);
        }
        else
        {
            setAllowCancel(true);
            setButtonCancelText("&Close");
        }

        if (d->AutoClose)
        {
            hide();
        }
    }
}

void KProgress::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KProgressDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kprogress.moc"
