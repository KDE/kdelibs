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
#include <kglobal.h>
#include <klocale.h>
#include <kkeysequence.h>

struct KStdAccelInfo
{
	const char* psName;
	const char* psDesc;
	int keyDefault, keyDefault4, keyDefault3B, keyDefault4B;
	int key, keyB;
	bool bInitialized;
};

static KStdAccelInfo g_infoStdAccel[] =
{
	{ I18N_NOOP("Open"), 0,     Qt::CTRL+Qt::Key_O },
	{ I18N_NOOP("New"), 0,      Qt::CTRL+Qt::Key_N },
	{ I18N_NOOP("Close"), 0,    Qt::CTRL+Qt::Key_W, Qt::CTRL+Qt::Key_Escape },
	{ I18N_NOOP("Save"), 0,     Qt::CTRL+Qt::Key_S },
	{ I18N_NOOP("Print"), 0,    Qt::CTRL+Qt::Key_P },
	{ I18N_NOOP("Quit"), 0,     Qt::CTRL+Qt::Key_Q },
	{ I18N_NOOP("Cut"), 0,      Qt::CTRL+Qt::Key_X, 0, Qt::SHIFT+Qt::Key_Delete },
	{ I18N_NOOP("Copy"), 0,     Qt::CTRL+Qt::Key_C, 0, Qt::CTRL+Qt::Key_Insert },
	{ I18N_NOOP("Paste"), 0,    Qt::CTRL+Qt::Key_V, 0, Qt::SHIFT+Qt::Key_Insert },
	{ I18N_NOOP("Undo"), 0,     Qt::CTRL+Qt::Key_Z },
	{ I18N_NOOP("Redo"), 0,     Qt::CTRL+Qt::SHIFT+Qt::Key_Z },
	{ I18N_NOOP("Find"), 0,     Qt::CTRL+Qt::Key_F },
	{ I18N_NOOP("Replace"), 0,  Qt::CTRL+Qt::Key_R },
	{ I18N_NOOP("Insert"), 0,   Qt::CTRL+Qt::Key_Insert },
	{ I18N_NOOP("Home"), 0,     Qt::CTRL+Qt::Key_Home },
	{ I18N_NOOP("End"), 0,      Qt::CTRL+Qt::Key_End },
	{ I18N_NOOP("Prior"), 0,    Qt::Key_Prior },
	{ I18N_NOOP("Next"), 0,     Qt::Key_Next },
	{ I18N_NOOP("Help"), 0,     Qt::Key_F1 },
	{ "FindNext", I18N_NOOP("Find Next"), Qt::Key_F3 },
	{ "FindPrev", I18N_NOOP("Find Prev"), Qt::SHIFT+Qt::Key_F3 },
	{ "ZoomIn", I18N_NOOP("Zoom In"), Qt::CTRL+Qt::Key_Plus },
	{ "ZoomOut", I18N_NOOP("Zoom Out"), Qt::CTRL+Qt::Key_Minus },
	{ "AddBookmark", I18N_NOOP("Add Bookmark"), Qt::CTRL+Qt::Key_B },
	{ "TextCompletion", I18N_NOOP("Text Completion"), Qt::CTRL+Qt::Key_E },
	{ "PrevCompletion", I18N_NOOP("Previous Completion Match"), Qt::CTRL+Qt::Key_Up },
	{ "NextCompletion", I18N_NOOP("Next Completion Match"), Qt::CTRL+Qt::Key_Down },
	{ "RotateUp", I18N_NOOP("Previous Item in List"), Qt::Key_Up },
	{ "RotateDown", I18N_NOOP("Next Item in List"), Qt::Key_Down },
	{ "PopupMenuContext", I18N_NOOP("Popup Menu Context"), Qt::Key_Menu },
	{ "WhatThis", I18N_NOOP("What's This"), Qt::SHIFT+Qt::Key_F1 },
	{ I18N_NOOP("Reload"), 0,   Qt::Key_F5 },
	{ "SelectAll", I18N_NOOP("Select All"), Qt::CTRL+Qt::Key_A },
	{ I18N_NOOP("Up"), 0,       Qt::ALT+Qt::Key_Up },
	{ I18N_NOOP("Back"), 0,     Qt::ALT+Qt::Key_Left },
	{ I18N_NOOP("Forward"), 0,  Qt::ALT+Qt::Key_Right },
	{ "ShowMenubar", I18N_NOOP("Show Menu Bar"), Qt::CTRL+Qt::Key_M },
	{ "GotoLine", I18N_NOOP("Go to Line"), Qt::CTRL+Qt::Key_G },
	{ "DeleteWordBack", I18N_NOOP("Delete Word Backwards"), Qt::CTRL+Qt::Key_Backspace },
	{ "DeleteWordForward", I18N_NOOP("Delete Word Forward"), Qt::CTRL+Qt::Key_Delete },
	{ "SubstringCompletion", I18N_NOOP("Substring Completion"), Qt::CTRL+Qt::Key_T }
};

