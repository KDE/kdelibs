/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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
#ifndef KSTDACTION_H
#define KSTDACTION_H

class KSelectAction;
class KToggleAction;
class KAction;
class QAction;
class QObject;

/**
 * Convenience methods to access all standard KDE actions.
 *
 * These actions should be used instead of hardcoding menubar and
 * toolbar items.  Using these actions helps your application easily
 * conform to the KDE UI Style Guide
 * ( @see http://developer.kde.org/documentation/standards/kde/style/basics/index.html ).
 */
class KStdAction
{
public:
  /**
   * The standard menubar and toolbar actions.
   **/
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

        // Tools Menu
        Spelling,

        // Options Menu
        ShowMenubar, ShowToolbar, ShowStatusbar, KeyBindings, Preferences,

        // Help Menu
        Help
    };

    /**
     * Constructor.
     **/
    KStdAction();
    /**
     * Destructor.
     **/
    ~KStdAction();

    /**
     * Retrieve the action corresponding to the
     * @ref KStdAction::StdAction enum.
     */
    static QAction *action(StdAction act_enum, const QObject *recvr = 0,
                           const char *slot = 0, QObject *parent = 0,
                           const char *name = 0L );

    /**
     * This will return the internal name of a given standard action.
     */
    static const char* stdName(StdAction act_enum);

    /** Create a new document or window. */
    static KAction *openNew(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0, const char *name = 0L );

    /**
     * Open an existing file.
     */
    static KAction *open(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Open a recently used document.
     */
    static KSelectAction *openRecent(const QObject *recvr = 0, const char *slot = 0,
                                     QObject *parent = 0, const char *name = 0L );

    /**
     * Save the current document.
     */
    static KAction *save(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Save the current document under a different name.
     */
    static KAction *saveAs(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0, const char *name = 0L );

    /**
     * Revert the current document to the last saved version
     * (essentially will undo all changes).
     */
    static KAction *revert(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0, const char *name = 0L );

    /**
     * Close the current document.
     */
    static KAction *close(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0, const char *name = 0L );

    /**
     * Print the current document.
     */
    static KAction *print(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0, const char *name = 0L );

    /**
     * Show a print preview of the current document.
     */
    static KAction *printPreview(const QObject *recvr = 0, const char *slot = 0,
                                 QObject *parent = 0, const char *name = 0L );

    /**
     * Quit the program.
     */
    static KAction *quit(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Undo the last operation.
     */
    static KAction *undo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Redo the last operation.
     */
    static KAction *redo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Cut selected area and store it in the clipboard.
     */
    static KAction *cut(const QObject *recvr = 0, const char *slot = 0,
                        QObject *parent = 0, const char *name = 0L );

    /**
     * Copy the selected area into the clipboard.
     */
    static KAction *copy(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Paste the contents of clipboard at the current mouse or cursor
     * position.
     */
    static KAction *paste(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0, const char *name = 0L );

    /**
     * Select all elements in the current document.
     */
    static KAction *selectAll(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0, const char *name = 0L );

    /**
     * Initiate a 'find' request in the current document.
     */
    static KAction *find(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Find the next instance of a stored 'find'.
     */
    static KAction *findNext(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

    /**
     * Find a previous instance of a stored 'find'.
     */
    static KAction *findPrev(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

    /**
     * Find and replace matches.
     */
    static KAction *replace(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0, const char *name = 0L );

    /**
     * View the document at its actual size.
     */
    static KAction *actualSize(const QObject *recvr = 0, const char *slot = 0,
                               QObject *parent = 0, const char *name = 0L );

    /**
     * Fit the document view to the size of the current window.
     */
    static KAction *fitToPage(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0, const char *name = 0L );

    /**
     * Fit the document view to the width of the current window.
     */
    static KAction *fitToWidth(const QObject *recvr = 0, const char *slot = 0,
                               QObject *parent = 0, const char *name = 0L );

    /**
     * Fit the document view to the height of the current window.
     */
    static KAction *fitToHeight(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0, const char *name = 0L );

    /**
     * Zoom in.
     */
    static KAction *zoomIn(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0, const char *name = 0L );

    /**
     * Zoom out.
     */
    static KAction *zoomOut(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0, const char *name = 0L );

    /**
     * Popup a zoom dialog.
     */
    static KAction *zoom(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Redisplay or redraw the document.
     */
    static KAction *redisplay(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0, const char *name = 0L );

    /**
     * Move up (web style menu).
     */
    static KAction *up(const QObject *recvr = 0, const char *slot = 0,
                       QObject *parent = 0, const char *name = 0L );

    /**
     * Move back (web style menu).
     */
    static KAction *back(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Move forward (web style menu).
     */
    static KAction *forward(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0, const char *name = 0L );

    /**
     * Go to the "Home" position or document.
     */
    static KAction *home(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Scroll up one page.
     */
    static KAction *prior(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0, const char *name = 0L );

    /**
     * Scroll down one page.
     */
    static KAction *next(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Go to a specific page (dialog).
     */
    static KAction *gotoPage(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

    /**
     * Jump to the first page.
     */
    static KAction *firstPage(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0, const char *name = 0L );

    /**
     * Jump to the last page.
     */
    static KAction *lastPage(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

    /**
     * Add the current page to the bookmarks tree.
     */
    static KAction *addBookmark(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0, const char *name = 0L );

    /**
     * Edit the application bookmarks.
     */
    static KAction *editBookmarks(const QObject *recvr = 0, const char *slot = 0,
                                  QObject *parent = 0, const char *name = 0L );

    /**
     * Popup the spell checker
     */
    static KAction *spelling(const QObject *recvr = 0, const char *slot = 0,
                                  QObject *parent = 0, const char *name = 0L );


    /**
     * Show/Hide the menubar.
     */
    static KToggleAction *showMenubar(const QObject *recvr = 0, const char *slot = 0,
				      QObject *parent = 0, const char *name = 0L );

    /**
     * Show/Hide the primary toolbar.
     */
    static KToggleAction *showToolbar(const QObject *recvr = 0, const char *slot = 0,
				      QObject *parent = 0, const char *name = 0L );

    /**
     * Show/Hide the statusbar.
     */
    static KToggleAction *showStatusbar(const QObject *recvr = 0, const char *slot = 0,
					QObject *parent = 0, const char *name = 0L );

    /**
     * Display the configure key bindings dialog.
     */
    static KAction *keyBindings(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0, const char *name = 0L );

    /**
     * Display the preferences/options dialog.
     */
    static KAction *preferences(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0, const char *name = 0L );

    /**
     * Display the help menu.
     */
    static KAction *help(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );
};

#endif // KSTDACTION_H
