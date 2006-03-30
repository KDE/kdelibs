/*
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

    Win32 port:
    Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _KKEYSERVER_X11_H
#define _KKEYSERVER_X11_H

#include "kshortcut.h"

/**
 * A collection of functions for the conversion of key presses and
 * their modifiers from the window system specific format
 * to the generic format and vice-versa.
 */
namespace KKeyServer
{
	static const int MODE_SWITCH = 0x2000;

	/**
	 * Initialises the values to return for the mod*() functions below.
	 * Called automatically by those functions if not already initialized.
	 */
	KDEUI_EXPORT bool initializeMods();

	/**
	 * Returns true if the current keyboard layout supports the Meta key.
	 * Specifically, whether the Super or Meta keys are assigned to an X modifier.
	 * @return true if the keyboard has a Meta key
	 * @see modXMeta()
	 */
	KDEUI_EXPORT bool keyboardHasMetaKey();

	/**
	 * Returns the X11 Shift modifier mask/flag.
	 * @return the X11 Shift modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXShift();

	/**
	 * Returns the X11 Lock modifier mask/flag.
	 * @return the X11 Lock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXLock();

	/**
	 * Returns the X11 Ctrl modifier mask/flag.
	 * @return the X11 Ctrl modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXCtrl();

	/**
	 * Returns the X11 Alt (Mod1) modifier mask/flag.
	 * @return the X11 Alt (Mod1) modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXAlt();

	/**
	 * Returns the X11 Win (Mod3) modifier mask/flag.
	 * @return the X11 Win (Mod3) modifier mask/flag.
	 * @see keyboardHasWinKey()
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXMeta();

	/**
	 * Returns the X11 Lock modifier mask/flag.
	 * @return the X11 Lock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXLock();

	/**
	 * Returns the X11 NumLock modifier mask/flag.
	 * @return the X11 NumLock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXNumLock();

	/**
	 * Returns the X11 ScrollLock modifier mask/flag.
	 * @return the X11 ScrollLock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXScrollLock();

	/**
	 * Returns the X11 Mode_switch modifier mask/flag.
	 * @return the X11 Mode_switch modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDEUI_EXPORT uint modXModeSwitch();

	/**
	 * Returns bitwise OR'ed mask containing Shift, Ctrl, Alt, and
	 * Win (if available).
	 * @see modXShift()
	 * @see modXLock()
	 * @see modXCtrl()
	 * @see modXAlt()
	 * @see modXNumLock()
	 * @see modXWin()
	 * @see modXScrollLock()
	 */
	KDEUI_EXPORT uint accelModMaskX();

	/**
	 * Extracts the symbol from the given Qt key and
	 * converts it to an X11 symbol + modifiers.
	 * @param keyQt the qt key code
	 * @param sym if successful, the symbol will be written here
	 * @return true if successful, false otherwise
	 */
	KDEUI_EXPORT bool keyQtToX( int keyQt, int& sym );

	/**
	 * Extracts the modifiers from the given Qt key and
	 * converts them in a mask of X11 modifiers.
	 * @param keyQt the qt key code
	 * @param mod if successful, the modifiers will be written here
	 * @return true if successful, false otherwise
	 */
	KDEUI_EXPORT bool keyQtToModX( int keyQt, uint& mod );

	/**
	 * Converts the given symbol to a Qt key code.
	 * @param sym the symbol
	 * @param keyQt if successful, the qt key code will be written here
	 * @return true if successful, false otherwise
	 */
	KDEUI_EXPORT bool symToKeyQt( uint sym, int& keyQt );

	/**
	 * Converts the mask of ORed X11 modifiers to
	 * a mask of ORed Qt key code modifiers.
	 * @param modX the mask of X11 modifiers
	 * @param modQt the mask of Qt key code modifiers will be written here
	 *        if successful
	 * @return true if successful, false otherwise
	 */
	KDEUI_EXPORT bool modXToQt( uint modX, int& modQt );

#ifdef Q_WS_WIN
	/**
	 * Converts the Qt-compatible button state to KKey modifier.
	 * Windows only.
	 */
	KDEUI_EXPORT int qtButtonStateToMod( Qt::KeyboardModifiers s );
#endif

	/**
	 * Converts the mask of ORed KKey::ModFlag modifiers to a
	 * user-readable string.
	 * @param mod the mask of ORed KKey::ModFlag modifiers
	 * @return the user-readable string
	 */
	KDEUI_EXPORT QString modToStringUser( uint mod );

	/**
	 * Converts the modifier given as user-readable string
         * to KKey::ModFlag modifier, or 0.
         * @internal
	 */
	KDEUI_EXPORT uint stringUserToMod( const QString& mod );
}

#endif // !_KKEYSERVER_X11_H
