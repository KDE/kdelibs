/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)
    Copyright (C) 2000 Nicolas Hadacek (haadcek@kde.org)
    Copyright (C) 2001 Ellis Whitehead (ellis@kde.org)

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

#include <kaccelbase.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kshortcut.h>

struct KStdAccelInfo
{
	KStdAccel::StdAccel id;
	const char* psName;
	const char* psDesc;
	int cutDefault, cutDefault4, cutDefault3B, cutDefault4B;
	KShortcut cut;
	bool bInitialized;
};

static KStdAccelInfo g_infoStdAccel[] =
{
	{ KStdAccel::Open,                I18N_NOOP("Open"), 0,     Qt::CTRL+Qt::Key_O, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::New,                 I18N_NOOP("New"), 0,      Qt::CTRL+Qt::Key_N, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Close,               I18N_NOOP("Close"), 0,    Qt::CTRL+Qt::Key_W, Qt::CTRL+Qt::Key_Escape, 0, 0, KShortcut(), false },
	{ KStdAccel::Save,                I18N_NOOP("Save"), 0,     Qt::CTRL+Qt::Key_S, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Print,               I18N_NOOP("Print"), 0,    Qt::CTRL+Qt::Key_P, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Quit,                I18N_NOOP("Quit"), 0,     Qt::CTRL+Qt::Key_Q, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Undo,                I18N_NOOP("Undo"), 0,     Qt::CTRL+Qt::Key_Z, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Redo,                I18N_NOOP("Redo"), 0,     Qt::CTRL+Qt::SHIFT+Qt::Key_Z, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Cut,                 I18N_NOOP("Cut"), 0,      Qt::CTRL+Qt::Key_X, 0, Qt::SHIFT+Qt::Key_Delete, 0, KShortcut(), false },
	{ KStdAccel::Copy,                I18N_NOOP("Copy"), 0,     Qt::CTRL+Qt::Key_C, 0, Qt::CTRL+Qt::Key_Insert, 0, KShortcut(), false },
	{ KStdAccel::Paste,               I18N_NOOP("Paste"), 0,    Qt::CTRL+Qt::Key_V, 0, Qt::SHIFT+Qt::Key_Insert, 0, KShortcut(), false },
	{ KStdAccel::SelectAll,           "SelectAll", I18N_NOOP("Select All"), Qt::CTRL+Qt::Key_A, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Deselect,            I18N_NOOP("Deselect"), 0, Qt::CTRL+Qt::SHIFT+Qt::Key_A, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::DeleteWordBack,      "DeleteWordBack", I18N_NOOP("Delete Word Backwards"), Qt::CTRL+Qt::Key_Backspace, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::DeleteWordForward,   "DeleteWordForward", I18N_NOOP("Delete Word Forward"), Qt::CTRL+Qt::Key_Delete, 0,  0, 0, KShortcut(), false },
	{ KStdAccel::Find,                I18N_NOOP("Find"), 0,     Qt::CTRL+Qt::Key_F, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::FindNext,            "FindNext", I18N_NOOP("Find Next"), Qt::Key_F3, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::FindPrev,            "FindPrev", I18N_NOOP("Find Prev"), Qt::SHIFT+Qt::Key_F3, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Replace,             I18N_NOOP("Replace"), 0,  Qt::CTRL+Qt::Key_R, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Home,                I18N_NOOP("Home"), 0,     Qt::CTRL+Qt::Key_Home, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::End,                 I18N_NOOP("End"), 0,      Qt::CTRL+Qt::Key_End, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Prior,               I18N_NOOP("Prior"), 0,    Qt::Key_Prior, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Next,                I18N_NOOP("Next"), 0,     Qt::Key_Next, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::GotoLine,            "GotoLine", I18N_NOOP("Go to Line"), Qt::CTRL+Qt::Key_G, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::AddBookmark,         "AddBookmark", I18N_NOOP("Add Bookmark"), Qt::CTRL+Qt::Key_B, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::ZoomIn,              "ZoomIn", I18N_NOOP("Zoom In"), Qt::CTRL+Qt::Key_Plus, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::ZoomOut,             "ZoomOut", I18N_NOOP("Zoom Out"), Qt::CTRL+Qt::Key_Minus, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Up,                  I18N_NOOP("Up"), 0,       Qt::ALT+Qt::Key_Up, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Back,                I18N_NOOP("Back"), 0,     Qt::ALT+Qt::Key_Left, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Forward,             I18N_NOOP("Forward"), 0,  Qt::ALT+Qt::Key_Right, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Reload,              I18N_NOOP("Reload"), 0,   Qt::Key_F5, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::PopupMenuContext,    "PopupMenuContext", I18N_NOOP("Popup Menu Context"), Qt::Key_Menu, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::ShowMenubar,         "ShowMenubar", I18N_NOOP("Show Menu Bar"), Qt::CTRL+Qt::Key_M, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::Help,                I18N_NOOP("Help"), 0,     Qt::Key_F1, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::WhatsThis,           "WhatThis", I18N_NOOP("What's This"), Qt::SHIFT+Qt::Key_F1, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::TextCompletion,      "TextCompletion", I18N_NOOP("Text Completion"), Qt::CTRL+Qt::Key_E, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::PrevCompletion,      "PrevCompletion", I18N_NOOP("Previous Completion Match"), Qt::CTRL+Qt::Key_Up, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::NextCompletion,      "NextCompletion", I18N_NOOP("Next Completion Match"), Qt::CTRL+Qt::Key_Down, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::SubstringCompletion, "SubstringCompletion", I18N_NOOP("Substring Completion"), Qt::CTRL+Qt::Key_T, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::RotateUp,            "RotateUp", I18N_NOOP("Previous Item in List"), Qt::Key_Up, 0, 0, 0, KShortcut(), false },
	{ KStdAccel::RotateDown,          "RotateDown", I18N_NOOP("Next Item in List"), Qt::Key_Down, 0, 0, 0, KShortcut(), false }
};

