/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

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

#ifndef __statusbarprogress_h__
#define __statusbarprogress_h__

#include "progressbase.h"

class QStackedWidget;
class QBoxLayout;
class QPushButton;
class QLabel;
class QProgressBar;

namespace KIO {

class Job;

/**
* This is a special IO progress widget.
*
* Similarly to DefaultProgress,
* it's purpose is to show a progress of the IO operation.
*
* Instead of creating a separate window, this is only a widget that can be
* easily embedded in a statusbar.
*
* Usage of StatusbarProgress is little different.
* This dialog will be a part of some application.
* \code
* // create a dialog
* StatusbarProgress *statusProgress;
* statusProgress = new StatusbarProgress( statusBar() );
* statusBar()->insertWidget( statusProgress, statusProgress->width() , 0 );
* ...
* // create job and connect it to the progress
* CopyJob* job = KIO::copy(...);
* statusProgress->setJob( job );
* ...
* \endcode
*
* @short IO progress widget for embedding in a statusbar.
* @author Matej Koss <koss@miesto.sk>
*/
class KIO_EXPORT StatusbarProgress : public ProgressBase {

  Q_OBJECT

public:

  /**
   * Creates a new StatusbarProgress.
   * @param parent the parent of this widget
   * @param button true to add an abort button. The button will be
   *               connected to ProgressBase::slotStop()
   */
  StatusbarProgress( QWidget* parent, bool button = true );
  ~StatusbarProgress() {}

  /**
   * Sets the job to monitor.
   * @param job the job to monitor
   */
  void setJob( KIO::Job *job );

public Q_SLOTS:
  virtual void slotClean();
  virtual void slotTotalSize( KIO::Job* job, KIO::filesize_t size );
  virtual void slotPercent( KIO::Job* job, unsigned long percent );
  virtual void slotSpeed( KIO::Job* job, unsigned long speed );

protected:
  QProgressBar* m_pProgressBar;
  QLabel* m_pLabel;
  QPushButton* m_pButton;

  KIO::filesize_t m_iTotalSize;

  enum Mode { None, Label, Progress };

  uint mode;
  bool m_bShowButton;

  void setMode();

  virtual bool eventFilter( QObject *, QEvent * );
  QBoxLayout *box;
  QStackedWidget *stack;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class StatusbarProgressPrivate* d;
};

} /* namespace */

#endif  //  __statusbarprogress_h__
