/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
  * Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "kpluginselector.h"
#include "kpluginselector_p.h"

#include <QFrame>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <QList>
#include <QLabel>
#include <QBrush>
#include <klocale.h>
#include <ktabwidget.h>
#include <kcomponentdata.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <kconfigbase.h>
#include <kiconloader.h>
#include <kcmodule.h>
#include "kcmoduleinfo.h"
#include "kcmoduleloader.h"
#include "kcmoduleproxy.h"
#include <kconfiggroup.h>

KPluginInfoLVI::KPluginInfoLVI(QString itemTitle, QTreeWidget *parent)
    : QTreeWidgetItem(parent, QStringList(itemTitle))
    , m_pluginInfo(0)
    , m_cfgGroup(0)
    , m_moduleProxy(0)
    , m_cfgWidget(0)
{
}

KPluginInfoLVI::KPluginInfoLVI(KPluginInfo *pluginInfo, QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent, QStringList(pluginInfo->name()))
    , m_pluginInfo(pluginInfo)
    , m_cfgGroup(0)
    , m_moduleProxy(0)
    , m_cfgWidget(0)
{
    QString toolTip = i18n("<qt><table>"
                           "<tr><td><b>Description:</b></td><td>%1</td></tr>"
                           "<tr><td><b>Author:</b></td><td>%2</td></tr>"
                           "<tr><td><b>Version:</b></td><td>%3</td></tr>"
                           "<tr><td><b>License:</b></td><td>%4</td></tr></table></qt>",
                           m_pluginInfo->comment(),
                           m_pluginInfo->author(),
                           m_pluginInfo->version(),
                           m_pluginInfo->license());

    setToolTip(0, toolTip);
}

KPluginInfoLVI::KPluginInfoLVI(QString itemTitle, QTreeWidgetItem *parent,
                               KPluginInfo *pluginInfo)
    : QTreeWidgetItem(parent, QStringList(itemTitle))
    , m_pluginInfo(pluginInfo)
    , m_cfgGroup(0)
    , m_moduleProxy(0)
    , m_cfgWidget(0)
{
    if (pluginInfo)
    {
        QString toolTip = i18n("<qt><table>"
                            "<tr><td><b>Description:</b></td><td>%1</td></tr>"
                            "<tr><td><b>Author:</b></td><td>%2</td></tr>"
                            "<tr><td><b>Version:</b></td><td>%3</td></tr>"
                            "<tr><td><b>License:</b></td><td>%4</td></tr></table></qt>",
                            m_pluginInfo->comment(),
                            m_pluginInfo->author(),
                            m_pluginInfo->version(),
                            m_pluginInfo->license());

        setToolTip(0, toolTip);
    }
}

KPluginInfoLVI::~KPluginInfoLVI()
{
    if (m_cfgWidget)

        delete m_cfgWidget;
}

void KPluginInfoLVI::setPluginInfo(KPluginInfo *pluginInfo)
{
    Q_ASSERT(pluginInfo);

    m_pluginInfo = pluginInfo;
}

void KPluginInfoLVI::setCfgGroup(KConfigGroup *cfgGroup)
{
    Q_ASSERT(cfgGroup);

    m_cfgGroup = cfgGroup;
}

void KPluginInfoLVI::setModuleProxy(KCModuleProxy *moduleProxy)
{
    Q_ASSERT(moduleProxy);

    m_moduleProxy = moduleProxy;
}

void KPluginInfoLVI::setCfgWidget(QWidget *cfgWidget)
{
    Q_ASSERT(cfgWidget);

    m_cfgWidget = cfgWidget;
}

void KPluginInfoLVI::setItemChecked(bool itemChecked)
{
    m_itemChecked = itemChecked;

    if (itemChecked)
        setCheckState(0, Qt::Checked);
    else
        setCheckState(0, Qt::Unchecked);
}

KPluginInfo* KPluginInfoLVI::pluginInfo() const
{
    return m_pluginInfo;
}

KConfigGroup* KPluginInfoLVI::cfgGroup() const
{
    return m_cfgGroup;
}

KCModuleProxy* KPluginInfoLVI::moduleProxy() const
{
    return m_moduleProxy;
}

QWidget* KPluginInfoLVI::cfgWidget() const
{
    return m_cfgWidget;
}