static void initialize(KStdAccel::StdAccel id)
{
	KConfigGroupSaver saver( KGlobal::config(), "Shortcuts" );
	KStdAccelInfo& info = g_infoStdAccel[id];

	if( info.id != id ) {
		kdWarning(125) << "KStdAccel: id's don't match!" << endl; // -- ellis
		return;
	}

	if( saver.config()->hasKey( info.psName ) ) {
		QString s = saver.config()->readEntry( info.psName );
		if( s != "none" )
			info.cut.init( s );
		else
			info.cut.clear();
	} else
		info.cut = KStdAccel::shortcutDefault( id );
	info.bInitialized = true;
}

const KShortcut& KStdAccel::shortcut(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KShortcut::null();

	if( !g_infoStdAccel[id].bInitialized )
		initialize( id );

	return g_infoStdAccel[id].cut;
}

QString KStdAccel::name(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return QString::null;
	return g_infoStdAccel[id].psName;
}

KShortcut KStdAccel::shortcutDefault(StdAccel id)
{
	return (KAccelAction::useFourModifierKeys())
		? shortcutDefault4(id) : shortcutDefault3(id);
}

KShortcut KStdAccel::shortcutDefault3(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KShortcut();
	KShortcut cut = g_infoStdAccel[id].cutDefault;

	if( !g_infoStdAccel[id].cutDefault3B )
		cut.insert( QKeySequence(g_infoStdAccel[id].cutDefault3B) );

	return cut;
}

KShortcut KStdAccel::shortcutDefault4(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KShortcut();
	KStdAccelInfo& info = g_infoStdAccel[id];

	KShortcut cut;
	KKeySequence key2;

	cut.init( (info.cutDefault4) ?
		QKeySequence(info.cutDefault) : QKeySequence(info.cutDefault4) );

	if( !info.cutDefault4B )
		key2.init( QKeySequence(info.cutDefault4B) );
	else if( !info.cutDefault3B )
		key2.init( QKeySequence(info.cutDefault3B) );

	if( key2.count() )
		cut.insert( key2 );

	return cut;
}

QString KStdAccel::desc(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return QString::null;
	return i18n((g_infoStdAccel[id].psDesc)
		? g_infoStdAccel[id].psDesc
		: g_infoStdAccel[id].psName);
}

