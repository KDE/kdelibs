/* This file is part of the KDE libraries
   Copyright (C) 1999 Kurt Granroth <granroth@kde.org>

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
#ifndef KSTDACTION_H
#define KSTDACTION_H

class KSelectAction;
class KToggleAction;
class KAction;
class QAction;
class QObject;

/**
 * Convenient methods to access all standard KDE actions.  These
 * actions should be preferred over hardcoding menubar and toolbar
 * items.  Using these actions helps your application easily conform
 * to the KDE UI Style Guide.
 */
class KStdAction
{
public:
    enum StdAction {
        // File Menu
        New=1, Open, OpenRecent, Save, SaveAs, Revert, Close,
        Print, PrintPreview, Quit,

        // Edit Menu
        Undo, Redo, Cut, Copy, Paste, SelectAll, Find, FindNext, FindPrev,
        Replace,

        // View Menu
        ActualSize, FitToPage, FitToWidth, FitToHeight, ZoomIn, ZoomOut,
        Zoom, Redisplay,

        // Go Menu
        Up, Back, Forward, Home, Prior, Next, Goto, FirstPage, LastPage,

        // Bookmarks Menu
        AddBookmark, EditBookmarks,

        // Options Menu
        ShowMenubar, ShowToolbar, ShowStatusbar, KeyBindings, Preferences,

        // Help Menu
        Help
    };

    KStdAction();
    ~KStdAction();

    /**
     * This function returns the action corresponding to the
     * @ref KStdAction::StdAction enum.
     */
    static QAction *action(StdAction act_enum, const QObject *recvr = 0,
                           const char *slot = 0, QObject *parent = 0);

    /** Create a new document or window. */
    static KAction *openNew(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0);

    /**
     * Open an existing file
     */
    static KAction *open(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Open a recently used document
     */
    static KSelectAction *openRecent(const QObject *recvr = 0, const char *slot = 0,
                                     QObject *parent = 0);

    /**
     * Save the current document
     */
    static KAction *save(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Save the current document under a different name
     */
    static KAction *saveAs(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0);

    /**
     * Revert the current document to the last saved version
     * (essentially will undo all changes)
     */
    static KAction *revert(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0);

    /**
     * Close the current document
     */
    static KAction *close(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0);

    /**
     * Print the current document
     */
    static KAction *print(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0);

    /**
     * Show a print preview of the current document
     */
    static KAction *printPreview(const QObject *recvr = 0, const char *slot = 0,
                                 QObject *parent = 0);

    /**
     * Quit the program
     */
    static KAction *quit(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Undo the last operation
     */
    static KAction *undo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Redo the last operation.
     */
    static KAction *redo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Cut selected area and store it in the clipboard
     */
    static KAction *cut(const QObject *recvr = 0, const char *slot = 0,
                        QObject *parent = 0);

    /**
     * Copy the selected area into the clipboard
     */
    static KAction *copy(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Paste the contents of clipboard at the current mouse or cursor
     * position
     */
    static KAction *paste(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0);

    /**
     * Select all elements in the current document
     */
    static KAction *selectAll(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0);

    /**
     * Initiate a 'find' request in the current document
     */
    static KAction *find(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Find the next instance of a stored 'find'
     */
    static KAction *findNext(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0);

    /**
     * Find a previous instance of a stored 'find'
     */
    static KAction *findPrev(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0);

    /**
     * Find and replace matches
     */
    static KAction *replace(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0);

    /**
     * View the actual size of the document
     */
    static KAction *actualSize(const QObject *recvr = 0, const char *slot = 0,
                               QObject *parent = 0);

    /**
     * Fit the document view to the size of the current window
     */
    static KAction *fitToPage(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0);

    /**
     * Fit the document view to the width of the current window
     */
    static KAction *fitToWidth(const QObject *recvr = 0, const char *slot = 0,
                               QObject *parent = 0);

    /**
     * Fit the document view to the height of the current window
     */
    static KAction *fitToHeight(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0);

    /**
     * Zoom in
     */
    static KAction *zoomIn(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0);

    /**
     * Zoom out
     */
    static KAction *zoomOut(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0);

    /**
     * Popup a zoom dialog
     */
    static KAction *zoom(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Redisplay or redraw the document
     */
    static KAction *redisplay(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0);

    /**
     * Move up (web style menu)
     */
    static KAction *up(const QObject *recvr = 0, const char *slot = 0,
                       QObject *parent = 0);

    /**
     * Move back (web style menu)
     */
    static KAction *back(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Move forward (web style menu)
     */
    static KAction *forward(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0);

    /**
     * Go to the "Home" position or document
     */
    static KAction *home(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Scroll up one page
     */
    static KAction *prior(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0);

    /**
     * Scroll down one page
     */
    static KAction *next(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);

    /**
     * Go to a specific page (dialog)
     */
    static KAction *gotoPage(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0);

    /**
     * Jump to the first page
     */
    static KAction *firstPage(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0);

    /**
     * Jump to the last page
     */
    static KAction *lastPage(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0);

    /**
     * Add the current page to the bookmarks tree
     */
    static KAction *addBookmark(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0);

    /**
     * Edit the application bookmarks
     */
    static KAction *editBookmarks(const QObject *recvr = 0, const char *slot = 0,
                                  QObject *parent = 0);

    /**
     * Show/Hide the menubar
     */
    static KToggleAction *showMenubar(const QObject *recvr = 0, const char *slot = 0,
				      QObject *parent = 0);

    /**
     * Show/Hide the primary toolbar
     */
    static KToggleAction *showToolbar(const QObject *recvr = 0, const char *slot = 0,
				      QObject *parent = 0);

    /**
     * Show/Hide the statusbar
     */
    static KToggleAction *showStatusbar(const QObject *recvr = 0, const char *slot = 0,
					QObject *parent = 0);

    /**
     * Display the configure key bindings dialog
     */
    static KAction *keyBindings(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0);

    /**
     * Display the preferences/options dialog
     */
    static KAction *preferences(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0);

    /**
     * Display the help menu
     */
    static KAction *help(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0);
};

#endif // KSTDACTION_H
