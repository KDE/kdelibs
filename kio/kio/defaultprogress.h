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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __defaultprogress_h__
#define __defaultprogress_h__

#include <qlabel.h>

#include <kio/global.h>

#include <kprogress.h>
#include <ksqueezedtextlabel.h>

#include "progressbase.h"

namespace KIO {

/*
 * @ref ProgressBase
 */
class DefaultProgress : public ProgressBase {

  Q_OBJECT

public:

  DefaultProgress( bool showNow = true );
  /**
   * @since 3.1
   */
  DefaultProgress( QWidget* parent, const char* name = 0 );
  ~DefaultProgress();

public slots:
  virtual void slotTotalSize( KIO::Job*, KIO::filesize_t bytes );
  virtual void slotTotalFiles( KIO::Job*, unsigned long files );
  virtual void slotTotalDirs( KIO::Job*, unsigned long dirs );

  virtual void slotProcessedSize( KIO::Job*, KIO::filesize_t bytes );
  virtual void slotProcessedFiles( KIO::Job*, unsigned long files );
  virtual void slotProcessedDirs( KIO::Job*, unsigned long dirs );

  virtual void slotSpeed( KIO::Job*, unsigned long bytes_per_second );
  virtual void slotPercent( KIO::Job*, unsigned long percent );
  virtual void slotInfoMessage( KIO::Job*, const QString & msg );

  virtual void slotCopying( KIO::Job*, const KURL& src, const KURL& dest );
  virtual void slotMoving( KIO::Job*, const KURL& src, const KURL& dest );
  virtual void slotDeleting( KIO::Job*, const KURL& url );
  /// @since 3.1
  void slotTransferring( KIO::Job*, const KURL& url );
  virtual void slotCreatingDir( KIO::Job*, const KURL& dir );
  virtual void slotStating( KIO::Job*, const KURL& dir );
  virtual void slotMounting( KIO::Job*, const QString & dev, const QString & point );
  virtual void slotUnmounting( KIO::Job*, const QString & point );
  virtual void slotCanResume( KIO::Job*, KIO::filesize_t );

  /// @since 3.1
  void slotClean();

protected:
  /// @since 3.1
  void init();
  void showTotals();
  void setDestVisible( bool visible );
  /// @since 3.1
  void checkDestination( const KURL& dest);

  KSqueezedTextLabel* sourceLabel;
  KSqueezedTextLabel* destLabel;
  QLabel* progressLabel;
  QLabel* destInvite;
  QLabel* speedLabel;
  QLabel* sizeLabel;
  QLabel* resumeLabel;

  KProgress* m_pProgressBar;

  KIO::filesize_t m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;

  KIO::filesize_t m_iProcessedSize;
  unsigned long m_iProcessedDirs;
  unsigned long m_iProcessedFiles;

  enum ModeType { Copy, Move, Delete, Create };
  ModeType mode;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class DefaultProgressPrivate;
  DefaultProgressPrivate* d;
private slots:
  void slotKeepOpenToggled(bool);
  void slotOpenFile();
  void slotOpenLocation();
};

} /* namespace */

#endif // __defaultprogress_h__

