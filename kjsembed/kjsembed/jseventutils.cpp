/* This file is part of the KDE libraries
    Copyright (C) 2003,2004,2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2003,2004,2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2003,2004,2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2003,2004,2005, 2006 Erik L. Bunce <kde@bunce.us>

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

#include "jseventutils.h"
#include "qobject_binding.h"

namespace KJSEmbed
{

namespace JSEventUtils
{
    KJS::JSObject *event( KJS::ExecState *exec, const QEvent *ev )
    {
        switch( ev->type() )
        {
            case QEvent::Resize:
                return convertEvent( exec, (QResizeEvent*)ev );
                break;
            case QEvent::Timer:
                return convertEvent( exec, (QTimerEvent*)ev );
                break;
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseMove:
            case QEvent::Enter:
            case QEvent::Leave:
                return convertEvent( exec, (QMouseEvent*)ev );
                break;
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
                return convertEvent( exec, (QKeyEvent*)ev );
                break;
            case QEvent::FocusIn:
            case QEvent::FocusOut:
                return convertEvent( exec, (QFocusEvent*)ev );
                break;
            case QEvent::Paint:
                return convertEvent( exec, (QPaintEvent*)ev );
                break;
            case QEvent::Move:
                return convertEvent( exec, (QMoveEvent*)ev );
                break;
            case QEvent::Create:
            case QEvent::Destroy:
            case QEvent::Show:
            case QEvent::Hide:
            case QEvent::Close:
                return convertEvent( exec, (QCloseEvent*)ev );
                break;
            case QEvent::Quit:
            case QEvent::ParentChange:
            case QEvent::ParentAboutToChange:
            case QEvent::ThreadChange:
            case QEvent::WindowActivate:
            case QEvent::WindowDeactivate:
            case QEvent::ShowToParent:
            case QEvent::HideToParent:
            case QEvent::Wheel:
            case QEvent::WindowTitleChange:
            case QEvent::WindowIconChange:
            case QEvent::ApplicationWindowIconChange:
            case QEvent::ApplicationFontChange:
            case QEvent::ApplicationLayoutDirectionChange:
            case QEvent::ApplicationPaletteChange:
            case QEvent::PaletteChange:
            case QEvent::Clipboard:
            case QEvent::Speech:
            case QEvent::MetaCall:
            case QEvent::SockAct:
            //case QEvent::WinEventAct: conflicts with QEvent::QueryWhatsThis
            case QEvent::DeferredDelete:
            case QEvent::DragEnter:
            case QEvent::DragMove:
            case QEvent::DragLeave:
            case QEvent::Drop:
            case QEvent::DragResponse:
            case QEvent::ChildAdded:
            case QEvent::ChildPolished:
            case QEvent::ChildRemoved:
            case QEvent::ShowWindowRequest:
            case QEvent::PolishRequest:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::UpdateRequest:
            case QEvent::EmbeddingControl:
            case QEvent::ActivateControl:
            case QEvent::DeactivateControl:
            case QEvent::ContextMenu:
            case QEvent::InputMethod:
            case QEvent::AccessibilityPrepare:
            case QEvent::TabletMove:
            case QEvent::LocaleChange:
            case QEvent::LanguageChange:
            case QEvent::LayoutDirectionChange:
            case QEvent::Style:
            case QEvent::TabletPress:
            case QEvent::TabletRelease:
            case QEvent::OkRequest:
            case QEvent::HelpRequest:
            case QEvent::IconDrag:
            case QEvent::FontChange:
            case QEvent::EnabledChange:
            case QEvent::ActivationChange:
            case QEvent::StyleChange:
            case QEvent::IconTextChange:
            case QEvent::ModifiedChange:
            case QEvent::MouseTrackingChange:
            case QEvent::WindowBlocked:
            case QEvent::WindowUnblocked:
            case QEvent::WindowStateChange:
            case QEvent::ToolTip:
            case QEvent::WhatsThis:
            case QEvent::StatusTip:
            case QEvent::ActionChanged:
            case QEvent::ActionAdded:
            case QEvent::ActionRemoved:
            case QEvent::FileOpen:
            case QEvent::Shortcut:
            case QEvent::ShortcutOverride:
            case QEvent::WhatsThisClicked:
            case QEvent::ToolBarChange:
            case QEvent::ApplicationActivated:
            case QEvent::ApplicationDeactivated:
            case QEvent::QueryWhatsThis:
            case QEvent::EnterWhatsThisMode:
            case QEvent::LeaveWhatsThisMode:
            case QEvent::ZOrderChange:
            case QEvent::HoverEnter:
            case QEvent::HoverLeave:
            case QEvent::HoverMove:
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
            case QEvent::AccessibilityHelp:
            case QEvent::AccessibilityDescription:
#endif
            default:
                return convertEvent( exec, ev );
                break;
        }
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QEvent *ev)
    {
        ObjectBinding *evnt = new ObjectBinding( exec, "QEvent", ev );
        evnt->put( exec, "type", KJS::jsNumber( (int) ev->type()) );
        evnt->put( exec, "spontaneous", KJS::jsBoolean(ev->spontaneous()) );
        evnt->put( exec, "isAccepted", KJS::jsBoolean(ev->isAccepted()) );
        return evnt;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QInputEvent *ev)
    {
        KJS::JSObject *iev = convertEvent( exec, (QEvent *)ev );
        iev->put( exec, "modifiers", KJS::jsNumber( ev->modifiers() ));
        return iev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QMouseEvent *ev)
    {
        KJS::JSObject *mev = convertEvent( exec, (QInputEvent *)ev );

        mev->put( exec, "pos", convertToValue( exec, ev->pos() ));
        mev->put( exec, "x", KJS::jsNumber( ev->x() ));
        mev->put( exec, "y", KJS::jsNumber( ev->y() ));

        mev->put( exec, "globalPos", convertToValue( exec, ev->globalPos() ));
        mev->put( exec, "globalX", KJS::jsNumber( ev->globalX() ));
        mev->put( exec, "globalY", KJS::jsNumber( ev->globalY() ));

        mev->put( exec, "button", KJS::jsNumber(ev->button()) );
        mev->put( exec, "buttons", KJS::jsNumber(ev->buttons()) );

        return mev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QPaintEvent *ev)
    {
        KJS::JSObject *pev = convertEvent( exec, (QEvent *)ev );

        pev->put( exec, "rect", convertToValue( exec, ev->rect() ) );
//         pev->put( exec, "region", convertToValue( exec, ev->region() ) );
        return pev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QKeyEvent *ev)
    {
        KJS::JSObject *kev = convertEvent( exec, (QInputEvent *)ev );

        kev->put( exec, "key", KJS::jsNumber(ev->key()) );
        kev->put( exec, "text", KJS::jsString(ev->text()));
        kev->put( exec, "isAutoRepeat", KJS::jsBoolean(ev->isAutoRepeat()) );
        kev->put( exec, "count", KJS::jsNumber( ev->count()) );
        return kev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QInputMethodEvent *ev)
    {
        KJS::JSObject *qev = convertEvent( exec, (QEvent *)ev );

        qev->put( exec, "commitString", KJS::jsString(ev->commitString()) );
        qev->put( exec, "preeditString", KJS::jsString(ev->preeditString()) );
        qev->put( exec, "replacementLength", KJS::jsNumber(ev->replacementLength()) );
        qev->put( exec, "replacementStart", KJS::jsNumber(ev->replacementStart()) );
        return qev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QResizeEvent *ev)
    {
        KJS::JSObject *rev = convertEvent( exec, (QEvent *)ev );

        rev->put( exec, "size", convertToValue( exec, ev->size() ));
        rev->put( exec, "oldSize", convertToValue( exec, ev->oldSize()  ));
        return rev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QCloseEvent *ev)
    {
        KJS::JSObject *cev = convertEvent( exec, (QEvent *)ev );

        cev->put( exec, "isAccepted", KJS::jsBoolean(ev->isAccepted()) );
        return cev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QMoveEvent *ev)
    {
        KJS::JSObject *mev = convertEvent( exec, (QEvent *)ev );

        mev->put( exec, "pos", convertToValue( exec, ev->pos() ));
        mev->put( exec, "oldPos", convertToValue( exec, ev->oldPos() ));
        return mev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QWheelEvent *ev)
    {
        KJS::JSObject *wev = convertEvent( exec, (QInputEvent *)ev );

        wev->put( exec, "delta", KJS::jsNumber(ev->delta()) );
        wev->put( exec, "pos", convertToValue( exec, ev->pos() ) );
        wev->put( exec, "globalPos", convertToValue( exec, ev->globalPos() ) );
        wev->put( exec, "x", KJS::jsNumber(ev->x()) );
        wev->put( exec, "y", KJS::jsNumber(ev->y()) );
        wev->put( exec, "globalX", KJS::jsNumber(ev->globalX()) );
        wev->put( exec, "globalY", KJS::jsNumber(ev->globalY()) );
        wev->put( exec, "orientation", KJS::jsNumber(ev->orientation()) );
        wev->put( exec, "buttons", KJS::jsNumber(ev->buttons()) );

        return wev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState * exec, const QDropEvent * ev)
    {
        KJS::JSObject *dev = convertEvent( exec, (QEvent *)ev );
        dev->put( exec, "pos", convertToValue( exec, ev->pos() ) );
        dev->put( exec, "source", new QObjectBinding(exec, (QObject*)ev->source() ) );
        dev->put( exec, "dropAction", KJS::jsNumber( (int)(ev->dropAction()) ) );
        dev->put( exec, "keyboardModifiers", KJS::jsNumber( (int)(ev->keyboardModifiers()) ) );
        dev->put( exec, "possibleActions", KJS::jsNumber( (int)(ev->possibleActions()) ) );
        dev->put( exec, "proposedAction", KJS::jsNumber( (int)(ev->proposedAction()) ) );
        return dev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QDragMoveEvent *ev)
    {
        KJS::JSObject *mev = convertEvent(exec, (QDropEvent *)ev );
        mev->put( exec, "answerRect", convertToValue( exec, ev->answerRect() ) );
        return mev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QTimerEvent *ev)
    {
        KJS::JSObject *dev = convertEvent( exec, (QEvent *)ev );
        dev->put( exec, "timerId", KJS::jsNumber(ev->timerId()) );
        return dev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QContextMenuEvent *ev)
    {
        KJS::JSObject *cxev = convertEvent( exec, (QInputEvent *)ev );
        cxev->put( exec, "x", KJS::jsNumber(ev->x()) );
        cxev->put( exec, "y", KJS::jsNumber(ev->y()) );
        cxev->put( exec, "globalX", KJS::jsNumber(ev->globalX()) );
        cxev->put( exec, "globalY", KJS::jsNumber(ev->globalY()) );
        cxev->put( exec, "pos",convertToValue( exec, ev->pos() ) );
        cxev->put( exec, "globalPos", convertToValue( exec, ev->globalPos() ) );
        cxev->put( exec, "reason", KJS::jsNumber((int)ev->reason()) );
        return cxev;
    }

    KJS::JSObject *convertEvent( KJS::ExecState *exec, const QFocusEvent *ev)
    {
         KJS::JSObject *dev = convertEvent( exec, (QEvent *)ev );
         dev->put( exec, "gotFocus", KJS::jsBoolean( ev->gotFocus() ) );
         dev->put( exec, "lostFocus", KJS::jsBoolean( ev->lostFocus() ));
         //Gold star trolltech!
         dev->put( exec, "reason", KJS::jsNumber((int) const_cast<QFocusEvent*>( ev )->reason() ) );
         return dev;
    }
}// namespace JSEventUtils
}// namespace KJSEmbed

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
