// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>

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

#ifndef __KFILEDIALOG_H__
#define __KFILEDIALOG_H__

#include <qcombobox.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qobjectlist.h>
#include <kfileviewitem.h>
#include <kdialogbase.h>

class KToolBar;
class KFileView;
class QPopupMenu;
class QCheckBox;
class KFileBookmarkManager;
class KFileBookmark;
class KFileReader;
class QLabel;
class QVBoxLayout;
class QGridLayout;
class QHBoxLayout;
class KFileFilter;
class KDirOperator;
class KFileComboBox;
struct KFileDialogPrivate;

/**
 * The KFileDialog widget provides a user (and developer) friendly way to
 * select files. The widget can be used as a drop in replacement for the
 * QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the two static methods
 * @ref getOpenFileName or @ref getSaveFileName.
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children which will be incorporated into the layout.
 *
 * @short A file selection dialog
 *
 * @author Richard J. Moore rich@kde.org
 */
class KFileDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum Mode {
	File = 1,
	Directory = 2,
	Files = 4,
    Preview = 8
    };
    /**
      * Construct a KFileDialog
      *
      * @param dirName  The name of the directory to start in.
      * @param filter   A shell glob that specifies which files to display.
      * see setFilter for details on how to use this argument
      * @param acceptURLs If set to false, kfiledialog will just accept
      * files on the local filesystem.
      */
    KFileDialog(const QString& dirName, const QString& filter,
		QWidget *parent, const char *name,
		bool modal);

    /**
     * Cleans up
     */
    ~KFileDialog();

    /**
      * Returns the fully qualified filename.
      */
    KURL selectedURL() const;

    /**
     * Returns the current directory
     */
    KURL baseURL() const;

    /**
      * @return full path in local filesystem. (Local files only)
      */
    QString selectedFile() const;

    /**
     * Sets the directory to view
     * @param name URL to show
     * @param clearforward indicate, if the forward queue
     * should be cleared
     */
    void setURL(const QString& name, bool clearforward = true);

    /**
     * Sets the directory to view
     * @param name URL to show
     * @param clearforward indicate, if the forward queue
     * should be cleared
     */
    void setURL(const KURL &url, bool clearforward = true);

    /**
     * Sets the filename to preselect.
     * It takes absolute and relative file names
     */
    void setSelection(const QString& name);

    /**
     * Sets the filter to be used to filter. You can set more
     * filters for the user to select seperated by '\n'. Every
     * filter entry is defined through namefilter|text to diplay.
     * If no | is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * <pre>
     * kfile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * kfile->setFilter("*.cpp");
     * kfile->setFilter("*.cpp|Sources (*.cpp)");
     * </pre>
     *
     * Note: the text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     */
    void setFilter(const QString& filter);

    /**
     * Adds a preview widget and enters the preview mode. In this mode
     * the dialog is splitted and the right part contains your widget.
     * This widget has to inherit QWidget and it has to implement
     * a slot <pre>showPreview(const KURL &);</pre> which is called
     * every time the file changes. You may want to look at
     * koffice/lib/kofficecore/koFilterManager.cc for some hints :)
     */
    void setPreviewWidget(const QWidget *w);

    /**
     * This method creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen. Note that with
     * this method the user must select an existing filename.
     *
     * @param dir This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param name The name of the dialog widget.
     */
    static QString getOpenFileName(const QString& dir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);

    /**
     * This method creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen. Note that with this
     * method the user need not select an existing filename.
     *
     * @param dir This specifies the path the dialog will start in.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getSaveFileName(const QString& dir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);

    /**
     * This method creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen. Note that with this
     * method the user need not select an existing directory.
     *
     * @param dir This specifies the directory the dialog will start in.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getExistingDirectory(const QString & dir = QString::null,
					QWidget * parent = 0,
					const QString& caption = QString::null);

    virtual void show();

    static void initIcons();

    /**
     * Set the mode of the dialog.
     */
    void setMode( Mode m );

    /**
     * Retrieves the mode of the filedialog.
     */
    Mode mode() const;

signals:
    /**
      * Emitted when the user selects a file.
      */
    void fileSelected(const QString&);

    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const QString&);

    /**
      * Emitted when the allowable history operations change.
      */
    void historyUpdate(bool, bool);

    /**
     * Emitted when in preview mode and the user selects a file
     */
    void showPreview(const KURL &url);

protected:
    KToolBar *toolbar;

    QStringList *visitedDirs;  // to fill the combo box
    static QString *lastDirectory;

    QPopupMenu *bookmarksMenu;
    KFileComboBox *locationEdit;

    KFileFilter *filterWidget;

    KFileBookmarkManager *bookmarks;
    QStringList history;

    /**
     * adds a entry of the current directory. If disableUpdating is set
     * to true, it will care about clever updating
     **/
    void addDirEntry(KFileViewItem *entry, bool disableUpdating);

    /**
      * rebuild geometry managment.
      *
      */
    virtual void initGUI();

    /**
      * takes action on the new location. If it's a directory, change
      * into it, if it's a file, correct the name, etc.
      * @param takeFiles if set to true, if will close the dialog, if
      * txt is a file name
      */
    void checkPath(const QString& txt, bool takeFiles = false);

protected slots:
    void urlEntered(const KURL&);
    void comboActivated(int);
    void toolbarCallback(int);
    void toolbarPressedCallback(int);
    void filterChanged();
    void locationChanged(const QString&);
    void fileHightlighted(const KFileViewItem *i);
    void fileSelected(const KFileViewItem *i);

    virtual void updateStatusLine(int dirs, int files);
    virtual void slotOk();
    virtual void returnPressed();

    void completion();

    /**
      * Add the current location to the global bookmarks list
      */
    void addToBookmarks();
    void bookmarksChanged();
    void fillBookmarkMenu( KFileBookmark *parent, QPopupMenu *menu, int &id );

private:

    // cleanup the static variables
    static void cleanup();
    KFileDialog(const KFileDialog&);
    KFileDialog operator=(const KFileDialog&);
    bool previewMode;


protected:
    KFileDialogPrivate *d;
    KDirOperator *ops;

};


class KFileComboBox : public QComboBox
{
  Q_OBJECT

public:
  KFileComboBox( bool rw, QWidget *parent=0, const char *name=0 )
    : QComboBox ( rw, parent, name ) {
    QObjectList *list = queryList( "QLineEdit" );
    QObjectListIt it( *list );
    edit = (QLineEdit*) it.current();
    connect(edit, SIGNAL(returnPressed()), SIGNAL(returnPressed()));
    edit->installEventFilter( this );
  }

  int cursorPosition() const { return edit->cursorPosition(); }
  void setCompletion( const QString& );

signals:
  void returnPressed();
  void completion();
  void next();
  void previous();


private:
  virtual bool eventFilter( QObject *o, QEvent *e );

  QLineEdit *edit;

};


#endif
