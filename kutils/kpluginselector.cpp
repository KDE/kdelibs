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

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QFrame>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QSplitter>
#include <QtGui/QHeaderView>
#include <QtGui/QBoxLayout>
#include <QtCore/QList>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QLabel>
#include <QtGui/QBrush>
#include <QtGui/QMessageBox>

#include "kcmoduleinfo.h"
#include "kcmoduleloader.h"
#include "kcmoduleproxy.h"
#include <klocalizedstring.h>
#include <ktabwidget.h>
#include <kcomponentdata.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <kconfigbase.h>
#include <kiconloader.h>
#include <kcmodule.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kstyle.h>
#include <kdialog.h>
#include <kurllabel.h>


KPluginSelector::Private::Private(KPluginSelector *parent)
    : QObject(parent)
    , parent(parent)
    , listView(0)
{
    pluginModel = new PluginModel(this);
    pluginDelegate = new PluginDelegate(this);

    pluginDelegate->setIconSize(48, 48);
    pluginDelegate->setMinimumItemWidth(200);
    pluginDelegate->setLeftMargin(20);
    pluginDelegate->setRightMargin(20);
    pluginDelegate->setSeparatorPixels(10);

    connect(pluginModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(emitChanged()));
    connect(pluginDelegate, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));
}

KPluginSelector::Private::~Private()
{
    delete pluginModel;
    delete pluginDelegate;
}

void KPluginSelector::Private::emitChanged()
{
    emit changed(true);
}


// =============================================================


KPluginSelector::Private::DependenciesWidget::DependenciesWidget(QWidget *parent)
    : QWidget(parent)
    , addedByDependencies(0)
    , removedByDependencies(0)
{
    setVisible(false);

    details = new QLabel();

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *dataLayout = new QVBoxLayout;
    dataLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->setAlignment(Qt::AlignLeft);
    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setPixmap(KIconLoader::global()->loadIcon("dialog-information", K3Icon::Dialog));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(label);
    KUrlLabel *link = new KUrlLabel();
    link->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    link->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    link->setGlowEnabled(false);
    link->setUnderline(false);
    link->setFloatEnabled(true);
    link->setUseCursor(false);
    link->setHighlightedColor(palette().color(QPalette::Link));
    link->setSelectedColor(palette().color(QPalette::Link));
    link->setText("Automatic changes have been performed due to plugin dependencies");
    dataLayout->addWidget(link);
    dataLayout->addWidget(details);
    layout->addLayout(dataLayout);
    setLayout(layout);

    connect(link, SIGNAL(leftClickedUrl()), this, SLOT(showDependencyDetails()));
}

KPluginSelector::Private::DependenciesWidget::~DependenciesWidget()
{
}

