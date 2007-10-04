/*  This file is part of the KDE libraries
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kmessageboxmessagehandler.h"

#include <kmessagebox.h>

class KMessageBoxMessageHandlerPrivate
{
    public:
        KMessageBoxMessageHandlerPrivate(KMessageBoxMessageHandler *q)
            : q(q)
        {
        }

        void showMessageBox(KMessage::MessageType messageType, const QString &text, const QString &caption);
        QWidget *parentWidget();

        KMessageBoxMessageHandler *q;
};


KMessageBoxMessageHandler::KMessageBoxMessageHandler(QWidget *parent)
 : QObject(parent), d(new KMessageBoxMessageHandlerPrivate(this))
{
}

KMessageBoxMessageHandler::~KMessageBoxMessageHandler()
{
    delete d;
}

void KMessageBoxMessageHandler::message(KMessage::MessageType messageType, const QString &text, const QString &caption)
{
    d->showMessageBox(messageType, text, caption);
}

void KMessageBoxMessageHandlerPrivate::showMessageBox(KMessage::MessageType messageType,
                                                      const QString &text, const QString &caption)
{
    KMessageBox::DialogType dlgType;

    switch (messageType)
    {
        case KMessage::Information:
        default:
            dlgType = KMessageBox::Information;
            break;
        case KMessage::Error:
        case KMessage::Fatal:
            dlgType = KMessageBox::Error;
            break;
        case KMessage::Warning:
        case KMessage::Sorry:
            dlgType = KMessageBox::Sorry;
            break;
    }

    KMessageBox::queuedMessageBox(parentWidget(), dlgType, text, caption);
}

QWidget *KMessageBoxMessageHandlerPrivate::parentWidget()
{
    return qobject_cast<QWidget*>(q->parent());
}

#include "kmessageboxmessagehandler.moc"
// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