bool KPluginInfoLVI::itemChecked() const
{
    return m_itemChecked;
}

KPluginSelector::KPluginSelector(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    connect(d, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    d->splitter = new QSplitter(Qt::Horizontal);
    d->treeView = new QTreeWidget(d->splitter);
    d->stackedWidget = new QStackedWidget(d->splitter);

    d->treeView->header()->setVisible(false);
    d->treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    d->treeView->setMinimumSize(200, 200);
    d->treeView->setFocusPolicy(Qt::NoFocus);
    d->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    /**
      * This QWidget will show the information text when no selection is done.
      * By default, it will be the 0 index in the QStackedWidget.
      */
    QVBoxLayout *noSelectionWidgetLayout = new QVBoxLayout;
    QWidget *noSelectionWidget = new QWidget(d->stackedWidget);
    noSelectionWidget->setLayout(noSelectionWidgetLayout);
    QLabel *noSelectionLabel = new QLabel(i18n("No item selected"), noSelectionWidget);
    noSelectionLabel->setAlignment(Qt::AlignCenter);
    noSelectionWidgetLayout->addWidget(noSelectionLabel);
    d->stackedWidget->insertWidget(0, noSelectionWidget);

    /**
      * This QWidget will show the information text when no configuration is available.
      * By default, it will be the 1 index in the QStackedWidget.
      */
    QVBoxLayout *noConfigWidgetLayout = new QVBoxLayout;
    QWidget *noConfigWidget = new QWidget(d->stackedWidget);
    noConfigWidget->setLayout(noConfigWidgetLayout);
    QLabel *noConfigLabel = new QLabel(i18n("This plugin is not configurable"), noConfigWidget);
    noConfigLabel->setAlignment(Qt::AlignCenter);
    noConfigWidgetLayout->addWidget(noConfigLabel);
    d->stackedWidget->insertWidget(1, noConfigWidget);

    layout->addWidget(d->splitter);

    connect(d->treeView, SIGNAL(itemClicked(QTreeWidgetItem*, int)), d,
            SLOT(treeWidgetClicked(QTreeWidgetItem*)));

    connect(d->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), d,
            SLOT(showContextMenu(const QPoint&)));
}

KPluginSelector::~KPluginSelector()
{
    delete d;
}

void KPluginSelector::addPlugins(const QString &componentName,
                                 const QString &categoryName,
                                 const QString &category,
                                 KSharedConfig::Ptr config)
{
    QStringList desktopFileNames = KGlobal::dirs()->findAllResources("data",
        componentName + "/kpartplugins/*.desktop", true, false);

    QList<KPluginInfo*> pluginInfoList = KPluginInfo::fromFiles(desktopFileNames);

    if (pluginInfoList.isEmpty())
        return;

    Q_ASSERT(config);
    if (!config)
        config = KSharedConfig::openConfig(componentName);

    KConfigGroup *cfgGroup = new KConfigGroup(config, "KParts Plugins");
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;

    d->configGroupList.append(cfgGroup);

    d->addPluginsInternal(pluginInfoList, categoryName, cfgGroup, category);
}

void KPluginSelector::addPlugins(const KComponentData &instance,
                                 const QString &categoryName,
                                 const QString &category,
                                 const KSharedConfig::Ptr &config)
{
    addPlugins(instance.componentName(), categoryName, category, config);
}

void KPluginSelector::addPlugins(const QList<KPluginInfo*> &pluginInfoList,
                                 const QString &categoryName,
                                 const QString &category,
                                 const KSharedConfig::Ptr &config)
{
    KConfigGroup *cfgGroup = new KConfigGroup(config ? config : KGlobal::config(), "Plugins");
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;

    d->configGroupList.append(cfgGroup);

    d->addPluginsInternal(pluginInfoList, categoryName, cfgGroup, category);
}

