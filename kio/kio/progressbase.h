/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __progressbase_h__
#define __progressbase_h__


#include <qwidget.h>

#include <kio/global.h>

class KURL;
namespace KIO {
  class Job;
  class CopyJob;
  class DeleteJob;
}

namespace KIO
{
  enum Progress {
    DEFAULT = 1,
    STATUSBAR = 2,
    LIST = 3
  };

/**
* This class does all initialization stuff for progress,
* like connecting signals to slots.
* All slots are implemented as pure virtual methods.
*
* All custom IO progress dialog should inherit this class.
* Add your GUI code to the constructor and implemement those virtual
* methods which you need in order to display progress.
*
* E.g. @ref #StatusbarProgress only implements @ref #slotTotalSize(),
* @ref #slotPercent() and @ref #slotSpeed().
*
* Custom progress dialog will be used like this :
* <pre>
* // create job
* CopyJob* job = KIO::copy(...);
* // create a dialog
* MyCustomProgress *customProgress;
* customProgress = new MyCustomProgress();
* // connect progress with job
* customProgress->setJob( job );
* ...
* </pre>
*
* There is a special method @ref #setStopOnClose() that controls the behavior of
* the dialog.
* @short Base class for IO progress dialogs.
* @author Matej Koss <koss@miesto.sk>
*/
class ProgressBase : public QWidget {

  Q_OBJECT

public:

  ProgressBase( QWidget *parent );
  ~ProgressBase() {}

  // assign job to this progress dialog
  void setJob( KIO::Job *job );
  void setJob( KIO::CopyJob *job );
  void setJob( KIO::DeleteJob *job );

  // should we stop the job when the dialog is closed ?
  void setStopOnClose( bool stopOnClose ) { m_bStopOnClose = stopOnClose; }
  bool stopOnClose() const { return m_bStopOnClose; }

  /**
   * This controls whether the dialog should be deleted or only cleaned when
   * the KIO::Job is finished (or canceled).
   *
   * If your dialog is an embedded widget and not a separate window, you should
   * setOnlyClean(true) in the constructor of your custom dialog.
   *
   * If true - Dialog will only call method @ref slotClean.
   * If false - Dialog will be deleted.
   */
  // should we delete the dialog or just clean it when the job is finished ?
  void setOnlyClean( bool onlyClean ) { m_bOnlyClean = onlyClean; }
  bool onlyClean() const { return m_bOnlyClean; }

  void finished();

public slots:
  /**
   * This method should be called for correct cancelation of IO operation
   * Connect this to the progress widgets buttons etc.
   */
  void slotStop();
  /**
   * This method is called when the widget should be cleaned (after job is finished).
   * redefine this for custom behavior.
   */
  virtual void slotClean();

  // progress slots
  virtual void slotTotalSize( KIO::Job*, KIO::filesize_t ) {}
  virtual void slotTotalFiles( KIO::Job*, unsigned long ) {}
  virtual void slotTotalDirs( KIO::Job*, unsigned long ) {}

  virtual void slotProcessedSize( KIO::Job*, KIO::filesize_t ) {}
  virtual void slotProcessedFiles( KIO::Job*, unsigned long ) {}
  virtual void slotProcessedDirs( KIO::Job*, unsigned long ) {}

  virtual void slotSpeed( KIO::Job*, unsigned long ) {}
  virtual void slotPercent( KIO::Job*, unsigned long ) {}

  virtual void slotCopying( KIO::Job*, const KURL&, const KURL& ) {}
  virtual void slotMoving( KIO::Job*, const KURL&, const KURL& ) {}
  virtual void slotDeleting( KIO::Job*, const KURL& ) {}
  virtual void slotCreatingDir( KIO::Job*, const KURL& ) {}

  virtual void slotCanResume( KIO::Job*, KIO::filesize_t ) {}

signals:
  void stopped();

protected slots:
  void slotFinished( KIO::Job* );

protected:

  virtual void closeEvent( QCloseEvent * );

  KIO::Job* m_pJob;

private:
  bool m_bOnlyClean;
  bool m_bStopOnClose;


protected:
    virtual void virtual_hook( int id, void* data );
private:
    class ProgressBasePrivate* d;
};

} /* namespace */

#endif // __progressbase_h__
