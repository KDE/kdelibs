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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kstdaccel.h"

#include <kaccelaction.h>
#include <kaccelbase.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kshortcut.h>
#include <kshortcutlist.h>

namespace KStdAccel
{

struct KStdAccelInfo
{
	StdAccel id;
	const char* psName;
	const char* psDesc;
	int cutDefault, cutDefault4, cutDefault3B, cutDefault4B;
	KShortcut cut;
	bool bInitialized;
};

static KStdAccelInfo g_infoStdAccel[] =
{
	{AccelNone,            "Group:File", I18N_NOOP("File"), 0, 0, 0, 0, KShortcut(), false },
	{ Open,                I18N_NOOP("Open"), 0,     Qt::CTRL+Qt::Key_O, 0, 0, 0, KShortcut(), false },
	{ New,                 I18N_NOOP("New"), 0,      Qt::CTRL+Qt::Key_N, 0, 0, 0, KShortcut(), false },
	{ Close,               I18N_NOOP("Close"), 0,    Qt::CTRL+Qt::Key_W, Qt::CTRL+Qt::Key_Escape, 0, 0, KShortcut(), false },
	{ Save,                I18N_NOOP("Save"), 0,     Qt::CTRL+Qt::Key_S, 0, 0, 0, KShortcut(), false },
	{ Print,               I18N_NOOP("Print"), 0,    Qt::CTRL+Qt::Key_P, 0, 0, 0, KShortcut(), false },
	{ Quit,                I18N_NOOP("Quit"), 0,     Qt::CTRL+Qt::Key_Q, 0, 0, 0, KShortcut(), false },
	{AccelNone,            "Group:Edit", I18N_NOOP("Edit"), 0, 0, 0, 0, KShortcut(), false },
	{ Undo,                I18N_NOOP("Undo"), 0,     Qt::CTRL+Qt::Key_Z, 0, 0, 0, KShortcut(), false },
	{ Redo,                I18N_NOOP("Redo"), 0,     Qt::CTRL+Qt::SHIFT+Qt::Key_Z, 0, 0, 0, KShortcut(), false },
	{ Cut,                 I18N_NOOP("Cut"), 0,      Qt::CTRL+Qt::Key_X, 0, Qt::SHIFT+Qt::Key_Delete, 0, KShortcut(), false },
	{ Copy,                I18N_NOOP("Copy"), 0,     Qt::CTRL+Qt::Key_C, 0, Qt::CTRL+Qt::Key_Insert, 0, KShortcut(), false },
	{ Paste,               I18N_NOOP("Paste"), 0,    Qt::CTRL+Qt::Key_V, 0, Qt::SHIFT+Qt::Key_Insert, 0, KShortcut(), false },
	{ SelectAll,           "SelectAll", I18N_NOOP("Select All"), Qt::CTRL+Qt::Key_A, 0, 0, 0, KShortcut(), false },
	{ Deselect,            I18N_NOOP("Deselect"), 0, Qt::CTRL+Qt::SHIFT+Qt::Key_A, 0, 0, 0, KShortcut(), false },
	{ DeleteWordBack,      "DeleteWordBack", I18N_NOOP("Delete Word Backwards"), Qt::CTRL+Qt::Key_Backspace, 0, 0, 0, KShortcut(), false },
	{ DeleteWordForward,   "DeleteWordForward", I18N_NOOP("Delete Word Forward"), Qt::CTRL+Qt::Key_Delete, 0,  0, 0, KShortcut(), false },
	{ Find,                I18N_NOOP("Find"), 0,     Qt::CTRL+Qt::Key_F, 0, 0, 0, KShortcut(), false },
	{ FindNext,            "FindNext", I18N_NOOP("Find Next"), Qt::Key_F3, 0, 0, 0, KShortcut(), false },
	{ FindPrev,            "FindPrev", I18N_NOOP("Find Prev"), Qt::SHIFT+Qt::Key_F3, 0, 0, 0, KShortcut(), false },
	{ Replace,             I18N_NOOP("Replace"), 0,  Qt::CTRL+Qt::Key_R, 0, 0, 0, KShortcut(), false },
	{AccelNone,            "Group:Navigation", I18N_NOOP("Navigation"), 0, 0, 0, 0, KShortcut(), false },
	{ Home,                I18N_NOOP("Home"), 0,     Qt::CTRL+Qt::Key_Home, 0, 0, 0, KShortcut(), false },
	{ End,                 I18N_NOOP("End"), 0,      Qt::CTRL+Qt::Key_End, 0, 0, 0, KShortcut(), false },
	{ Prior,               I18N_NOOP("Prior"), 0,    Qt::Key_Prior, 0, 0, 0, KShortcut(), false },
	{ Next,                I18N_NOOP("Next"), 0,     Qt::Key_Next, 0, 0, 0, KShortcut(), false },
	{ GotoLine,            "GotoLine", I18N_NOOP("Go to Line"), Qt::CTRL+Qt::Key_G, 0, 0, 0, KShortcut(), false },
	{ AddBookmark,         "AddBookmark", I18N_NOOP("Add Bookmark"), Qt::CTRL+Qt::Key_B, 0, 0, 0, KShortcut(), false },
	{ ZoomIn,              "ZoomIn", I18N_NOOP("Zoom In"), Qt::CTRL+Qt::Key_Plus, 0, 0, 0, KShortcut(), false },
	{ ZoomOut,             "ZoomOut", I18N_NOOP("Zoom Out"), Qt::CTRL+Qt::Key_Minus, 0, 0, 0, KShortcut(), false },
	{ Up,                  I18N_NOOP("Up"), 0,       Qt::ALT+Qt::Key_Up, 0, 0, 0, KShortcut(), false },
	{ Back,                I18N_NOOP("Back"), 0,     Qt::ALT+Qt::Key_Left, 0, 0, 0, KShortcut(), false },
	{ Forward,             I18N_NOOP("Forward"), 0,  Qt::ALT+Qt::Key_Right, 0, 0, 0, KShortcut(), false },
	{ Reload,              I18N_NOOP("Reload"), 0,   Qt::Key_F5, 0, 0, 0, KShortcut(), false },
	{ PopupMenuContext,    "PopupMenuContext", I18N_NOOP("Popup Menu Context"), Qt::Key_Menu, 0, 0, 0, KShortcut(), false },
	{ ShowMenubar,         "ShowMenubar", I18N_NOOP("Show Menu Bar"), Qt::CTRL+Qt::Key_M, 0, 0, 0, KShortcut(), false },
	{AccelNone,            "Group:Help", I18N_NOOP("Help"), 0, 0, 0, 0, KShortcut(), false },
	{ Help,                I18N_NOOP("Help"), 0,     Qt::Key_F1, 0, 0, 0, KShortcut(), false },
	{ WhatsThis,           "WhatsThis", I18N_NOOP("What's This"), Qt::SHIFT+Qt::Key_F1, 0, 0, 0, KShortcut(), false },
	{AccelNone,            "Group:TextCompletion", I18N_NOOP("Text Completion"), 0, 0, 0, 0, KShortcut(), false },
	{ TextCompletion,      "TextCompletion", I18N_NOOP("Text Completion"), Qt::CTRL+Qt::Key_E, 0, 0, 0, KShortcut(), false },
	{ PrevCompletion,      "PrevCompletion", I18N_NOOP("Previous Completion Match"), Qt::CTRL+Qt::Key_Up, 0, 0, 0, KShortcut(), false },
	{ NextCompletion,      "NextCompletion", I18N_NOOP("Next Completion Match"), Qt::CTRL+Qt::Key_Down, 0, 0, 0, KShortcut(), false },
	{ SubstringCompletion, "SubstringCompletion", I18N_NOOP("Substring Completion"), Qt::CTRL+Qt::Key_T, 0, 0, 0, KShortcut(), false },
	{ RotateUp,            "RotateUp", I18N_NOOP("Previous Item in List"), Qt::Key_Up, 0, 0, 0, KShortcut(), false },
	{ RotateDown,          "RotateDown", I18N_NOOP("Next Item in List"), Qt::Key_Down, 0, 0, 0, KShortcut(), false },
	{ AccelNone,           0, 0, 0, 0, 0, 0, KShortcut(), false }
};

static KStdAccelInfo* infoPtr( StdAccel id )
{
	if( id != AccelNone ) {
		for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
			if( g_infoStdAccel[i].id == id )
				return &g_infoStdAccel[i];
		}
	}
	return 0;
}