const KShortcut& KStdAccel::open()                  { return shortcut( Open ); }
const KShortcut& KStdAccel::openNew()               { return shortcut( New ); }
const KShortcut& KStdAccel::close()                 { return shortcut( Close ); }
const KShortcut& KStdAccel::save()                  { return shortcut( Save ); }
const KShortcut& KStdAccel::print()                 { return shortcut( Print ); }
const KShortcut& KStdAccel::quit()                  { return shortcut( Quit ); }
const KShortcut& KStdAccel::cut()                   { return shortcut( Cut ); }
const KShortcut& KStdAccel::copy()                  { return shortcut( Copy ); }
const KShortcut& KStdAccel::paste()                 { return shortcut( Paste ); }
const KShortcut& KStdAccel::deleteWordBack()        { return shortcut( DeleteWordBack ); }
const KShortcut& KStdAccel::deleteWordForward()     { return shortcut( DeleteWordForward ); }
const KShortcut& KStdAccel::undo()                  { return shortcut( Undo ); }
const KShortcut& KStdAccel::redo()                  { return shortcut( Redo ); }
const KShortcut& KStdAccel::find()                  { return shortcut( Find ); }
const KShortcut& KStdAccel::findNext()              { return shortcut( FindNext ); }
const KShortcut& KStdAccel::findPrev()              { return shortcut( FindPrev ); }
const KShortcut& KStdAccel::replace()               { return shortcut( Replace ); }
const KShortcut& KStdAccel::home()                  { return shortcut( Home ); }
const KShortcut& KStdAccel::end()                   { return shortcut( End ); }
const KShortcut& KStdAccel::prior()                 { return shortcut( Prior ); }
const KShortcut& KStdAccel::next()                  { return shortcut( Next ); }
const KShortcut& KStdAccel::gotoLine()              { return shortcut( GotoLine ); }
const KShortcut& KStdAccel::addBookmark()           { return shortcut( AddBookmark ); }
const KShortcut& KStdAccel::zoomIn()                { return shortcut( ZoomIn ); }
const KShortcut& KStdAccel::zoomOut()               { return shortcut( ZoomOut ); }
const KShortcut& KStdAccel::help()                  { return shortcut( Help ); }
const KShortcut& KStdAccel::completion()            { return shortcut( TextCompletion ); }
const KShortcut& KStdAccel::prevCompletion()        { return shortcut( PrevCompletion ); }
const KShortcut& KStdAccel::nextCompletion()        { return shortcut( NextCompletion ); }
const KShortcut& KStdAccel::rotateUp()              { return shortcut( RotateUp ); }
const KShortcut& KStdAccel::rotateDown()            { return shortcut( RotateDown ); }
const KShortcut& KStdAccel::substringCompletion()   { return shortcut( SubstringCompletion ); }
const KShortcut& KStdAccel::popupMenuContext()      { return shortcut( PopupMenuContext ); }
const KShortcut& KStdAccel::whatsThis()             { return shortcut( WhatsThis ); }
const KShortcut& KStdAccel::reload()                { return shortcut( Reload ); }
const KShortcut& KStdAccel::selectAll()             { return shortcut( SelectAll ); }
const KShortcut& KStdAccel::up()                    { return shortcut( Up ); }
const KShortcut& KStdAccel::back()                  { return shortcut( Back ); }
const KShortcut& KStdAccel::forward()               { return shortcut( Forward ); }
const KShortcut& KStdAccel::showMenubar()           { return shortcut( ShowMenubar ); }

#ifndef KDE_NO_COMPAT
QString KStdAccel::action(StdAccel id)
	{ return name(id); }
QString KStdAccel::description(StdAccel id)
	{ return desc(id); }
int KStdAccel::key(StdAccel id)
	{ return shortcut(id).keyCodeQt(); }
int KStdAccel::defaultKey(StdAccel id)
	{ return shortcutDefault(id).keyCodeQt(); }

bool KStdAccel::isEqual(const QKeyEvent* ev, int keyQt)
{
	KKey key1( ev ), key2( keyQt );
	return key1 == key2;
}
#endif // !KDE_NO_COMPAT
