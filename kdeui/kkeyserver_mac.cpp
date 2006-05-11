/*
 Copyright (C) 2006 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 
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

#include <config.h>

#include <qnamespace.h>

#ifdef Q_WS_MACX // Only compile this module if we're compiling for Mac OS X

#include "kkeyserver_mac.h"
#include "kshortcut.h"
#include <kdebug.h>
#include <QMultiMap>
#include <Carbon/Carbon.h>

namespace KKeyServer {
    struct TransKey {
        int qt_code;
        int mac_code;
    };
    
    static TransKey qtKeyToChar[] = {
    {Qt::Key_Escape,     kEscapeCharCode},
    {Qt::Key_Tab,        kTabCharCode},
    {Qt::Key_Backtab,    kTabCharCode},  // Backtab == tab with different modifiers
    {Qt::Key_Backspace,  kBackspaceCharCode},
    {Qt::Key_Return,     kReturnCharCode},
    {Qt::Key_Enter,      kEnterCharCode},
        // Insert
    {Qt::Key_Delete,     kDeleteCharCode},
        // Pause, Print, SysReq
    {Qt::Key_Clear,      kClearCharCode},
    {Qt::Key_Home,       kHomeCharCode},
    {Qt::Key_End,        kEndCharCode},
    {Qt::Key_Left,       kLeftArrowCharCode},
    {Qt::Key_Up,         kUpArrowCharCode},
    {Qt::Key_Right,      kRightArrowCharCode},
    {Qt::Key_Down,       kDownArrowCharCode},
    {Qt::Key_PageUp,     kPageUpCharCode},
    {Qt::Key_PageDown,   kPageDownCharCode},
        // Shift, Control, Meta, Alt, CapsLock, NumLock, ScrollLock
        // Super_L, Super_R, Menu, Hyper_L, Hyper_R
    {Qt::Key_Help,       kHelpCharCode},
        // Direction_L, Direction_R
    {Qt::Key_nobreakspace, kNonBreakingSpaceCharCode},
    {0, 0}
    };
    
    static QMultiMap<int, uint> scancodes;
    static long lastLayoutID = -1;
    static KeyboardLayoutRef lastLayout = NULL;
    
    void updateScancodes() {
        KeyboardLayoutRef layout;
        if (KLGetCurrentKeyboardLayout(&layout) != noErr) {
            kWarning() << "Error retrieving current layout" << endl;
        }
        if (layout != lastLayout) {
#ifndef NDEBUG
            void *name;
            KLGetKeyboardLayoutProperty(layout, kKLName, const_cast<const void**>(&name));
            kDebug() << "Layout changed to: " << CFStringGetCStringPtr((CFStringRef) name, 0) << endl;
#endif
            lastLayout = layout;
            scancodes.clear();
            void *kchr;
            if (KLGetKeyboardLayoutProperty(layout, kKLKCHRData, const_cast<const void**>(&kchr)) != noErr) {
                kWarning() << "Couldn't load active keyboard layout" << endl;
            } else {
                for (int i = 0; i < 128; i++) {
                    UInt32 tmpState = 0;
                    UInt32 chr = KeyTranslate(kchr, i, &tmpState);
                    if (chr && chr != kFunctionKeyCharCode) {
                        scancodes.insert(chr, i);
                    }
                }
            }
        }
    }
    
#define SCANCODE(name, value) { Qt::Key_ ## name, value }    
    static TransKey functionKeys[] = {
        SCANCODE(F1, 122),
        SCANCODE(F2, 120),
        SCANCODE(F3, 99),
        SCANCODE(F4, 118),
        SCANCODE(F5, 96),
        SCANCODE(F6, 97),
        SCANCODE(F7, 98),
        SCANCODE(F8, 100),
        SCANCODE(F9, 101),
        SCANCODE(F10, 109),
        //TODO: figure out scancodes of other F* keys
    { 0, 0 }
    };
#undef SCANCODE
    
    bool keyQtToSymMac( int keyQt, int& sym )
    {
        // Printable ascii values, before A
        if (keyQt >= 0x20 && keyQt < Qt::Key_A) {
            sym = keyQt;
            return true;
        }
        // Letters, return lower-case equivalent
        if (keyQt >= Qt::Key_A && keyQt <= Qt::Key_Z) {
            sym = keyQt - Qt::Key_A + 'a';
            return true;
        }
        // Printable ascii values up to lower-case a
        if (keyQt > Qt::Key_Z && keyQt <= 0x60) {
            sym = keyQt;
            return true;
        }
        // Remainder of printable ascii values
        if (keyQt >= 0x7B && keyQt < 0x7F) {
            sym = keyQt;
            return true;
        }
        // Function keys
        if (keyQt >= Qt::Key_F1 && keyQt <= Qt::Key_F35) {
            sym = kFunctionKeyCharCode;
            return true;
        }
        // Try to find in lookup table
        for (int i = 0; qtKeyToChar[i].qt_code; i++) {
            if (qtKeyToChar[i].qt_code == keyQt) {
                sym = qtKeyToChar[i].mac_code;
                return true;
            }
        }

        // Not found
        return false;
    }
    
    bool keyQtToCodeMac( int keyQt, QList<uint>& keyCodes )
    {
        updateScancodes();
        keyCodes.clear();
        keyQt &= ~Qt::KeyboardModifierMask;
        int chr;
        if (!keyQtToSymMac( keyQt, chr ) ) return false;
        
        if (chr == kFunctionKeyCharCode) {
            for (int i = 0; functionKeys[i].qt_code; i++) {
                if (functionKeys[i].qt_code == keyQt) {
                    keyCodes.append(functionKeys[i].mac_code);
                }
            }
        } else {
            keyCodes += scancodes.values(chr);
        }
        
        return keyCodes.count() > 0;
    }
    
    bool keyQtToModMac( int keyQt, uint& mod )
    {
        mod = 0;
        if (keyQt & Qt::ShiftModifier) {
            mod |= shiftKey;
        }
        if (keyQt & Qt::ControlModifier) {
            mod |= cmdKey;
        }
        if (keyQt & Qt::AltModifier) {
            mod |= optionKey;
        }
        if (keyQt & Qt::MetaModifier) {
            mod |= controlKey;
        }
        if (keyQt & Qt::KeypadModifier) {
            mod |= kEventKeyModifierNumLockMask;
        }
        // Special case for Qt::Key_Backtab
        if ((keyQt & ~Qt::KeyboardModifierMask) == Qt::Key_Backtab) {
            mod |= shiftKey;
        }
        
        return true;
    }
} // end of namespace KKeyServer

#endif // Q_WS_MACX

