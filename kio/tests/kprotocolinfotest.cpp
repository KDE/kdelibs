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
#include <QFile>
#include <kprotocolmanager.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <assert.h>

int main(int argc, char **argv) {
    KAboutData aboutData(QByteArray("kprotocolinfotest"), QByteArray(), ki18n("KProtocolinfo Test"), QByteArray("1.0"));

    KComponentData componentData(&aboutData);
    QCoreApplication app(argc,argv); // needed by QEventLoop in ksycoca.cpp

    KUrl url("/tmp");
    assert( KProtocolManager::supportsListing( KUrl( "ftp://10.1.1.10") ) );
    assert( KProtocolManager::inputType(url) == KProtocolInfo::T_NONE );
    assert( KProtocolManager::outputType(url) == KProtocolInfo::T_FILESYSTEM );
    assert( KProtocolManager::supportsReading(url) == true );
    KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(url);
    KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
    for ( ; extraFieldsIt != extraFields.end() ; ++extraFieldsIt )
        kDebug() << (*extraFieldsIt).name << " " << (*extraFieldsIt).type;

    assert( KProtocolInfo::showFilePreview( "file" ) == true );
    assert( KProtocolInfo::showFilePreview( "audiocd" ) == false );
    assert( KGlobalSettings::showFilePreview( KUrl( "audiocd:/" ) ) == false );

    QString proxy;
    QString protocol = KProtocolManager::slaveProtocol( KUrl( "http://bugs.kde.org" ), proxy );
    assert( protocol == "http" );

    QStringList capabilities = KProtocolInfo::capabilities( "imap" );
    kDebug() << "kio_imap capabilities: " << capabilities;
    //assert(capabilities.contains("ACL"));

    if (!QFile::exists(KStandardDirs::locate("services", "zip.protocol"))) {
        //QSKIP("kdebase not installed", SkipAll);
    } else {
        QString zip = KProtocolManager::protocolForArchiveMimetype("application/zip");
        assert( zip == "zip");
    }


    return 0;
}
