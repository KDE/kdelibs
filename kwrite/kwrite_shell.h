/*
    $Id$

    Copyright (C) 2000 Michael Koch <koch@kde.org>

    This library is free software; you can redistribute it and/or
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

#ifndef __kwrite_part_h__
#define __kwrite_part_h__

#include <kparts/mainwindow.h>

#include <kwrite/kwrite_part.h>

#define KWRITE_VERSION "1.9.0"

class KHelpMenu;
class KStatusBar;
class KListAction;
class KToggleAction;
class KSelectAction;
class KRecentFilesAction;

class KWriteShell : public KParts::MainWindow
{
  Q_OBJECT

public:

  KWriteShell( const KURL& = KURL() );
  virtual ~KWriteShell();

  void setupActions();

public slots:

  virtual void openURL( const KURL& );

  virtual void slotFileNew();
  virtual void slotFileOpen();
  virtual void slotFileOpenRecent( int );
  virtual void slotFileSave();
  virtual void slotFileSaveAs();
  virtual void slotFileRevert();
  virtual void slotFilePrint();
  virtual void slotFileNewView();
  virtual void slotFileNewWindow();

  virtual void slotShowToolbar();
  virtual void slotShowStatusbar();
  virtual void slotShowPath();
  virtual void slotConfigureToolbars();

  virtual void saveOptions();
  virtual void restoreOptions();

  virtual void slotHelpAboutKWrite();

  virtual void newCursorPos( const QPoint& );
  virtual void setCaption( const QString& caption = QString::null );
  virtual void statusMsg( const QString& text = QString::null );

protected:
  virtual bool queryClose();

private:

  KRecentFilesAction *m_recent;
  //KListAction   *m_recentFiles;
  KSelectAction *m_eolAction;
  KToggleAction *m_vertical;
  KToggleAction *m_toolbar;
  KToggleAction *m_statusbar;
  KToggleAction *m_path;

  KStatusBar    *m_pStatusBar;
  KWritePart    *m_part;
  KHelpMenu     *m_helpMenu;
};

#endif // __kwrite_part_h__
