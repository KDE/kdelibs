/* This file is part of the KDE libraries

   Copyright (C) 1999 Mattias Ettrich (ettrich@kde.org)

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

#include <X11/X.h>
#include <X11/Xlib.h>

/**
 * This class implements the "old style" KDE IPC mechanism. This will be
 * obsoleted in the near future by DCOP -- The Desktop Communications
 * Protocol. (current date is Oct 18, 1999)
 *
 * All methods are static here so no need to instantiate.
 *
 * @author Geert Jansen <g.t.jansen@stud.tue.nl>
 * @version $Id$
 */ 

class KIPC
{
public:
    /**
     * Send a message to a window.
     *
     * @param Atom The X Atom identifying the message.
     * @param Window The Window ID of the target window.
     * @param data You can pass one in of data throught this.
     */
    static void sendMessage(Atom msg, Window w, int data=0);

    /**
     * Send a message to a window.
     *
     * @param msg A pointer to a null terminated array of chars containing 
     * the message.
     */
    static void sendMessage(const char *msg, Window w, int data=0);

    /**
     * Send a message to all KDE windows.
     *
     * @param Atom The X Atom identifying the message.
     */
    static void sendMessageAll(Atom msg, int data=0);

    /**
     * Send a message to all KDE windows.
     *
     * @param msg A pointer to a null terminated array of chars containing 
     * the message.
     */
    static void sendMessageAll(const char *msg, int data=0);

    /**
     * Used internally
     */
    static int dropError(Display *, XErrorEvent *);

    /**
     * Used internally
     */
    static long getSimpleProperty(Window w, Atom a);
};
#endif
