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

#include <qdialog.h>
#include <qstring.h>
#include <qstack.h>
#include <qstrlist.h>
#include <qpixmap.h>

#include "kfileinfo.h"

class KToolBar;
class KDirListBox;
class KFileInfoContents;
class QPopupMenu;
class QCheckBox;
class QComboBox;
class KCombo;
class KBookmarkManager;
class KBookmark;
class QStrIList;
class QLineEdit;
class KDir;
class QLabel;
class QVBoxLayout;
class QGridLayout;
class QHBoxLayout;

/**
 * The KFileDialog widget provides a user (and developer) friendly way to
 * select files. The widget can be used as a drop in replacement for the
 * QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the two static methods
 * KFileDialog::getOpenFileName or KFileDialog::getCloseFileName.
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children which will be incorperated into the layout.
 *
 * @short A file selection dialog
 *
 * @author Richard J. Moore rich@kde.org
 * @version $Id$
 */
class KFileDialog : public QDialog
{
    Q_OBJECT
    
public:
    /**
      * Construct a KFileDialog
      *
      * @param dirName  The name of the directory to start in.
      * @param filter   A shell glob that specifies which files to display.
      * Unlike QFileDialog this supports filters consisting of more than one
      * glob seperated by space.
      * @param acceptURLs If set to false, kfiledialog will just accept
      * files on the local filesystem.
      */
    KFileDialog(const char *dirName, const char *filter= 0,
		QWidget *parent= 0, const char *name= 0, 
		bool modal = false, bool acceptURLs = true);
    
    
    /**
      * Cleans up
      */
    ~KFileDialog();
    
    /**
      * Returns the fully qualified filename.
      */
    QString selectedFile();
    
    /**
      * Return the path of the selected directory.
      */
    QString dirPath();
    
    /**
      * Set the directory to view.
      */
    void setDir(const char *);
    
    /**
      * Rereads the currently selected directory.
      */
    void rereadDir();
    
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
    static QString getOpenFileName(const char *dir= 0, const char *filter= 0,
				   QWidget *parent= 0, const char *name= 0);


    static QString getDirectory(const char *url, QWidget *parent = 0,  
				const char *name = 0);

    /**
      * This method creates a modal file dialog and returns the selected 
      * filename or an empty string if none was chosen. Note that with this 
      * method the user need not select an existing filename.
      *
      * @param dir This specifies the path the dialog will start in.
      * @param filter This is a space seperated list of shell globs.
      * @param parent The widget the dialog will be centered on initially.
      * @param name The name of the dialog widget.
      */
    static QString getSaveFileName(const char *dir= 0, const char *filter= 0,
				   QWidget *parent= 0, const char *name= 0);
    
    /**
      * Go back to the previous directory if this is not the first.
      */
    void back();
    
    /**
      * Go forward to the next directory if this is not the last.
      */
    void forward();
    
    /**
      * Go home.
      */
    void home();
    
    /**
      * Go to parent.
      */
    void cdUp();

    /**
      * Makes a new directory in current directory
      */
    void mkdir();
    
    /**
      * Returns true for local files, false for remote files.
      */
    bool dirIsLocal() const { return !acceptUrls; }
    
    /**
      * Returns the url of the selected filename
      */
    QString selectedFileURL();
    
    /**
      * This function is similar to getOpenFilename() but allows the
      * user to select a local file or a remote file.
      */
    static QString getOpenFileURL(const char *url= 0, const char *filter= 0,
				  QWidget *parent= 0, const char *name= 0);
    
    /**
      * This function is similar to getOpenFilename() but allows the
      * user to select a local file or a remote file.
      */
    static QString getSaveFileURL(const char *url= 0, const char *filter= 0,
				  QWidget *parent= 0, const char *name= 0);
    
    /**
      * Returns the URLs of the selected files.
      */
    QStrList selectedFileURLList();
    
    /**
      * If the argument is true the dialog will accept multiple selections.
      */
    void setMultiSelection(bool multi= true);
    
    /**
      * Returns true if multiple selections are enabled.
      */
    bool isMultiSelection() const { return false; }
    
    /**
      * This a multiple selection version of getOpenFileURL().
      */
    QStrList getOpenFileURLList(const char *url= 0,
				const char *filter= 0,
				QWidget *parent= 0,
				const char *name= 0);
    
