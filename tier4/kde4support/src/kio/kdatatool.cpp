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

#include <kactioncollection.h>

#include <kservicetypetrader.h>

#include <QPixmap>
#include <QtCore/QFile>
#include <QDebug>

/*************************************************
 *
 * KDataToolInfo
 *
 *************************************************/
class KDataToolInfo::KDataToolInfoPrivate
{
public:
    KDataToolInfoPrivate()
     : service(0)
    {}

    KService::Ptr service;
    QString componentName;
};

KDataToolInfo::KDataToolInfo()
    : d(new KDataToolInfoPrivate)
{
}

KDataToolInfo::KDataToolInfo(const KService::Ptr& service, const QString &componentName)
    : d(new KDataToolInfoPrivate)
{
    d->service = service;
    d->componentName = componentName;

    if ( !d->service && !d->service->serviceTypes().contains( "KDataTool" ) )
    {
        /*qDebug() << "The service" << d->service->name()
                       << "does not feature the service type KDataTool";*/
        d->service = 0;
    }
}

KDataToolInfo::KDataToolInfo( const KDataToolInfo& info )
    : d(new KDataToolInfoPrivate)
{
    d->service = info.service();
    d->componentName = info.componentName();
}

KDataToolInfo& KDataToolInfo::operator= ( const KDataToolInfo& info )
{
    d->service = info.service();
    d->componentName = info.componentName();
    return *this;
}

KDataToolInfo::~KDataToolInfo()
{
    delete d;
}

QString KDataToolInfo::dataType() const
{
    if ( !d->service )
        return QString();

    return d->service->property( "DataType" ).toString();
}

QStringList KDataToolInfo::mimeTypes() const
{
    if ( !d->service )
        return QStringList();

    return d->service->property( "DataMimeTypes" ).toStringList();
}

bool KDataToolInfo::isReadOnly() const
{
    if ( !d->service )
        return true;

    return d->service->property( "ReadOnly" ).toBool();
}

QString KDataToolInfo::iconName() const
{
    if ( !d->service )
        return QString();
    return d->service->icon();
}

QStringList KDataToolInfo::commands() const
{
    if ( !d->service )
        return QStringList();

    return d->service->property( "Commands" ).toStringList();
}

QStringList KDataToolInfo::userCommands() const
{
    if ( !d->service )
        return QStringList();

    return d->service->comment().split( ',', QString::SkipEmptyParts );
}

KDataTool* KDataToolInfo::createTool( QObject* parent ) const
{
    if ( !d->service )
        return 0;

    KDataTool* tool = d->service->createInstance<KDataTool>(parent);
    if ( tool )
        tool->setComponentName(d->componentName);
    return tool;
}

KService::Ptr KDataToolInfo::service() const
{
    return d->service;
}

QString KDataToolInfo::componentName() const
{
    return d->componentName;
}

QList<KDataToolInfo> KDataToolInfo::query(const QString& datatype, const QString& mimetype, const QString &componentName)
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
    if (!componentName.isEmpty())
    {
        QString tmp = QString::fromLatin1( "not ('%1' in ExcludeFrom)" ).arg(componentName);
        if ( constr.isEmpty() )
            constr = tmp;
        else
            constr = constr + " and " + tmp;
    } */

    // Query the trader
    //qDebug() << constr;
    const KService::List offers = KServiceTypeTrader::self()->query( "KDataTool", constr );

    KService::List::ConstIterator it = offers.begin();
    for( ; it != offers.end(); ++it )
    {
        // Temporary replacement for the non-working trader query above
        if (componentName.isEmpty() || !(*it)->property("ExcludeFrom").toStringList()
             .contains(componentName)) {
            lst.append(KDataToolInfo(*it, componentName));
        } else {
            //qDebug() << (*it)->entryPath() << " excluded.";
        }
    }

    return lst;
}

bool KDataToolInfo::isValid() const
{
    return( d->service );
}

/*************************************************
 *
 * KDataToolAction
 *
 *************************************************/
class KDataToolAction::KDataToolActionPrivate
{
public:
    KDataToolActionPrivate() {}

    QString command;
    KDataToolInfo info;
};

KDataToolAction::KDataToolAction( const QString & text, const KDataToolInfo & info, const QString & command,
                                  QObject *parent )
    : QAction( text, parent ),
      d(new KDataToolActionPrivate)
{
    setIcon( QIcon::fromTheme( info.iconName() ) );
    d->command = command;
    d->info = info;
}

KDataToolAction::~KDataToolAction()
{
    delete d;
}

void KDataToolAction::slotActivated()
{
    emit toolActivated( d->info, d->command );
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
            qWarning() << "KDataTool desktop file error (" << (*entry).service()->entryPath()
                        << ")." << commands.count() << "commands and"
                        << userCommands.count() << " descriptions.";
        QStringList::ConstIterator uit = userCommands.begin();
        QStringList::ConstIterator cit = commands.begin();
        for (; uit != userCommands.end() && cit != commands.end(); ++uit, ++cit )
        {
            //qDebug() << "creating action " << *uit << " " << *cit;
            const QString name = (*entry).service()->entryPath(); // something unique
            KDataToolAction * action = new KDataToolAction( *uit, *entry, *cit, parent );
            parent->addAction( name, action );
            connect( action, SIGNAL(toolActivated(KDataToolInfo,QString)),
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
class KDataTool::KDataToolPrivate
{
public:
    KDataToolPrivate() {}

    QString componentName;
};

KDataTool::KDataTool( QObject* parent )
    : QObject(parent), d(new KDataToolPrivate)
{
}

KDataTool::~KDataTool()
{
    delete d;
}

void KDataTool::setComponentName(const QString &componentName)
{
    d->componentName = componentName;
}

QString KDataTool::componentName() const
{
   return d->componentName;
}

