// -*- C++ -*-

//
//  kiconloader
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@mail.bonn.netsurf.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef KICONLOADER_H
#define KICONLOADER_H

#include <qapp.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qstring.h>
#include <qtablevw.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>

#include <kapp.h>
#include <kpixmap.h>


class KIconLoaderCanvas : public QTableView
{
  Q_OBJECT;
public:
  KIconLoaderCanvas (QWidget *parent=0, const char *name=0);
  ~KIconLoaderCanvas ();

  void loadDir(QString dir_name, QString filter);
  QString getCurrent() { return name_list.at(sel_id); }

signals:
  void nameChanged( const char * );
  void doubleClicked();

protected:
  virtual void resizeEvent( QResizeEvent *e );

  void paintCell( QPainter *p, int r, int c );
  void enterEvent( QEvent *e ) { setMouseTracking(TRUE); }
  void leaveEvent( QEvent *e ) { setMouseTracking(FALSE); }
  void mouseMoveEvent( QMouseEvent *e );
  void mousePressEvent( QMouseEvent *e );
  void mouseDoubleClickEvent( QMouseEvent *e );

  int            sel_id;
  int            max_width;
  int            max_height;
  QList<QPixmap> pixmap_list;
  QStrList       name_list;
};

class KIconLoaderDialog : public QDialog
{
  Q_OBJECT;
public:
  KIconLoaderDialog ( QWidget *parent=0, const char *name=0 );
  ~KIconLoaderDialog ();

  QString getCurrent() { return canvas->getCurrent(); }
  void setDir( QString n ) { dir_name = n; }
  int exec(QString filter);

protected slots:
  void filterChanged();

protected:
  virtual void resizeEvent( QResizeEvent *e );

  KIconLoaderCanvas *canvas;
  QString            dir_name;
  QLabel            *l_name;
  QLineEdit         *i_filter;
  QLabel            *l_filter;
  QPushButton       *ok;
  QPushButton       *cancel;
  QLabel            *text;
};

/// Load icons from disk
/**
   KIconLoader is a derived class from QObject.
   It supports loading of icons from disk. It puts the icon and its name
   into a QList and if you call loadIcon() for a second time, the icon is taken
   out of the list and not reread from disk.
   So you can call loadIcon() as many times as you wish and you don't have
   to take care about multiple copies of the icon in memory.
   Another function of KIconLoader is selectIcon(). This function pops up
   a modal dialog from which you may select the icons by picture not name.
*/
class KIconLoader : public QObject
{
  Q_OBJECT;
public:
  /// Constructor
  /**
	 config is the pointer to a KConfig object; 
	 normally the global KConfig object.
	 group is the name of a group in a config file.
	 key is the name of an entry within this group.
	 
	 Normaly group == "Icons" and key == "Path"
	 Example for an entry in .kderc:
	 [Icons]
	 Path=/usr/local/lib/kde/lib/pics
	 
	 This gives KIconLoader the path to search the icons in.
	 
	 If you want to use another path in your application then write into
	 your .my_application_rc:
	 [MyApplication]
	 PixmapPath=/..../my_pixmap_path
	 and call KIconLoader( config, "MyApplication", "PixmapPath" ).
*/
  KIconLoader ( KConfig *conf, const QString &app_name, const QString &var_name );
  ~KIconLoader ();

  /// Load an icon from disk
  /**
	 This function searches for the icon called name 
	 and returns a QPixmap object
	 of this icon if it was found and NULL otherwise.
	 If name starts with "/..." loadIcon treats it as an absolut pathname.
	 LoadIcon() creates a list of all loaded icons, 
	 so calling loadIcon() a second time
	 with the same name argument won't load the icon again, but gets it out of
	 its cache. By this you don't have to worry about multiple copies
	 of one and the same icon in memory, and you can call loadIcon() 
	 as often as you like.
  */
  QPixmap loadIcon( const QString &name );

  /// Select an icon from a modal choose dialog
  /**
	 This function pops up a modal dialog and lets you select an icon by its
	 picture not name. The function returns a QPixmap object and the icons 
	 name in 'name'
	 if the user has selected an icon, or NULL if the user has pressed the 
	 cancel button. So check the result before taking any action.
	 The argument filter specifies a filter for the names of the icons to 
	 display. For example "*" displays all icons and "mini*" displays only 
	 those icons which names start with 'mini'.
	 */
  QPixmap selectIcon( QString &name, const QString &filter);

  /// Turn caching on or off
  /**
	 Normally, KIconLoader will destroy the dialog after a call to
	 selectIcon(). This may be undesirable if the user wants to change
	 several icons, because loading of many icons takes its time.
	 So it is a good idea to call setCaching(TRUE) before a call to
	 selectIcon() and call setCaching(FALSE) if you know that the
	 dialog is not needed any more. NOTE: This affects only the dialog
	 and not the normal cache created by loadIcon(). 
  */
  void    setCaching( Bool b );

protected:
  KConfig           *config;
  QString            pixmap_path;
  QStrList           name_list;
  QList<QPixmap>     pixmap_list;
  KIconLoaderDialog *pix_dialog;
  Bool               caching;
};

#endif // KICONLOADER_H




