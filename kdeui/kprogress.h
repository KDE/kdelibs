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
/*****************************************************************************
*                                                                            *
*  KProgress -- progress indicator widget for KDE                            *
*  Original QRangeControl-based version written by Martynas Kunigelis        *
*  Current QProgressBar based version by Aaron Seigo                         *
*                                                                            *
*****************************************************************************/

#ifndef _KPROGRESS_H
#define _KPROGRESS_H "$Id$"

#include <qprogressbar.h>
#include <kdialogbase.h>

/**
 * A stylized progress bar.
 *
 * KProgress is derived from @ref QProgressBar, so
 * you can use all the methods from that class. The only real difference
 * is that a signal is emitted on changes to the value and you do not need
 * to subclass KProgress just to change the format of the indicator text. 
 *
 * @sect Details
 *
 * @short A progress indicator widget.
 * @author Aaron Seigo
 * @version $Id$
 */
class KProgress : public QProgressBar
{
  Q_OBJECT

public:
  /**
   * Construct a progress bar.
   */
  KProgress(QWidget *parent=0, const char *name=0, WFlags f = 0);

  /**
   * Construct a progress bar with a total number of steps.
   * The totalSteps is the total number of steps that need to be completed for the operation which this progress
   * bar represents. For example, if the operation is to examine 50 files, this value would be 50. Before examining
   * the first file, call setProgress(0); call setProgress(50) after examining the last file.
   */
  KProgress(int totalSteps, QWidget *parent=0, const char *name=0, WFlags f = 0);

  /**
   * Destruct the progress bar.
   */
  ~KProgress();

  /**
   * If this is set to @p true, the progress text will be displayed.
   *
   */
  void setTextEnabled(bool);

  /**
   * @deprecated Retrieves the current status, use progress() instead
   *
   * @see setValue()
   */
  int value() const;

  /**
   * Returns @p true if progress text will be displayed,
   * @p false otherwise.
   *
   * @see setFormat()
   */
  bool textEnabled() const;

  /**
   * Retrieve the current format for printing status text.
   * @see setFormat()
   */
  QString format() const;

  /**
   * @deprecated but kept for source compatibility with KDE2's KProgress.
   * Use @ref setTotalSteps() instead
   */
  void setRange(int min, int max);

  /**
   * @deprecated used @ref totalSteps() instead
   */
  int maxValue();

public slots:

  /**
   * Set the format of the text to use to display status.
   *
   * The default format is "%p%" (which looks like "42%".)
   *
   * @param format %p is replaced by percentage done, %v is replaced by actual
   * value, %m is replaced by the maximum value.
   */
  void setFormat(const QString & format);

  /**
    * Set the current total number of steps in the action tat the progress bar
    * is representing.
    */
  void setTotalSteps(int totalSteps);

  /**
    * Set the current value of the progress bar to @p value.
    * @reimplemented
    */
  virtual void setProgress(int progress);

  /**
    * @deprecated. Use @ref setProgress(int) instead
    */
  void setValue(int progress);

  /**
   * Advance the progress bar by @p prog.
   *
   * This method is
   * provided for convenience and is equivalent with
   * @ref setValue(value()+prog).
   */
  virtual void advance(int offset);

signals:
  /**
   * Emitted when the state of the progress bar changes.
   */
  void percentageChanged(int);

protected:
  /**
   * @reimplemented
   */
  virtual bool setIndicator(QString & indicator, int progress, int totalSteps);

private:
  QString   mFormat;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KProgressPrivate;
  KProgressPrivate *d;
};

/**
 * KProgressDialog provides a dialog with a text label, a progress bar
 * and an optional cancel button with a KDE look 'n feel.
 *
 * Since knowing how long it can take to complete an action and it is
 * undesireable to show a dialog for a split second before hiding it,
 * there are a few ways to control the timing behavior of KProgressDialog.
 * There is a time out that can be set before showing the dialog as well
 * as an option to autohide or keep displaying the dialog once complete.
 *
 * All the functionality of KProgress is available through direct access
 * to the progress bar widget via progressBar();
 *
 * @short A dialog with a progress bar
 * @author Aaron J. Seigo
 */
class KProgressDialog : public KDialogBase
{
    Q_OBJECT

    public:
        /**
         * Constructs a KProgressDialog
         *
         * @param parent Parent of the widget
         * @param name Widget name
         * @param caption Text to display in window title bar
         * @param text Text to display in the dialog
         * @param modal Set to true to make the dialog modal
         */
        KProgressDialog(QWidget* parent = 0, const char* name = 0,
                        const QString& caption = QString::null,
                        const QString& text = QString::null,
                        bool modal = false);

        /**
         * Desctructor
         */
        ~KProgressDialog();

        /**
         * Returns the @ref KProgress used in this dialog. 
         * To set the number of steps or other progress bar related
         * settings, access the KProgress object directly via this method.
         */
        KProgress* progressBar();

        /**
         * Sets the text in the dialog
         *
         * @param text the text to display
         */
        void    setLabel(const QString&);
        
        /**
         * Returns the current dialog text
         */
        QString labelText();

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
         * Returns true if the dialog can be cancelled, false otherwise
         */
        bool allowCancel();

        /**
         * Sets whether the cancel button is visible. @ref setAllowCancel(false)
         * implies showCancelButton(false)
         *
         * @param show Whether or not the cancel button should be shown
         */
        void showCancelButton(bool show);

        /**
         * Sets whether the dialog should close automagically when
         * all the steps in the KProgress have been completed.
         */
        void setAutoClose(bool close);

        /**
         * Returns true if the dialog will close upon completion,
         * or false otherwise
         */
        bool autoClose();

        /**
         * Sets whether the dialog should reset the KProgress dialog
         * back to 0 steps compelete when all steps have been completed.
         * This is useful for KProgressDialogs that will be reused.
         */
        void setAutoReset(bool autoReset);

        /**
         * Returns true if the KProgress widget will be reset
         * upon completion, or false otherwise
         */
        bool autoReset();

        /**
         * Returns true if the dialog was closed or cancelled
         * before completion. If the dialog is not cancellable
         * it will always return false.
         */
        bool wasCancelled();

        /**
         * Sets the text to appear on the cancel button.
         */
        void setButtonText(const QString&);

        /**
         * Returns the text on the cancel button
         */
        QString buttonText();

        /** 
         * Set the minimum number of milliseconds to wait before
         * actually showing the dialog
         */
        void setMinimumDuration(int ms);

        /**
         * Returns the wait duration in milliseconds
         */
        int  minimumDuration();

    protected slots:
        void slotAutoShow();
        void slotAutoActions(int percentage);
        void slotCancel();

    private:
        bool       mAutoClose;
        bool       mAutoReset;
        bool       mCancelled;
        bool       mAllowCancel;
        bool       mShown;
        QString    mCancelText;
        QLabel*    mLabel;
        KProgress* mProgressBar;
        QTimer*    mShowTimer;
        int        mMinDuration;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class KProgressDialogPrivate;
        KProgressDialogPrivate *d;
};

#endif
