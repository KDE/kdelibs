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
#include <dcopobject.h>
#include <kio/global.h>
#include <kurl.h>
#include <ktmainwindow.h>

#include <klistview.h>

#include "defaultprogress.h"

class QTimer;
class ProgressListView;

namespace KIO {
  class Job;
};

/**
* One item in the @ref #ProgressListView
* @internal
*/
class ProgressItem : public QObject, public QListViewItem {

  Q_OBJECT

public:
  ProgressItem( ProgressListView* view, QListViewItem *after, QCString app_id, int job_id );
  ~ProgressItem();

  QCString appId() { return m_sAppId; }
  int jobId() { return m_iJobId; }

  void showDefaultProgress();

  void remove() { //m_pJob->kill();
  }

  void totalSize( unsigned long bytes );
  void totalFiles( unsigned long files );
  void totalDirs( unsigned long dirs );

  void processedSize( unsigned long size );
  void processedFiles( unsigned long files );
  void processedDirs( unsigned long dirs );

  void percent( unsigned long bytes );
  void speed( unsigned long bytes_per_second );

  void copying( const KURL& from, const KURL& to );
  void moving( const KURL& from, const KURL& to );
  void deleting( const KURL& url );
  void creatingDir( const KURL& dir );
  void renaming( const KURL& old_name, const KURL& new_name );

  void canResume( bool );

protected slots:
  void slotCanceled();

signals:
  void jobCanceled( ProgressItem* );

protected:
  int m_iJobId;
  QCString m_sAppId;

  DefaultProgress *defaultProgress;

  unsigned long m_iTotalSize;
  unsigned int m_iTotalFiles;
  unsigned int m_iTotalDirs;

  ProgressListView *listView;
};


/**
* List view in the @ref #UIServer.
* @internal
*/
class ProgressListView : public KListView {

  Q_OBJECT

public:

  ProgressListView (QWidget *parent = 0, const char *name = 0 );

  virtual ~ProgressListView();

  /**
   * Field constants
   */
  enum ProgressListViewFields {
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
 * The main advantage is, that all downloads, deletions etc. are shown in one place,
 * thus saving precious desktop space.
 *
 * All IO operations ( jobs ) are displayed in this window, one line per operation.
 * User can cancel operations with Cancel button on statusbar.
 *
 * Double clicking on an item in the list opens a small download window ( @ref #DefaultProgress ).
 *
 * @short Graphical server for progress information with all-in-one progress window.
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
  ProgressListView *myListView;

  KToolBar::BarPosition toolbarPos;
  QString properties;

  void readSettings();
  void writeSettings();

private:

  // Matt: call this when the user presses cancel
  // I suggest that the dialog stores both the appid and the
  // job's progress ID. If that's not possible, then I'll
  // put some dicts here... Well we need a dict id<->dialog box probably, anyway.
  void killJob( QCString observerAppId, int progressId );

  static int s_jobId;
};

// -*- mode: c++; c-basic-offset: 2 -*-
#endif
