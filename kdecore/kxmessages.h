/****************************************************************************

 $Id$

 Copyright (C) 2001 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#ifndef __KXMESSAGES_H
#define __KXMESSAGES_H

#include <qwidget.h>
#include <qcstring.h>
#include <qstring.h>
#include <qmap.h>
#include <X11/X.h>

// TODO docs
/**
 * Description coming soon ... and you most probably don't want to use this anyway.
 */
class KXMessages
    : public QWidget
    {
    Q_OBJECT
    public:
        KXMessages( const char* accept_broadcast = NULL, QWidget* parent = NULL );
        virtual ~KXMessages();
        void sendMessage( WId w, const char* msg_type, const QString& message );
        void broadcastMessage( const char* msg_type, const QString& message );
        static bool sendMessageX( Display* disp, WId w, const char* msg_type,
            const QString& message );
        static bool broadcastMessageX( Display* disp, const char* msg_type,
            const QString& message );
    signals:
        void gotMessage( const QString& message );
    protected:
        virtual bool x11Event( XEvent* ev );
        static void send_message_internal( WId w_P, const QString& msg_P, long mask_P,
            Display* disp, Atom atom_P, Window handle_P );
        QWidget* handle;
        Atom cached_atom;
        QCString cached_atom_name;
        Atom accept_atom;
        QMap< WId, QCString > incoming_messages;
    private:
        class Private;
        Private* d;
    };

#endif