void KPluginSelector::Private::DependenciesWidget::addDependency(const QString &dependency, const QString &pluginCausant, bool added)
{
    if (!isVisible())
        setVisible(true);

    struct FurtherInfo furtherInfo;
    furtherInfo.added = added;
    furtherInfo.pluginCausant = pluginCausant;

    if (dependencyMap.contains(dependency)) // The dependency moved from added to removed or vice-versa
    {
        if (added && removedByDependencies)
            removedByDependencies--;
        else if (addedByDependencies)
            addedByDependencies--;

        dependencyMap[dependency] = furtherInfo;
    }
    else
        dependencyMap.insert(dependency, furtherInfo);

    if (added)
        addedByDependencies++;
    else
        removedByDependencies++;

    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::userOverrideDependency(const QString &dependency)
{
    if (dependencyMap.contains(dependency))
    {
        if (addedByDependencies && dependencyMap[dependency].added)
            addedByDependencies--;
        else if (removedByDependencies)
            removedByDependencies--;

        dependencyMap.remove(dependency);
    }

    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::clearDependencies()
{
    addedByDependencies = 0;
    removedByDependencies = 0;
    dependencyMap.clear();
    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::showDependencyDetails()
{
    QString message = i18n("Automatically changes have been performed in order to satisfy plugin dependencies:\n");
    foreach(const QString &dependency, dependencyMap.keys())
    {
        if (dependencyMap[dependency].added)
            message += i18n("\n    %1 plugin has been automatically checked because the dependency of %2 plugin", dependency, dependencyMap[dependency].pluginCausant);
        else
            message += i18n("\n    %1 plugin has been automatically unchecked becase its dependency on %2 plugin", dependency, dependencyMap[dependency].pluginCausant);
    }
    QMessageBox::information(0, i18n("Dependency Check"), message);

    addedByDependencies = 0;
    removedByDependencies = 0;
    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::updateDetails()
{
    if (!dependencyMap.count())
    {
        setVisible(false);
        return;
    }

    QString message;

    if (addedByDependencies)
        message += i18np("%1 plugin added", "%1 plugins added", addedByDependencies);

    if (removedByDependencies && !message.isEmpty())
        message += i18n(", ");

    if (removedByDependencies)
        message += i18np("%1 plugin removed", "%1 plugins removed", removedByDependencies);

    if (!message.isEmpty())
        message += i18n(" since the last time you asked for details");

    if (message.isEmpty())
        details->setVisible(false);
    else
    {
        details->setVisible(true);
        details->setText(message);
    }
}


// =============================================================


KPluginSelector::Private::QListViewSpecialized::QListViewSpecialized(QWidget *parent)
    : QListView(parent)
{
    setMouseTracking(true);
}

KPluginSelector::Private::QListViewSpecialized::~QListViewSpecialized()
{
}

QStyleOptionViewItem KPluginSelector::Private::QListViewSpecialized::viewOptions() const
{
    return QListView::viewOptions();
}


// =============================================================


KPluginSelector::Private::PluginModel::PluginModel(KPluginSelector::Private *parent)
    : QAbstractListModel()
    , parent(parent)
{
}

KPluginSelector::Private::PluginModel::~PluginModel()
{
}

void KPluginSelector::Private::PluginModel::appendPluginList(const KPluginInfo::List &pluginInfoList,
                                                             const QString &categoryName,
                                                             const QString &categoryKey,
                                                             KConfigGroup *configGroup)
{
    QString myCategoryKey = categoryKey.toLower();

    if (!pluginInfoByCategory.contains(categoryName))
    {
        pluginInfoByCategory.insert(categoryName, KPluginInfo::List());
    }

    int addedPlugins = 0;
    foreach (KPluginInfo *pluginInfo, pluginInfoList)
    {
        if (!pluginInfo->isHidden() &&
             ((myCategoryKey.isEmpty()) ||
              (pluginInfo->category().toLower() == myCategoryKey)))
        {
            pluginInfo->load(configGroup);
            pluginInfoByCategory[categoryName].append(pluginInfo);

            struct AdditionalInfo pluginAdditionalInfo;

            if (pluginInfo->isPluginEnabled())
                pluginAdditionalInfo.itemChecked = Qt::Checked;
            else
                pluginAdditionalInfo.itemChecked = Qt::Unchecked;

            pluginAdditionalInfo.configGroup = configGroup;

            additionalInfo.insert(pluginInfo, pluginAdditionalInfo);

            addedPlugins++;
        }
    }

    if (pluginCount.contains(categoryName))
    {
        pluginCount[categoryName] += addedPlugins;
    }
    else
    {
        pluginCount.insert(categoryName, addedPlugins);
    }
}

bool KPluginSelector::Private::PluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !value.isValid() || index.row() >= rowCount())
        return false;

    switch (role)
    {
        case PluginDelegate::Checked:
            if (value.toBool())
                additionalInfo[static_cast<KPluginInfo*>(index.internalPointer())].itemChecked = Qt::Checked;
            else
                additionalInfo[static_cast<KPluginInfo*>(index.internalPointer())].itemChecked = Qt::Unchecked;
            break;
        default:
            return false;
    }

    emit dataChanged(index, index);

    return true;
}

QVariant KPluginSelector::Private::PluginModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    if (index.internalPointer()) // Is a plugin item
    {
        KPluginInfo *pluginInfo = static_cast<KPluginInfo*>(index.internalPointer());

        switch (role)
        {
            case PluginDelegate::Name:
                return pluginInfo->name();
            case PluginDelegate::Comment:
                return pluginInfo->comment();
            case PluginDelegate::Icon:
                return pluginInfo->icon();
            case PluginDelegate::Author:
                return pluginInfo->author();
            case PluginDelegate::Email:
                return pluginInfo->email();
            case PluginDelegate::Category:
                return pluginInfo->category();
            case PluginDelegate::InternalName:
                return pluginInfo->pluginName();
            case PluginDelegate::Version:
                return pluginInfo->version();
            case PluginDelegate::Website:
                return pluginInfo->website();
            case PluginDelegate::License:
                return pluginInfo->license();
            case PluginDelegate::Checked:
                return additionalInfo.value(static_cast<KPluginInfo*>(index.internalPointer())).itemChecked;
        }
    }
    else // Is a category
    {
        switch (role)
        {
            case PluginDelegate::Checked:
                return additionalInfo.value(static_cast<KPluginInfo*>(index.internalPointer())).itemChecked;

            case Qt::DisplayRole:
                int currentPosition = 0;
                foreach (QString category, pluginInfoByCategory.keys())
                {
                    if (currentPosition == index.row())
                        return category;

                    currentPosition += pluginInfoByCategory[category].count() + 1;
                }
        }
    }

    return QVariant();
}

Qt::ItemFlags KPluginSelector::Private::PluginModel::flags(const QModelIndex &index) const
{
    QModelIndex modelIndex = this->index(index.row(), index.column());

    if (modelIndex.internalPointer()) // Is a plugin item
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled; // We don't want items to be selectable
    else // Is a category
        return Qt::ItemIsEnabled;
}

QModelIndex KPluginSelector::Private::PluginModel::index(int row, int column, const QModelIndex &parent) const
{
    int currentPosition = 0;

    if ((row < 0) || (row >= rowCount()))
        return QModelIndex();

    foreach (QString category, pluginInfoByCategory.keys())
    {
        if (currentPosition == row)
            return createIndex(row, column, 0); // Is a category

        foreach (KPluginInfo *pluginInfo, pluginInfoByCategory[category])
        {
            currentPosition++;

            if (currentPosition == row)
                return createIndex(row, column, pluginInfo); // Is a plugin item
        }

        currentPosition++;
    }

    return QModelIndex();
}

int KPluginSelector::Private::PluginModel::rowCount(const QModelIndex &parent) const
{
    int retValue = pluginInfoByCategory.count(); // We have pluginInfoCategory.count() categories

    foreach (QString category, pluginInfoByCategory.keys())
    {
        if (pluginCount.contains(category))
            retValue += pluginCount[category];
    }

    return retValue;
}

QList<KService::Ptr> KPluginSelector::Private::PluginModel::services(const QModelIndex &index) const
{
    if (index.internalPointer()) // Is a plugin item
    {
        KPluginInfo *pluginInfo = static_cast<KPluginInfo*>(index.internalPointer());

        return pluginInfo->kcmServices();
    }

    return QList<KService::Ptr>(); // We were asked for a category
}

KConfigGroup *KPluginSelector::Private::PluginModel::configGroup(const QModelIndex &index) const
{
    return additionalInfo.value(static_cast<KPluginInfo*>(index.internalPointer())).configGroup;
}

void KPluginSelector::Private::PluginModel::setParentComponents(const QModelIndex &index, const QStringList &parentComponents)
{
    additionalInfo[static_cast<KPluginInfo*>(index.internalPointer())].parentComponents = parentComponents;
}

QStringList KPluginSelector::Private::PluginModel::parentComponents(const QModelIndex &index) const
{
    return additionalInfo.value(static_cast<KPluginInfo*>(index.internalPointer())).parentComponents;
}

void KPluginSelector::Private::PluginModel::updateDependencies(const QString &dependency, const QString &pluginCausant, CheckWhatDependencies whatDependencies, QStringList &dependenciesPushed)
{
    const KPluginInfo *pluginInfo;

    QModelIndex theIndex;
    if (whatDependencies == DependenciesINeed)
    {
        for (int i = 0; i < rowCount(); i++)
        {
            theIndex = index(i, 0);

            if (data(theIndex, PluginDelegate::InternalName).toString() == dependency)
            {
                pluginInfo = static_cast<const KPluginInfo*>(theIndex.internalPointer());

                if (!data(theIndex, PluginDelegate::Checked).toBool())
                {
                    parent->dependenciesWidget->addDependency(pluginInfo->name(), pluginCausant, true);

                    setData(theIndex, true, PluginDelegate::Checked);
                    dependenciesPushed.append(pluginInfo->name());
                }

                foreach(const QString &indirectDependency, pluginInfo->dependencies())
                {
                    updateDependencies(indirectDependency, pluginInfo->name(), whatDependencies, dependenciesPushed);
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < rowCount(); i++)
        {
            theIndex = index(i, 0);

            if (theIndex.internalPointer())
            {
                pluginInfo = static_cast<const KPluginInfo*>(theIndex.internalPointer());

                if (pluginInfo->dependencies().contains(dependency))
                {
                    if (data(theIndex, PluginDelegate::Checked).toBool())
                    {
                        parent->dependenciesWidget->addDependency(pluginInfo->name(), pluginCausant, false);

                        setData(theIndex, false, PluginDelegate::Checked);
                        dependenciesPushed.append(pluginInfo->name());
                    }

                    updateDependencies(pluginInfo->pluginName(), pluginCausant, whatDependencies, dependenciesPushed);
                }
            }
        }
    }
}


// =============================================================


KPluginSelector::KPluginSelector(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    connect(d, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(d, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    d->listView = new Private::QListViewSpecialized();
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);

    d->listView->setModel(d->pluginModel);
    d->listView->setItemDelegate(d->pluginDelegate);

    d->listView->viewport()->installEventFilter(d->pluginDelegate);

    d->dependenciesWidget = new Private::DependenciesWidget;

    layout->addWidget(d->listView);
    layout->addWidget(d->dependenciesWidget);
}

KPluginSelector::~KPluginSelector()
{
    delete d;
}

void KPluginSelector::addPlugins(const QString &componentName,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 KSharedConfig::Ptr config)
{
    QStringList desktopFileNames = KGlobal::dirs()->findAllResources("data",
        componentName + "/kpartplugins/*.desktop", KStandardDirs::Recursive);

    QList<KPluginInfo*> pluginInfoList = KPluginInfo::fromFiles(desktopFileNames);

    if (pluginInfoList.isEmpty())
        return;

    Q_ASSERT(config);
    if (!config)
        config = KSharedConfig::openConfig(componentName);

    KConfigGroup *cfgGroup = new KConfigGroup(config, "KParts Plugins");
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;

    d->pluginModel->appendPluginList(pluginInfoList, categoryName, categoryKey, cfgGroup);
}

void KPluginSelector::addPlugins(const KComponentData &instance,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 const KSharedConfig::Ptr &config)
{
    addPlugins(instance.componentName(), categoryName, categoryKey, config);
}

void KPluginSelector::addPlugins(const QList<KPluginInfo*> &pluginInfoList,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 const KSharedConfig::Ptr &config)
{
    if (pluginInfoList.isEmpty())
        return;

    KConfigGroup *cfgGroup = new KConfigGroup(config ? config : KGlobal::config(), "Plugins");
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;

    d->pluginModel->appendPluginList(pluginInfoList, categoryName, categoryKey, cfgGroup);
}

void KPluginSelector::load()
{
    QModelIndex currentIndex;
    KPluginInfo *currentPlugin;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            currentPlugin = static_cast<KPluginInfo*>(currentIndex.internalPointer());
            currentPlugin->load(d->pluginModel->configGroup(currentIndex));
            d->pluginModel->setData(currentIndex, currentPlugin->isPluginEnabled(), Private::PluginDelegate::Checked);
        }
    }

    emit changed(false);
}

void KPluginSelector::save()
{
    QModelIndex currentIndex;
    KPluginInfo *currentPlugin;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            KConfigGroup *configGroup = d->pluginModel->configGroup(currentIndex);
            currentPlugin = static_cast<KPluginInfo*>(currentIndex.internalPointer());
            currentPlugin->setPluginEnabled(d->pluginModel->data(currentIndex, Private::PluginDelegate::Checked).toBool());
            currentPlugin->save(configGroup);
            configGroup->sync();
        }
    }

    d->dependenciesWidget->clearDependencies();
}

