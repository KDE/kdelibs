/*
 *  -*- C++ -*-
 *
 *  kiconloaderdialog.
 *
 *  Copyright (C) 1997 Christoph Neerfeld
 *  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

// CHANGES
// Torben, added KIconLoaderButton



#ifndef KICONLOADERDIALOG_H
#define KICONLOADERDIALOG_H

#include <qpixmap.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qiconview.h>

#include <kapp.h>
#include <kdialogbase.h>
#include <kiconloader.h>

class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QTimer;

/**
* Internal display class for @ref KIconLoaderDialog
* @short Internal display class for KIconLoaderDialog
* @version $Id$
* @author Christoph.Neerfeld@bonn.netsurf.de
*/
class KIconLoaderCanvas : public QIconView
{
  Q_OBJECT

  public:
    KIconLoaderCanvas (QWidget *parent=0, const char* name=0);
    virtual ~KIconLoaderCanvas();

    void loadDir(QString dirname, QString filter);
    QString getCurrent( void );
    QString currentDir( void );

  signals:
    void nameChanged( const QString& );
    void doubleClicked();
    void interrupted();
    void startLoading( int steps );
    void progress( int p );
    void finished();

  protected:
    /**
     * Makes sure Key_Escape is ignored
     */
    virtual void keyPressEvent(QKeyEvent *e);

  private slots:
    void slotLoadDir();
    void slotCurrentChanged( QIconViewItem *item );

  private:
    QString dir_name, filter;
    QTimer *loadTimer;

};


/**
 * Dialog for interactive selection of icons.
 *
 * KIconLoaderDialog is a derived class from @ref KDialogBase.
 * It provides one function selectIcon() which displays a dialog.
 * This dialog lets you select the icons within the IconPath by image.
 */
class KIconLoaderDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /**
     * The KIconLoaderDialog is a modal dialog; i.e. it has its own eventloop
     * and the normal program will stop after a call to selectIcon() until
     * selectIcon() returns.
     * This constructor creates a KIconLoaderDialog that will call
     * KApplication::getKApplication()->getIconLoader() to load any icons.
     * Note that it will not use this KIconLoader to display the icons, but
     * the QPixmap that it returns will be know to this KIconLoader.
     * KIconLoaderDialog caches all icons it has loaded as long as they 
     * are in the same directory between two calls to selectIcon(). So it 
     * is a good idea to delete the KIconLoaderDialog when it is not 
     * needed anymore.
     */
    KIconLoaderDialog ( QWidget *parent=0, const char *name=0 );

    /**
     * If you want to use another KIconLoader you can create the 
     * KIconLoaderDialog with this constructor which accepts a pointer to a 
     * KIconLoader. Make sure that this pointer is valid.
     */
    KIconLoaderDialog ( KIconLoader *loader, QWidget *parent=0, 
			const char *name=0  );

    /**
     * Destructor
     */
    ~KIconLoaderDialog ();

    /**
     * Get an icon from a modal selector dialog.
     * This method pops up a modal dialog and lets you select an icon by its
     * picture not name. The function returns a QPixmap object and the icons
     * name in 'name'.
     * if the user has selected an icon, or null if the user has pressed the
     * cancel button. So check the result before taking any action.
     * The argument filter specifies a filter for the names of the icons to
     * display. For example "*" displays all icons and "mini*" displays only
     * those icons which names start with 'mini'.
     */
    QPixmap selectIcon( QString &name, const QString &filter);

    /**
     * sets the directories to choose from. By default these are
     * all directories that keep toolbar icons
     */
    void changeDirs( const QStringList &l );
    int exec(QString filter);

  protected:
    void init();

  protected slots:
    void filterChanged();
    void dirChanged(const QString&);
    void needReload();

    void initProgressBar( int steps );
    void progress( int p );
    void hideProgressBar( void );

  protected:
    KIconLoaderCanvas *canvas;
    QLabel	      *l_name;
    QLineEdit	      *i_filter;
    QLabel	      *l_filter;
    QLabel	      *text;
    QComboBox	      *cb_dirs;
    KIconLoader	      *icon_loader;
    QProgressBar *progressBar;
};

/**
 * This is a button that uses the @ref KIconLoaderDialog.
 * It shows the currently selected icon. Pressing on
 * the icon will open the dialog. If the icon changes, a
 * signal is emitted and the buttons pixmap becomes updated.
 *
 * You can set the resource type for locating the icon pixmaps.
 * See @ref setIconType for changing the default setting,
 * which is "toolbar".
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
     * Sets the resource type for locating icon pixmaps and reloads
     * the icon, if a name has already been given. The default
     * resource type is "toolbar".
     *
     * @param _resType is a resource type known to @ref KStandardDirs.
     */
    void setIconType(const QString& _resType);
    /**
     * Set the buttons icon.
     *
     * @param _icon is a parameter as usually passed to @ref KIconLoader.
     */
    void setIcon( const QString& _icon );
    /**
     * @return the name of the icon without path.
     */
    const QString icon() { return iconStr; }
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
    void iconChanged( const QString& icon );

protected:
    KIconLoaderDialog *loaderDialog;
    QString iconStr;
    QString resType;
    KIconLoader *iconLoader;
};


#endif // KICONLOADERDIALOG_H



