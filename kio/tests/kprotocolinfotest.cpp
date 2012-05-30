/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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


#include <kstandarddirs.h>
#include <QApplication>
#include <QFile>
#include <QUrl>
#include <kprotocolmanager.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <QtTest>
#include <kde_qt5_compat.h>

// Tests both KProtocolInfo and KProtocolManager

class KProtocolInfoTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testBasic()
    {
        QUrl url = QUrl::fromLocalFile("/tmp");
        QVERIFY( KProtocolManager::supportsListing( QUrl( "ftp://10.1.1.10") ) );
        QCOMPARE( KProtocolManager::inputType(url), KProtocolInfo::T_NONE );
        QCOMPARE( KProtocolManager::outputType(url), KProtocolInfo::T_FILESYSTEM );
        QVERIFY(KProtocolManager::supportsReading(url));
    }

    void testExtraFields()
    {
        KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(QUrl("trash:/"));
        KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
        for ( ; extraFieldsIt != extraFields.end() ; ++extraFieldsIt )
            qDebug() << (*extraFieldsIt).name << " " << (*extraFieldsIt).type;
        // TODO
    }

    void testShowFilePreview()
    {
        QVERIFY(KProtocolInfo::showFilePreview("file"));
        QVERIFY(!KProtocolInfo::showFilePreview("audiocd"));
        QVERIFY(!KGlobalSettings::showFilePreview(QUrl("audiocd:/")));
    }

    void testSlaveProtocol()
    {
        QString proxy;
        QString protocol = KProtocolManager::slaveProtocol(QUrl("http://bugs.kde.org"), proxy);
        QCOMPARE(protocol, QString::fromLatin1("http"));
    }

    void testCapabilities()
    {
        QStringList capabilities = KProtocolInfo::capabilities( "imap" );
        qDebug() << "kio_imap capabilities: " << capabilities;
        //QVERIFY(capabilities.contains("ACL"));
    }

    void testProtocolForArchiveMimetype()
    {
        if (!QFile::exists(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kde5/services/") + "zip.protocol"))) {
            QSKIP_PORTING("kdebase not installed", SkipAll);
        } else {
            const QString zip = KProtocolManager::protocolForArchiveMimetype("application/zip");
            QCOMPARE(zip, QString("zip"));
        }
    }

};

QTEST_MAIN(KProtocolInfoTest)

#include "kprotocolinfotest.moc"
