/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "kdatatool.h"

#include <kstandarddirs.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kicon.h>
#include <kcomponentdata.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>

#include <kservicetypetrader.h>

#include <qpixmap.h>
#include <qfile.h>

/*************************************************
 *
 * KDataToolInfo
 *
 *************************************************/

KDataToolInfo::KDataToolInfo()
{
    m_service = 0;
}

KDataToolInfo::KDataToolInfo(const KService::Ptr& service, const KComponentData &componentData)
{
    m_service = service;
    m_componentData = componentData;

    if ( !!m_service && !m_service->serviceTypes().contains( "KDataTool" ) )
    {
        kDebug(30003) << "The service " << m_service->name()
                       << " does not feature the service type KDataTool" << endl;
        m_service = 0;
    }
}

KDataToolInfo::KDataToolInfo( const KDataToolInfo& info )
{
    m_service = info.service();
    m_componentData = info.componentData();
}

KDataToolInfo& KDataToolInfo::operator= ( const KDataToolInfo& info )
{
    m_service = info.service();
    m_componentData = info.componentData();
    return *this;
}

QString KDataToolInfo::dataType() const
{
    if ( !m_service )
        return QString();

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
        return true;

    return m_service->property( "ReadOnly" ).toBool();
}

QPixmap KDataToolInfo::icon() const
{
    if ( !m_service )
        return QPixmap();

    QPixmap pix;
    QStringList lst = KGlobal::dirs()->resourceDirs("icon");
    QStringList::ConstIterator it = lst.begin();
    while (!pix.load( *it + '/' + m_service->icon() ) && it != lst.end() )
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
    while (!pix.load( *it + '/' + m_service->icon() ) && it != lst.end() )
        it++;

    return pix;
}

QString KDataToolInfo::iconName() const
{
    if ( !m_service )
        return QString();
    return m_service->icon();
}

QStringList KDataToolInfo::commands() const
{
    if ( !m_service )
        return QStringList();

    return m_service->property( "Commands" ).toStringList();
}

QStringList KDataToolInfo::userCommands() const
{
    if ( !m_service )
        return QStringList();

    return m_service->comment().split( ',', QString::SkipEmptyParts );
}

KDataTool* KDataToolInfo::createTool( QObject* parent ) const
{
    if ( !m_service )
        return 0;

    KDataTool* tool = KService::createInstance<KDataTool>( m_service, parent );
    if ( tool )
        tool->setComponentData(m_componentData);
    return tool;
}

KService::Ptr KDataToolInfo::service() const
{
    return m_service;
}

QList<KDataToolInfo> KDataToolInfo::query(const QString& datatype, const QString& mimetype, const KComponentData &componentData)
{
    QList<KDataToolInfo> lst;

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
/* Bug in KServiceTypeTrader ? Test with HEAD-kdelibs!
    if ( componentData )
    {
        QString tmp = QString::fromLatin1( "not ('%1' in ExcludeFrom)" ).arg( componentData.componentName() );
        if ( constr.isEmpty() )
            constr = tmp;
        else
            constr = constr + " and " + tmp;
    } */

    // Query the trader
    //kDebug() << "KDataToolInfo::query " << constr << endl;
    KService::List offers = KServiceTypeTrader::self()->query( "KDataTool", constr );

    KService::List::ConstIterator it = offers.begin();
    for( ; it != offers.end(); ++it )
    {
        // Temporary replacement for the non-working trader query above
        if (!componentData.isValid() || !(*it)->property("ExcludeFrom").toStringList()
             .contains(componentData.componentName())) {
            lst.append(KDataToolInfo(*it, componentData));
        } else {
            kDebug() << (*it)->entryPath() << " excluded." << endl;
        }
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
                                  QObject *parent )
    : KAction( text, parent ),
      m_command( command ),
      m_info( info )
{
    setIcon( KIcon( info.iconName() ) );
}

void KDataToolAction::slotActivated()
{
    emit toolActivated( m_info, m_command );
}

QList<QAction*> KDataToolAction::dataToolActionList( const QList<KDataToolInfo> & tools, const QObject *receiver, const char* slot, KActionCollection* parent )
{
    QList<QAction*> actionList;
    if ( tools.isEmpty() )
        return actionList;

    QAction *sep_action = new QAction(parent);
    sep_action->setSeparator(true);
    actionList.append( sep_action );
    QList<KDataToolInfo>::ConstIterator entry = tools.begin();
    for( ; entry != tools.end(); ++entry )
    {
        const QStringList userCommands = (*entry).userCommands();
        const QStringList commands = (*entry).commands();
        Q_ASSERT(!commands.isEmpty());
        if ( commands.count() != userCommands.count() )
            kWarning() << "KDataTool desktop file error (" << (*entry).service()->entryPath()
                        << "). " << commands.count() << " commands and "
                        << userCommands.count() << " descriptions." << endl;
        QStringList::ConstIterator uit = userCommands.begin();
        QStringList::ConstIterator cit = commands.begin();
        for (; uit != userCommands.end() && cit != commands.end(); ++uit, ++cit )
        {
            //kDebug() << "creating action " << *uit << " " << *cit << endl;
            const QString name = (*entry).service()->entryPath(); // something unique
            KDataToolAction * action = new KDataToolAction( *uit, *entry, *cit, parent );
            parent->addAction( name, action );
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

KDataTool::KDataTool( QObject* parent )
    : QObject(parent)
{
}

const KComponentData &KDataTool::componentData() const
{
   return m_componentData;
}

#include "kdatatool.moc"
