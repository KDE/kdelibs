/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "dialog.h"
#include "dialog_p.h"

#include "dispatcher.h"
#include "componentsdialog_p.h"

#include <klocale.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <kicon.h>
#include <kservicetypetrader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kiconloader.h>

namespace KSettings
{

Dialog::Dialog(QWidget *parent)
    : KCMultiDialog(parent), d_ptr(new DialogPrivate(this, parent))
{
    Q_D(Dialog);
    d->q_ptr = this;
    d->services = d->instanceServices();
    d->removeDuplicateServices();
}

Dialog::Dialog(const QStringList &components, QWidget *parent)
    : KCMultiDialog(parent), d_ptr(new DialogPrivate(this, parent))
{
    Q_D(Dialog);
    d->q_ptr = this;
    d->services = d->instanceServices() + d->parentComponentsServices(components);
    d->removeDuplicateServices();
}

Dialog::~Dialog()
{
    delete d_ptr;
}

void Dialog::setComponentSelection(ComponentSelection selection)
{
    Q_D(Dialog);
    d->staticlistview = (selection == NoComponentSelection);
}

void Dialog::setKCMArguments(const QStringList& arguments)
{
    Q_D(Dialog);
    d->arguments = arguments;
}

void Dialog::setComponentBlacklist(const QStringList& blacklist)
{
    Q_D(Dialog);
    d->componentBlacklist = blacklist;
}

void Dialog::addPluginInfos( const QList<KPluginInfo*> & plugininfos )
{
    Q_D(Dialog);
	for( QList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
			it != plugininfos.end(); ++it )
	{
		d->registeredComponents.append( ( *it )->pluginName() );
		d->services += ( *it )->kcmServices();
		d->plugininfomap[ ( *it )->pluginName() ] = *it;
	}
}

void Dialog::show()
{
    Q_D(Dialog);
    if (0 == d->dlg) {
        d->createDialogFromServices(this);
    }
    Dispatcher::syncConfiguration();
	return d->dlg->show();
}

DialogPrivate::DialogPrivate(Dialog *d, QWidget *p)
    : pagetree(d), dlg(0), parentwidget(p), staticlistview(true)
{
}

QList<KService::Ptr> DialogPrivate::instanceServices()
{
	kDebug( 700 ) << k_funcinfo << endl;
	QString componentName = KGlobal::mainComponent().componentName();
    registeredComponents.append(componentName);
	kDebug( 700 ) << "calling KServiceGroup::childGroup( " << componentName
		<< " )" << endl;
	KServiceGroup::Ptr service = KServiceGroup::childGroup( componentName );

	QList<KService::Ptr> ret;

	if( service && service->isValid() )
	{
		kDebug( 700 ) << "call was successful" << endl;
		KServiceGroup::List list = service->entries();
		for( KServiceGroup::List::ConstIterator it = list.begin();
				it != list.end(); ++it )
		{
			KSycocaEntry::Ptr p = (*it);
			if( p->isType( KST_KService ) )
			{
				//kDebug( 700 ) << "found service" << endl;
				ret << KService::Ptr::staticCast( p );
			}
			else
				kWarning( 700 ) << "KServiceGroup::childGroup returned"
					" something else than a KService" << endl;
		}
	}

	return ret;
}

QList<KService::Ptr> DialogPrivate::parentComponentsServices(const QStringList &kcdparents)
{
    registeredComponents += kcdparents;
    QString constraint = kcdparents.join("' in [X-KDE-ParentComponents]) or ('");
	constraint = "('" + constraint + "' in [X-KDE-ParentComponents])";

	kDebug( 700 ) << "constraint = " << constraint << endl;
	return KServiceTypeTrader::self()->query( "KCModule", constraint );
}

bool DialogPrivate::isPluginForKCMEnabled(KCModuleInfo *moduleinfo) const
{
	// if the user of this class requested to hide disabled modules
	// we check whether it should be enabled or not
	bool enabled = true;
	kDebug( 700 ) << "check whether the " << moduleinfo->moduleName()
		<< " KCM should be shown" << endl;
	// for all parent components
	QStringList parentComponents = moduleinfo->service()->property(
			"X-KDE-ParentComponents" ).toStringList();
	for( QStringList::ConstIterator pcit = parentComponents.begin();
			pcit != parentComponents.end(); ++pcit )
	{
		// if the parentComponent is not registered ignore it
        if (!registeredComponents.contains(*pcit)) {
			continue;
        }

		// we check if the parent component is a plugin
        if (!plugininfomap.contains(*pcit)) {
			// if not the KCModule must be enabled
			enabled = true;
			// we're done for this KCModuleInfo
			break;
		}
		// if it is a plugin we check whether the plugin is enabled
        KPluginInfo *pinfo = plugininfomap[*pcit];
		pinfo->load();
		enabled = pinfo->isPluginEnabled();
		kDebug( 700 ) << "parent " << *pcit << " is "
			<< ( enabled ? "enabled" : "disabled" ) << endl;
		// if it is enabled we're done for this KCModuleInfo
		if( enabled )
			break;
	}
	return enabled;
}

void DialogPrivate::parseGroupFile( const QString & filename )
{
	KConfig file( filename, KConfig::OnlyLocal );
	QStringList groups = file.groupList();
	for( QStringList::ConstIterator it = groups.begin(); it != groups.end();
			++it )
	{
		GroupInfo group;
		QString id = *it;
                KConfigGroup conf(&file, id.toUtf8() );
		group.id = id;
		group.name = conf.readEntry( "Name" );
		group.comment = conf.readEntry( "Comment" );
		group.weight = conf.readEntry( "Weight", 100 );
		group.parentid = conf.readEntry( "Parent" );
		group.icon = conf.readEntry( "Icon" );
        pagetree.insert(group);
	}
}

void DialogPrivate::createDialogFromServices(KCMultiDialog *parent)
{
	// read .setdlg files
	QString setdlgpath = KStandardDirs::locate( "appdata",
                                                    KGlobal::mainComponent().componentName() + ".setdlg" );
	QStringList setdlgaddon = KGlobal::dirs()->findAllResources( "appdata",
			"ksettingsdialog/*.setdlg" );
    if (!setdlgpath.isNull()) {
        parseGroupFile(setdlgpath);
    }
    if (setdlgaddon.size() > 0) {
        for (QStringList::ConstIterator it = setdlgaddon.begin(); it != setdlgaddon.end(); ++it) {
            parseGroupFile(*it);
        }
    }

	// now we process the KCModule services
    for (QList<KService::Ptr>::ConstIterator it = services.begin(); it != services.end(); ++it) {
		// we create the KCModuleInfo
		KCModuleInfo * info = new KCModuleInfo( *it );
        bool blacklisted = false;
        foreach(QString comp, (*it)->property( "X-KDE-ParentComponents" ).toStringList())
        {
            if( componentBlacklist.contains(comp) ) {
                blacklisted = true;
                break;
            }
        }
        if( blacklisted ) {
            continue;
        }
		QString parentid;
		QVariant tmp = info->service()->property( "X-KDE-CfgDlgHierarchy",
			QVariant::String );
		if( tmp.isValid() )
			parentid = tmp.toString();
        pagetree.insert(info, parentid);
	}

    // At this point pagetree holds a nice structure of the pages we want
	// to show. It's not going to change anymore so we can sort it now.
    pagetree.sort();

	KPageDialog::FaceType faceType = KPageDialog::List;
    if (pagetree.needTree()) {
		faceType = KPageDialog::Tree;
    } else if(pagetree.singleChild()) {
		faceType = KPageDialog::Plain;
    }

	kDebug( 700 ) << "creating KCMultiDialog" << endl;
    dlg = parent;
    dlg->setFaceType(faceType);

	// TODO: Don't show the reset button until the issue with the
	// KPluginSelector::load() method is solved.
	// Problem:
	// KCMultiDialog::show() call KCModule::load() to reset all KCMs
	// (KPluginSelector::load() resets all plugin selections and all plugin
	// KCMs).
	// The reset button calls KCModule::load(), too but in this case we want the
	// KPluginSelector to only reset the current visible plugin KCM and not
	// touch the plugin selections.
	// I have no idea how to check that in KPluginSelector::load()...
    //dlg->showButton(KDialog::User1, true);

/** tokoe: FIXME
    if(!staticlistview) {
        dlg->addButtonBelowList(i18n("Select Components..."), this, SLOT(_k_configureTree()));
    }
*/
    Q_Q(Dialog);
    QObject::connect(dlg, SIGNAL(okClicked()), q, SLOT(_k_syncConfiguration()));
    QObject::connect(dlg, SIGNAL(applyClicked()), q, SLOT(_k_syncConfiguration()));
    QObject::connect(dlg, SIGNAL(configCommitted(const QByteArray &)), q,
            SLOT(_k_reparseConfiguration(const QByteArray &)));

    pagetree.addToDialog(dlg, arguments);
}

void DialogPrivate::_k_syncConfiguration()
{
    Dispatcher::syncConfiguration();
}

void DialogPrivate::_k_reparseConfiguration(const QByteArray &a)
{
    Dispatcher::reparseConfiguration(a);
}

void DialogPrivate::_k_configureTree()
{
	kDebug( 700 ) << k_funcinfo << endl;
    ComponentsDialog *subdlg = new ComponentsDialog(dlg);
    subdlg->setPluginInfos(plugininfomap);
	subdlg->show();
    Q_Q(Dialog);
    QObject::connect(subdlg, SIGNAL(okClicked()), q, SLOT(_k_updateTreeList()));
    QObject::connect(subdlg, SIGNAL(applyClicked()), q, SLOT(_k_updateTreeList()));
    QObject::connect(subdlg, SIGNAL(okClicked()), q, SIGNAL(pluginSelectionChanged()));
    QObject::connect(subdlg, SIGNAL(applyClicked()), q, SIGNAL(pluginSelectionChanged()));
    QObject::connect(subdlg, SIGNAL(finished()), subdlg, SLOT(delayedDestruct()));
}

void DialogPrivate::_k_updateTreeList()
{
	kDebug( 700 ) << k_funcinfo << endl;

    pagetree.makeDirty();

	// remove all pages from the dialog and then add them again. This is needed
	// because KDialogBase/KJanusWidget can only append to the end of the list
	// and we need to have a predefined order.

    pagetree.removeFromDialog(dlg);
    pagetree.addToDialog(dlg, arguments);
}

void DialogPrivate::removeDuplicateServices()
{
	QSet<QString> usedNames;
	QList<KService::Ptr> newlist;
    foreach (KService::Ptr svc, services) {
		if( !usedNames.contains( svc->desktopEntryPath() ) )
		{
			usedNames.insert( svc->desktopEntryPath() );
			newlist.append( svc );
		}
	}
    services.clear();
    services = newlist;
}

} //namespace

#include "dialog.moc"

// vim: ts=4
