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
//#include "componentsdialog_p.h"

#include <klocale.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <kservicetypetrader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kiconloader.h>
#include <QtCore/QFile>
#include <QtGui/QCheckBox>
#include <QtCore/QStack>

uint qHash(const KCModuleInfo &info)
{
    return qHash(info.fileName());
}

namespace KSettings
{

Dialog::Dialog(QWidget *parent)
    : KCMultiDialog(*new DialogPrivate, new KPageWidget, parent)
{
}

Dialog::Dialog(const QStringList &components, QWidget *parent)
    : KCMultiDialog(*new DialogPrivate, new KPageWidget, parent)
{
    Q_D(Dialog);
    d->components = components;
}

Dialog::~Dialog()
{
}

void Dialog::setAllowComponentSelection(bool selection)
{
    d_func()->staticlistview = !selection;
}

bool Dialog::allowComponentSelection() const
{
    return !d_func()->staticlistview;
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

void Dialog::addPluginInfos(const KPluginInfo::List &plugininfos)
{
    Q_D(Dialog);
    for (KPluginInfo::List::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it ) {
        d->registeredComponents.append(it->pluginName());
        if (it->kcmServices().isEmpty()) {
            // this plugin has no kcm services, still we want to show the disable/enable stuff
            // so add a dummy kcm
            KService::Ptr service = it->service();
            d->kcmInfos << KCModuleInfo(service);
            continue;
        }
        foreach (const KService::Ptr &service, it->kcmServices()) {
            d->kcmInfos << KCModuleInfo(service);
        }
    }

    // The plugin, when disabled, disables all the KCMs described by kcmServices().
    // - Normally they are grouped using a .setdlg file so that the group parent can get a
    // checkbox to enable/disable the plugin.
    // - If the plugin does not belong to a group and has only one KCM the checkbox can be
    // used with this KCM.
    // - If the plugin belongs to a group but there are other modules in the group that do not
    // belong to this plugin we give a kError and show no checkbox
    // - If the plugin belongs to multiple groups we give a kError and show no checkbox
    d->plugininfos = plugininfos;
}

KPluginInfo::List Dialog::pluginInfos() const
{
    return d_func()->plugininfos;
}

void Dialog::showEvent(QShowEvent *)
{
    Q_D(Dialog);
    if (d->firstshow) {
        setUpdatesEnabled(false);
        d->kcmInfos += d->instanceServices();
        if (!d->components.isEmpty()) {
            d->kcmInfos += d->parentComponentsServices(d->components);
        }
        d->createDialogFromServices();
        d->firstshow = false;
        setUpdatesEnabled(true);
    }
    Dispatcher::syncConfiguration();
}

DialogPrivate::DialogPrivate()
    : staticlistview(true), firstshow(true), pluginStateDirty(0)
{
}

QSet<KCModuleInfo> DialogPrivate::instanceServices()
{
    //kDebug(700) ;
	QString componentName = KGlobal::mainComponent().componentName();
    registeredComponents.append(componentName);
    //kDebug(700) << "calling KServiceGroup::childGroup( " << componentName << " )";
	KServiceGroup::Ptr service = KServiceGroup::childGroup( componentName );

    QSet<KCModuleInfo> ret;

	if( service && service->isValid() )
	{
        //kDebug(700) << "call was successful";
		const KServiceGroup::List list = service->entries();
		for( KServiceGroup::List::ConstIterator it = list.begin();
				it != list.end(); ++it )
		{
			KSycocaEntry::Ptr p = (*it);
			if( p->isType( KST_KService ) )
			{
				//kDebug( 700 ) << "found service";
                ret << KCModuleInfo(KService::Ptr::staticCast(p));
			}
			else
				kWarning( 700 ) << "KServiceGroup::childGroup returned"
					" something else than a KService" << endl;
		}
	}

	return ret;
}

QSet<KCModuleInfo> DialogPrivate::parentComponentsServices(const QStringList &kcdparents)
{
    registeredComponents += kcdparents;
    QString constraint = kcdparents.join("' in [X-KDE-ParentComponents]) or ('");
	constraint = "('" + constraint + "' in [X-KDE-ParentComponents])";

    //kDebug(700) << "constraint = " << constraint;
    const QList<KService::Ptr> services = KServiceTypeTrader::self()->query("KCModule", constraint);
    QSet<KCModuleInfo> ret;
    foreach (const KService::Ptr &service, services) {
        ret << KCModuleInfo(service);
    }
    return ret;
}

bool DialogPrivate::isPluginForKCMEnabled(const KCModuleInfo *moduleinfo, KPluginInfo &pinfo) const
{
	// if the user of this class requested to hide disabled modules
	// we check whether it should be enabled or not
	bool enabled = true;
    //kDebug(700) << "check whether the '" << moduleinfo->moduleName() << "' KCM should be shown";
	// for all parent components
	const QStringList parentComponents = moduleinfo->service()->property(
			"X-KDE-ParentComponents" ).toStringList();
	for( QStringList::ConstIterator pcit = parentComponents.begin();
			pcit != parentComponents.end(); ++pcit )
	{
		// if the parentComponent is not registered ignore it
        if (!registeredComponents.contains(*pcit)) {
			continue;
        }

		// we check if the parent component is a plugin
        // if not the KCModule must be enabled
        enabled = true;
        if (pinfo.pluginName() == *pcit) {
            // it is a plugin: we check whether the plugin is enabled
            pinfo.load();
            enabled = pinfo.isPluginEnabled();
            //kDebug(700) << "parent " << *pcit << " is " << (enabled ? "enabled" : "disabled");
        }
        // if it is enabled we're done for this KCModuleInfo
        if (enabled) {
            return true;
        }
	}
	return enabled;
}

bool DialogPrivate::isPluginImmutable(const KPluginInfo &pinfo) const
{
    return pinfo.property("X-KDE-PluginInfo-Immutable").toBool();
}

KPageWidgetItem *DialogPrivate::createPageItem(KPageWidgetItem *parentItem,
                                               const QString &name, const QString &comment,
                                               const QString &iconName, int weight)
{
    Q_Q(Dialog);
    QWidget * page = new QWidget( q );

    QCheckBox *checkBox = new QCheckBox(i18n("Enable component"), page);
    QLabel *iconLabel = new QLabel(page);
    QLabel *commentLabel = new QLabel(comment, page);
    commentLabel->setTextFormat(Qt::RichText);
    QVBoxLayout * layout = new QVBoxLayout(page);
    layout->addWidget(checkBox);
    layout->addWidget(iconLabel);
    layout->addWidget(commentLabel);
    layout->addStretch();
    page->setLayout(layout);

    KPageWidgetItem *item = new KPageWidgetItem(page, name);
    item->setIcon(KIcon(iconName));
    iconLabel->setPixmap(item->icon().pixmap(128, 128));
    item->setProperty("_k_weight", weight);
    checkBoxForItem.insert(item, checkBox);

    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
    Q_ASSERT(model);

    if (parentItem) {
        const QModelIndex parentIndex = model->index(parentItem);
        const int siblingCount = model->rowCount(parentIndex);
        int row = 0;
        for (; row < siblingCount; ++row) {
            KPageWidgetItem *siblingItem = model->item(parentIndex.child(row, 0));
            if (siblingItem->property("_k_weight").toInt() > weight) {
                // the item we found is heavier than the new module
                q->insertPage(siblingItem, item);
                break;
            }
        }
        if (row == siblingCount) {
            // the new module is either the first or the heaviest item
            q->addSubPage(parentItem, item);
        }
    } else {
        const int siblingCount = model->rowCount();
        int row = 0;
        for (; row < siblingCount; ++row) {
            KPageWidgetItem *siblingItem = model->item(model->index(row, 0));
            if (siblingItem->property("_k_weight").toInt() > weight) {
                // the item we found is heavier than the new module
                q->insertPage(siblingItem, item);
                break;
            }
        }
        if (row == siblingCount) {
            // the new module is either the first or the heaviest item
            q->addPage(item);
        }
    }

    return (item);
}

void DialogPrivate::parseGroupFile( const QString & filename )
{
	KConfig file( filename, KConfig::SimpleConfig );
    const QStringList groups = file.groupList();
    foreach (const QString &group, groups) {
        if (group.isEmpty()) {
            continue;
        }
        KConfigGroup conf(&file, group);

        const QString parentId = conf.readEntry("Parent");
        KPageWidgetItem *parentItem = pageItemForGroupId.value(parentId);
        KPageWidgetItem *item = createPageItem(parentItem, conf.readEntry("Name"), conf.readEntry("Comment"),
                                               conf.readEntry("Icon"), conf.readEntry("Weight", 100));
        pageItemForGroupId.insert(group, item);
    }
}

void DialogPrivate::createDialogFromServices()
{
    Q_Q(Dialog);
	// read .setdlg files
	QString setdlgpath = KStandardDirs::locate( "appdata",
                                                    KGlobal::mainComponent().componentName() + ".setdlg" );
	const QStringList setdlgaddon = KGlobal::dirs()->findAllResources( "appdata",
			"ksettingsdialog/*.setdlg" );
    if (!setdlgpath.isNull()) {
        parseGroupFile(setdlgpath);
    }
    if (setdlgaddon.size() > 0) {
        for (QStringList::ConstIterator it = setdlgaddon.begin(); it != setdlgaddon.end(); ++it) {
            parseGroupFile(*it);
        }
    }

    //kDebug(700) << kcmInfos.count();
    foreach (const KCModuleInfo &info, kcmInfos) {
        const QStringList parentComponents = info.service()->property("X-KDE-ParentComponents").toStringList();
        bool blacklisted = false;
        foreach (const QString &parentComponent, parentComponents) {
            if (componentBlacklist.contains(parentComponent)) {
                blacklisted = true;
                break;
            }
        }
        if (blacklisted) {
            continue;
        }
        const QString parentId = info.service()->property("X-KDE-CfgDlgHierarchy", QVariant::String).toString();
        KPageWidgetItem *parent = pageItemForGroupId.value(parentId);
        if (!parent) {
            // dummy kcm
            bool foundPlugin = false;
            foreach (const KPluginInfo &pinfo, plugininfos) {
                if (pinfo.service() == info.service()) {
                    if (!pinfo.kcmServices().count()) {
                        const KService::Ptr service = info.service();
                        // FIXME get weight from service or plugin info
                        const int weight = 1000;
                        KPageWidgetItem *item = createPageItem(0, service->name(), service->comment(), service->icon(), weight);
                        connectItemCheckBox(item, pinfo, pinfo.isPluginEnabled());
                        foundPlugin = true;
                        break;
                    }
                }
            }
            if (foundPlugin) {
                continue;
            }
        }
        KPageWidgetItem *item = q->addModule(info, parent, arguments);
        kDebug(700) << "added KCM '" << info.moduleName() << "'";
        foreach (KPluginInfo pinfo, plugininfos) {
            kDebug(700) << pinfo.pluginName();
            if (pinfo.kcmServices().contains(info.service())) {
                const bool isEnabled = isPluginForKCMEnabled(&info, pinfo);
                kDebug(700) << "correct KPluginInfo for this KCM";
                // this KCM belongs to a plugin
                if (parent && pinfo.kcmServices().count() >= 1) {
                    item->setEnabled(isEnabled);
                    const KPluginInfo &plugin = pluginForItem.value(parent);
                    if (plugin.isValid()) {
                        if (plugin != pinfo) {
                            kError(700) << "A group contains more than one plugin: '"
                                << plugin.pluginName() << "' and '" << pinfo.pluginName()
                                << "'. Now it won't be possible to enable/disable the plugin."
                                << endl;
                            parent->setCheckable(false);
                            q->disconnect(parent, SIGNAL(toggled(bool)), q, SLOT(_k_updateEnabledState(bool)));
                        }
                        // else everything is fine
                    } else {
                        connectItemCheckBox(parent, pinfo, isEnabled);
                    }
                } else {
                    pluginForItem.insert(item, pinfo);
                    item->setCheckable(!isPluginImmutable(pinfo));
                    item->setChecked(isEnabled);
                    q->connect(item, SIGNAL(toggled(bool)), q, SLOT(_k_updateEnabledState(bool)));
                }
                break;
            }
        }
    }
    // now that the KCMs are in, check for empty groups and remove them again
    {
        const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
        const QHash<QString, KPageWidgetItem *>::ConstIterator end = pageItemForGroupId.constEnd();
        QHash<QString, KPageWidgetItem *>::ConstIterator it = pageItemForGroupId.constBegin();
        for (; it != end; ++it) {
            const QModelIndex index = model->index(it.value());
            KPluginInfo pinfo;
            foreach (const KPluginInfo &p, plugininfos) {
                if (p.name()==it.key()) {
                    pinfo = p;
                    break;
                }
            }
            bool allowEmpty = false;
            if (pinfo.isValid()) {
                allowEmpty = pinfo.property("X-KDE-PluginInfo-AllowEmptySettings").toBool();
            }

            if (!index.child(0, 0).isValid()) {
                // no children, and it's not allowed => remove this item
                if (!allowEmpty) {
                    q->removePage(it.value());
                } else {
                    connectItemCheckBox(it.value(), pinfo, pinfo.isPluginEnabled());
                }
            }
        }
    }

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
    //q->showButton(KDialog::User1, true);

    QObject::connect(q, SIGNAL(okClicked()), q, SLOT(_k_syncConfiguration()));
    QObject::connect(q, SIGNAL(applyClicked()), q, SLOT(_k_syncConfiguration()));
    QObject::connect(q, SIGNAL(configCommitted(const QByteArray &)), q,
            SLOT(_k_reparseConfiguration(const QByteArray &)));
}

void DialogPrivate::connectItemCheckBox(KPageWidgetItem *item, const KPluginInfo &pinfo, bool isEnabled)
{
    Q_Q(Dialog);
    QCheckBox *checkBox = checkBoxForItem.value(item);
    Q_ASSERT(checkBox);
    pluginForItem.insert(item, pinfo);
    item->setCheckable(!isPluginImmutable(pinfo));
    item->setChecked(isEnabled);
    checkBox->setVisible(!isPluginImmutable(pinfo));
    checkBox->setChecked(isEnabled);
    q->connect(item, SIGNAL(toggled(bool)), q, SLOT(_k_updateEnabledState(bool)));
    q->connect(item, SIGNAL(toggled(bool)), checkBox, SLOT(setChecked(bool)));
    q->connect(checkBox, SIGNAL(clicked(bool)), item, SLOT(setChecked(bool)));
}

void DialogPrivate::_k_syncConfiguration()
{
    Q_Q(Dialog);
    const QHash<KPageWidgetItem *, KPluginInfo>::Iterator endIt = pluginForItem.end();
    QHash<KPageWidgetItem *, KPluginInfo>::Iterator it = pluginForItem.begin();
    for (; it != endIt; ++it) {
        KPageWidgetItem *item = it.key();
        KPluginInfo pinfo = it.value();
        pinfo.setPluginEnabled(item->isChecked());
        pinfo.save();
    }
    if (pluginStateDirty > 0) {
      emit q->pluginSelectionChanged();
      pluginStateDirty = 0;
    }
    Dispatcher::syncConfiguration();
}

void DialogPrivate::_k_reparseConfiguration(const QByteArray &a)
{
    Dispatcher::reparseConfiguration(a);
}

/*
void DialogPrivate::_k_configureTree()
{
	kDebug( 700 ) ;
    QObject::connect(subdlg, SIGNAL(okClicked()), q, SLOT(_k_updateTreeList()));
    QObject::connect(subdlg, SIGNAL(applyClicked()), q, SLOT(_k_updateTreeList()));
    QObject::connect(subdlg, SIGNAL(okClicked()), q, SIGNAL(pluginSelectionChanged()));
    QObject::connect(subdlg, SIGNAL(applyClicked()), q, SIGNAL(pluginSelectionChanged()));
}
*/

void DialogPrivate::_k_clientChanged()
{
    if (pluginStateDirty > 0) {
        Q_Q(Dialog);
        q->enableButton(KDialog::Apply, true);
    } else {
        KCMultiDialogPrivate::_k_clientChanged();
    }
}

void DialogPrivate::_k_updateEnabledState(bool enabled)
{
    Q_Q(Dialog);
    KPageWidgetItem *item = qobject_cast<KPageWidgetItem *>(q->sender());
    if (!item) {
        kWarning(700) << "invalid sender";
        return;
    }

    // iterate over all child KPageWidgetItem objects and check whether they need to be enabled/disabled
    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
    Q_ASSERT(model);
    QModelIndex index = model->index(item);
    if (!index.isValid()) {
        kWarning(700) << "could not find item in model";
        return;
    }

    const KPluginInfo &pinfo = pluginForItem.value(item);
    if (!pinfo.isValid()) {
        kWarning(700) << "could not find KPluginInfo in item";
        return;
    }
    if (pinfo.isPluginEnabled() != enabled) {
        ++pluginStateDirty;
    } else {
        --pluginStateDirty;
    }
    if (pluginStateDirty < 2) {
        _k_clientChanged();
    }

    //kDebug(700) ;

    QModelIndex firstborn = index.child(0, 0);
    if (firstborn.isValid()) {
        //kDebug(700) << "iterating over children";
        // change all children
        index = firstborn;
        QStack<QModelIndex> stack;
        while (index.isValid()) {
            //kDebug(700) << index;
            KPageWidgetItem *item = model->item(index);
            //kDebug(700) << "item->setEnabled(" << enabled << ')';
            item->setEnabled(enabled);
            firstborn = index.child(0, 0);
            if (firstborn.isValid()) {
                stack.push(index);
                index = firstborn;
            } else {
                index = index.sibling(index.row() + 1, 0);
                while (!index.isValid() && !stack.isEmpty()) {
                    index = stack.pop();
                    index = index.sibling(index.row() + 1, 0);
                }
            }
        }
    }
}

} //namespace

#include "dialog.moc"

// vim: ts=4