void KPluginSelector::load()
{
    KPluginInfoLVI *curItem;
    for (QList<KPluginInfoLVI*>::Iterator it = d->treeItemList.begin();
         it != d->treeItemList.end(); ++it)
    {
        curItem = (*it);

        curItem->pluginInfo()->load(curItem->cfgGroup());
        curItem->setItemChecked(curItem->pluginInfo()->isPluginEnabled());
    }

    KCModuleProxy *curModuleProxy;
    for(QList<KCModuleProxy*>::Iterator it = d->moduleProxyList.begin();
        it != d->moduleProxyList.end(); ++it )
    {
        curModuleProxy = (*it);

        if(curModuleProxy->changed())
            curModuleProxy->load();
    }

    QList<QTreeWidgetItem*> selectedItems = d->treeView->selectedItems();

    if (selectedItems.size() == 0)
        return;

    KPluginInfoLVI *selectedPluginInfoLVI = static_cast<KPluginInfoLVI*>(selectedItems[0]);

    if (selectedPluginInfoLVI->moduleProxy())

        d->stackedWidget->setEnabled(selectedPluginInfoLVI->pluginInfo()->isPluginEnabled());
}

void KPluginSelector::save()
{
    KPluginInfoLVI *curItem;
    for (QList<KPluginInfoLVI*>::Iterator it = d->treeItemList.begin();
         it != d->treeItemList.end(); ++it)
    {
        curItem = (*it);

        curItem->pluginInfo()->setPluginEnabled(curItem->itemChecked());
        curItem->pluginInfo()->save(curItem->cfgGroup());
    }

    /**
      * We actually have to save changes by writing into the config files
      */
    KConfigGroup *curConfigGroup;
    for (QList<KConfigGroup*>::Iterator it = d->configGroupList.begin();
         it != d->configGroupList.end(); ++it)
    {
        curConfigGroup = (*it);

        curConfigGroup->sync();
    }

    KCModuleProxy *curModuleProxy;
    for (QList<KCModuleProxy*>::Iterator it = d->moduleProxyList.begin();
         it != d->moduleProxyList.end(); ++it)
    {
        curModuleProxy = (*it);

        if (curModuleProxy->changed())
        {
            curModuleProxy->save();

            QStringList names = d->moduleParentComponents[curModuleProxy];

            if (names.isEmpty())
               names.append(QString());

            QString curName;
            QStringList updatedModules;
            for (QStringList::ConstIterator it = names.begin();
                 it != names.end(); ++it )
            {
                curName = (*it);

                if (!updatedModules.contains(curName))
                    updatedModules.append(curName);
            }

            for (QStringList::ConstIterator it = updatedModules.begin();
                 it != updatedModules.end(); ++it)
            {
                curName = (*it);

                emit configCommitted(curName.toLatin1());
            }
        }
    }
}


void KPluginSelector::defaults()
{
    /**
      * This method may use defaults() from KCModuleProxy, but since by now this does not do
      * anything, we can set this to a more useful behaviour: loading the last saved config (ereslibre)
      */

    load();
}

QWidget* KPluginSelector::Private::insertKCM(QWidget *parent,
                                    const KCModuleInfo &moduleinfo,
                                    KPluginInfoLVI *listViewItem)
{
    Q_ASSERT(listViewItem);

    KCModuleProxy *module = new KCModuleProxy(moduleinfo, parent);

    if (!module->realModule())
    {
        QString errorTitle = i18n("<b>Error while retrieving plugin configuration dialog</b>");
        QString errorMessage = i18n("A error ocurred while trying to load the configuration dialog of the current plugin (<b>%1</b>) in the library <b>%2</b>", module->moduleInfo().moduleName(), module->moduleInfo().library());

        QVBoxLayout *errorTitleLayout = new QVBoxLayout;
        QWidget *errorWidget = new QWidget;
        errorWidget->setLayout(errorTitleLayout);
        QLabel *errorLabel = new QLabel(errorTitle, errorWidget);
        errorLabel->setWordWrap(true);
        QLabel *errorLabel2 = new QLabel(errorMessage, errorWidget);
        errorLabel2->setWordWrap(true);
        errorLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        errorLabel2->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        errorTitleLayout->addWidget(errorLabel);
        errorTitleLayout->addWidget(errorLabel2);

        return errorWidget;
    }

    QStringList parentComponents = moduleinfo.service()->property("X-KDE-ParentComponents").toStringList();
    moduleParentComponents.insert(module, parentComponents);

    connect(module, SIGNAL(changed(bool)), this->parent, SIGNAL(changed(bool)));

    moduleProxyList.append(module);

    listViewItem->setModuleProxy(module);

    return module;
}

