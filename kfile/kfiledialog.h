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
#include <qlineedit.h>
#include <qstring.h>
#include <qobjectlist.h>

#include <kdialogbase.h>

#include "kfilereader.h"

class QCheckBox;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QPopupMenu;
class QVBoxLayout;

class KDirOperator;
class KFileBookmark;
class KFileBookmarkManager;
class KFileComboBox;
class KFileFilter;
class KFileView;
class KToolBar;

struct KFileDialogPrivate;

/**
 * Provide a user (and developer) friendly way to
 * select files.
 *
 * The widget can be used as a drop in replacement for the
 * @ref  QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the two static methods
 * @ref getOpenFileName() or @ref getSaveFileName().
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children which will be incorporated into the layout.
 *
 * @short A file selection dialog.
 *
 * @author Richard J. Moore rich@kde.org
 */
class KFileDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Modes of operation for the dialog.
     * @li @p File - Get a single file name from the user.
     * @li @p Directory - Get a directory name from the user.
     * @li @p Files - Get multiple file names from the user.
     * @li @p Preview - Show a preview of the file that has the user's attention.
     **/
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
      * @param acceptURLs If set to false, @ref KFileDialog will just accept
      * files on the local filesystem.
      */
    KFileDialog(const QString& dirName, const QString& filter,
		QWidget *parent, const char *name,
		bool modal);

    /**
     * Clean up.
     */
    ~KFileDialog();

    /**
      * Retrieve the fully qualified filename.
      */
    KURL selectedURL() const;

    /**
     * Retrieve the current directory.
     */
    KURL baseURL() const;

    /**
      * @return Full path in local filesystem. (Local files only)
      */
    QString selectedFile() const;

    /**
     * Set the directory to view.
     *
     * @param name URL to show
     * @param clearforward Indicate whether the forward queue
     * should be cleared.
     */
    void setURL(const QString& name, bool clearforward = true);

    /**
     * Set the directory to view.
     *
     * @param name URL to show
     * @param clearforward Indicate whether the forward queue
     * should be cleared.
     */
    void setURL(const KURL &url, bool clearforward = true);

    /**
     * Set the filename to preselect.
     *
     * This takes absolute and relative file names.
     */
    void setSelection(const QString& name);

    /**
     * Set the filter to be used to filter.
     *
     * You can set more
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
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     */
    void setFilter(const QString& filter);

    /**
     * Add a preview widget and enter the preview mode.
     *
     * In this mode
     * the dialog is split and the right part contains your widget.
     * This widget has to inherit @ref QWidget and it has to implement
     * a slot showPreview(const KURL &); which is called
     * every time the file changes. You may want to look at
     * koffice/lib/kofficecore/koFilterManager.cc for some hints :)
     */
    void setPreviewWidget(const QWidget *w);

    /**
     * Thi method creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
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
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
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
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing directory.
     *
     * @param dir The directory the dialog will start in.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getExistingDirectory(const QString & dir = QString::null,
					QWidget * parent = 0,
					const QString& caption = QString::null);

    /**
     * Show the widget.
     **/
    virtual void show();

    /**
     * Set the mode of the dialog.
     */
    void setMode( Mode m );

    /**
     * Retrieve the mode of the filedialog.
     */
    Mode mode() const;

    /**
     * sets the text to be displayed in front of the
     * selection. The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString& text);

    /**
     * KFileDialog uses chhdir() to change the current working directory to
     * increase performance. If you don't like this, disable it.
     *
     * When using one of the static methods, e.g. @ref getOpenFileName,
     * and chdir is enabled, the working directory will be restored when the
     * dialog is closed.
     * Default is enabled.
     * @see #isChdirEnabled
     */
    static void setEnableChdir( bool enable ) {
	KFileReader::setEnableChdir( enable );
    }

    /**
     * @returns whether KFileDialog changes the current working directory
     * (via chdir()).
     * @see #setEnableChdir
     */
    static bool isChdirEnabled() { return KFileReader::isChdirEnabled(); }


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
