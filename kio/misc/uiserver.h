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
#include <qtimer.h>

#include <dcopobject.h>
#include <kio/global.h>
#include <kio/authinfo.h>
#include <kurl.h>
#include <kmainwindow.h>
#include <kdatastream.h>
#include <klistview.h>
#include <ksslcertdlg.h>

class ListProgress;
class KSqueezedTextLabel;
class ProgressItem;

namespace KIO {
  class Job;
  class DefaultProgress;
};


struct ListProgressColumnConfig
{
   QString title;
   int index;
   int width;
   bool enabled;
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
    TB_COUNT = 3,     //lv_count
    TB_PROGRESS = 4,  // lv_progress
    TB_TOTAL = 5,
    TB_SPEED = 6,
    TB_REMAINING_TIME = 7,
    TB_ADDRESS = 8,
    TB_MAX = 9
  };

  friend class ProgressItem;

protected slots:
  void columnWidthChanged(int column);
protected:

  void writeConfig();
  void readConfig();
  void createColumns();

  bool m_showHeader;
  bool m_fixedColumnWidths;
  ListProgressColumnConfig m_lpcc[TB_MAX];
  //hack, alexxx
  KSqueezedTextLabel *m_squeezer;
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
  void setDefaultProgressVisible( bool visible );
  bool isVisible() const { return m_visible; }

  void setTotalSize( KIO::filesize_t bytes );
  void setTotalFiles( unsigned long files );
  void setTotalDirs( unsigned long dirs );

  void setProcessedSize( KIO::filesize_t size );
  void setProcessedFiles( unsigned long files );
  void setProcessedDirs( unsigned long dirs );

  void setPercent( unsigned long percent );
  void setSpeed( unsigned long bytes_per_second );
  void setInfoMessage( const QString & msg );

  void setCopying( const KURL& from, const KURL& to );
  void setMoving( const KURL& from, const KURL& to );
  void setDeleting( const KURL& url );
  void setTransferring( const KURL& url );
  void setCreatingDir( const KURL& dir );
  void setStating( const KURL& url );
  void setMounting( const QString & dev, const QString & point );
  void setUnmounting( const QString & point );

  void setCanResume( KIO::filesize_t offset );

  KIO::filesize_t totalSize() { return m_iTotalSize; }
  unsigned long totalFiles() { return m_iTotalFiles; }
  KIO::filesize_t processedSize() { return m_iProcessedSize; }
  unsigned long processedFiles() { return m_iProcessedFiles; }
  unsigned long speed() { return m_iSpeed; }
  QTime remainingTime() { return m_remainingTime; }

  const QString& fullLengthAddress() const {return m_fullLengthAddress;}
  void setText(ListProgress::ListProgressFields field, const QString& text);
public slots:
  void slotShowDefaultProgress();
  void slotToggleDefaultProgress();

protected slots:
  void slotCanceled();

signals:
  void jobCanceled( ProgressItem* );

protected:
  void updateVisibility();

  // ids that uniquely identify this progress item
  QCString m_sAppId;
  int m_iJobId;

  // whether shown or not (it is hidden if a rename dialog pops up for the same job)
  bool m_visible;
  bool m_defaultProgressVisible;

  // parent listview
  ListProgress *listProgress;

  // associated default progress dialog
  KIO::DefaultProgress *defaultProgress;

  // we store these values for calculation of totals ( for statusbar )
  KIO::filesize_t m_iTotalSize;
  unsigned long m_iTotalFiles;
  KIO::filesize_t m_iProcessedSize;
  unsigned long m_iProcessedFiles;
  unsigned long m_iSpeed;
  QTime m_remainingTime;
  QTimer m_showTimer;
  QString m_fullLengthAddress;
};

class QResizeEvent;

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
class UIServer : public KMainWindow, public DCOPObject {

  K_DCOP
  Q_OBJECT

public:

  UIServer();
  virtual ~UIServer();

k_dcop:

  /**
   * Signal a new job
   * @param the DCOP application id of the job's parent application
   * @see KIO::Observer::newJob
   * @param showProgress whether to popup the progress for the job.
   *   Usually true, but may be false when we use kio_uiserver for
   *   other things, like SSL dialogs.
   * @return the job id
   */
  int newJob( QCString appId, bool showProgress );

  ASYNC jobFinished( int id );

  ASYNC totalSize( int id, unsigned long size );
  ASYNC totalSize64( int id, KIO::filesize_t size );
  ASYNC totalFiles( int id, unsigned long files );
  ASYNC totalDirs( int id, unsigned long dirs );

