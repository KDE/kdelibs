/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2001 David Faure <david@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qpixmap.h>
#include <qfile.h>
#include "kdatatool.h"
#include <kinstance.h>
#include <ktrader.h>

#include <kstandarddirs.h>

#include <klibloader.h>
#include <kdebug.h>

/*************************************************
 *
 * KDataToolInfo
 *
 *************************************************/

KDataToolInfo::KDataToolInfo()
{
    m_service = 0;
}

KDataToolInfo::KDataToolInfo( const KService::Ptr& service, KInstance* instance )
{
    m_service = service;
    m_instance = instance;

    if ( !!m_service && !m_service->serviceTypes().contains( "KDataTool" ) )
    {
        kdDebug(30003) << "The service " << m_service->name().latin1()
                       << " does not feature the service type KDataTool" << endl;
        m_service = 0;
    }
}

KDataToolInfo::KDataToolInfo( const KDataToolInfo& info )
{
    m_service = info.service();
    m_instance = info.instance();
}

KDataToolInfo& KDataToolInfo::operator= ( const KDataToolInfo& info )
{
    m_service = info.service();
    m_instance = info.instance();
    return *this;
}

QString KDataToolInfo::dataType() const
{
    if ( !m_service )
        return QString::null;

    return m_service->property( "DataType" ).toString();
}

QStringList KDataToolInfo::mimeTypes() const
{
    if ( !m_service )
        return QStringList();

    return m_service->property( "DataMimeTypes" ).toStringList();
}

bool KDataToolInfo::isReadOnly() const
{
    if ( !m_service )
        return TRUE;

    return m_service->property( "ReadOnly" ).toBool();
}

QPixmap KDataToolInfo::icon() const
{
    if ( !m_service )
        return QPixmap();

    QPixmap pix;
    QStringList lst = KGlobal::dirs()->resourceDirs("icon");
    QStringList::ConstIterator it = lst.begin();
    while (!pix.load( *it + "/" + m_service->icon() ) && it != lst.end() )
        it++;

    return pix;
}

QPixmap KDataToolInfo::miniIcon() const
{
    if ( !m_service )
        return QPixmap();

    QPixmap pix;
    QStringList lst = KGlobal::dirs()->resourceDirs("mini");
    QStringList::ConstIterator it = lst.begin();
    while (!pix.load( *it + "/" + m_service->icon() ) && it != lst.end() )
        it++;

    return pix;
}

QString KDataToolInfo::iconName() const
{
    if ( !m_service )
        return QString::null;
    return m_service->icon();
}

QStringList KDataToolInfo::commands() const
{
    if ( !m_service )
        return QString::null;

    return m_service->property( "Commands" ).toStringList();
}

QStringList KDataToolInfo::userCommands() const
{
    if ( !m_service )
        return QString::null;

    return QStringList::split( ',', m_service->comment() );
}

KDataTool* KDataToolInfo::createTool( QObject* parent, const char* name ) const
{
    if ( !m_service )
        return 0;

    KLibFactory* factory = KLibLoader::self()->factory( QFile::encodeName(m_service->library()) );

    if( !factory )
        return 0;

    QObject* obj = factory->create( parent, name );
    if ( !obj || !obj->inherits( "KDataTool" ) )
    {
        delete obj;
        return 0;
    }
    KDataTool * tool = static_cast<KDataTool *>(obj);
    tool->setInstance( m_instance );
    return tool;
}

KService::Ptr KDataToolInfo::service() const
{
    return m_service;
}

QValueList<KDataToolInfo> KDataToolInfo::query( const QString& datatype, const QString& mimetype, KInstance* instance )
{
    QValueList<KDataToolInfo> lst;

    QString constr;

    if ( !datatype.isEmpty() )
    {
        constr = QString::fromLatin1( "DataType == '%1'" ).arg( datatype );
    }
    if ( !mimetype.isEmpty() )
    {
        QString tmp = QString::fromLatin1( "'%1' in DataMimeTypes" ).arg( mimetype );
        if ( constr.isEmpty() )
            constr = tmp;
        else
            constr = constr + " and " + tmp;
    }
/* Bug in KTrader ? Test with HEAD-kdelibs!
    if ( instance )
    {
        QString tmp = QString::fromLatin1( "not ('%1' in ExcludeFrom)" ).arg( instance->instanceName() );
        if ( constr.isEmpty() )
            constr = tmp;
        else
            constr = constr + " and " + tmp;
    } */

    // Query the trader
    //kdDebug() << "KDataToolInfo::query " << constr << endl;
    KTrader::OfferList offers = KTrader::self()->query( "KDataTool", constr );

    KTrader::OfferList::ConstIterator it = offers.begin();
    for( ; it != offers.end(); ++it )
    {
        // Temporary replacement for the non-working trader query above
        if ( !instance || !(*it)->property("ExcludeFrom").toStringList()
             .contains( instance->instanceName() ) )
            lst.append( KDataToolInfo( *it, instance ) );
        else
            kdDebug() << (*it)->entryPath() << " excluded." << endl;
    }

    return lst;
}

bool KDataToolInfo::isValid() const
{
    return( m_service );
}

/*************************************************
 *
 * KDataToolAction
 *
 *************************************************/
KDataToolAction::KDataToolAction( const QString & text, const KDataToolInfo & info, const QString & command,
                                    QObject * parent, const char * name )
    : KAction( text, info.iconName(), 0, parent, name ),
      m_command( command ),
      m_info( info )
{
}

void KDataToolAction::slotActivated()
{
    emit toolActivated( m_info, m_command );
}

QPtrList<KAction> KDataToolAction::dataToolActionList( const QValueList<KDataToolInfo> & tools, const QObject *receiver, const char* slot )
{
    QPtrList<KAction> actionList;
    if ( tools.isEmpty() )
        return actionList;

    actionList.append( new KActionSeparator() );
    QValueList<KDataToolInfo>::ConstIterator entry = tools.begin();
    for( ; entry != tools.end(); ++entry )
    {
        QStringList userCommands = (*entry).userCommands();
        QStringList commands = (*entry).commands();
        Q_ASSERT(!commands.isEmpty());
        if ( commands.count() != userCommands.count() )
            kdWarning() << "KDataTool desktop file error (" << (*entry).service()->entryPath()
                        << "). " << commands.count() << " commands and "
                        << userCommands.count() << " descriptions." << endl;
        QStringList::ConstIterator uit = userCommands.begin();
        QStringList::ConstIterator cit = commands.begin();
        for (; uit != userCommands.end() && cit != commands.end(); ++uit, ++cit )
        {
            //kdDebug() << "creating action " << *uit << " " << *cit << endl;
            KDataToolAction * action = new KDataToolAction( *uit, *entry, *cit );
            connect( action, SIGNAL( toolActivated( const KDataToolInfo &, const QString & ) ),
                     receiver, slot );
            actionList.append( action );
        }
    }

    return actionList;
}

/*************************************************
 *
 * KDataTool
 *
 *************************************************/

KDataTool::KDataTool( QObject* parent, const char* name )
    : QObject( parent, name ), m_instance( 0L )
{
}

KInstance* KDataTool::instance() const
{
   return m_instance;
}

void KDataToolAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KDataTool::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdatatool.moc"