void KPluginSelector::defaults()
{
    QModelIndex currentIndex;
    KPluginInfo *currentPlugin;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            currentPlugin = static_cast<KPluginInfo*>(currentIndex.internalPointer());
            currentPlugin->defaults();
            d->pluginModel->setData(currentIndex, currentPlugin->isPluginEnabled(), Private::PluginDelegate::Checked);
        }
    }
}


// =============================================================


KPluginSelector::Private::PluginDelegate::PluginDelegate(KPluginSelector::Private *parent)
    : QItemDelegate(0)
    , configDialog(0)
    , parent(parent)
{
    iconLoader = new KIconLoader();
}

KPluginSelector::Private::PluginDelegate::~PluginDelegate()
{
    foreach(KCModuleProxy *moduleProxy, currentModuleProxyList)
    {
        delete moduleProxy;
    }
    currentModuleProxyList.clear();

    delete iconLoader;
    delete configDialog;
}

void KPluginSelector::Private::PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy(option);
    const PluginModel *model = static_cast<const PluginModel*>(index.model());

    QRect theCheckRect = checkRect(index, optionCopy);
    QFontMetrics fontMetrics = painter->fontMetrics();

    QColor unselectedTextColor = optionCopy.palette.text().color();
    QColor selectedTextColor = optionCopy.palette.highlightedText().color();
    QPen currentPen = painter->pen();
    QPen unselectedPen = QPen(currentPen);
    QPen selectedPen = QPen(currentPen);
    QPen linkPen = QPen(currentPen);

    QString details = i18n("More Options");
    QString about = i18n("About");

    unselectedPen.setColor(unselectedTextColor);
    selectedPen.setColor(selectedTextColor);
    linkPen.setColor(option.palette.color(QPalette::Link));

    QPixmap iconPixmap = icon(index, iconWidth, iconHeight);

    QFont title(painter->font());
    QFont previousFont(painter->font());
    QFont configureFont(painter->font());

    title.setPointSize(title.pointSize() + 2);
    title.setWeight(QFont::Bold);

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (index.internalPointer())
    {
        if (optionCopy.state & QStyle::State_Selected)
        {
            painter->fillRect(optionCopy.rect, optionCopy.palette.highlight());
            painter->setPen(selectedPen);
        }
        else
        {
            if (((index.row() - 1) >= 0) && !(index.model()->index(index.row() - 1, 0).internalPointer()))
                painter->fillRect(optionCopy.rect, optionCopy.palette.color(QPalette::Base));
            else if ((index.row() - 1) % 2)
                painter->fillRect(optionCopy.rect, optionCopy.palette.color(QPalette::AlternateBase));
        }

        QString display;
        QString secondaryDisplay = fontMetrics.elidedText(comment(index), Qt::ElideRight, optionCopy.rect.width() - leftMargin - rightMargin - iconPixmap.width() - separatorPixels * 2 - theCheckRect.width());

        QPen prevPen(painter->pen());
        painter->setPen(linkPen);

        painter->setFont(title);
        if (model->services(index).size()) // has configuration dialog
        {
            display = painter->fontMetrics().elidedText(name(index), Qt::ElideRight, optionCopy.rect.width() - leftMargin - rightMargin - iconPixmap.width() - separatorPixels * 2 - theCheckRect.width() - clickableLabelRect(optionCopy, details).width());

            if (clickableLabelRect(optionCopy, details).contains(relativeMousePosition))
            {
                configureFont.setUnderline(true);
                painter->setFont(configureFont);
            }
            else
            {
                painter->setFont(previousFont);
            }

            painter->drawText(clickableLabelRect(optionCopy, details), Qt::AlignLeft, details);
        }
        else
        {
            display = painter->fontMetrics().elidedText(name(index), Qt::ElideRight, optionCopy.rect.width() - leftMargin - rightMargin - iconPixmap.width() - separatorPixels * 2 - theCheckRect.width() - clickableLabelRect(optionCopy, about).width());

            if (clickableLabelRect(optionCopy, about).contains(relativeMousePosition))
            {
                configureFont.setUnderline(true);
                painter->setFont(configureFont);
            }
            else
            {
                painter->setFont(previousFont);
            }

            painter->drawText(clickableLabelRect(optionCopy, about), Qt::AlignLeft, about);
        }

        painter->setPen(prevPen);
        painter->setFont(title);

        painter->drawText(leftMargin + separatorPixels * 2 + iconPixmap.width() + theCheckRect.width(), separatorPixels + optionCopy.rect.top(), painter->fontMetrics().width(display), painter->fontMetrics().height(), Qt::AlignLeft, display);

        painter->setFont(previousFont);

        painter->drawText(leftMargin + separatorPixels * 2 + iconPixmap.width() + theCheckRect.width(), optionCopy.rect.height() - separatorPixels - fontMetrics.height() + optionCopy.rect.top(), fontMetrics.width(secondaryDisplay), fontMetrics.height(), Qt::AlignLeft, secondaryDisplay);

        painter->drawPixmap(leftMargin + separatorPixels + theCheckRect.width(), calculateVerticalCenter(optionCopy.rect, iconPixmap.height()) + optionCopy.rect.top(), iconPixmap);

        QStyleOptionViewItem optionCheck(optionCopy);

        if (checkRect(index, optionCopy).contains(relativeMousePosition))
        {
            optionCheck.state |= QStyle::State_MouseOver;
        }

        drawCheck(painter, optionCheck, checkRect(index, optionCheck), (Qt::CheckState) index.model()->data(index, Checked).toInt());
    }
    else
    {
        QString display = painter->fontMetrics().elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, optionCopy.rect.width() - leftMargin - rightMargin);

        QStyleOptionButton opt;

        opt.rect = QRect(leftMargin, separatorPixels + optionCopy.rect.top(), optionCopy.rect.width() - leftMargin - rightMargin, painter->fontMetrics().height());
        opt.palette = optionCopy.palette;
        opt.direction = optionCopy.direction;
        opt.text = display;

        QFont painterFont = painter->font();
        painterFont.setWeight(QFont::Bold);
        painterFont.setPointSize(painterFont.pointSize() + 2);
        QFontMetrics metrics(painterFont);
        painter->setFont(painterFont);

        opt.fontMetrics = painter->fontMetrics();

        /*if (const KStyle *style = dynamic_cast<const KStyle*>(QApplication::style()))
        {
            opt.rect.setHeight(sizeHint(optionCopy, index).height() - separatorPixels);

            style->drawControl(KStyle::CE_Category, &opt, painter, 0);
        }
        else
        {*/
            QRect auxRect(optionCopy.rect.left() + leftMargin,
                          optionCopy.rect.bottom() - 2,
                          optionCopy.rect.width() - leftMargin - rightMargin,
                          2);

            QPainterPath path;
            path.addRect(auxRect);

            QLinearGradient gradient(optionCopy.rect.topLeft(),
                                                        optionCopy.rect.bottomRight());
            gradient.setColorAt(0, Qt::black);
            gradient.setColorAt(1, Qt::white);

            painter->setBrush(gradient);
            painter->fillPath(path, gradient);

            QRect auxRect2(optionCopy.rect.left() + leftMargin,
                           option.rect.top(),
                           optionCopy.rect.width() - leftMargin - rightMargin,
                           option.rect.height());

            painter->drawText(auxRect2, Qt::AlignVCenter | Qt::AlignLeft,
                              display);
        //}
    }

    painter->restore();
}

