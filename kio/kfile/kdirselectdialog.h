/*
  Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>
  Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef KDIRSELECTDIALOG_H
#define KDIRSELECTDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class QVBoxLayout;
class KConfig;
class KFileTreeView;

/**
 * A pretty dialog for a KDirSelect control for selecting directories.
 * @author Michael Jarrett <michaelj@corel.com>
 * @see KFileDialog
 */
class KDirSelectDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * The constructor. Creates a dialog to select a directory (url).
     * @internal use the static selectDirectory function
     * @param startDir the directory, initially shown
     * @param parent the parent for the dialog, usually 0L
     * @param name the QObject::name
     * @param modal if the dialog is modal or not
     */
    KDirSelectDialog(const QString& startDir = QString::null,
                     bool localOnly = false,
                     QWidget *parent = 0L,
                     const char *name = 0, bool modal = false);

    /**
     */
    ~KDirSelectDialog();

    /**
     * Returns the currently-selected URL, or a blank URL if none is selected.
     * @return The currently-selected URL, if one was selected.
     */
    KURL url() const;

    KFileTreeView * view() const { return m_treeView; }

    bool localOnly() const { return m_localOnly; }

    /**
     * Creates a KDirSelectDialog, and returns the result.
     * @param startDir the directory, initially shown
     * The tree will display this directory and subdirectories of it.
     * @param localOnly unused. You can only select paths below the startDir
     * @return The URL selected, or an empty URL if the user cancelled
     * or no URL was selected.
     */
    static KURL selectDirectory( const QString& startDir = QString::null,
                                 bool localOnly = false, QWidget *parent = 0L,
                                 const QString& caption = QString::null);

    /**
     * @return The path for the root node
     */
    QString startDir() const { return m_startDir; }

public slots:
    void setCurrentURL( const KURL& url );

protected:
    virtual void accept();

    // Layouts protected so that subclassing is easy
    QVBoxLayout *m_mainLayout;
    QString m_startDir;

private slots:
    void slotCurrentChanged();
    void slotURLActivated( const QString& );
    void slotNextDirToList( KFileTreeViewItem *dirItem );

private:
    void readConfig( KConfig *config, const QString& group );
    void saveConfig( KConfig *config, const QString& group );
    void openNextDir( KFileTreeViewItem *parent );
    KFileTreeBranch * createBranch( const KURL& url );

    KFileTreeView *m_treeView;
    bool m_localOnly;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KDirSelectDialogPrivate;
    KDirSelectDialogPrivate *d;
};

#endif
