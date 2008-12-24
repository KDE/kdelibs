/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis // krazy:exclude=copyright (email unknown)
   Copyright (C) 2006-2007 Urs Wolfer <uwolfer at kde.org>

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

#include "kprogressdialog.h"

#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QTimer>

#include <kguiitem.h>
#include <kpushbutton.h>

class KProgressDialog::KProgressDialogPrivate
{
public:
    KProgressDialogPrivate(KProgressDialog *q)
        : q(q),
          cancelButtonShown(true),
          mAutoClose(true),
          mAutoReset(false),
          mCancelled(false),
          mAllowCancel(true),
          mShown(false),
          mMinDuration(2000)
    {
    }

    void slotAutoShow();
    void slotAutoActions(int percentage);

    KProgressDialog *q;
    bool          cancelButtonShown : 1;
    bool          mAutoClose : 1;
    bool          mAutoReset : 1;
    bool          mCancelled : 1;
    bool          mAllowCancel : 1;
    bool          mShown : 1;
    QString       mCancelText;
    QLabel*       mLabel;
    QProgressBar* mProgressBar;
    QTimer*       mShowTimer;
    int           mMinDuration;
};

KProgressDialog::KProgressDialog(QWidget* parent, const QString& caption,
                                 const QString& text, Qt::WFlags flags)
  : KDialog(parent, flags),
    d(new KProgressDialogPrivate(this))
{
    setCaption(caption);
    setButtons(KDialog::Cancel);

    d->mShowTimer = new QTimer(this);

    d->mCancelText = KDialog::buttonText(KDialog::Cancel);

    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(mainWidget);
    layout->setMargin(10);

    d->mLabel = new QLabel(text, mainWidget);
    layout->addWidget(d->mLabel);

    d->mProgressBar = new QProgressBar(mainWidget);
    layout->addWidget(d->mProgressBar);

    setMainWidget(mainWidget);

    connect(d->mProgressBar, SIGNAL(valueChanged(int)),
            this, SLOT(slotAutoActions(int)));
    connect(d->mShowTimer, SIGNAL(timeout()), this, SLOT(slotAutoShow()));
    d->mShowTimer->setSingleShot(true);
    d->mShowTimer->start(d->mMinDuration);
}

KProgressDialog::~KProgressDialog()
{
    delete d;
}

void KProgressDialog::KProgressDialogPrivate::slotAutoShow()
{
    if (mShown || mCancelled)
    {
        return;
    }

    q->show();
}

void KProgressDialog::showEvent(QShowEvent *event)
{
    d->mShown = true;
    KDialog::showEvent(event);
}

void KProgressDialog::reject()
{
    d->mCancelled = true;

    if (d->mAllowCancel)
    {
        KDialog::reject();
    }
}

bool KProgressDialog::wasCancelled() const
{
    return d->mCancelled;
}

void KProgressDialog::ignoreCancel()
{
    d->mCancelled = false;
}

void KProgressDialog::setMinimumDuration(int ms)
{
    d->mMinDuration = ms;
    if (!d->mShown)
    {
        d->mShowTimer->stop();
        d->mShowTimer->setSingleShot(true);
        d->mShowTimer->start(d->mMinDuration);
    }
}

int KProgressDialog::minimumDuration() const
{
    return d->mMinDuration;
}

void KProgressDialog::setAllowCancel(bool allowCancel)
{
    d->mAllowCancel = allowCancel;
    showCancelButton(allowCancel);
}

bool KProgressDialog::allowCancel() const
{
    return d->mAllowCancel;
}

QProgressBar* KProgressDialog::progressBar()
{
    return d->mProgressBar;
}

const QProgressBar* KProgressDialog::progressBar() const
{
    return d->mProgressBar;
}

void KProgressDialog::setLabelText(const QString& text)
{
    d->mLabel->setText(text);
}

QString KProgressDialog::labelText() const
{
    return d->mLabel->text();
}

void KProgressDialog::showCancelButton(bool show)
{
    showButton(Cancel, show);
}

bool KProgressDialog::autoClose() const
{
    return d->mAutoClose;
}

void KProgressDialog::setAutoClose(bool autoClose)
{
    d->mAutoClose = autoClose;
}

bool KProgressDialog::autoReset() const
{
    return d->mAutoReset;
}

void KProgressDialog::setAutoReset(bool autoReset)
{
    d->mAutoReset = autoReset;
}

void KProgressDialog::setButtonText(const QString& text)
{
    d->mCancelText = text;
    setButtonGuiItem(Cancel, KGuiItem(text));
}

QString KProgressDialog::buttonText() const
{
    return d->mCancelText;
}

void KProgressDialog::KProgressDialogPrivate::slotAutoActions(int percentage)
{
    if (percentage < mProgressBar->maximum() ||
        (mProgressBar->minimum() == mProgressBar->maximum())) // progress dialogs with busy indicators (see #178648)
    {
        if (!cancelButtonShown)
        {
            q->setButtonGuiItem(KDialog::Cancel, KGuiItem(mCancelText));
            cancelButtonShown = true;
        }
        return;
    }

    mShowTimer->stop();

    if (mAutoReset)
    {
        mProgressBar->setValue(0);
    }
    else
    {
        q->setAllowCancel(true);
        q->setButtonGuiItem(Cancel, KStandardGuiItem::close());
        cancelButtonShown = false;
    }

    if (mAutoClose)
    {
        if (mShown)
        {
            q->hide();
        }
        else
        {
            emit q->finished();
        }
    }
}

#include "kprogressdialog.moc"
