// -*- c++ -*-

/*
*  Copyright (C) 2003, Richard J. Moore <rich@kde.org>
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*/

#ifndef KJSEMBED_JSEVENTUTILS_H
#define KJSEMBED_JSEVENTUTILS_H

#include <QEvent>
#include <QInputEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QWheelEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QTimerEvent>
#include <QContextMenuEvent>

#include <kjs/object.h>

#include "global.h"

namespace KJSEmbed {

class JSObjectProxy;
class JSFactory;

/**
* Utility class that contains the methods for converting event types to JS.
*/
namespace JSEventUtils
{
    KJS::JSObject *event( KJS::ExecState *exec, const QEvent *ev );
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QInputEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QMouseEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QPaintEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QKeyEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QInputMethodEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QResizeEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QFocusEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QCloseEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QMoveEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QWheelEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QDropEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QDragMoveEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QTimerEvent *ev);
    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QContextMenuEvent *ev);
}

} // KJSEmbed

#endif // KJSEMBED_JSEVENTUTILS_H