QSize KPluginSelector::Private::PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.internalPointer())
        return QSize(68, 68);

    /*if (const KStyle *style = dynamic_cast<const KStyle*>(QApplication::style()))
    {
        QSize retSize = style->sizeFromContents(KStyle::CT_Category, &option, QSize(1, option.fontMetrics.height()), 0);

        retSize.setHeight(retSize.height() + separatorPixels);

        return retSize;
    }*/

    return QSize(34, 34);
}

void KPluginSelector::Private::PluginDelegate::setIconSize(int width, int height)
{
    this->iconWidth = width;
    this->iconHeight = height;
}

void KPluginSelector::Private::PluginDelegate::setMinimumItemWidth(int minimumItemWidth)
{
    this->minimumItemWidth = minimumItemWidth;
}

void KPluginSelector::Private::PluginDelegate::setLeftMargin(int leftMargin)
{
    this->leftMargin = leftMargin;
}

void KPluginSelector::Private::PluginDelegate::setRightMargin(int rightMargin)
{
    this->rightMargin = rightMargin;
}

void KPluginSelector::Private::PluginDelegate::setSeparatorPixels(int separatorPixels)
{
    this->separatorPixels = separatorPixels;
}

bool KPluginSelector::Private::PluginDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if ((event->type() == QEvent::MouseButtonPress) ||
        (event->type() == QEvent::KeyRelease))
    {
        const QKeyEvent *keyEvent = dynamic_cast<const QKeyEvent*>(event);
        if (keyEvent && (keyEvent->key() != Qt::Key_Space))
        {
            return false;
        }

        EventReceived eventReceived;
        if (event->type() == QEvent::MouseButtonPress)
        {
            eventReceived = MouseEvent;
        }
        else
        {
            eventReceived = KeyboardEvent;
        }

        const QWidget *viewport = qobject_cast<const QWidget*>(watched);
        if (viewport)
        {
            QModelIndex currentIndex;

            QListViewSpecialized *listView = dynamic_cast<QListViewSpecialized*>(viewport->parent());
            if ((eventReceived == MouseEvent) && listView)
            {
                currentIndex = listView->indexAt(viewport->mapFromGlobal(QCursor::pos()));
            }
            else if ((eventReceived == KeyboardEvent) && listView)
            {
                currentIndex = listView->currentIndex();
            }

            if (listView && currentIndex.isValid())
            {
                QStyleOptionViewItem optionViewItem(listView->viewOptions());
                optionViewItem.rect = listView->visualRect(currentIndex);

                if (const KPluginInfo *pluginInfo = static_cast<const KPluginInfo*>(currentIndex.internalPointer()))
                {
                    if (pluginInfo->kcmServices().size())
                        updateCheckState(currentIndex, optionViewItem,
                                         viewport->mapFromGlobal(QCursor::pos()), listView, eventReceived, i18n("More Options"));
                    else
                        updateCheckState(currentIndex, optionViewItem,
                                         viewport->mapFromGlobal(QCursor::pos()), listView, eventReceived, i18n("About"));
                }
            }
        }

        return QItemDelegate::eventFilter(watched, event);
    }
    else if (event->type() == QEvent::MouseMove)
    {
        if (QWidget *viewport = qobject_cast<QWidget*>(watched))
        {
            relativeMousePosition = viewport->mapFromGlobal(QCursor::pos());
            viewport->update();
        }
    }
    else if (event->type() == QEvent::Leave)
    {
        QWidget *viewport = qobject_cast<QWidget*>(watched);
        if (viewport)
        {
            relativeMousePosition = QPoint(0, 0);
            viewport->update();
        }
    }

    return false;
}