static void initialize(KStdAccel::StdAccel id)
{
	KConfigGroupSaver saver( KGlobal::config(), "Shortcuts" );
	KStdAccelInfo& info = g_infoStdAccel[id];

	if( saver.config()->hasKey( info.psName ) ) {
		QString s = saver.config()->readEntry( info.psName );
		if( s != "none" ) {
			KAccelShortcuts cuts( s );
			if( cuts.size() >= 1 )
				info.key = cuts[0].getSequence(0).getKey().keyQt();
			if( cuts.size() >= 2 )
				info.keyB = cuts[1].getSequence(0).getKey().keyQt();
		} else
			info.key = info.keyB = 0;
	} else {
		if( KKeySequence::useFourModifierKeys() ) {
			info.key = KStdAccel::defaultKey4( id ).keyQt();
			info.keyB = info.keyDefault4B;
			if( !info.keyB )
				info.keyB = info.keyDefault3B;
		} else {
			info.key = info.keyDefault;
			info.keyB = info.keyDefault3B;
		}
	}
	info.bInitialized = true;
}

uint KStdAccel::key(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return 0;
	if( !g_infoStdAccel[id].bInitialized )
		initialize(id);
	return g_infoStdAccel[id].key;
}

KShortcuts KStdAccel::shorcuts(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return 0;

	KShortcuts cuts( g_infoStdAccel[id].key );
	KAccelShortcuts& acuts = cuts.base();
	KAccelShortcut cut;

	if( g_infoStdAccel[id].keyB ) {
		cut.init( g_infoStdAccel[id].keyB );
		acuts.push_back( cut );
	}

	return cuts;
}

bool KStdAccel::isEqual(QKeyEvent* ev, int skey)
{
	int key = ev->key();
	if( ev->state() & Qt::ShiftButton )   key |= Qt::ALT;
	if( ev->state() & Qt::ControlButton ) key |= Qt::CTRL;
	if( ev->state() & Qt::AltButton )     key |= Qt::SHIFT;
	
	return (key == skey);
}

QString KStdAccel::action(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return QString::null;
	return g_infoStdAccel[id].psName;
}

KKeySequence KStdAccel::defaultKey(StdAccel id)
{
    return (KKeySequence::useFourModifierKeys())
        ? defaultKey4(id) : defaultKey3(id);
}

KKeySequence KStdAccel::defaultKey3(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KKeySequence();
	return g_infoStdAccel[id].keyDefault;
}

KKeySequence KStdAccel::defaultKey4(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KKeySequence();

	if( g_infoStdAccel[id].keyDefault4 )
		return g_infoStdAccel[id].keyDefault4;
	else
		return g_infoStdAccel[id].keyDefault;
}

KShortcuts KStdAccel::defaultShortcuts(StdAccel id)
{
    return (KKeySequence::useFourModifierKeys())
        ? defaultShortcuts4(id) : defaultShortcuts3(id);
}