void KPluginSelector::Private::treeWidgetClicked(QTreeWidgetItem *item)
{
    KPluginInfoLVI *treeWidgetItem = static_cast<KPluginInfoLVI*>(item);
    bool isCategory = false;
    int widgetIndex = -1;

    if (!treeItemList.contains(treeWidgetItem)) // Is a category title
    {
        isCategory = true;

        if (item->isExpanded()) 

            item->setExpanded(false);

        else

            item->setExpanded(true);
    }

    if (treeWidgetItem->cfgWidget())

        widgetIndex = stackedWidget->indexOf(treeWidgetItem->cfgWidget());

    else if (!isCategory)
    {
        QVBoxLayout *newWidgetLayout = new QVBoxLayout;

        int numServices = treeWidgetItem->pluginInfo()->kcmServices().size();

        if (numServices == 0)

            stackedWidget->setCurrentIndex(1);

        else if (numServices == 1)
        {
            KCModuleInfo moduleinfo(treeWidgetItem->pluginInfo()->kcmServices().front());
            QWidget *module = insertKCM(stackedWidget, moduleinfo, treeWidgetItem);
            module->setLayout(newWidgetLayout);

            widgetIndex = stackedWidget->addWidget(module);
            treeWidgetItem->setCfgWidget(module);
        }
        else
        {
            KTabWidget *newTabWidget = new KTabWidget(stackedWidget);
            newTabWidget->setLayout(newWidgetLayout);

            KService::Ptr servicePtr;
            for(QList<KService::Ptr>::ConstIterator it =
                treeWidgetItem->pluginInfo()->kcmServices().begin();
                it != treeWidgetItem->pluginInfo()->kcmServices().end(); ++it)
            {
                servicePtr = (*it);

                if(!servicePtr->noDisplay())
                {
                    KCModuleInfo moduleinfo(servicePtr);
                    QWidget *module = insertKCM(newTabWidget, moduleinfo, treeWidgetItem);
                    newTabWidget->addTab(module, moduleinfo.moduleName());
                }
            }

            widgetIndex = stackedWidget->addWidget(newTabWidget);
            treeWidgetItem->setCfgWidget(newTabWidget);
        }
    }
    else
    {
        QString title = i18n("Category: <b>%1</b>", treeWidgetItem->text(0));

        QVBoxLayout *categoryWidgetLayout = new QVBoxLayout;
        QWidget *categoryWidget = new QWidget(stackedWidget);
        categoryWidget->setLayout(categoryWidgetLayout);
        QLabel *categoryLabel = new QLabel(title, categoryWidget);
        categoryLabel->setWordWrap(true);
        QLabel *categoryLabel2 = new QLabel(i18n("You can navigate through category items by selecting them on the tree"), categoryWidget);
        categoryLabel2->setWordWrap(true);
        categoryLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        categoryLabel2->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        categoryWidgetLayout->addWidget(categoryLabel);
        categoryWidgetLayout->addWidget(categoryLabel2);
        stackedWidget->addWidget(categoryWidget);

        widgetIndex = stackedWidget->addWidget(categoryWidget);
        treeWidgetItem->setCfgWidget(categoryWidget);
    }

    if (!isCategory)
    {
        if (treeWidgetItem->itemChecked() != (item->checkState(0) == Qt::Checked))
        {
            treeWidgetItem->setItemChecked(item->checkState(0) == Qt::Checked);

            emit changed(true);
        }

        // Check for dependencies
        if (treeWidgetItem->itemChecked())
            checkDependencies(treeWidgetItem->pluginInfo(),
                              DependenciesINeed);
        else
            checkDependencies(treeWidgetItem->pluginInfo(),
                              DependenciesNeedMe);
    }

    if ((treeWidgetItem->moduleProxy() == 0) || isCategory)

        stackedWidget->setEnabled(true);

    else

        stackedWidget->setEnabled(treeWidgetItem->itemChecked());

    stackedWidget->setCurrentIndex(widgetIndex);
}

void KPluginSelector::Private::showContextMenu(const QPoint &point)
{
    contextualMenu->exec(treeView->mapToGlobal(point));
}

