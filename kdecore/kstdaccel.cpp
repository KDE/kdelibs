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

#include "kaccelaction.h"
#include "kaccelbase.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "kshortcut.h"
#include "kshortcutlist.h"

#include <qkeysequence.h>

namespace KStdAccel
{

struct KStdAccelInfo
{
	StdAccel id;
	const char* psName;
	const char* psDesc;
	int cutDefault, cutDefault2;
	KShortcut cut;
	bool bInitialized;
};

#define CTRL(x) Qt::CTRL+Qt::Key_##x
#define SHIFT(x) Qt::SHIFT+Qt::Key_##x
#define CTRLSHIFT(x) Qt::CTRL+Qt::SHIFT+Qt::Key_##x
#define ALT(x) Qt::ALT+Qt::Key_##x

/** Array of predefined KStdAccelInfo objects, which cover all
    the "standard" accelerators. Each enum value from StdAccel
    should appear in this table.
*/
static KStdAccelInfo g_infoStdAccel[] =
{
	{AccelNone,            "Group:File", I18N_NOOP("File"), 0, 0, KShortcut(), false },
	{ Open,                I18N_NOOP("Open"),  0,    CTRL(O), 0, KShortcut(), false },
	{ New,                 I18N_NOOP("New"),   0,    CTRL(N), 0, KShortcut(), false },
	{ Close,               I18N_NOOP("Close"), 0,    CTRL(W), CTRL(Escape), KShortcut(), false },
	{ Save,                I18N_NOOP("Save"),  0,    CTRL(S), 0, KShortcut(), false },
	{ Print,               I18N_NOOP("Print"), 0,    CTRL(P), 0, KShortcut(), false },
	{ Quit,                I18N_NOOP("Quit"),  0,    CTRL(Q), 0, KShortcut(), false },

	{AccelNone,            "Group:Edit", I18N_NOOP("Edit"), 0, 0, KShortcut(), false },
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
	
	{AccelNone,            "Group:Navigation", I18N_NOOP("Navigation"), 0, 0, KShortcut(), false },
	{ Home,                I18N_NOOP2("Opposite to End","Home"), 0, CTRL(Home), Qt::Key_HomePage, KShortcut(), false },
	{ End,                 I18N_NOOP("End"), 0,      CTRL(End), 0, KShortcut(), false },
	{ BeginningOfLine,     "BeginningOfLine", I18N_NOOP("Beginning of Line"), Qt::Key_Home, 0, KShortcut(), false},
	{ EndOfLine,           "EndOfLine", I18N_NOOP("End of Line"), Qt::Key_End, 0, KShortcut(), false},
	{ Prior,               I18N_NOOP("Prior"), 0,    Qt::Key_PageUp, 0,KShortcut(), false },
	{ Next,                I18N_NOOP2("Opposite to Prior","Next"), 0, Qt::Key_PageDown, 0, KShortcut(), false },

	{ GotoLine,            "GotoLine", I18N_NOOP("Go to Line"), CTRL(G), 0, KShortcut(), false },
	{ AddBookmark,         "AddBookmark", I18N_NOOP("Add Bookmark"), CTRL(B), 0, KShortcut(), false },
	{ ZoomIn,              "ZoomIn", I18N_NOOP("Zoom In"), CTRL(Plus), 0, KShortcut(), false },
	{ ZoomOut,             "ZoomOut", I18N_NOOP("Zoom Out"), CTRL(Minus), 0, KShortcut(), false },

	{ Up,                  I18N_NOOP("Up"), 0,       ALT(Up), 0, KShortcut(), false },
	{ Back,                I18N_NOOP("Back"), 0,     ALT(Left), Qt::Key_Back, KShortcut(), false },
	{ Forward,             I18N_NOOP("Forward"), 0,  ALT(Right), Qt::Key_Forward, KShortcut(), false },
	{ Reload,              I18N_NOOP("Reload"), 0,   Qt::Key_F5, Qt::Key_Refresh, KShortcut(), false },
	
	{ PopupMenuContext,    "PopupMenuContext", I18N_NOOP("Popup Menu Context"), Qt::Key_Menu, 0, KShortcut(), false },
	{ ShowMenubar,         "ShowMenubar", I18N_NOOP("Show Menu Bar"),CTRL(M), 0, KShortcut(), false },
	{ BackwardWord,        "BackwardWord", I18N_NOOP("Backward Word"), CTRL(Left), 0, KShortcut(), false },
	{ ForwardWord,         "ForwardWord", I18N_NOOP("Forward Word"), CTRL(Right), 0, KShortcut(), false },
	{ TabNext,             I18N_NOOP("Activate Next Tab"), 0, CTRL(Period), CTRL(BracketRight), KShortcut(), false },
	{ TabPrev,             I18N_NOOP("Activate Previous Tab"), 0, CTRL(Comma), CTRL(BracketLeft), KShortcut(), false },
	{ FullScreen,          "FullScreen", I18N_NOOP("Full Screen Mode"), CTRLSHIFT(F), 0, KShortcut(), false },

	{AccelNone,            "Group:Help",      I18N_NOOP("Help"),        0,          0, KShortcut(), false },
	{ Help,                I18N_NOOP("Help"), 0,                        Qt::Key_F1, 0, KShortcut(), false },
	{ WhatsThis,           "WhatsThis",       I18N_NOOP("What's This"), SHIFT(F1),  0, KShortcut(), false },

	{AccelNone,            "Group:TextCompletion", I18N_NOOP("Text Completion"),           0,          0, KShortcut(), false },
	{ TextCompletion,      "TextCompletion",       I18N_NOOP("Text Completion"),           CTRL(E),    0, KShortcut(), false },
	{ PrevCompletion,      "PrevCompletion",       I18N_NOOP("Previous Completion Match"), CTRL(Up),   0, KShortcut(), false },
	{ NextCompletion,      "NextCompletion",       I18N_NOOP("Next Completion Match"),     CTRL(Down), 0, KShortcut(), false },
	{ SubstringCompletion, "SubstringCompletion",  I18N_NOOP("Substring Completion"),      CTRL(T),    0, KShortcut(), false },
	
	{ RotateUp,            "RotateUp",   I18N_NOOP("Previous Item in List"), Qt::Key_Up,   0, KShortcut(), false },
	{ RotateDown,          "RotateDown", I18N_NOOP("Next Item in List"),     Qt::Key_Down, 0, KShortcut(), false },
	{ AccelNone,            0, 0, 0, 0, KShortcut(), false }
};

/** Search for the KStdAccelInfo object associated with the given @p id. */
static KStdAccelInfo* infoPtr( StdAccel id )
{
	if( id != AccelNone ) {
		// Linear search. Changing the data structure doesn't seem possible
		// (since we need groups for the config stuff), but maybe a little
		// additional hashtable wouldn't hurt.
		for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
			if( g_infoStdAccel[i].id == id )
				return &g_infoStdAccel[i];
		}
	}
	return 0;
}

