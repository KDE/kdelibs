/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

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
#ifndef __kio_uiserver_h__
#define __kio_uiserver_h__

#include <qintdict.h>
#include <qdatetime.h>
#include <dcopobject.h>
#include <kio/global.h>
#include <kurl.h>
#include <ktmainwindow.h>

#include <klistview.h>

#include "defaultprogress.h"

class QTimer;
class ListProgress;

namespace KIO {
  class Job;
};

/**
* One item in the @ref #ListProgress
* @internal
*/
class ProgressItem : public QObject, public QListViewItem {

  Q_OBJECT

public:
  ProgressItem( ListProgress* view, QListViewItem *after, QCString app_id, int job_id,
		bool showDefault = true );
  ~ProgressItem();

  QCString appId() { return m_sAppId; }
  int jobId() { return m_iJobId; }

  void setTotalSize( unsigned long bytes );
  void setTotalFiles( unsigned long files );
  void setTotalDirs( unsigned long dirs );

  void setProcessedSize( unsigned long size );
  void setProcessedFiles( unsigned long files );
  void setProcessedDirs( unsigned long dirs );

  void setPercent( unsigned long percent );
  void setSpeed( unsigned long bytes_per_second );

  void setCopying( const KURL& from, const KURL& to );
  void setMoving( const KURL& from, const KURL& to );
  void setDeleting( const KURL& url );
  void setCreatingDir( const KURL& dir );
  void setRenaming( const KURL& old_name, const KURL& new_name );

  void setCanResume( bool );

  unsigned long totalSize() { return m_iTotalSize; }
  unsigned long totalFiles() { return m_iTotalFiles; }
  unsigned long processedSize() { return m_iProcessedSize; }
  unsigned long processedFiles() { return m_iProcessedFiles; }
  unsigned long speed() { return m_iSpeed; }
  QTime remainingTime() { return m_remainingTime; }

public slots:
  void slotShowDefaultProgress();

protected slots:
  void slotCanceled();

signals:
  void jobCanceled( ProgressItem* );

protected:

  // ids that uniquely identify this progress item
  QCString m_sAppId;
  int m_iJobId;

  // parent listview
  ListProgress *listProgress;

  // associated default progress dialog
  DefaultProgress *defaultProgress;

  // we store these values for calculation of totals ( for statusbar )
  unsigned long m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iProcessedSize;
  unsigned long m_iProcessedFiles;
  unsigned long m_iSpeed;
  QTime m_remainingTime;
};


/**
* List view in the @ref #UIServer.
* @internal
*/
class ListProgress : public KListView {

  Q_OBJECT

public:

  ListProgress (QWidget *parent = 0, const char *name = 0 );

  virtual ~ListProgress();

  /**
   * Field constants
   */
  enum ListProgressFields {
    TB_OPERATION = 0,
    TB_LOCAL_FILENAME = 1,
    TB_RESUME = 2,
    TB_COUNT = 3,
    TB_PROGRESS = 4,
    TB_TOTAL = 5,
    TB_SPEED = 6,
    TB_REMAINING_TIME = 7,
    TB_ADDRESS = 8
  };

  friend ProgressItem;

protected:

  void readConfig();
  void writeConfig();

  // ListView IDs
  int lv_operation, lv_filename, lv_resume, lv_count, lv_progress;
  int lv_total, lv_speed, lv_remaining, lv_url;
};


/**
 * It's purpose is to show progress of IO operations.
 * There is only one instance of this window for all jobs.
 *
 * All IO operations ( jobs ) are displayed in this window, one line per operation.
 * User can cancel operations with Cancel button on toolbar.
 *
 * Double clicking an item in the list opens a small download window ( @ref #DefaultProgress ).
 *
 * @short Graphical server for progress information with an optional all-in-one progress window.
 * @author David Faure <faure@kde.org>
 * @author Matej Koss <koss@miesto.sk>
 */
class UIServer : public KTMainWindow, public DCOPObject {

  K_DCOP
  Q_OBJECT

public:

  UIServer();
  virtual ~UIServer();

k_dcop:
  /**
   * Signal a new job
   * @param the DCOP application id of the job's parent application
   *   (@see KIO::Observer::newJob)
   * @return the job id
   */
  int newJob( QCString appId );

  void jobFinished( int id );

  void totalSize( int id, unsigned long size );
  void totalFiles( int id, unsigned long files );
  void totalDirs( int id, unsigned long dirs );

  void processedSize( int id, unsigned long bytes );
  void processedFiles( int id, unsigned long files );
  void processedDirs( int id, unsigned long dirs );

  void percent( int id, unsigned long ipercent );
  void speed( int id, unsigned long bytes_per_second );

  void copying( int id, KURL from, KURL to );
  void moving( int id, KURL from, KURL to );
  void deleting( int id, KURL url );
  void renaming( int id, KURL old_name, KURL new_name );
  void creatingDir( int id, KURL dir );

  void canResume( int id, unsigned int can_resume );

protected slots:

  void slotUpdate();

  void cancelCurrent();

  void slotDefaultProgress( QListViewItem * );
  void slotSelection();

  void slotJobCanceled( ProgressItem * );

protected:

  ProgressItem* findItem( int id );

  void closeEvent( QCloseEvent * );

  QTimer* updateTimer;
  ListProgress* listProgress;

  KToolBar::BarPosition toolbarPos;
  QString properties;

  void readSettings();
  void writeSettings();

private:

  void killJob( QCString observerAppId, int progressId );

  bool m_bShowList;

  static int s_jobId;
};

// -*- mode: c++; c-basic-offset: 2 -*-
#endif