void KPluginSelector::Private::contextualAction(QAction *action)
{
    if (action->text() == i18n("Expand all"))

        treeView->expandAll();

    else

        treeView->collapseAll();
}

void KPluginSelector::Private::checkDependencies(const KPluginInfo *info,
                                                 CheckWhatDependencies whatDependencies)
{
    if (whatDependencies == DependenciesINeed) // If I was checked, check my dependencies
    {
        QString curDependency;
        for (QStringList::ConstIterator it = info->dependencies().begin();
            it != info->dependencies().end(); ++it)
        {
            curDependency = (*it);

            if (treeItemPluginNames.contains(curDependency))
            {
                if (!treeItemPluginNames[curDependency]->itemChecked())
                {
                    treeItemPluginNames[curDependency]->setItemChecked(true);

                    checkDependencies(treeItemPluginNames[curDependency]->pluginInfo(),
                                      DependenciesINeed);
                }
            }
        }
    }
    else // If I was unchecked, uncheck all items that depends on me
    {
        KPluginInfoLVI *pluginInfo;
        bool dependencyFound;
        for (QList<KPluginInfoLVI*>::ConstIterator it = treeItemList.begin();
             it != treeItemList.end(); ++it)
        {
            pluginInfo = (*it);
            dependencyFound = false;

            QString curDependency;
            for (QStringList::ConstIterator it = pluginInfo->pluginInfo()->dependencies().begin();
                 it != pluginInfo->pluginInfo()->dependencies().end() && !dependencyFound; ++it)
            {
                curDependency = (*it);

                if (curDependency == info->pluginName())
                {
                    if (pluginInfo->itemChecked())
                    {
                        pluginInfo->setItemChecked(false);

                        checkDependencies(pluginInfo->pluginInfo(),
                                          DependenciesNeedMe);
                    }

                    dependencyFound = true;
                }
            }
        }
    }
}

void KPluginSelector::Private::addPluginsInternal(const QList<KPluginInfo*> &pluginInfoList,
                                                  const QString &categoryName,
                                                  KConfigGroup *cfgGroup,
                                                  const QString &category)
{
    QTreeWidgetItem *currentTitle;
    QString pluginCategory = categoryName.toLower();

    int validPlugins = 0;

    /**
      * First check for valid plugins. If a plugin is hidden or the category doesn't
      * match the category in which we currently are, is not valid. If no valid plugins
      * on the list, continuing has no sense
      */

    KPluginInfo *pluginInfo;
    for (QList<KPluginInfo*>::ConstIterator it = pluginInfoList.begin();
         it != pluginInfoList.end() && validPlugins == 0; ++it)
    {
        pluginInfo = (*it);

        if (!pluginInfo->isHidden() &&
            (category.isNull() || pluginInfo->category() == category))

            validPlugins++;
    }

    if (validPlugins == 0)
        return;

    /**
      * Check whether the category was created before or not. If we match
      * a previous added category, plugins will be added to that title
      */
    if (!categories.contains(pluginCategory))
    {
        KPluginInfoLVI *title = new KPluginInfoLVI(categoryName, treeView);

        currentTitle = title;

        treeView->expandItem(title);

        titles.insert(pluginCategory, title);
    }
    else

        currentTitle = titles[pluginCategory];

    KPluginInfoLVI *newItem;
    for (QList<KPluginInfo*>::ConstIterator it = pluginInfoList.begin();
         it != pluginInfoList.end(); ++it)
    {
        pluginInfo = (*it);

        if (!pluginInfo->isHidden() && (category.isNull() ||
                                        pluginInfo->category() == category))
        {
            newItem = new KPluginInfoLVI(pluginInfo, currentTitle);

            categories.insert(pluginCategory, newItem);

            if(!pluginInfo->icon().isEmpty())
                newItem->setIcon(0, SmallIcon(pluginInfo->icon(), IconSize(K3Icon::Small)));

            newItem->setCfgGroup(cfgGroup);
            newItem->pluginInfo()->load(newItem->cfgGroup());
            newItem->setItemChecked(newItem->pluginInfo()->isPluginEnabled());

            treeItemList.append(newItem);
            treeItemPluginNames.insert(pluginInfo->pluginName(), newItem);
        }
    }
}

#include "kpluginselector_p.moc"
#include "kpluginselector.moc"
