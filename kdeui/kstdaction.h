/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

class QObject;
class KAction;
class KActionCollection;
class KRecentFilesAction;
class KToggleAction;
class KToggleToolBarAction;

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
 * first place.  Usually, you would use the member functions as
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
namespace KStdAction
{
	/**
	 * The standard menubar and toolbar actions.
	 */
	enum StdAction {
		ActionNone,

		// File Menu
		New, Open, OpenRecent, Save, SaveAs, Revert, Close,
		Print, PrintPreview, Mail, Quit,

		// Edit Menu
		Undo, Redo, Cut, Copy, Paste, SelectAll, Deselect, Find, FindNext, FindPrev,
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
		Help, HelpContents, WhatsThis, TipofDay, ReportBug, AboutApp, AboutKDE
	};

	/**
	 * Creates an action corresponding to the
	 * @ref KStdAction::StdAction enum.
	 */
	KAction* create( StdAction id, const char *name,
		const QObject *recvr, const char *slot,
		KActionCollection* parent );

	inline KAction* create( StdAction id,
		const QObject *recvr, const char *slot,
		KActionCollection* parent )
		{ return KStdAction::create( id, 0, recvr, slot, parent ); }

	/**
	* @obsolete. Creates an action corresponding to the
	* @ref KStdAction::StdAction enum.
	*/
	inline KAction *action(StdAction act_enum,
		const QObject *recvr, const char *slot,
		KActionCollection *parent, const char *name = 0L )
		{ return KStdAction::create( act_enum, name, recvr, slot, parent ); }

	/**
	 * This will return the internal name of a given standard action.
	 */
	const char* name( StdAction id );
	inline const char* stdName(StdAction act_enum) { return name( act_enum ); }

	/**
	 * Create a new document or window.
	 */
	KAction *openNew(const QObject *recvr, const char *slot, KActionCollection* parent, const char *name = 0 );

	/**
	 * Open an existing file.
	 */
	KAction *open(const QObject *recvr, const char *slot, KActionCollection* parent, const char *name = 0 );

	/**
	 * Open a recently used document.
	 * @param slot The SLOT to invoke when a URL is selected.
	 * Its signature is of the form slotURLSelected( const KURL & ).
	 */
	KRecentFilesAction *openRecent(const QObject *recvr, const char *slot, KActionCollection* parent, const char *name = 0 );

	/**
	 * Save the current document.
	 */
	KAction *save(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Save the current document under a different name.
	*/
	KAction *saveAs(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Revert the current document to the last saved version
	* (essentially will undo all changes).
	*/
	KAction *revert(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Close the current document.
	*/
	KAction *close(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Print the current document.
	*/
	KAction *print(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Show a print preview of the current document.
	*/
	KAction *printPreview(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Mail this document.
	*/
	KAction *mail(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Quit the program.
	*/
	KAction *quit(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Undo the last operation.
	*/
	KAction *undo(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Redo the last operation.
	*/
	KAction *redo(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Cut selected area and store it in the clipboard.
	*/
	KAction *cut(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Copy the selected area into the clipboard.
	*/
	KAction *copy(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Paste the contents of clipboard at the current mouse or cursor
	* position.
	*/
	KAction *paste(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Select all elements in the current document.
	*/
	KAction *selectAll(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Deselect any selected elements in the current document.
	*/
	KAction *deselect(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Initiate a 'find' request in the current document.
	*/
	KAction *find(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Find the next instance of a stored 'find'.
	*/
	KAction *findNext(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Find a previous instance of a stored 'find'.
	*/
	KAction *findPrev(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Find and replace matches.
	*/
	KAction *replace(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* View the document at its actual size.
	*/
	KAction *actualSize(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the size of the current window.
	*/
	KAction *fitToPage(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the width of the current window.
	*/
	KAction *fitToWidth(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the height of the current window.
	*/
	KAction *fitToHeight(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Zoom in.
	*/
	KAction *zoomIn(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Zoom out.
	*/
	KAction *zoomOut(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Popup a zoom dialog.
	*/
	KAction *zoom(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Redisplay or redraw the document.
	*/
	KAction *redisplay(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Move up (web style menu).
	*/
	KAction *up(const QObject *recvr, const char *slot,
			KActionCollection* parent, const char *name = 0 );

	/**
	* Move back (web style menu).
	*/
	KAction *back(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Move forward (web style menu).
	*/
	KAction *forward(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Go to the "Home" position or document.
	*/
	KAction *home(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Scroll up one page.
	*/
	KAction *prior(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Scroll down one page.
	*/
	KAction *next(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Go to somewhere in general.
	*/
	KAction *goTo(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );


	/**
	* Go to a specific page (dialog).
	*/
	KAction *gotoPage(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Go to a specific line (dialog).
	*/
	KAction *gotoLine(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Jump to the first page.
	*/
	KAction *firstPage(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Jump to the last page.
	*/
	KAction *lastPage(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Add the current page to the bookmarks tree.
	*/
	KAction *addBookmark(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Edit the application bookmarks.
	*/
	KAction *editBookmarks(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Pop up the spell checker.
	*/
	KAction *spelling(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );


	/**
	* Show/Hide the menubar.
	*/
	KToggleAction *showMenubar(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* @obsolete. toolbar actions are created automatically now in the Settings menu. Don't use this anymore
	* Show/Hide the primary toolbar.
	*/
	KToggleAction *showToolbar(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );
	/**
	* @obsolete. toolbar actions are created automatically now in the Settings menu. Don't use this anymore
	* Show/Hide the primary toolbar.
	*/
	KToggleToolBarAction *showToolbar(const char* toolBarName,
					KActionCollection* parent, const char *name = 0 );
	
	/**
	* Show/Hide the statusbar.
	*/
	KToggleAction *showStatusbar(const QObject *recvr, const char *slot,
						KActionCollection* parent, const char *name = 0 );

	/**
	* Display the save options dialog.
	*/
	KAction *saveOptions(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Display the configure key bindings dialog.
	*/
	KAction *keyBindings(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Display the preferences/options dialog.
	*/
	KAction *preferences(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* The Customize Toolbar dialog.
	*/
	KAction *configureToolbars(const QObject *recvr,
					const char *slot,
					KActionCollection* parent,
					const char *name = 0 );

	/**
	* Display the help.
	*/
	KAction *help(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display the help contents.
	*/
	KAction *helpContents(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Trigger the What's This cursor.
	*/
	KAction *whatsThis(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display "Tip of the Day"
	*/
	KAction *tipOfDay(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Open up the Report Bug dialog.
	*/
	KAction *reportBug(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display the application's About box.
	*/
	KAction *aboutApp(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display the About KDE dialog.
	*/
	KAction *aboutKDE(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );
};

#endif // KSTDACTION_H
