// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1999 Stephan Kulow <coolo@kde.org>

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
#ifndef KDIROPERATOR_H_
#define KDIROPERATOR_H_

#include <qobject.h>
#include <qstack.h>
#include <kcompletion.h>
#include <kfileviewitem.h>

// for public enums - may move to KFile (TODO)
#include <kfiledialog.h>

class QTimer;

class KFileReader;
class KFileView;
class QWidgetStack;
class KProgress;

class KDirOperator : public QWidget {
    Q_OBJECT
	
 public:

    enum FileView { Default = 0, Simple = 1, Detail = 2};

    KDirOperator(const QString& dirName = QString::null,
		 QWidget *parent = 0, const char* name = 0);
    virtual ~KDirOperator();

    void setShowHiddenFiles ( bool s );
    bool showHiddenFiles () const;

    void close();

    void setNameFilter(const QString& filter);

    KURL url() const;

    void setURL(const KURL& url, bool clearforward);

    void rereadDir();

    //this also reads the current url(), so you better call this after setURL()
    void setView(KFileView *view);
    void setView(FileView view, bool separateDirs = false);

    bool isRoot() const;

    KFileReader *fileReader() const { return dir; }

    void setMode( KFileDialog::Mode m );
    KFileDialog::Mode mode() const;

    int numDirs() const;
    int numFiles() const;


 protected:
    void setFileReader( KFileReader *reader );
    void resizeEvent( QResizeEvent * );


 private:
    // represents the check box. Initialized by "ShowHidden"
    bool showHidden;

    /**
     * Contains all URLs you can reach with the back button.
     */
    QStack<KURL> backStack;

    /**
     * Contains all URLs you can reach with the forward button.
     */
    QStack<KURL> forwardStack;

    static QString *lastDirectory;

    KFileReader *dir;

    KCompletion myCompletion;
    bool myCompleteListDirty;

    /**
      * takes action on the new location. If it's a directory, change
      * into it, if it's a file, correct the name, etc.
      */
    void checkPath(const QString& txt, bool takeFiles = false);

    void connectView(KFileView *);

    // flag for perfomance hype ;)
    bool repaint_files;
    // for the handling of the cursor
    bool finished;

    KFileView *fileList;
    KFileView *oldList;

    KFileViewItemList pendingMimeTypes;

    int viewKind;

    KFileDialog::Mode _mode;
    KProgress *progress;

 public slots:
    void back();
    void forward();
    void home();
    void cdUp();
    void mkdir();
    QString makeCompletion(const QString&);

  protected slots:
    void resetCursor();
    void readNextMimeType();
    void slotKIOError(int, const QString& );
    void pathChanged();
    void filterChanged();
    void insertNewFiles(const KFileViewItemList *newone, bool ready);
    void itemsDeleted(const KFileViewItemList *);

    void selectDir(const KFileViewItem*);
    void selectFile(const KFileViewItem*);
    void highlightFile(const KFileViewItem*);
    void activatedMenu( const KFileViewItem * );

    void detailedView();
    void simpleView();
    void toggleHidden();
    void toggleMixDirsAndFiles();

    void deleteOldView();

    void slotCompletionMatch(const QString&);
    void slotCompletionMatches(const QStringList&);

  signals:
    void urlEntered(const KURL& );
    void updateInformation(int files, int dirs);
    void completion(const QString&);

    void fileHighlighted(const KFileViewItem*);
    void dirActivated(const KFileViewItem*);
    void fileSelected(const KFileViewItem*);

};

#endif
