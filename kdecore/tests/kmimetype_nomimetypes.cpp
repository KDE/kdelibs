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
#include <kcomponentdata.h>
#include <kmimetype.h>
#include <kmessage.h>
#include <kglobal.h>
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

    ::setenv("XDG_DATA_DIRS", "/doesnotexist", 1);
    ::setenv("XDG_DATA_HOME", "/doesnotexisteither", 1);

    KComponentData cData("foo");

    KTestMessageHandler* msgHandler = new KTestMessageHandler;
    KMessage::setMessageHandler(msgHandler);

    KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
    if (s0) {
        qDebug() << "Found" << s0->name() << "for application/x-zerosize that should not exist";
        Q_ASSERT(!s0); // should NOT be found, otherwise this test is bogus!
        abort();
        return 1;
    }

    KMimeType::Ptr mime = KMimeType::findByUrl(KUrl("/"));
    if (!mime) {
        qDebug() << "KMimeType::findByUrl(KUrl(\"/\")) returned no mime";
        Q_ASSERT(mime);
        abort();
        return 2;
    }

    if (mime->name() != "application/octet-stream") {
        qDebug() << "KMimeType::findByUrl(KUrl(\"/\")) returned a mime different than application/octet-stream";
        Q_ASSERT(mime->name() == "application/octet-stream");
        abort();
        return 3;
    }

    qDebug() << mime->name();

    Q_ASSERT(msgHandler->m_messages.count() > 0);

    return 0;
}

