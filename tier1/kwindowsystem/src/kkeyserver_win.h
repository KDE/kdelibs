/*
 Copyright (C) 2008 Carlo Segato <brandon.ml@gmail.com>
 
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

#ifndef _KKEYSERVER_WIN_H
#define _KKEYSERVER_WIN_H

#include <kwindowsystem_export.h>
#include <QtGlobal>

namespace KKeyServer
{
    /**
    * Extracts the modifiers from the given Qt key and
    * converts them in a mask of Windows modifiers.
    * @param keyQt the qt key code
    * @param mod if successful, the modifiers will be written here
    * @return true if successful, false otherwise
    */
    KWINDOWSYSTEM_EXPORT bool keyQtToModWin( int keyQt, uint* mod );

    KWINDOWSYSTEM_EXPORT bool modWinToKeyQt( uint mod, int *keyQt );

   /**
    * Extracts the symbol from the given Qt key and
    * converts it to a Windows symbol.
    * @param keyQt the qt key code
    * @param sym if successful, the symbol will be written here
    * @return true if successful, false otherwise
    */
    KWINDOWSYSTEM_EXPORT bool keyQtToCodeWin( int keyQt, uint* sym );

    KWINDOWSYSTEM_EXPORT bool codeWinToKeyQt( uint sym, int* keyQt );
}

#endif