  ASYNC processedSize( int id, unsigned long bytes );
  ASYNC processedSize64( int id, KIO::filesize_t bytes );
  ASYNC processedFiles( int id, unsigned long files );
  ASYNC processedDirs( int id, unsigned long dirs );

  ASYNC percent( int id, unsigned long ipercent );
  ASYNC speed( int id, unsigned long bytes_per_second );
  ASYNC infoMessage( int id, const QString & msg );

  ASYNC copying( int id, KURL from, KURL to );
  ASYNC moving( int id, KURL from, KURL to );
  ASYNC deleting( int id, KURL url );
  ASYNC transferring( int id, KURL url );
  ASYNC creatingDir( int id, KURL dir );
  ASYNC stating( int id, KURL url );

  ASYNC mounting( int id, QString dev, QString point );
  ASYNC unmounting( int id, QString point );

  ASYNC canResume( int id, unsigned long offset );
  ASYNC canResume64( int id, KIO::filesize_t offset );

  /**
   * @deprecated (it blocks other apps).
   * Use KIO::PasswordDialog::getNameAndPassword instead.
   * To be removed in KDE 4.0.
   */
  QByteArray openPassDlg( const KIO::AuthInfo &info );

  /**
   * Popup a message box.
   * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
   *   This enum is defined in slavebase.h, it currently is:
   *   QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3,
   *   WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6
   * @param text Message string. May contain newlines.
   * @param caption Message box title.
   * @param buttonYes The text for the first button.
   *                  The default is i18n("&Yes").
   * @param buttonNo  The text for the second button.
   *                  The default is i18n("&No").
   * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
   *       and for Information, none is used.
   * @return a button code, as defined in KMessageBox, or 0 on communication error.
   */
  int messageBox( int id, int type, const QString &text, const QString &caption,
                  const QString &buttonYes, const QString &buttonNo );

  /**
   * @deprecated (it blocks other apps).
   * Use KIO::open_RenameDlg instead.
   * To be removed in KDE 4.0.
   */
  QByteArray open_RenameDlg64( int id,
                             const QString & caption,
                             const QString& src, const QString & dest,
                             int /* KIO::RenameDlg_Mode */ mode,
                             KIO::filesize_t sizeSrc,
                             KIO::filesize_t sizeDest,
                             unsigned long /* time_t */ ctimeSrc,
                             unsigned long /* time_t */ ctimeDest,
                             unsigned long /* time_t */ mtimeSrc,
                             unsigned long /* time_t */ mtimeDest
                             );
  /**
   * @deprecated (it blocks other apps).
   * Use KIO::open_RenameDlg instead.
   * To be removed in KDE 4.0.
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
   * @deprecated (it blocks other apps).
   * Use KIO::open_SkipDlg instead.
   * To be removed in KDE 4.0.
   */
  int open_SkipDlg( int id,
                    int /*bool*/ multi,
                    const QString & error_text );

  /**
   * Switch to or from list mode - called by the kcontrol module
   */
  void setListMode( bool list );

  /**
   * Hide or show a job. Typically, we hide a job while a "skip" or "rename" dialog
   * is being shown for this job. This prevents killing it from the uiserver.
   */
  void setJobVisible( int id, bool visible );

  /**
   * Show a SSL Information Dialog
   */
  void showSSLInfoDialog(const QString &url, const KIO::MetaData &data);

  /*
   * Show an SSL Certificate Selection Dialog
   */
  KSSLCertDlgRet showSSLCertDialog(const QString& host, const QStringList& certList);

protected slots:

  void slotUpdate();

  void cancelCurrent();

  void slotToggleDefaultProgress( QListViewItem * );
  void slotSelection();

  void slotJobCanceled( ProgressItem * );

protected:

  ProgressItem* findItem( int id );

  virtual void resizeEvent(QResizeEvent* e);
  virtual void closeEvent( QCloseEvent * );

  void setItemVisible( ProgressItem * item, bool visible );

  QTimer* updateTimer;
  ListProgress* listProgress;

  KToolBar::BarPosition toolbarPos;
  QString properties;

  void readSettings();
  void writeSettings();
private:

  void killJob( QCString observerAppId, int progressId );

  int m_initWidth;
  int m_initHeight;
  bool m_bShowList;
  bool m_showStatusBar;
  bool m_showToolBar;
  bool m_keepListOpen;

  // true if there's a new job that hasn't been shown yet.
  bool m_bUpdateNewJob;

  static int s_jobId;
};

// -*- mode: c++; c-basic-offset: 2 -*-
#endif