KShortcuts KStdAccel::defaultShortcuts3(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KShortcuts();

	KShortcuts cuts( defaultKey3( id ) );
	KAccelShortcuts& acuts = cuts.base();
	KAccelShortcut cut;

	if( g_infoStdAccel[id].keyDefault3B ) {
		cut.init( g_infoStdAccel[id].keyDefault3B );
		acuts.push_back( cut );
	}

	return cuts;
}

KShortcuts KStdAccel::defaultShortcuts4(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return KShortcuts();

	KShortcuts cuts( defaultKey4( id ) );
	KAccelShortcuts& acuts = cuts.base();
	KAccelShortcut cut;

	int keyB = (g_infoStdAccel[id].keyDefault4B) ?
		g_infoStdAccel[id].keyDefault4B : g_infoStdAccel[id].keyDefault3B;
	if( keyB ) {
		cut.init( keyB );
		acuts.push_back( cut );
	}

	return cuts;
}

QString KStdAccel::description(StdAccel id)
{
	if( id < 0 || id >= NB_STD_ACCELS )
		return QString::null;
	return i18n((g_infoStdAccel[id].psDesc)
		? g_infoStdAccel[id].psDesc
		: g_infoStdAccel[id].psName);
}

uint KStdAccel::open()                  { return key(Open); }
uint KStdAccel::openNew()               { return key(New); }
uint KStdAccel::close()                 { return key(Close); }
uint KStdAccel::save()                  { return key(Save); }
uint KStdAccel::print()                 { return key(Print); }
uint KStdAccel::quit()                  { return key(Quit); }
uint KStdAccel::cut()                   { return key(Cut); }
uint KStdAccel::copy()                  { return key(Copy); }
uint KStdAccel::paste()                 { return key(Paste); }
uint KStdAccel::deleteWordBack()        { return key(DeleteWordBack); }
uint KStdAccel::deleteWordForward()     { return key(DeleteWordForward); }
uint KStdAccel::undo()                  { return key(Undo); }
uint KStdAccel::redo()                  { return key(Redo); }
uint KStdAccel::find()                  { return key(Find); }
uint KStdAccel::findNext()              { return key(FindNext); }
uint KStdAccel::findPrev()              { return key(FindPrev); }
uint KStdAccel::replace()               { return key(Replace); }
uint KStdAccel::zoomIn()                { return key(ZoomIn); }
uint KStdAccel::zoomOut()               { return key(ZoomOut); }
uint KStdAccel::insert()                { return key(Insert); }
uint KStdAccel::home()                  { return key(Home); }
uint KStdAccel::end()                   { return key(End); }
uint KStdAccel::prior()                 { return key(Prior); }
uint KStdAccel::next()                  { return key(Next); }
uint KStdAccel::gotoLine()              { return key(GotoLine); }
uint KStdAccel::addBookmark()           { return key(AddBookmark); }
uint KStdAccel::help()                  { return key(Help); }
uint KStdAccel::completion()            { return key(TextCompletion); }
uint KStdAccel::prevCompletion()        { return key(PrevCompletion); }
uint KStdAccel::nextCompletion()        { return key(NextCompletion); }
uint KStdAccel::rotateUp()              { return key(RotateUp); }
uint KStdAccel::rotateDown()            { return key(RotateDown); }
uint KStdAccel::substringCompletion()   { return key(SubstringCompletion); }
uint KStdAccel::popupMenuContext()      { return key(PopupMenuContext); }
uint KStdAccel::whatsThis()             { return key(WhatThis); }
uint KStdAccel::reload()                { return key(Reload); }
uint KStdAccel::selectAll()             { return key(SelectAll); }
uint KStdAccel::up()                    { return key(Up); }
uint KStdAccel::back()                  { return key(Back); }
uint KStdAccel::forward()               { return key(Forward); }
uint KStdAccel::showMenubar()           { return key(ShowMenubar); }
