/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#ifndef __kbookmark_h__
#define __kbookmark_h__

#include <qlist.h>
#include <qstring.h>
#include <qpixmap.h>

#include <ksimpleconfig.h>

class KBookmarkManager;
class KBookmark;

////// KBookmarkOwner and KBookmarkMenu not here because they rely on openparts.
////// If you want openparts menus, take the code from konqueror
////// Otherwise, we could re-add here the standard menus
////// Or create an abstract class, that has to be reimplemented for use
////// with qpopupmenu or opMenu. Depends on people's needs. (David).

/**
 * Internal Class
 */
class KBookmark
{
  friend KBookmarkManager;
  
public:
  enum { URL, Folder };

  /**
   * Creates a real bookmark ( type = Folder ) and saves the bookmark on the disk.
   */
  KBookmark( KBookmarkManager *, KBookmark *_parent, const char *_text, const char *_url );
  
  const char *text() { return m_text; }
  const char *url() { return m_url; }
  int type() { return m_type; }
  int id() { return m_id; }
  const char* file() { return m_file; }
  QPixmap* pixmap( bool _mini );
  
  void append( KBookmark *_bm ) { m_lstChildren.append( _bm ); }
  
  QList<KBookmark> *children() { return &m_lstChildren; }
  
  KBookmark* findBookmark( int _id );
 
  static QString encode( const char* );
  static QString decode( const char* );
  
protected:
  /**
   * Creates a folder.
   */
  KBookmark( KBookmarkManager *, KBookmark *_parent, const char *_text );
  /**
   * Creates a bookmark from a file.
   */
  KBookmark( KBookmarkManager *, KBookmark *_parent, const char *_text,
             KSimpleConfig& _cfg, const char * _group );

  void clear();
  
  QString m_text;
  QString m_url;
  QString m_file;
  
  QPixmap* m_pPixmap;
  QPixmap* m_pMiniPixmap;
  
  int m_type;
  int m_id;
  
  QList<KBookmark> m_lstChildren;

  KBookmarkManager *m_pManager;
};

/**
 * This class holds the data structures for the bookmarks.
 * You must create one instance of this class on startup.
 * In addition you have to overload @ref editBookmarks.
 */
class KBookmarkManager : public QObject
{
  friend KBookmark;
  
  Q_OBJECT
public:
  KBookmarkManager();
  ~KBookmarkManager() {};

  /**
   * For internal use only
   */
  KBookmark* root() { return &m_Root; }
  /**
   * For internal use only
   */
  KBookmark* findBookmark( int _id ) { return m_Root.findBookmark( _id ); }

  /**
   * For internal use only
   */
  void emitChanged();

  /**
   * Called if the user wants to edit the bookmarks. You must somehow open a filemanager
   * and display the given URL.
   */
  virtual void editBookmarks( const char *_url ) = 0L;
  
  /**
   * @return a pointer to the instance of the KBookmarkManager. Make shure that
   *         an instance of this class or a derived one exists before you call this
   *         function. It wont return 0L in this case, but it will raise an error.
   */
  static KBookmarkManager* self();
  
signals:
  /**
   * For internal use only
   */
  void changed();

public slots:
  /**
   * If you know that something in the bookmarks directory tree changed, call this
   * function to update the bookmark menus. If the given URL is not of interest for
   * the bookmarks then nothing will happen.
   */
  void slotNotify( const char* _url );
  /**
   * For internal use only
   */
  void slotEditBookmarks();
  
protected:
  void scan( const char *filename );
  void scanIntern( KBookmark*, const char *filename );

  void disableNotify() { m_bNotify = false; }
  void enableNotify() { m_bNotify = true; }
    
  KBookmark m_Root;
  bool m_bAllowSignalChanged;
  bool m_bNotify;

  /**
   * This list is to prevent infinite looping while
   * scanning directories with ugly symbolic links
   */
  QList<QString> m_lstParsedDirs;

  static KBookmarkManager* s_pSelf;
};

#endif