void KPluginSelector::Private::PluginDelegate::slotDefaultClicked()
{
    foreach(KCModuleProxy *moduleProxy, currentModuleProxyList)
    {
        moduleProxy->defaults();
    }
}

QRect KPluginSelector::Private::PluginDelegate::checkRect(const QModelIndex &index, const QStyleOptionViewItem &option) const
{
    QSize canvasSize = sizeHint(option, index);
    QRect checkDimensions = QApplication::style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &option);

    QRect retRect;
    retRect.setTopLeft(QPoint(option.rect.left() + leftMargin,
                       ((canvasSize.height() / 2) - (checkDimensions.height() / 2)) + option.rect.top()));
    retRect.setBottomRight(QPoint(option.rect.left() + leftMargin + checkDimensions.width(),
                           ((canvasSize.height() / 2) - (checkDimensions.height() / 2)) + option.rect.top() + checkDimensions.height()));

    return retRect;
}

QRect KPluginSelector::Private::PluginDelegate::clickableLabelRect(const QStyleOptionViewItem &option, const QString &caption) const
{
    QRect delegateRect = option.rect;
    QFontMetrics fontMetrics(option.font);

    return QRect(delegateRect.right() - rightMargin - fontMetrics.width(caption), separatorPixels + delegateRect.top(), fontMetrics.width(caption), fontMetrics.height());
}

