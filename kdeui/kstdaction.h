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
class QStringList;
class QWidget;
class KAction;
class KActionCollection;
class KRecentFilesAction;
class KToggleAction;
class KToggleToolBarAction;
class KToggleFullScreenAction;

#include <kdelibs_export.h>

/**
 * Convenience methods to access all standard KDE actions.
 *
 * These actions should be used instead of hardcoding menubar and
 * toolbar items.  Using these actions helps your application easily
 * conform to the KDE UI Style Guide
 * @see http://developer.kde.org/documentation/standards/kde/style/basics/index.html .
 *
 * All of the documentation for KAction holds for KStdAction
 * also.  When in doubt on how things work, check the KAction
 * documention first.
 *
 * <b>Simple Example:</b>\n
 *
 * In general, using standard actions should be a drop in replacement
 * for regular actions.  For example, if you previously had:
 *
 * \code
 * KAction *newAct = new KAction(i18n("&New"), QIconSet(BarIcon("filenew")),
 *                               KStdAccel::shortcut(KStdAccel::New), this,
 *                               SLOT(fileNew()), actionCollection());
 * \endcode
 *
 * You could drop that and replace it with:
 *
 * \code
 * KAction *newAct = KStdAction::openNew(this, SLOT(fileNew()),
 *                                       actionCollection());
 * \endcode
 *
 * <b>Non-standard Usages</b>\n
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
 * names of each of the standard actions by using the stdName
 * action like so: KStdAction::stdName(KStdAction::Cut) would return
 * 'edit_cut'.  The XML building code will match 'edit_cut' to the
 * attribute in the global XML file and place your action there.
 *
 * However, you can change the internal name.  In this example, just
 * do something like:
 *
 * \code
 * (void)KStdAction::cut(this, SLOT(editCut()), actionCollection(), "my_cut");
 * \endcode
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
 * \code
 * KAction *cut = actionCollection()->action(KStdAction::stdName(KStdAction::Cut));
 * \endcode
 *
 * Another non-standard usage concerns instantiating the action in the
 * first place.  Usually, you would use the member functions as
 * shown above (e.g., KStdAction::cut(this, SLOT, parent)).  You
 * may, however, do this using the enums provided.  This author can't
 * think of a reason why you would want to, but, hey, if you do,
 * here's how:
 *
 * \code
 * (void)KStdAction::action(KStdAction::New, this, SLOT(fileNew()), actionCollection());
 * (void)KStdAction::action(KStdAction::Cut, this, SLOT(editCut()), actionCollection());
 * \endcode
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
		ShowMenubar, ShowToolbar, ShowStatusbar,
		SaveOptions, KeyBindings,
		Preferences, ConfigureToolbars,

		// Help Menu
		Help, HelpContents, WhatsThis, ReportBug, AboutApp, AboutKDE,
		TipofDay, ///< @since 3.1

		// Another settings menu item
		ConfigureNotifications,
		FullScreen, ///< @since 3.2
		Clear, ///< @since 3.2
        PasteText ///< @since 3.2
	};

	/**
	 * Creates an action corresponding to the
	 * KStdAction::StdAction enum.
	 */
	KDEUI_EXPORT KAction* create( StdAction id, const char *name,
		const QObject *recvr, const char *slot,
		KActionCollection* parent );

	inline KAction* create( StdAction id,
		const QObject *recvr, const char *slot,
		KActionCollection* parent )
		{ return KStdAction::create( id, 0, recvr, slot, parent ); }

	/**
	* @obsolete. Creates an action corresponding to the
	* KStdAction::StdAction enum.
	*/
	inline KAction *action(StdAction act_enum,
		const QObject *recvr, const char *slot,
		KActionCollection *parent, const char *name = 0L )
		{ return KStdAction::create( act_enum, name, recvr, slot, parent ); }

	/**
	 * This will return the internal name of a given standard action.
	 */
	KDEUI_EXPORT const char* name( StdAction id );

        /// @obsolete. Use name()
	inline const char* stdName(StdAction act_enum) { return name( act_enum ); }

       /**
         * Returns a list of all standard names. Used by KAccelManager
         * to give those heigher weight.
	 * @since 3.1
        */
        KDEUI_EXPORT QStringList stdNames();

	/**
	 * Create a new document or window.
	 */
	KDEUI_EXPORT KAction *openNew(const QObject *recvr, const char *slot, KActionCollection* parent, const char *name = 0 );

	/**
	 * Open an existing file.
	 */
	KDEUI_EXPORT KAction *open(const QObject *recvr, const char *slot, KActionCollection* parent, const char *name = 0 );

	/**
	 * Open a recently used document. The signature of the slot being called
	 * is of the form slotURLSelected( const KURL & ).
	 * @param recvr object to receive slot
	 * @param slot The SLOT to invoke when a URL is selected. The slot's
	 * signature is slotURLSelected( const KURL & ).
	 * @param parent parent widget
	 * @param name name of widget
	 */
	KDEUI_EXPORT KRecentFilesAction *openRecent(const QObject *recvr, const char *slot, KActionCollection* parent, const char *name = 0 );

	/**
	 * Save the current document.
	 */
	KDEUI_EXPORT KAction *save(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Save the current document under a different name.
	*/
	KDEUI_EXPORT KAction *saveAs(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Revert the current document to the last saved version
	* (essentially will undo all changes).
	*/
	KDEUI_EXPORT KAction *revert(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Close the current document.
	*/
	KDEUI_EXPORT KAction *close(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Print the current document.
	*/
	KDEUI_EXPORT KAction *print(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Show a print preview of the current document.
	*/
	KDEUI_EXPORT KAction *printPreview(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Mail this document.
	*/
	KDEUI_EXPORT KAction *mail(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Quit the program.
	*/
	KDEUI_EXPORT KAction *quit(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Undo the last operation.
	*/
	KDEUI_EXPORT KAction *undo(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Redo the last operation.
	*/
	KDEUI_EXPORT KAction *redo(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Cut selected area and store it in the clipboard.  Calls cut() on the widget with the current focus.
	*/
	KDEUI_EXPORT KAction *cut(KActionCollection* parent);
	
	/**
	* Copy selected area and store it in the clipboard.  Calls copy() on the widget with the current focus.
	*/
	KDEUI_EXPORT KAction *copy(KActionCollection* parent);
	
	/**
	* Paste the contents of clipboard at the current mouse or cursor
	* Calls paste() on the widget with the current focus.
	*/
	KDEUI_EXPORT KAction *paste(KActionCollection* parent);
 
	/**
	* Clear selected area.  Calls clear() on the widget with the current focus.
 	* Note that for some widgets, this may not provide the intended bahavior.  For
	* example if you make use of the code above and a KListView has the focus, clear()
	* will clear all of the items in the list.  If this is not the intened behavior
	* and you want to make use of this slot, you can subclass KListView and reimplement
	* this slot.  For example the following code would implement a KListView without this
	* behavior:
	*
	* \code
	* class MyListView : public KListView {
	*   Q_OBJECT
	* public:
	*   MyListView( QWidget * parent = 0, const char * name = 0, WFlags f = 0 ) : KListView( parent, name, f ) {}
	*   virtual ~MyListView() {}
	* public slots:
	*   virtual void clear() {}
	* };
	* \endcode
	*/
	KDEUI_EXPORT KAction *clear(KActionCollection* parent);

	/**
	* Calls selectAll() on the widget with the current focus.
	*/
	KDEUI_EXPORT KAction *selectAll(KActionCollection* parent);
	      
	/**
	* Cut selected area and store it in the clipboard.
	*/
	KDEUI_EXPORT KAction *cut(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Copy the selected area into the clipboard.
	*/
	KDEUI_EXPORT KAction *copy(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Paste the contents of clipboard at the current mouse or cursor
	* position.
	*/
	KDEUI_EXPORT KAction *paste(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Paste the contents of clipboard at the current mouse or cursor
	* position. Provide a button on the toolbar with the clipboard history
	* menu if Klipper is running.
	* @since 3.2
	*/
	KDEUI_EXPORT KAction *pasteText(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Clear the content of the focus widget
	* @since 3.2
	*/
	KDEUI_EXPORT KAction *clear(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Select all elements in the current document.
	*/
	KDEUI_EXPORT KAction *selectAll(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Deselect any selected elements in the current document.
	*/
	KDEUI_EXPORT KAction *deselect(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Initiate a 'find' request in the current document.
	*/
	KDEUI_EXPORT KAction *find(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Find the next instance of a stored 'find'.
	*/
	KDEUI_EXPORT KAction *findNext(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Find a previous instance of a stored 'find'.
	*/
	KDEUI_EXPORT KAction *findPrev(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Find and replace matches.
	*/
	KDEUI_EXPORT KAction *replace(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* View the document at its actual size.
	*/
	KDEUI_EXPORT KAction *actualSize(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the size of the current window.
	*/
	KDEUI_EXPORT KAction *fitToPage(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the width of the current window.
	*/
	KDEUI_EXPORT KAction *fitToWidth(const QObject *recvr, const char *slot,
		KActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the height of the current window.
	*/
	KDEUI_EXPORT KAction *fitToHeight(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Zoom in.
	*/
	KDEUI_EXPORT KAction *zoomIn(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Zoom out.
	*/
	KDEUI_EXPORT KAction *zoomOut(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Popup a zoom dialog.
	*/
	KDEUI_EXPORT KAction *zoom(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Redisplay or redraw the document.
	*/
	KDEUI_EXPORT KAction *redisplay(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Move up (web style menu).
	*/
	KDEUI_EXPORT KAction *up(const QObject *recvr, const char *slot,
			KActionCollection* parent, const char *name = 0 );

	/**
	* Move back (web style menu).
	*/
	KDEUI_EXPORT KAction *back(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Move forward (web style menu).
	*/
	KDEUI_EXPORT KAction *forward(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Go to the "Home" position or document.
	*/
	KDEUI_EXPORT KAction *home(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Scroll up one page.
	*/
	KDEUI_EXPORT KAction *prior(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Scroll down one page.
	*/
	KDEUI_EXPORT KAction *next(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Go to somewhere in general.
	*/
	KDEUI_EXPORT KAction *goTo(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );


	/**
	* Go to a specific page (dialog).
	*/
	KDEUI_EXPORT KAction *gotoPage(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Go to a specific line (dialog).
	*/
	KDEUI_EXPORT KAction *gotoLine(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Jump to the first page.
	*/
	KDEUI_EXPORT KAction *firstPage(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Jump to the last page.
	*/
	KDEUI_EXPORT KAction *lastPage(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Add the current page to the bookmarks tree.
	*/
	KDEUI_EXPORT KAction *addBookmark(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Edit the application bookmarks.
	*/
	KDEUI_EXPORT KAction *editBookmarks(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Pop up the spell checker.
	*/
	KDEUI_EXPORT KAction *spelling(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );


	/**
	* Show/Hide the menubar.
	*/
	KDEUI_EXPORT KToggleAction *showMenubar(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Show/Hide the statusbar.
	*/
	KDEUI_EXPORT KToggleAction *showStatusbar(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Switch to/from full screen mode
	* @since 3.2
	*/
	KDEUI_EXPORT KToggleFullScreenAction *fullScreen(const QObject *recvr, const char *slot,
					KActionCollection* parent, QWidget* window, const char *name = 0 );

	/**
	* Display the save options dialog.
	*/
	KDEUI_EXPORT KAction *saveOptions(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Display the configure key bindings dialog.
	*
	*  Note that you might be able to use the pre-built KXMLGUIFactory's fuction:
	*  KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), actionCollection());
        */
	KDEUI_EXPORT KAction *keyBindings(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Display the preferences/options dialog.
	*/
	KDEUI_EXPORT KAction *preferences(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* The Customize Toolbar dialog.
	*/
	KDEUI_EXPORT KAction *configureToolbars(const QObject *recvr,
					const char *slot,
					KActionCollection* parent,
					const char *name = 0 );

	/**
	* The Configure Notifications dialog.
	* @since 3.1
	*/
	KDEUI_EXPORT KAction *configureNotifications(const QObject *recvr,
					const char *slot,
					KActionCollection *parent,
					const char *name = 0);

	/**
	* Display the help.
	*/
	KDEUI_EXPORT KAction *help(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display the help contents.
	*/
	KDEUI_EXPORT KAction *helpContents(const QObject *recvr, const char *slot,
					KActionCollection* parent, const char *name = 0 );

	/**
	* Trigger the What's This cursor.
	*/
	KDEUI_EXPORT KAction *whatsThis(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display "Tip of the Day"
	* @since 3.1
	*/
	KDEUI_EXPORT KAction *tipOfDay(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Open up the Report Bug dialog.
	*/
	KDEUI_EXPORT KAction *reportBug(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display the application's About box.
	*/
	KDEUI_EXPORT KAction *aboutApp(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );

	/**
	* Display the About KDE dialog.
	*/
	KDEUI_EXPORT KAction *aboutKDE(const QObject *recvr, const char *slot,
				KActionCollection* parent, const char *name = 0 );
}

#endif // KSTDACTION_H
