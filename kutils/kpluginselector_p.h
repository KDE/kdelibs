/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
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

#include <QtCore/QAbstractListModel>

#include <kplugininfo.h>
#include <kcategorizedsortfilterproxymodel.h>

class QLabel;

class KLineEdit;
class KCategorizedView;

class KPluginSelector::Private
    : public QObject
{
    Q_OBJECT

public:
    enum CheckWhatDependencies
    {
        /// If an item was checked, check all dependencies of that item
        DependenciesINeed = 0,
        /// If an item was unchecked, uncheck all items that depends on that item
        DependenciesNeedMe
    };

    Private(KPluginSelector *parent);
    ~Private();

    void checkIfShowIcons(const QList<KPluginInfo> &pluginInfoList);

Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray &componentName);

private Q_SLOTS:
    void emitChanged();

public:
    struct PluginEntry;
    class PluginModel;
    class ProxyModel;
    class DependenciesWidget;
    KPluginSelector *parent;
    KLineEdit *lineEdit;
    KCategorizedView *listView;
    PluginModel *pluginModel;
    ProxyModel *proxyModel;
    DependenciesWidget *dependenciesWidget;
    bool showIcons;
};

struct KPluginSelector::Private::PluginEntry
{
    QString category;
    KPluginInfo pluginInfo;
};


/**
 * This widget will inform the user about changes that happened automatically
 * due to plugin dependencies.
 */
class KPluginSelector::Private::DependenciesWidget
    : public QWidget
{
    Q_OBJECT

public:
    DependenciesWidget(QWidget *parent = 0);
    ~DependenciesWidget();

    void addDependency(const QString &dependency, const QString &pluginCausant, bool added);
    void userOverrideDependency(const QString &dependency);

    void clearDependencies();

private Q_SLOTS:
    void showDependencyDetails();

private:
    struct FurtherInfo
    {
        bool added;
        QString pluginCausant;
    };

    void updateDetails();

    QLabel *details;
    QMap<QString, struct FurtherInfo> dependencyMap;
    int addedByDependencies;
    int removedByDependencies;
};


class KPluginSelector::Private::PluginModel
    : public QAbstractListModel
{
public:
    PluginModel(QObject *parent = 0);
    ~PluginModel();

    void addPlugins(const QList<KPluginInfo> &pluginList, const QString &categoryName);

    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

private:
    QList<PluginEntry> pluginEntryList;
};

class KPluginSelector::Private::ProxyModel
    : public KCategorizedSortFilterProxyModel
{
public:
    ProxyModel(KPluginSelector::Private *q, QObject *parent = 0);
    ~ProxyModel();

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    virtual bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    KPluginSelector::Private *q;
};

#endif // KPLUGINSELECTOR_P_H
