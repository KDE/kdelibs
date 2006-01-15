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
*  KProgressBar -- progress indicator widget for KDE                         *
*  Original QRangeControl-based version written by Martynas Kunigelis        *
*  QProgressBar based version by Aaron Seigo                                 *
*  Current Qt4 QProgressBar based version by Urs Wolfer                      *
*                                                                            *
*****************************************************************************/

#ifndef KPROGRESSBAR_H
#define KPROGRESSBAR_H

#include <QProgressBar>

#include <kdialog.h>

/**
 * @short A progress indicator widget.
 *
 * KProgressBar is derived from QProgressBar, so you can use all the
 * methods from that class. The only real difference is that you can
 * change the format of the descriptive text shown on the progress bar.
 *
 * \b Details \n
 *
 * \image html kprogressbar.png "KDE Progress Widget"
 *
 * @author Aaron Seigo
 * @author Urs Wolfer uwolfer @ fwo.ch
 */
class KDEUI_EXPORT KProgressBar : public QProgressBar
{
  Q_OBJECT

public:
  /**
   * Construct a progress bar.
   */
  KProgressBar(QWidget *parent=0);

  /**
   * Construct a progress bar with a total number of steps.
   * The totalSteps is the total number of steps that need to be completed for the operation which this progress
   * bar represents. For example, if the operation is to examine 50 files, this value would be 50. Before examining
   * the first file, call setProgress(0); call setProgress(50) after examining the last file.
   */
  KProgressBar(int totalSteps, QWidget *parent=0);

  /**
   * Destruct the progress bar.
   */
  ~KProgressBar();

  /**
   * Retrieve the current format for printing status text.
   * @see setFormat()
   */
  QString format() const;

public slots:

  /**
   * Set the format of the text to use to display status.
   *
   * The default format is "%p%" (which looks like "42%".)
   *
   * Note: Setting the format to anything other then "%p%" will force centerIndicator to true,
   * since it's often impossible to layout a progressbar with a more general format with the
   * indicator string anywhere else.
   *
   * @param format "%p" is replaced by percentage done, "%v" is replaced by actual
   * value, "%m" is replaced by the maximum value.
   */
  void setFormat(const QString & format);

  /**
   * Advance the progress bar by @p offset.
   *
   * This method is
   * provided for convenience and is equivalent with
   * setProgress(progress()+offset).
   */
  virtual void advance(int offset);

protected:
  virtual QString text() const;
  virtual void virtual_hook( int id, void* data );

private:
  struct KProgressBarPrivate;
  KProgressBarPrivate *d;
};

#endif
