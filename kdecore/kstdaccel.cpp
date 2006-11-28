/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)
    Copyright (C) 2000 Nicolas Hadacek (haadcek@kde.org)
    Copyright (C) 2001,2002 Ellis Whitehead (ellis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#define KSTDACCEL_CPP 1

#include "kstdaccel.h"

#include "kconfig.h"
#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "kshortcut.h"

#include <qkeysequence.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

namespace KStdAccel
{

struct KStdAccelInfo
{
	StdAccel id;
	const char* name;
	const char* description;
	int cutDefault, cutDefault2;
	KShortcut cut;
	bool isInitialized;
};

#define CTRL(x) Qt::CTRL+Qt::Key_##x
#define SHIFT(x) Qt::SHIFT+Qt::Key_##x
#define CTRLSHIFT(x) Qt::CTRL+Qt::SHIFT+Qt::Key_##x
#define ALT(x) Qt::ALT+Qt::Key_##x

/** Array of predefined KStdAccelInfo objects, which cover all
    the "standard" accelerators. Each enum value from StdAccel
    should appear in this table.
*/
// STUFF WILL BREAK IF YOU DON'T READ THIS!!!
// Read the comments of the big enum in kstdaccel.h before you change anything!
static KStdAccelInfo g_infoStdAccel[] =
{
//Group File,
	{AccelNone,            0, 0, 0, 0, KShortcut(), false },
	{ Open,                I18N_NOOP("Open"),  0,    CTRL(O), 0, KShortcut(), false },
	{ New,                 I18N_NOOP("New"),   0,    CTRL(N), 0, KShortcut(), false },
	{ Close,               I18N_NOOP("Close"), 0,    CTRL(W), CTRL(Escape), KShortcut(), false },
	{ Save,                I18N_NOOP("Save"),  0,    CTRL(S), 0, KShortcut(), false },
	{ Print,               I18N_NOOP("Print"), 0,    CTRL(P), 0, KShortcut(), false },
	{ Quit,                I18N_NOOP("Quit"),  0,    CTRL(Q), 0, KShortcut(), false },

//Group Edit
	{ Undo,                I18N_NOOP("Undo"),  0,           CTRL(Z), 0, KShortcut(), false },
	{ Redo,                I18N_NOOP("Redo"),  0,           CTRLSHIFT(Z), 0, KShortcut(), false },
	{ Cut,                 I18N_NOOP("Cut"),   0,           CTRL(X), SHIFT(Delete), KShortcut(), false },
	{ Copy,                I18N_NOOP("Copy"),  0,           CTRL(C), CTRL(Insert), KShortcut(), false },
	{ Paste,               I18N_NOOP("Paste"), 0,           CTRL(V), SHIFT(Insert), KShortcut(), false },
	{ PasteSelection,      I18N_NOOP("Paste Selection"),    0, CTRLSHIFT(Insert), 0, KShortcut(), false },

	{ SelectAll,           "SelectAll", I18N_NOOP("Select All"), CTRL(A), 0, KShortcut(), false },
	{ Deselect,            I18N_NOOP("Deselect"), 0, CTRLSHIFT(A), 0, KShortcut(), false },
	{ DeleteWordBack,      "DeleteWordBack", I18N_NOOP("Delete Word Backwards"), CTRL(Backspace), 0, KShortcut(), false },
	{ DeleteWordForward,   "DeleteWordForward", I18N_NOOP("Delete Word Forward"),CTRL(Delete), 0,  KShortcut(), false },

	{ Find,                I18N_NOOP("Find"),  0,     CTRL(F), 0, KShortcut(), false },
	{ FindNext,            "FindNext", I18N_NOOP("Find Next"), Qt::Key_F3, 0, KShortcut(), false },
	{ FindPrev,            "FindPrev", I18N_NOOP("Find Prev"), SHIFT(F3), 0, KShortcut(), false },
	{ Replace,             I18N_NOOP("Replace"), 0,  CTRL(R), 0, KShortcut(), false },

//Group Navigation
	{ Home,                I18N_NOOP2("Opposite to End","Home"), 0, CTRL(Home), Qt::Key_HomePage, KShortcut(), false },
	{ End,                 I18N_NOOP("End"), 0,      CTRL(End), 0, KShortcut(), false },
	{ Prior,               I18N_NOOP("Prior"), 0,    Qt::Key_PageUp, 0,KShortcut(), false },
	{ Next,                I18N_NOOP2("Opposite to Prior","Next"), 0, Qt::Key_PageDown, 0, KShortcut(), false },

	{ Up,                  I18N_NOOP("Up"), 0,       ALT(Up), 0, KShortcut(), false },
	{ Back,                I18N_NOOP("Back"), 0,     ALT(Left), Qt::Key_Back, KShortcut(), false },
	{ Forward,             I18N_NOOP("Forward"), 0,  ALT(Right), Qt::Key_Forward, KShortcut(), false },
	{ Reload,              I18N_NOOP("Reload"), 0,   Qt::Key_F5, Qt::Key_Refresh, KShortcut(), false },

	{ BeginningOfLine,     "BeginningOfLine", I18N_NOOP("Beginning of Line"), Qt::Key_Home, 0, KShortcut(), false},
	{ EndOfLine,           "EndOfLine", I18N_NOOP("End of Line"), Qt::Key_End, 0, KShortcut(), false},
	{ GotoLine,            "GotoLine", I18N_NOOP("Go to Line"), CTRL(G), 0, KShortcut(), false },
	{ BackwardWord,        "BackwardWord", I18N_NOOP("Backward Word"), CTRL(Left), 0, KShortcut(), false },
	{ ForwardWord,         "ForwardWord", I18N_NOOP("Forward Word"), CTRL(Right), 0, KShortcut(), false },

	{ AddBookmark,         "AddBookmark", I18N_NOOP("Add Bookmark"), CTRL(B), 0, KShortcut(), false },
	{ ZoomIn,              "ZoomIn", I18N_NOOP("Zoom In"), CTRL(Plus), 0, KShortcut(), false },
	{ ZoomOut,             "ZoomOut", I18N_NOOP("Zoom Out"), CTRL(Minus), 0, KShortcut(), false },
	{ FullScreen,          "FullScreen", I18N_NOOP("Full Screen Mode"), CTRLSHIFT(F), 0, KShortcut(), false },

	{ PopupMenuContext,    "PopupMenuContext", I18N_NOOP("Popup Menu Context"), Qt::Key_Menu, 0, KShortcut(), false },
	{ ShowMenubar,         "ShowMenubar", I18N_NOOP("Show Menu Bar"),CTRL(M), 0, KShortcut(), false },
	{ TabNext,             I18N_NOOP("Activate Next Tab"), 0, CTRL(Period), CTRL(BracketRight), KShortcut(), false },
	{ TabPrev,             I18N_NOOP("Activate Previous Tab"), 0, CTRL(Comma), CTRL(BracketLeft), KShortcut(), false },

//Group Help
	{ Help,                I18N_NOOP("Help"), 0,                        Qt::Key_F1, 0, KShortcut(), false },
	{ WhatsThis,           "WhatsThis",       I18N_NOOP("What's This"), SHIFT(F1),  0, KShortcut(), false },

//Group TextCompletion
	{ TextCompletion,      "TextCompletion",       I18N_NOOP("Text Completion"),           CTRL(E),    0, KShortcut(), false },
	{ PrevCompletion,      "PrevCompletion",       I18N_NOOP("Previous Completion Match"), CTRL(Up),   0, KShortcut(), false },
	{ NextCompletion,      "NextCompletion",       I18N_NOOP("Next Completion Match"),     CTRL(Down), 0, KShortcut(), false },
	{ SubstringCompletion, "SubstringCompletion",  I18N_NOOP("Substring Completion"),      CTRL(T),    0, KShortcut(), false },

	{ RotateUp,            "RotateUp",   I18N_NOOP("Previous Item in List"), Qt::Key_Up,   0, KShortcut(), false },
	{ RotateDown,          "RotateDown", I18N_NOOP("Next Item in List"),     Qt::Key_Down, 0, KShortcut(), false },

//dummy entry to catch simple off-by-one errors. Insert new entries before this line.
	{ AccelNone,            0, 0, 0, 0, KShortcut(), false }
};


/** Search for the KStdAccelInfo object associated with the given @p id.
    Return a dummy entry with no name and an empty shortcut if @p id is invalid.
*/
static KStdAccelInfo *guardedStdAccelInfo( StdAccel id )
{
	if ( id >= static_cast<int>(sizeof(g_infoStdAccel) / sizeof(KStdAccelInfo)) ||
             id < 0  ) {
		kWarning(125) << "KStdAccel: id not found!" << endl;
		return &g_infoStdAccel[AccelNone];
	} else
		return &g_infoStdAccel[id];
}

/** Initialize the accelerator @p id by checking if it is overridden
    in the configuration file (and if it isn't, use the default).
    On X11, if QApplication was initialized with GUI disabled,
    the default will always be used.
*/
static void initialize( StdAccel id )
{
	KStdAccelInfo *info = guardedStdAccelInfo( id );

	KConfigGroup cg( KGlobal::config(), "Shortcuts" );

#ifdef Q_WS_X11
	// Code within this block breaks if we aren't running in GUI mode.
	if( QX11Info::display() && cg.hasKey( info->name ) ) {
#else
	if( cg.hasKey( info->name ) ) {
#endif
		QString s = cg.readEntry( info->name );
		if( s != "none" )
			info->cut = KShortcut( s );
		else
			info->cut.clear();
	} else
		info->cut = shortcutDefault( id );

	info->isInitialized = true;
}

void saveShortcut(StdAccel id, const KShortcut &newShortcut)
{
	KStdAccelInfo *info = guardedStdAccelInfo( id );
	if( info->id == AccelNone )
		return;

	KConfigGroup cg( KGlobal::config(), "Shortcuts" );

	info->cut = newShortcut;
	bool sameAsDefault = (newShortcut == shortcutDefault( id ));

	if (sameAsDefault)
		if( cg.hasKey( info->name ) )
			cg.deleteEntry( info->name );
		else
			return;

	cg.writeEntry( info->name, info->cut.toStringInternal() );
}

QString name( StdAccel id )
{
	return guardedStdAccelInfo( id )->name;
}

QString label( StdAccel id )
{
	KStdAccelInfo *info = guardedStdAccelInfo( id );
	return i18n((info->description) ? info->description : info->name);
}

// TODO: Add psWhatsThis entry to KStdAccelInfo
QString whatsThis( StdAccel /*id*/ )
{
//	KStdAccelInfo* info = guardedStdAccelInfo( id );
//	if( info && info->whatsThis )
//		return i18n(info->whatsThis);
//	else
		return QString();
}

const KShortcut &shortcut( StdAccel id )
{
	KStdAccelInfo *info = guardedStdAccelInfo( id );

	if( !info->isInitialized )
		initialize( id );

	return info->cut;
}

StdAccel findStdAccel( const QKeySequence &seq )
{
	if( !seq.isEmpty() ) {
		for( uint i = 0; i < sizeof(g_infoStdAccel) / sizeof(KStdAccelInfo); i++ ) {
			StdAccel id = g_infoStdAccel[i].id;
			if( id != AccelNone ) {
				if( !g_infoStdAccel[i].isInitialized )
					initialize( id );
				if( g_infoStdAccel[i].cut.contains( seq ) )
					return id;
			}
		}
	}
	return AccelNone;
}

StdAccel findStdAccel( const char *keyName )
{
	for( uint i = 0; i < sizeof(g_infoStdAccel) / sizeof(KStdAccelInfo); i++ )
		if (qstrcmp(g_infoStdAccel[i].name, keyName))
			return g_infoStdAccel[i].id;

	return AccelNone;
}

KShortcut shortcutDefault( StdAccel id )
{
	KShortcut cut;
	KStdAccelInfo *info = guardedStdAccelInfo( id );

	return KShortcut( info->cutDefault, info->cutDefault2 );
}

const KShortcut& open()                  { return shortcut( Open ); }
const KShortcut& openNew()               { return shortcut( New ); }
const KShortcut& close()                 { return shortcut( Close ); }
const KShortcut& save()                  { return shortcut( Save ); }
const KShortcut& print()                 { return shortcut( Print ); }
const KShortcut& quit()                  { return shortcut( Quit ); }
const KShortcut& cut()                   { return shortcut( Cut ); }
const KShortcut& copy()                  { return shortcut( Copy ); }
const KShortcut& paste()                 { return shortcut( Paste ); }
const KShortcut& pasteSelection()        { return shortcut( PasteSelection ); }
const KShortcut& deleteWordBack()        { return shortcut( DeleteWordBack ); }
const KShortcut& deleteWordForward()     { return shortcut( DeleteWordForward ); }
const KShortcut& undo()                  { return shortcut( Undo ); }
const KShortcut& redo()                  { return shortcut( Redo ); }
const KShortcut& find()                  { return shortcut( Find ); }
const KShortcut& findNext()              { return shortcut( FindNext ); }
const KShortcut& findPrev()              { return shortcut( FindPrev ); }
const KShortcut& replace()               { return shortcut( Replace ); }
const KShortcut& home()                  { return shortcut( Home ); }
const KShortcut& end()                   { return shortcut( End ); }
const KShortcut& beginningOfLine()       { return shortcut( BeginningOfLine ); }
const KShortcut& endOfLine()             { return shortcut( EndOfLine ); }
const KShortcut& prior()                 { return shortcut( Prior ); }
const KShortcut& next()                  { return shortcut( Next ); }
const KShortcut& backwardWord()          { return shortcut( BackwardWord ); }
const KShortcut& forwardWord()           { return shortcut( ForwardWord ); }
const KShortcut& gotoLine()              { return shortcut( GotoLine ); }
const KShortcut& addBookmark()           { return shortcut( AddBookmark ); }
const KShortcut& tabNext()               { return shortcut( TabNext ); }
const KShortcut& tabPrev()               { return shortcut( TabPrev ); }
const KShortcut& fullScreen()            { return shortcut( FullScreen ); }
const KShortcut& zoomIn()                { return shortcut( ZoomIn ); }
const KShortcut& zoomOut()               { return shortcut( ZoomOut ); }
const KShortcut& help()                  { return shortcut( Help ); }
const KShortcut& completion()            { return shortcut( TextCompletion ); }
const KShortcut& prevCompletion()        { return shortcut( PrevCompletion ); }
const KShortcut& nextCompletion()        { return shortcut( NextCompletion ); }
const KShortcut& rotateUp()              { return shortcut( RotateUp ); }
const KShortcut& rotateDown()            { return shortcut( RotateDown ); }
const KShortcut& substringCompletion()   { return shortcut( SubstringCompletion ); }
const KShortcut& popupMenuContext()      { return shortcut( PopupMenuContext ); }
const KShortcut& whatsThis()             { return shortcut( WhatsThis ); }
const KShortcut& reload()                { return shortcut( Reload ); }
const KShortcut& selectAll()             { return shortcut( SelectAll ); }
const KShortcut& up()                    { return shortcut( Up ); }
const KShortcut& back()                  { return shortcut( Back ); }
const KShortcut& forward()               { return shortcut( Forward ); }
const KShortcut& showMenubar()           { return shortcut( ShowMenubar ); }

}

#undef KSTDACCEL_CPP