static void initialize( StdAccel id )
{
	KConfigGroupSaver saver( KGlobal::config(), "Shortcuts" );
	KStdAccelInfo* pInfo = infoPtr( id );

	if( !pInfo ) {
		kdWarning(125) << "KStdAccel: id not found!" << endl; // -- ellis
		return;
	}

	if( saver.config()->hasKey( pInfo->psName ) ) {
		QString s = saver.config()->readEntry( pInfo->psName );
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
		return QString::null;
	return pInfo->psName;
}

QString label( StdAccel id )
{
	KStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return QString::null;
	return i18n((pInfo->psDesc) ? pInfo->psDesc : pInfo->psName);
}

// TODO: Add psWhatsThis entry to KStdAccelInfo
QString whatsThis( StdAccel /*id*/ )
{
//	KStdAccelInfo* pInfo = infoPtr( id );
//	if( pInfo && pInfo->psWhatsThis )
//		return i18n(pInfo->psWhatsThis);
//	else
		return QString::null;
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
	return (KAccelAction::useFourModifierKeys())
		? shortcutDefault4(id) : shortcutDefault3(id);
}

KShortcut shortcutDefault3( StdAccel id )
{
	KShortcut cut;

	KStdAccelInfo* pInfo = infoPtr( id );
	if( pInfo ) {
		if( pInfo->cutDefault )
			cut.init( pInfo->cutDefault );
		// FIXME: if there is no cutDefault, then this we be made the primary
		//  instead of alternate shortcut.
		if( pInfo->cutDefault3B )
			cut.append( QKeySequence(pInfo->cutDefault3B) );
	}

	return cut;
}

KShortcut shortcutDefault4( StdAccel id )
{
	KShortcut cut;

	KStdAccelInfo* pInfo = infoPtr( id );
	if( pInfo ) {
		KStdAccelInfo& info = *pInfo;
		KKeySequence key2;

		cut.init( (info.cutDefault4) ?
			QKeySequence(info.cutDefault) : QKeySequence(info.cutDefault4) );

		if( info.cutDefault4B )
			key2.init( QKeySequence(info.cutDefault4B) );
		else if( info.cutDefault3B )
			key2.init( QKeySequence(info.cutDefault3B) );

		if( key2.count() )
			cut.append( key2 );
	}

	return cut;
}

void createAccelActions( KAccelActions& actions )
{
	actions.clear();

	for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
		StdAccel id = g_infoStdAccel[i].id;
		KStdAccelInfo* pInfo = &g_infoStdAccel[i];

		if( id != AccelNone ) {
			actions.insert( pInfo->psName,
				i18n((pInfo->psDesc) ? pInfo->psDesc : pInfo->psName),
				QString::null, // pInfo->psWhatsThis,
				shortcutDefault3(id),
				shortcutDefault4(id) );
		} else
			actions.insert( pInfo->psName, i18n(pInfo->psDesc) );
	}
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
const KShortcut& prior()                 { return shortcut( Prior ); }
const KShortcut& next()                  { return shortcut( Next ); }
const KShortcut& gotoLine()              { return shortcut( GotoLine ); }
const KShortcut& addBookmark()           { return shortcut( AddBookmark ); }
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
	{ return QString::null; }

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

bool ShortcutList::setOther( Other, uint, QVariant )
	{ return false; }

bool ShortcutList::save() const
{
	return writeSettings( QString::null, 0, false, true );
}

#ifndef KDE_NO_COMPAT
QString action(StdAccel id)
	{ return name(id); }
QString description(StdAccel id)
	{ return label(id); }
int key(StdAccel id)
	{ return shortcut(id).keyCodeQt(); }
int defaultKey(StdAccel id)
	{ return shortcutDefault(id).keyCodeQt(); }

bool isEqual(const QKeyEvent* ev, int keyQt)
{
	KKey key1( ev ), key2( keyQt );
	return key1 == key2;
}
#endif // !KDE_NO_COMPAT

};