void KPluginSelector::Private::PluginDelegate::updateCheckState(const QModelIndex &index, const QStyleOptionViewItem &option,
                                                                const QPoint &cursorPos, QListView *listView, EventReceived eventReceived,
                                                                const QString &caption)
{
    if (!index.isValid())
        return;

    PluginModel *model = static_cast<PluginModel*>(listView->model());
    KPluginInfo *pluginInfo = 0;

    switch (eventReceived)
    {
        case MouseEvent:
            if (!index.internalPointer())
            {
                return;
            }
            // We don't want to break
        case KeyboardEvent:
        {
            pluginInfo = static_cast<KPluginInfo*>(index.internalPointer());

            if (checkRect(index, option).contains(cursorPos))
            {
                listView->model()->setData(index, !listView->model()->data(index, Checked).toBool(), Checked);

                parent->dependenciesWidget->userOverrideDependency(pluginInfo->name());

                if (listView->model()->data(index, Checked).toBool()) // Item was checked
                    checkDependencies(model, pluginInfo, DependenciesINeed);
                else
                    checkDependencies(model, pluginInfo, DependenciesNeedMe);
            }

            QList<KService::Ptr> services = model->services(index);

            if (clickableLabelRect(option, caption).contains(cursorPos))
            {
                foreach(KCModuleProxy *moduleProxy, currentModuleProxyList)
                {
                    delete moduleProxy;
                }
                currentModuleProxyList.clear();

                configDialog = new KDialog;
                configDialog->setWindowTitle(pluginInfo->name());
                KTabWidget *newTabWidget = new KTabWidget(configDialog);
                bool configurable = false;

                QObject::connect(configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));

                KService::Ptr servicePtr;
                for(QList<KService::Ptr>::ConstIterator it =
                    services.begin(); it != services.end(); ++it)
                {
                    servicePtr = (*it);

                    if(!servicePtr->noDisplay())
                    {
                        KCModuleInfo moduleinfo(servicePtr);
                        model->setParentComponents(index, moduleinfo.service()->property("X-KDE-ParentComponents").toStringList());
                        KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleinfo, newTabWidget);
                        if (currentModuleProxy->realModule())
                        {
                            currentModuleProxyList.append(currentModuleProxy);
                            newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                            configurable = true;
                        }
                    }
                }

                if (!configurable)
                    configDialog->setButtons(KDialog::Close);
                else
                    configDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);

                QWidget *aboutWidget = new QWidget(newTabWidget);
                QVBoxLayout *layout = new QVBoxLayout;
                aboutWidget->setLayout(layout);

                QLabel *description = new QLabel(i18n("Description:\n\t%1", pluginInfo->comment()));
                QLabel *author = new QLabel(i18n("Author:\n\t%1", pluginInfo->author()));
                QLabel *authorEmail = new QLabel(i18n("E-Mail:\n\t%1", pluginInfo->email()));
                QLabel *website = new QLabel(i18n("Website:\n\t%1", pluginInfo->website()));
                QLabel *version = new QLabel(i18n("Version:\n\t%1", pluginInfo->version()));
                QLabel *license = new QLabel(i18n("License:\n\t%1", pluginInfo->license()));

                layout->addWidget(description);
                layout->addWidget(author);
                layout->addWidget(authorEmail);
                layout->addWidget(website);
                layout->addWidget(version);
                layout->addWidget(license);
                layout->insertStretch(-1);

                newTabWidget->addTab(aboutWidget, i18n("About"));
                configDialog->setMainWidget(newTabWidget);

                KDialog::centerOnScreen(configDialog);

                if (configDialog->exec() == QDialog::Accepted)
                {
                    foreach (KCModuleProxy *moduleProxy, currentModuleProxyList)
                    {
                        moduleProxy->save();
                        foreach (const QString &parentComponent, model->parentComponents(index))
                        {
                            emit configCommitted(parentComponent.toLatin1());
                        }
                    }
                }

                delete configDialog;
            }
        }
    }
}

