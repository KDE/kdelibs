/* This file is part of the KDE libraries

   Copyright (C) 1999 Mattias Ettrich (ettrich@kde.org)
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KIPC_h_Included__
#define __KIPC_h_Included__

/**
 * This class implements a very simple IPC mechanism for KDE. You can send
 * a message of a predefined type to either a specific application, or to all
 * KDE application on the current display. The message can carry one integer of
 * data.
 *
 * KIPC is mainly used in KDE for sending "Change Messages", i.e. a message to
 * all KDE apps that a certain setting (the font, for example) has changed.
 * For anything more complex it is recommended to use DCOP -- the Desktop
 * Communications Protocol.
 *
 * Messages with id code < 32 are called "System Messages". These are
 * directly handled by KApplication. Examples are: PaletteChanged and
 * StyleChanged. Messages with id code >= 32 are user messages. KApplication
 * emits the signal kipcMessage(id,arg) for each user message it receives.
 *
 * KIPC is implemented using X11 ClientMessage events.
 *
 * @author Geert Jansen <jansen@kde.org>
 * @version $Id$
 */
class KIPC
{
public:
    /**
     * A identifier for messages. Messages below UserMessage are system
     * messages, messages above can be defined by the user.
     */
    enum Message { PaletteChanged=0, FontChanged, StyleChanged,
                   BackgroundChanged, SettingsChanged, IconChanged, ToolbarStyleChanged,
                   ClipboardConfigChanged, /// @since 3.1
                   UserMessage=32 };

    /**
     * Send a message to a specific application.
     *
     * @param msg The message to send.
     * @param w The window id of a toplevel window of the target application.
     * @param data An optional integer of data.
     */
    static void sendMessage(Message msg, WId w, int data=0);

    /**
     * Send a message to all KDE application on the current display.
     *
     * @param msg The message to send.
     * @param data An optional integer of data.
     */
    static void sendMessageAll(Message msg, int data=0);
};
#endif
