// -*- C++ -*-

//
//  kiconloaderdialog
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

// CHANGES
// Torben, added KIconLoaderButton

#ifndef KICONLOADERDIALOG_H
#define KICONLOADERDIALOG_H

#include <qapplication.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qstrlist.h>
#include <qstring.h>
#include <qtableview.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtimer.h>

#include <kapp.h>
#include <kiconloader.h>

/**
* Internal display class for @ref KIconLoaderDialog
* @short Internal display class for KIconLoaderDialog
* @version $Id$
* @author Christoph.Neerfeld@bonn.netsurf.de
*/
class KIconLoaderCanvas : public QTableView
{
  Q_OBJECT
public:
  KIconLoaderCanvas (QWidget *parent=0, const char *name=0);
  ~KIconLoaderCanvas ();

  void loadDir(QString dirname, QString filter);
  QString getCurrent() { if(name_list.isEmpty()) return ""; return name_list.at(sel_id); }

signals:
  void nameChanged( const char * );
  void doubleClicked();

protected slots:
  void process();

protected:
  virtual void resizeEvent( QResizeEvent *e );

  void paintCell( QPainter *p, int r, int c );
  void enterEvent( QEvent * ) { setMouseTracking(TRUE); }
  void leaveEvent( QEvent * ) { setMouseTracking(FALSE); }
  void mouseMoveEvent( QMouseEvent *e );
  void mousePressEvent( QMouseEvent *e );
  void mouseDoubleClickEvent( QMouseEvent *e );

  int            sel_id;
  int            max_width;
  int            max_height;
  int            curr_indx;
  QList<QPixmap> pixmap_list;
  QStrList       file_list;
  QStrList       name_list;
  QTimer         *timer;
  QString        dir_name;
};

/** 
* Dialog for interactive selection of icons.
*
* KIconLoaderDialog is a derived class from QDialog.
* It provides one function selectIcon() which displays a dialog.
* This dialog lets you select the icons within the IconPath by image.
*/
class KIconLoaderDialog : public QDialog
{
  Q_OBJECT
public:
  /// Constructor
  /**
     The KIconLoaderDialog is a modal dialog; i.e. it has its own eventloop
     and the normal program will stop after a call to selectIcon() until
     selectIcon() returns.
     This constructor creates a KIconLoaderDialog that will call
     KApplication::getKApplication()->getIconLoader() to load any icons.
     Note that it will not use this KIconLoader to display the icons, but
     the QPixmap that it returns will be know to this KIconLoader.
     KIconLoaderDialog caches all icons it has loaded as long as they are in the
     same directory between two calls to selectIcon(). So it is a good idea to
     delete the KIconLoaderDialog when it is not needed anymore.
  */
  KIconLoaderDialog ( QWidget *parent=0, const char *name=0 );

  /**
     If you want to use another KIconLoader you can create the KIconLoaderDialog
     with this constructor which accepts a pointer to a KIconLoader.
     Make sure that this pointer is valid.
  */
  KIconLoaderDialog ( KIconLoader *loader, QWidget *parent=0, const char *name=0 );

  /// Destructor
  ~KIconLoaderDialog ();

  /// Select an icon from a modal choose dialog
  /**
	 This function pops up a modal dialog and lets you select an icon by its
	 picture not name. The function returns a QPixmap object and the icons 
	 name in 'name'
	 if the user has selected an icon, or null if the user has pressed the 
	 cancel button. So check the result before taking any action.
	 The argument filter specifies a filter for the names of the icons to 
	 display. For example "*" displays all icons and "mini*" displays only 
	 those icons which names start with 'mini'.
  */
  QPixmap selectIcon( QString &name, const QString &filter);
  void setDir( const QStrList *l ) { cb_dirs->clear(); cb_dirs->insertStrList(l); }
  int exec(QString filter);

protected slots:
  void filterChanged();
  void dirChanged(const char *);
  
protected:
  void init();
  virtual void resizeEvent( QResizeEvent *e );

  KIconLoaderCanvas *canvas;
  QLabel            *l_name;
  QLineEdit         *i_filter;
  QLabel            *l_filter;
  QPushButton       *ok;
  QPushButton       *cancel;
  QLabel            *text;
  QComboBox         *cb_dirs;
  KIconLoader       *icon_loader;
};

/**
 * This is a button that uses the @ref KIconLoaderDialog.
 * It shows the currently selected icon. Pressing on
 * the icon will open the dialog. If the icon changes, a
 * signal is emitted and the buttons pixmap becomes updated.
 *
 * HACK
 * Since I did not want to break binary compatibility, it does
 * NOT use the same search path as the dialog. This IS a bug.
 */
class KIconLoaderButton : public QPushButton
{
    Q_OBJECT
public:
    /**
     * Creates a new button.
     */
    KIconLoaderButton( QWidget *_widget );
    KIconLoaderButton( KIconLoader *_icon_loader, QWidget *_widget );
    ~KIconLoaderButton();
    
    /**
     * Set the buttons icon. 
     *
     * @param _icon is a parameter as usually passed to @ref KIconLoader.
     */
    void setIcon( const char *_icon );
    /**
     * @return the name of the icon without path.
     */
    const char* icon() { return iconStr.data(); }
    /**
     * @return a reference to the icon loader dialog used.
     */
    KIconLoaderDialog* iconLoaderDialog() { return loaderDialog; }
    
public slots:
    void slotChangeIcon();
    
signals:
    /**
     * Emitted if the icons has changed.
     */
    void iconChanged( const char *icon );
    
protected:
    KIconLoaderDialog *loaderDialog;
    QString iconStr;
    KIconLoader *iconLoader;
};


#endif // KICONLOADERDIALOG_H