/** Initialize the accelerator @p id by checking if it is overridden
    in the configuration file (and if it isn't, use the default).
*/
static void initialize( StdAccel id )
{
	KConfigGroup cg( KGlobal::config(), "Shortcuts" );
	KStdAccelInfo* pInfo = infoPtr( id );

	if( !pInfo ) {
		kWarning(125) << "KStdAccel: id not found!" << endl; // -- ellis
		return;
	}

	if( cg.hasKey( pInfo->psName ) ) {
		QString s = cg.readEntry( pInfo->psName );
		if( s != "none" )
			pInfo->cut.init( s );
		else
			pInfo->cut.clear();
	} else
		pInfo->cut = shortcutDefault( id );
	pInfo->bInitialized = true;
}

QString name( StdAccel id )
{
	KStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return QString();
	return pInfo->psName;
}

QString label( StdAccel id )
{
	KStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return QString();
	return i18n((pInfo->psDesc) ? pInfo->psDesc : pInfo->psName);
}

// TODO: Add psWhatsThis entry to KStdAccelInfo
QString whatsThis( StdAccel /*id*/ )
{
//	KStdAccelInfo* pInfo = infoPtr( id );
//	if( pInfo && pInfo->psWhatsThis )
//		return i18n(pInfo->psWhatsThis);
//	else
		return QString();
}

const KShortcut& shortcut( StdAccel id )
{
	KStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return KShortcut::null();

	if( !pInfo->bInitialized )
		initialize( id );

	return pInfo->cut;
}

StdAccel findStdAccel( const KKeySequence& seq )
{
	if( !seq.isNull() ) {
		for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
			StdAccel id = g_infoStdAccel[i].id;
			if( id != AccelNone ) {
				if( !g_infoStdAccel[i].bInitialized )
					initialize( id );
				if( g_infoStdAccel[i].cut.contains( seq ) )
					return id;
			}
		}
	}
	return AccelNone;
}

KShortcut shortcutDefault( StdAccel id )
{
	KShortcut cut;

	KStdAccelInfo* pInfo = infoPtr( id );
	if( pInfo ) {
		KStdAccelInfo& info = *pInfo;
		KKeySequence key2;

		cut.init( (info.cutDefault) ) ;

		if( info.cutDefault2 )
			key2.init( QKeySequence(info.cutDefault2) );

		if( key2.count() )
			cut.append( key2 );
	}

	return cut;
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

//---------------------------------------------------------------------
// ShortcutList
//---------------------------------------------------------------------

ShortcutList::ShortcutList()
	{ }

ShortcutList::~ShortcutList()
	{ }

uint ShortcutList::count() const
{
	static uint g_nAccels = 0;
	if( g_nAccels == 0 ) {
		for( ; g_infoStdAccel[g_nAccels].psName != 0; g_nAccels++ )
			;
	}
	return g_nAccels;
}

QString ShortcutList::name( uint i ) const
	{ return g_infoStdAccel[i].psName; }

QString ShortcutList::label( uint i ) const
	{ return i18n((g_infoStdAccel[i].psDesc) ? g_infoStdAccel[i].psDesc : g_infoStdAccel[i].psName); }

QString ShortcutList::whatsThis( uint ) const
	{ return QString(); }

const KShortcut& ShortcutList::shortcut( uint i ) const
{
	if( !g_infoStdAccel[i].bInitialized )
		initialize( g_infoStdAccel[i].id );
	return g_infoStdAccel[i].cut;
}

const KShortcut& ShortcutList::shortcutDefault( uint i ) const
{
	static KShortcut cut;
	cut = KStdAccel::shortcutDefault( g_infoStdAccel[i].id );
	return cut;
}

bool ShortcutList::isConfigurable( uint i ) const
	{ return (g_infoStdAccel[i].id != AccelNone); }

bool ShortcutList::setShortcut( uint i, const KShortcut& cut )
	{ g_infoStdAccel[i].cut = cut; return true; }

QVariant ShortcutList::getOther( Other, uint ) const
	{ return QVariant(); }

bool ShortcutList::setOther( Other, uint, const QVariant &)
	{ return false; }

bool ShortcutList::save() const
{
	return writeSettings( QString(), 0, false, true );
}

}

#undef KSTDACCEL_CPP