    /**
      * This a multiple selection version of getSaveFileURL().
      */
    QStrList getSaveFileURLList(const char *url= 0, const char *filter= 0,
				QWidget *parent= 0, const char *name= 0);
    
    
    signals:
    /**
      * Emitted when the user selects a file.
      */
    void fileSelected(const char *);
    
    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const char *);
    
    /**
      * Emitted when a new directory is entered.
      */
    void dirEntered(const char *);
    
    /**
      * Emitted when the allowable history operations change.
      */
    void historyUpdate(bool, bool);


    
protected:
    KToolBar *toolbar;
    KFileInfoContents *fileList;
    
    QStrIList *visitedDirs;  // to fill the combo box
   
    QPopupMenu *bookmarksMenu; 
    QCheckBox *hiddenToggle;
    KCombo *locationEdit;

    // one of them is 0. If there are more filters, the Combobox
    // is used, otherwise the LineEdit
    QLineEdit *filterEdit;
    QComboBox *filterCombo;

    KBookmarkManager *bookmarks;
    QStrList history;
    KDir *dir;
    
    QLabel *myStatusLine;
    
    // represents the check box. Initialized by "ShowHidden"
    bool showHidden;

    /*
     * indicates, if the status bar should be shown. 
     * Initialized by "ShowStatusLine"
     */ 
    bool showStatusLine;
    bool showFilter;
    bool acceptUrls;

     /**
      * Contains all URLs you can reach with the back button.
      */
    QStack<QString> backStack;
    
    /**
      * Contains all URLs you can reach with the forward button.
      */
    QStack<QString> forwardStack;
    
    /**
      * Lock @ref #backStack and @ref #forwardStack .
      */
    bool stackLock;
    
    void resizeEvent(QResizeEvent *);
    
    /**
      * Subclasses should overload this method to insert new widgets into
      * the dialog which will be displayed between the toolbar and the
      * directory/file views.
      */
    virtual void initUpperChildren();
    
    /**
      * Subclasses should overload this method to insert new widgets into
      * the dialog which will be displayed between the filter and the
      * button area.
      */
    virtual void initLowerChildren();

    /**
      * Subclasses should reimplement this method to swallow the main
      * layout. This allows the addition of children that are outside
      * the existing layout. The function should return the widget that
      * should be the parent of the main layout.
      */
    virtual QWidget *swallower() { return this; }

    /**
      * Subclasses, that want another view should override this 
      * function to provide another file view.
      * It will be used to display files.
      **/
    virtual KFileInfoContents *initFileList( QWidget *parent );

    /**
      * Set the directory to view for internal use
      * @internal
      */
    void setDir2(const char *, bool clearforward = true);

    /**
      * adds a entry of the current directory. If disableUpdating is set
      * to true, it will care about clever updating
      **/
    void addDirEntry(KFileInfo *entry, bool disableUpdating);

    void initGUI();

protected slots:
    void pathChanged();
    void comboActivated(int);
    void toolbarCallback(int);
    void toolbarPressedCallback(int);
    void dirActivated();
    void fileActivated();
    void fileHighlighted();
    void updateHistory(bool, bool);
    void filterChanged();
    void locationChanged(const char*);
    
    void setHiddenToggle(bool);
    void slotDirEntry(KFileInfo *);
    void insertFile(const KFileInfo *);
    void slotFinished();
    void slotKfmError(int, const char *);
    void insertNewFiles(const KFileInfoList *newone);
    void completion();
    void updateStatusLine();

    /**
      * You should override this method if you change the user interface of
      * this dialog in a subclass in order to invoke your updated user help.
      */
    virtual void help();
    
    /**
      * Add the current location to the global bookmarks list
      */
    void addToBookmarks();
    void bookmarksChanged();
    void fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id );

private:
    // the father of the widget. By default itself, but
    // to make it customable, this can be overriden by 
    // overriding 
    QWidget *wrapper;
    QString filename_;
    QStrList *filters;

    // flag for perfomance hype ;)
    bool repaint_files; 
    // for the handling of the cursor
    bool finished;
    
    // indicates, if the file selector accepts just existing
    // files or not. If set to true, it will check for local
    // files, if they exist
    bool acceptOnlyExisting;

    // now following all kind of widgets, that I need to rebuild
    // the geometry managment
    QVBoxLayout *boxLayout;
    QGridLayout *lafBox;
    QHBoxLayout *btngroup;
    QPushButton *bOk, *bCancel, *bHelp;
    QLabel *locationLabel, *filterLabel;
    
};

#endif





