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
/*****************************************************************************
*                                                                            *
*  KProgressDialog -- a dialog with a progress indicator                     *
*  Original QRangeControl-based version written by Martynas Kunigelis        *
*  QProgressBar based version by Aaron Seigo                                 *
*  Current Qt4 QProgressBar based version by Urs Wolfer                      *
*                                                                            *
*****************************************************************************/

#ifndef KPROGRESSDIALOG_H
#define KPROGRESSDIALOG_H

#include <kdialog.h>
#include <kprogressbar.h>

/**
 * KProgressDialog provides a dialog with a text label, a progress bar
 * and an optional cancel button with a KDE look 'n feel.
 *
 * Since knowing how long it can take to complete an action and it is
 * undesirable to show a dialog for a split second before hiding it,
 * there are a few ways to control the timing behavior of KProgressDialog.
 * There is a time out that can be set before showing the dialog as well
 * as an option to autohide or keep displaying the dialog once complete.
 *
 * All the functionality of KProgressBar is available through direct access
 * to the progress bar widget via progressBar();
 *
 * @short A dialog with a progress bar
 * @author Aaron J. Seigo
 * @author Urs Wolfer uwolfer @ fwo.ch
 */
class KDEUI_EXPORT KProgressDialog : public KDialog
{
    Q_OBJECT

    public:
        /**
         * Constructs a KProgressDialog
         *
         * @param parent Parent of the widget
         * @param caption Text to display in window title bar
         * @param text Text to display in the dialog
         * @param modal Set to true to make the dialog modal
         */
        KProgressDialog(QWidget* parent = 0, const QString& caption = QString(),
                        const QString& text = QString(), bool modal = false);

        /**
         * Destructor
         */
        ~KProgressDialog();

        /**
         * Returns the KProgressBar used in this dialog.
         * To set the number of steps or other progress bar related
         * settings, access the KProgressBar object directly via this method.
         */
        KProgressBar* progressBar();

        /**
         * Returns the KProgressBar used in this dialog.
         * To set the number of steps or other progress bar related
         * settings, access the KProgressBar object directly via this method.
         */
        const KProgressBar* progressBar() const;

        /**
         * Sets the text in the dialog
         *
         * @param text the text to display
         */
        void setLabel(const QString & text);

        /**
         * Returns the current dialog text
         */
        QString labelText() const;

        /**
         * Sets whether or not the user can cancel the process.
         * If the dialog is cancellable, the Cancel button will be shown
         * and the user can close the window using the window decorations.
         * If the process is not (or should not be) interuptable,
         * set the dialog to be modal and not cancellable.
         *
         * @param allowCancel Set to true to make the dialog non-closable
         */
        void setAllowCancel(bool allowCancel);

        /**
         * Returns true if the dialog can be canceled, false otherwise
         */
        bool allowCancel() const;

        /**
         * Sets whether the cancel button is visible. setAllowCancel(false)
         * implies showCancelButton(false)
         *
         * @param show Whether or not the cancel button should be shown
         */
        void showCancelButton(bool show);

        /**
         * Sets whether the dialog should close automagically when
         * all the steps in the KProgressBar have been completed.
         */
        void setAutoClose(bool close);

        /**
         * Returns true if the dialog will close upon completion,
         * or false otherwise
         */
        bool autoClose() const;

        /**
         * Sets whether the dialog should reset the KProgressBar dialog
         * back to 0 steps compelete when all steps have been completed.
         * This is useful for KProgressDialogs that will be reused.
         */
        void setAutoReset(bool autoReset);

        /**
         * Returns true if the KProgressBar widget will be reset
         * upon completion, or false otherwise
         */
        bool autoReset() const;

        /**
         * Returns true if the dialog was closed or canceled
         * before completion. If the dialog is not cancellable
         * it will always return false.
         */
        bool wasCancelled() const;

        /**
         * Sets the text to appear on the cancel button.
         */
        void setButtonText(const QString&);

        /**
         * Returns the text on the cancel button
         */
        QString buttonText() const;

        /**
         * Set the minimum number of milliseconds to wait before
         * actually showing the dialog
         */
        void setMinimumDuration(int ms);

        /**
         * Returns the wait duration in milliseconds
         */
        int  minimumDuration() const;

        virtual void reject();

    protected Q_SLOTS:
        void slotAutoShow();
        void slotAutoActions(int percentage);

    protected:
        virtual void virtual_hook( int id, void* data );

    private:
        struct KProgressDialogPrivate;
        KProgressDialogPrivate *d;
};

#endif
