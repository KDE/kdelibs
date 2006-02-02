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


#include <kprotocolinfo.h>
#include <kprotocolmanager.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <assert.h>

int main(int argc, char **argv) {
    KAboutData aboutData("kprotocolinfotest", "KProtocolinfo Test",
                        "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KUrl url;
    url.setPath("/tmp");
    assert( KProtocolInfo::supportsListing( KUrl( "ftp://10.1.1.10") ) );
    assert( KProtocolInfo::inputType(url) == KProtocolInfo::T_NONE );
    assert( KProtocolInfo::outputType(url) == KProtocolInfo::T_FILESYSTEM );
    assert( KProtocolInfo::supportsReading(url) == true );
    KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(url);
    KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
    for ( ; extraFieldsIt != extraFields.end() ; ++extraFieldsIt )
        kDebug() << (*extraFieldsIt).name << " " << (*extraFieldsIt).type << endl;

    assert( KProtocolInfo::showFilePreview( "file" ) == true );
    assert( KProtocolInfo::showFilePreview( "audiocd" ) == false );
    assert( KGlobalSettings::showFilePreview( "audiocd:/" ) == false );

    QString proxy;
    QString protocol = KProtocolManager::slaveProtocol( "http://bugs.kde.org", proxy );
    assert( protocol == "http" );

    QStringList capabilities = KProtocolInfo::capabilities( "imap" );
    kDebug() << "kio_imap capabilities: " << capabilities << endl;
    //assert(capabilities.contains("ACL"));

    return 0;
}
