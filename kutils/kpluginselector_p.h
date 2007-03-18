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

#ifndef KPLUGINSELECTOR_P_H
#define KPLUGINSELECTOR_P_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <kconfigbase.h>
#include <klocale.h>

#include <kdelibs_export.h>
#include <kconfiggroup.h>

class QTreeWidget;
class KPluginInfo;
class KCModuleProxy;
class QSplitter;
class QStackedWidget;
class KPluginSelectionWidget;
class KPluginInfoLVI;

class KPluginSelector::Private
    : public QObject
{
    Q_OBJECT

public:
    Private(KPluginSelector *parent)
        : QObject(parent)
        , parent(parent)
        , splitter(0)
        , treeView(0)
        , stackedWidget(0)
        , contextualMenu(new QMenu(parent))
    {
        contextualMenu->addAction(i18n("Expand all"));
        contextualMenu->addAction(i18n("Collapse all"));

        connect(contextualMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextualAction(QAction*)));
        /*connect(treeView, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this,
                SLOT(treeWidgetClicked(QTreeWidgetItem*)));*/
    }

    ~Private()
    {
        KConfigGroup *curConfigGroup;
        for (QList<KConfigGroup*>::Iterator it = configGroupList.begin();
             it != configGroupList.end(); ++it)
        {
            curConfigGroup = (*it);

            delete curConfigGroup;
        }
    }

    /**
      * Returns the widget that holds the @p moduleinfo configuration. It could be
      * a KTabWidget if more than one service is provided by @p moduleinfo
      */
    QWidget* insertKCM(QWidget *parent,
                       const KCModuleInfo &moduleinfo,
                       KPluginInfoLVI *listViewItem);

    void checkDependencies(const KPluginInfo *info,
                           CheckWhatDependencies whatDependencies);

    void addPluginsInternal(const QList<KPluginInfo*> &pluginInfoList,
                            const QString &categoryName,
                            KConfigGroup *cfgGroup,
                            const QString &category);

public Q_SLOTS:
    void treeWidgetClicked(QTreeWidgetItem *item);

    void showContextMenu(const QPoint &point);

    void contextualAction(QAction *action);

Q_SIGNALS:
    /**
      * Tells you whether the tree is changed or not.
      */
    void changed(bool hasChanged);

public:
    KPluginSelector *parent;
    QSplitter *splitter;
    QTreeWidget *treeView;
    QStackedWidget *stackedWidget;
    QMenu *contextualMenu;
    QList<KPluginInfoLVI*> treeItemList;
    QList<KCModuleProxy*> moduleProxyList;
    QList<KConfigGroup*> configGroupList;
    QMap<QString, KPluginInfoLVI*> categories;
    QMap<QString, QTreeWidgetItem*> titles;
    QMap<KCModuleProxy*, QStringList> moduleParentComponents;
    QHash<QString, KPluginInfoLVI*> treeItemPluginNames;
};

class KPluginInfoLVI
    : public QTreeWidgetItem
{
public:
    KPluginInfoLVI(const QString &itemTitle, QTreeWidget *parent);
    KPluginInfoLVI(KPluginInfo *pluginInfo, QTreeWidgetItem *parent);
    KPluginInfoLVI(const QString &itemTitle, QTreeWidgetItem *parent,
                   KPluginInfo *pluginInfo = 0);
    ~KPluginInfoLVI();

    /**
      * Setter methods
      */
    void setPluginInfo(KPluginInfo *pluginInfo);
    void setCfgGroup(KConfigGroup *cfgGroup);
    void setModuleProxy(KCModuleProxy *moduleProxy);
    void setCfgWidget(QWidget *cfgWidget);
    void setItemChecked(bool itemChecked);

    /**
      * Getter methods
      */
    KPluginInfo* pluginInfo() const;
    KConfigGroup* cfgGroup() const;
    KCModuleProxy* moduleProxy() const;
    QWidget* cfgWidget() const;
    bool itemChecked() const;

private:
    KPluginInfo *m_pluginInfo;
    KConfigGroup *m_cfgGroup;
    KCModuleProxy *m_moduleProxy;
    QWidget *m_cfgWidget;
    bool m_itemChecked;
};

#endif // KPLUGINSELECTOR_P_H
