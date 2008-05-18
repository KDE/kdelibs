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
#include <QtGui/QAbstractItemDelegate>

#include <kplugininfo.h>
#include <goya/kwidgetitemdelegate.h>
#include <kcategorizedsortfilterproxymodel.h>

class QLabel;
class QCheckBox;
class QPushButton;
class QAbstractItemView;

class KLineEdit;
class KCategorizedView;

class KPluginSelector::Private
    : public QObject
{
    Q_OBJECT

public:
    enum ExtraRoles
    {
        CommentRole = 0x19FC6DE2,
        ServicesCountRole = 0x1422E2AA
    };

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
    class PluginDelegate;
    class DependenciesWidget;
    KPluginSelector *parent;
    KLineEdit *lineEdit;
    KCategorizedView *listView;
    PluginModel *pluginModel;
    ProxyModel *proxyModel;
    PluginDelegate *pluginDelegate;
    DependenciesWidget *dependenciesWidget;
    bool showIcons;
};

class KPluginSelector::Private::PluginEntry
{
public:
    QString category;
    KPluginInfo pluginInfo;

    bool operator==(const KPluginSelector::Private::PluginEntry &pe) const
    {
        return pluginInfo.entryPath() == pe.pluginInfo.entryPath();
    }
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

    void addPlugins(const QList<KPluginInfo> &pluginList, const QString &categoryName, const QString &categoryKey);
    QList<KService::Ptr> pluginServices(const QModelIndex &index) const;

    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
//     virtual Qt::ItemFlags flags(const QModelIndex &index) const;

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


class KPluginSelector::Private::PluginDelegate
    : public KWidgetItemDelegate
{
public:
    PluginDelegate(QAbstractItemView *itemView, QObject *parent = 0);
    ~PluginDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    virtual QList<QWidget*> createItemWidgets() const;
    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const;

private:
    QCheckBox *checkBox;
    QPushButton *pushButton;
};

#endif // KPLUGINSELECTOR_P_H
