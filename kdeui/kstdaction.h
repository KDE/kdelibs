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

class KRecentFilesAction;
class KToggleAction;
class KAction;
class QObject;

class KStdActionPrivate;

/**
 * Convenience methods to access all standard KDE actions.
 *
 * These actions should be used instead of hardcoding menubar and
 * toolbar items.  Using these actions helps your application easily
 * conform to the KDE UI Style Guide
 * @see http://developer.kde.org/documentation/standards/kde/style/basics/index.html .
 *
 * All of the documentation for @ref KAction holds for KStdAction
 * also.  When in doubt on how things work, check the @ref KAction
 * documention first.
 *
 * @sect Simple Example:
 *
 * In general, using standard actions should be a drop in replacement
 * for regular actions.  For example, if you previously had:
 *
 * <PRE>
 * KAction *newAct = new KAction(i18n("&New"), QIconSet(BarIcon("filenew")),
 *                               KStdAccel::key(KStdAccel::New), this,
 *                               SLOT(fileNew()), actionCollection());
 * </PRE>
 *
 * You could drop that and replace it with:
 *
 * <PRE>
 * KAction *newAct = KStdAction::openNew(this, SLOT(fileNew()),
 *                                       actionCollection());
 * </PRE>
 *
 * @sect Non-standard Usages
 *
 * It is possible to use the standard actions in various
 * non-recommended ways.  Say, for instance, you wanted to have a
 * standard action (with the associated correct text and icon and
 * accelerator, etc) but you didn't want it to go in the standard
 * place (this is not recommended, by the way).  One way to do this is
 * to simply not use the XML UI framework and plug it into wherever
 * you want.  If you do want to use the XML UI framework (good!), then
 * it is still possible.
 *
 * Basically, the XML building code matches names in the XML code with
 * the internal names of the actions.  You can find out the internal
 * names of each of the standard actions by using the @ref stdName
 * action like so: @ref KStdAction::stdName(KStdAction::Cut) would return
 * 'edit_cut'.  The XML building code will match 'edit_cut' to the
 * attribute in the global XML file and place your action there.
 *
 * However, you can change the internal name.  In this example, just
 * do something like:
 *
 * <PRE>
 * (void)KStdAction::cut(this, SLOT(editCut()), actionCollection(), "my_cut");
 * </PRE>
 *
 * Now, in your local XML resource file (e.g., yourappui.rc), simply
 * put 'my_cut' where you want it to go.
 *
 * Another non-standard usage concerns getting a pointer to an
 * existing action if, say, you want to enable or disable the action.
 * You could do it the recommended way and just grab a pointer when
 * you instantiate it as in the the 'openNew' example above... or you
 * could do it the hard way:
 *
 * <pre>
 * KAction *cut = actionCollection()->action(KStdAction::stdName(KStdAction::Cut));
 * </pre>
 *
 * Another non-standard usage concerns instantiating the action in the
 * first place.  Usually, you would use the static member functions as
 * shown above (e.g., KStdAction::cut(this, SLOT, parent)).  You
 * may, however, do this using the enums provided.  This author can't
 * think of a reason why you would want to, but, hey, if you do,
 * here's how:
 *
 * <pre>
 * (void)KStdAction::action(KStdAction::New, this, SLOT(fileNew()), actionCollection());
 * (void)KStdAction::action(KStdAction::Cut, this, SLOT(editCut()), actionCollection());
 * </pre>
 *
 * @author Kurt Granroth <granroth@kde.org>
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
        Print, PrintPreview, Mail, Quit,

        // Edit Menu
        Undo, Redo, Cut, Copy, Paste, SelectAll, Find, FindNext, FindPrev,
        Replace,

        // View Menu
        ActualSize, FitToPage, FitToWidth, FitToHeight, ZoomIn, ZoomOut,
        Zoom, Redisplay,

        // Go Menu
        Up, Back, Forward, Home, Prior, Next, Goto, GotoPage, GotoLine,
        FirstPage, LastPage,

        // Bookmarks Menu
        AddBookmark, EditBookmarks,

        // Tools Menu
        Spelling,

        // Settings Menu
        ShowMenubar, ShowToolbar, ShowStatusbar, SaveOptions, KeyBindings,
        Preferences, ConfigureToolbars,

        // Help Menu
        Help, HelpContents, WhatsThis, ReportBug, AboutApp, AboutKDE,

        // NULL entries for possible later use
        NULL1, NULL2, NULL3, NULL4, NULL5, NULL6, NULL7, NULL8, NULL9,
        NULL10, NULL11, NULL12, NULL13, NULL14, NULL15, NULL16, NULL17,
        NULL18, NULL19, NULL20
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
     * Creates an action corresponding to the
     * @ref KStdAction::StdAction enum.
     */
    static KAction *action(StdAction act_enum, const QObject *recvr = 0,
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
     * @param slot The SLOT to invoke when a URL is selected.
     * Its signature is of the form slotURLSelected( const KURL & ).
     */
    static KRecentFilesAction *openRecent(const QObject *recvr = 0,
                                          const char *slot = 0,
                                          QObject *parent = 0,
                                          const char *name = 0L );

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
     * Mail this document.
     */
    static KAction *mail(const QObject *recvr = 0, const char *slot = 0,
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
     * Go to somewhere in general.
     */
    static KAction *goTo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );


    /**
     * Go to a specific page (dialog).
     */
    static KAction *gotoPage(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

    /**
     * Go to a specific line (dialog).
     */
    static KAction *gotoLine(const QObject *recvr = 0, const char *slot = 0,
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
     * Pop up the spell checker.
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
     * Display the save options dialog.
     */
    static KAction *saveOptions(const QObject *recvr = 0, const char *slot = 0,
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
     * The Customize Toolbar dialog.
     */
    static KAction *configureToolbars(const QObject *recvr = 0,
                                      const char *slot = 0,
                                      QObject *parent = 0,
                                      const char *name = 0L );

    /**
     * Display the help.
     */
    static KAction *help(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Display the help contents.
     */
    static KAction *helpContents(const QObject *recvr = 0, const char *slot = 0,
                                 QObject *parent = 0, const char *name = 0L );

    /**
     * Trigger the What's This cursor.
     */
    static KAction *whatsThis(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0, const char *name = 0L );

    /**
     * Open up the Report Bug dialog.
     */
    static KAction *reportBug(const QObject *recvr = 0, const char *slot = 0,
                              QObject *parent = 0, const char *name = 0L );

    /**
     * Display the application's About box.
     */
    static KAction *aboutApp(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

    /**
     * Display the About KDE dialog.
     */
    static KAction *aboutKDE(const QObject *recvr = 0, const char *slot = 0,
                             QObject *parent = 0, const char *name = 0L );

private:
    KStdActionPrivate *d;
};

#endif // KSTDACTION_H