void KPluginSelector::Private::PluginDelegate::checkDependencies(PluginModel *model,
                                                                 const KPluginInfo *info,
                                                                 CheckWhatDependencies whatDependencies)
{
    QStringList dependenciesPushed;

    if (whatDependencies == DependenciesINeed)
    {
        foreach(const QString &dependency, info->dependencies())
        {
            model->updateDependencies(dependency, info->name(), whatDependencies, dependenciesPushed);
        }
    }
    else
    {
        model->updateDependencies(info->pluginName(), info->name(), whatDependencies, dependenciesPushed);
    }
}

QString KPluginSelector::Private::PluginDelegate::name(const QModelIndex &index) const
{
    return index.model()->data(index, Name).toString();
}

QString KPluginSelector::Private::PluginDelegate::comment(const QModelIndex &index) const
{
    return index.model()->data(index, Comment).toString();
}

QPixmap KPluginSelector::Private::PluginDelegate::icon(const QModelIndex &index, int width, int height) const
{
    return KIcon(index.model()->data(index, Icon).toString(), iconLoader).pixmap(width, height);
}

QString KPluginSelector::Private::PluginDelegate::author(const QModelIndex &index) const
{
    return index.model()->data(index, Author).toString();
}

QString KPluginSelector::Private::PluginDelegate::email(const QModelIndex &index) const
{
    return index.model()->data(index, Email).toString();
}

QString KPluginSelector::Private::PluginDelegate::category(const QModelIndex &index) const
{
    return index.model()->data(index, Category).toString();
}

QString KPluginSelector::Private::PluginDelegate::internalName(const QModelIndex &index) const
{
    return index.model()->data(index, InternalName).toString();
}

QString KPluginSelector::Private::PluginDelegate::version(const QModelIndex &index) const
{
    return index.model()->data(index, Version).toString();
}

QString KPluginSelector::Private::PluginDelegate::website(const QModelIndex &index) const
{
    return index.model()->data(index, Website).toString();
}

QString KPluginSelector::Private::PluginDelegate::license(const QModelIndex &index) const
{
    return index.model()->data(index, License).toString();
}

int KPluginSelector::Private::PluginDelegate::calculateVerticalCenter(const QRect &rect, int pixmapHeight) const
{
    return (rect.height() / 2) - (pixmapHeight / 2);
}


#include "kpluginselector_p.moc"
#include "kpluginselector.moc"
