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
    newString.replace(QRegExp(QString::fromLatin1("%p")),
                      QString::number((progress * 100) / totalSteps));
    newString.replace(QRegExp(QString::fromLatin1("%v")),
                      QString::number(progress));
    newString.replace(QRegExp(QString::fromLatin1("%m")),
                      QString::number(totalSteps));

    if (newString != indicator)
    {
        indicator = newString;
        return true;
    }

    return false;
}


/*
 * KProgressDialog implementation
 */
KProgressDialog::KProgressDialog(QWidget* parent, const char* name,
                                 const QString& caption, const QString& text,
                                 bool modal)
    : KDialogBase(KDialogBase::Plain, caption, KDialogBase::Cancel,
                  KDialogBase::Cancel, parent, name, modal),
      mAutoClose(true),
      mAutoReset(false),
      mCancelled(false),
      mAllowCancel(true),
      mShown(false),
      mMinDuration(2000)
{
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
    mShowTimer = new QTimer(this);
    
    showButton(KDialogBase::Close, false);
    mCancelText = actionButton(KDialogBase::Cancel)->text();

    QFrame* mainWidget = plainPage();
    QVBoxLayout* layout = new QVBoxLayout(mainWidget, 10);

    mLabel = new QLabel(text, mainWidget);
    layout->addWidget(mLabel);

    mProgressBar = new KProgress(mainWidget);
    layout->addWidget(mProgressBar);

    connect(mProgressBar, SIGNAL(percentageChanged(int)),
            this, SLOT(slotAutoActions(int)));
    connect(mShowTimer, SIGNAL(timeout()), this, SLOT(slotAutoShow()));
    mShowTimer->start(mMinDuration, true);
}

KProgressDialog::~KProgressDialog()
{
}

void KProgressDialog::slotAutoShow()
{
    if (mShown || mCancelled)
    {
        return;
    }

    show();
    kapp->processEvents();
    mShown = true;
}

void KProgressDialog::slotCancel()
{
    mCancelled = true;

    if (mAllowCancel)
    {
        KDialogBase::slotCancel();
    }
}

bool KProgressDialog::wasCancelled()
{
    return mCancelled;
}

void KProgressDialog::setMinimumDuration(int ms)
{
    mMinDuration = ms;
    if (!mShown)
    {
        mShowTimer->stop();
        mShowTimer->start(mMinDuration, true);
    }
}

int KProgressDialog::minimumDuration()
{
    return mMinDuration;
}

void KProgressDialog::setAllowCancel(bool allowCancel)
{
    mAllowCancel = allowCancel;
    showCancelButton(allowCancel);
}

bool KProgressDialog::allowCancel()
{
    return mAllowCancel;
}

KProgress* KProgressDialog::progressBar()
{
    return mProgressBar;
}

void KProgressDialog::setLabel(const QString& text)
{
    mLabel->setText(text);
}

QString KProgressDialog::labelText()
{
    return mLabel->text();
}

void KProgressDialog::showCancelButton(bool show)
{
    showButtonCancel(show);
}

bool KProgressDialog::autoClose()
{
    return mAutoClose;
}

void KProgressDialog::setAutoClose(bool autoClose)
{
    mAutoClose = autoClose;
}

bool KProgressDialog::autoReset()
{
    return mAutoReset;
}

void KProgressDialog::setAutoReset(bool autoReset)
{
    mAutoReset = autoReset;
}

void KProgressDialog::setButtonText(const QString& text)
{
    mCancelText = text;
    setButtonCancelText(mCancelText);
}

QString KProgressDialog::buttonText()
{
    return mCancelText;
}

void KProgressDialog::slotAutoActions(int percentage)
{
    if (percentage < 100)
    {
        setButtonCancelText(mCancelText);
    }
    else
    {
        if (mAutoReset)
        {
            mProgressBar->setProgress(0);
        }
        else
        {
            setAllowCancel(true);
            setButtonCancelText("&Close");
        }

        if (mAutoClose)
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
