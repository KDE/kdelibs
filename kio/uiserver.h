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
#include <kdatastream.h>

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

  void setVisible( bool visible );
  bool isVisible() const { return m_visible; }

  void setTotalSize( unsigned long bytes );
  void setTotalFiles( unsigned long files );
  void setTotalDirs( unsigned long dirs );

  void setProcessedSize( unsigned long size );
  void setProcessedFiles( unsigned long files );
  void setProcessedDirs( unsigned long dirs );

  void setPercent( unsigned long percent );
  void setSpeed( unsigned long bytes_per_second );
  void setInfoMessage( const QString & msg );

  void setCopying( const KURL& from, const KURL& to );
  void setMoving( const KURL& from, const KURL& to );
  void setDeleting( const KURL& url );
  void setCreatingDir( const KURL& dir );
  void setStating( const KURL& url );
  void setMounting( const QString & dev, const QString & point );
  void setUnmounting( const QString & point );

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

  // whether shown or not (it is hidden if a rename dialog pops up for the same job)
  bool m_visible;

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
 * @internal
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
  void infoMessage( int id, const QString & msg );

  void copying( int id, KURL from, KURL to );
  void moving( int id, KURL from, KURL to );
  void deleting( int id, KURL url );
  void creatingDir( int id, KURL dir );
  void stating( int id, KURL url );

  void mounting( int id, QString dev, QString point );
  void unmounting( int id, QString point );

  // currently unused
  void canResume( int id, unsigned int can_resume );

  /**
   * Invoke this method to request autorization info from the user
   * or query the password daemon to see if one is stored.
   *
   * @param user  the username to be authenticated
   * @param head  the resource the requires authorization
   * @param key   the key used to cache the password.
   *
   * @return serialized autorization info: (bool authorized, QString user, QString password)
   */
  QByteArray authorize( const QString& /*user*/, const QString& /*head*/, const QString& /*key*/ );

  /**
   * See renamedlg.h
   * @return serialized answer: (RenameDlg_Result result, QString newDest)
   */
  QByteArray open_RenameDlg( int id,
                             const QString & caption,
                             const QString& src, const QString & dest,
                             int /* KIO::RenameDlg_Mode */ mode,
                             unsigned long sizeSrc,
                             unsigned long sizeDest,
                             unsigned long /* time_t */ ctimeSrc,
                             unsigned long /* time_t */ ctimeDest,
                             unsigned long /* time_t */ mtimeSrc,
                             unsigned long /* time_t */ mtimeDest
                             );

  /**
   * See skiplg.h
   */
  int open_SkipDlg( int id,
                    int /*bool*/ multi,
                    const QString & error_text );

  /**
   * Switch to or from list mode - called by the kcontrol module
   */
  void setListMode( bool list );

protected slots:

  void slotUpdate();

  void cancelCurrent();

  void slotDefaultProgress( QListViewItem * );
  void slotSelection();

  void slotJobCanceled( ProgressItem * );

protected:

  ProgressItem* findItem( int id );

  void closeEvent( QCloseEvent * );

  void setItemVisible( ProgressItem * item, bool visible );

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
