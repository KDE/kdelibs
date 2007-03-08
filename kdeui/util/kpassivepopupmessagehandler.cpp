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
#include "kpassivepopupmessagehandler.h"

#include <QtCore/QLatin1String>
#include <QtGui/QPixmap>

#include <kpassivepopup.h>
#include <kicon.h>

KPassivePopupMessageHandler::KPassivePopupMessageHandler(QWidget *parent)
 : QObject(parent), KMessageHandler()
{}

void KPassivePopupMessageHandler::message(KMessage::MessageType messageType, const QString &text, const QString &caption)
{
    showPassivePopup(messageType, text, caption);
}

void KPassivePopupMessageHandler::showPassivePopup(KMessage::MessageType messageType, const QString &text, const QString &caption)
{
    QPixmap resultIcon;
    QString iconName;

    switch(messageType)
    {
        case KMessage::Information:
        default:
            iconName = QLatin1String("dialog-information");
            break;
        case KMessage::Sorry:
        case KMessage::Warning:
            iconName = QLatin1String("dialog-warning");
            break;
        case KMessage::Error:
        case KMessage::Fatal:
            iconName = QLatin1String("dialog-error");
            break;
    }

    resultIcon = KIcon(iconName).pixmap(32);

    KPassivePopup::message(caption, text, resultIcon, parentWidget());
}

QWidget *KPassivePopupMessageHandler::parentWidget()
{
    return qobject_cast<QWidget*>( parent() );
}

#include "kpassivepopupmessagehandler.moc"
// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
