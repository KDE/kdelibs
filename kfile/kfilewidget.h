/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#ifndef KFILEWIDGET_H
#define KFILEWIDGET_H

#include <qevent.h>
#include <qwidget.h>

#include "kdiroperator.h"
#include "kfilereader.h"
#include "kfileview.h"

class KFileWidget : public QWidget
{
  Q_OBJECT

public:
  enum FileView { Simple, Detail, SimpleCombi, DetailCombi };

  KFileWidget( FileView view, QWidget *parent=0, const char *name=0 );
  virtual ~KFileWidget();

  /**
   * Set the current url.
   * An url without a protocol (file:/, ftp://) will be treated as a
   * local directory and file:/ will be prepended.
   */
  virtual void setURL( const KURL & );
  KURL url() const;

  /**
   * Set the current name filter.
   */
  void setNameFilter(const QString&);
  void setShowHiddenFiles(bool);
  bool showHiddenFiles() const;

  bool isRoot() const;

  /**
   * Returns the number of directory entries read.
   */
  uint count() const;

  /**
   * Returns true if KFM has finished the operation.
   */
  //  bool isFinished() const;

  /**
   * Indicates if the path is readable. That means, if there are
   * entries to expect
   **/
  bool isReadable() const;

  void 		setView( FileView );

  void 		setView( KFileView * );

protected:
  void 		setDirOperator( KDirOperator * );
  KDirOperator *dirOperator() const { return myDirOperator; }

  virtual void	resizeEvent( QResizeEvent * );


private:
  void 		connectSignals();

  KDirOperator 	*myDirOperator;

public slots:
  void home();
  void cdUp();
  void back();
  void forward();
  void rereadDir();


protected slots:
  //  void 		slotFinished();


signals:

  /**
   * Emitted when the user highlights a file.
   */
  void fileHighlighted(const KFileViewItem *);

  /**
   * Emitted when the user selects a file.
   */
  void fileSelected(const KFileViewItem *);

  /**
   * Emitted, when the user selectes a directory
   */
  void dirSelected(const KFileViewItem *);

  /**
   * Emitted when the user enters a directory/url
   */
  void urlEntered( const KURL& );

  /**
   * Emitted when the directory has been completely loaded.
   */
  //  void finished();

  /**
   * Emitted if a network transfer goes wrong.
   */
  void error(int, const QString&);

};


#endif // KFILEWIDGET_H
