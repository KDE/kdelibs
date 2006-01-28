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
 * KProgressDialog -- a dialog with a progress indicator
 */

#include <QLabel>
#include <QLayout>
#include <QTimer>

#include <kpushbutton.h>

#include "kprogressdialog.h"

struct KProgressDialog::KProgressDialogPrivate
{
    KProgressDialogPrivate()
        : cancelButtonShown(true),
          mAutoClose(true),
          mAutoReset(false),
          mCancelled(false),
          mAllowCancel(true),
          mShown(false),
          mMinDuration(2000)
    {
    }
    bool          cancelButtonShown;
    bool          mAutoClose;
    bool          mAutoReset;
    bool          mCancelled;
    bool          mAllowCancel;
    bool          mShown;
    QString       mCancelText;
    QLabel*       mLabel;
    KProgressBar* mProgressBar;
    QTimer*       mShowTimer;
    int           mMinDuration;
};

KProgressDialog::KProgressDialog(QWidget* parent, const QString& caption,
                                 const QString& text, bool modal)
  : KDialog(parent, caption, KDialog::Cancel),
    d(new KProgressDialogPrivate)
{
    setModal(modal);

    d->mShowTimer = new QTimer(this);

    d->mCancelText = actionButton(KDialog::Cancel)->text();

    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(mainWidget);
    layout->setMargin(10);

    d->mLabel = new QLabel(text, mainWidget);
    layout->addWidget(d->mLabel);

    d->mProgressBar = new KProgressBar(mainWidget);
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

void KProgressDialog::slotAutoShow()
{
    if (d->mShown || d->mCancelled)
    {
        return;
    }

    show();
    d->mShown = true;
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

KProgressBar* KProgressDialog::progressBar()
{
    return d->mProgressBar;
}

const KProgressBar* KProgressDialog::progressBar() const
{
    return d->mProgressBar;
}

void KProgressDialog::setLabel(const QString& text)
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
    setButtonGuiItem(Cancel, text);
}

QString KProgressDialog::buttonText() const
{
    return d->mCancelText;
}

void KProgressDialog::slotAutoActions(int percentage)
{
    if (percentage < d->mProgressBar->maximum())
    {
        if (!d->cancelButtonShown)
        {
            setButtonGuiItem(Cancel, d->mCancelText);
            d->cancelButtonShown = true;
        }
        return;
    }

    d->mShowTimer->stop();

    if (d->mAutoReset)
    {
        d->mProgressBar->setValue(0);
    }
    else
    {
        setAllowCancel(true);
        setButtonGuiItem(Cancel, KStdGuiItem::close());
        d->cancelButtonShown = false;
    }

    if (d->mAutoClose)
    {
        if (d->mShown)
        {
            hide();
        }
        else
        {
            emit finished();
        }
    }
}

void KProgressDialog::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }

#include "kprogressdialog.moc"
