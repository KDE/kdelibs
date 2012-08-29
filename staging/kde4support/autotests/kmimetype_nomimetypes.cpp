/*  This file is part of the KDE project
    Copyright (C) 2011 David Faure <faure@kde.org>

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

#include <QtCore/QCoreApplication>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>
#include <kcomponentdata.h>
#include <kmimetype.h>
#include <kmessage.h>
#include <kdebug.h>

class KTestMessageHandler : public KMessageHandler
{
public:
    virtual void message(KMessage::MessageType type, const QString &text, const QString &caption)
    {
        Q_UNUSED(type);
        Q_UNUSED(caption);
        m_messages.append(text);
    }

    QStringList m_messages;
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    // Test what happens when there are no mimetypes installed.

    qputenv("XDG_DATA_DIRS", "/doesnotexist");

    KComponentData cData("foo");

    KTestMessageHandler* msgHandler = new KTestMessageHandler;
    KMessage::setMessageHandler(msgHandler);

    KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
    Q_ASSERT(s0); // QMimeType falls back to its own copy freedesktop.org.xml, so this works!
    if (!s0) {
        abort();
        return 1;
    }

    KMimeType::Ptr mime = KMimeType::findByUrl(QUrl::fromLocalFile("/"));
    Q_ASSERT(mime);
    Q_ASSERT(mime->name() == "inode/directory");
    if (!mime) {
        abort();
        return 2;
    }

    Q_ASSERT(msgHandler->m_messages.count() == 0);

    return 0;
}

