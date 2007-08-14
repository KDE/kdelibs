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
#include <kicon.h>
#include <kstyle.h>
#include <kdialog.h>
#include <kurllabel.h>
#include <kurl.h>
#include <krun.h>
#include <kmessagebox.h>


KPluginSelector::Private::Private(KPluginSelector *parent)
    : QObject(parent)
    , parent(parent)
    , listView(0)
{
    pluginModel = new PluginModel(this);
    pluginDelegate = new PluginDelegate(this);

    pluginDelegate->setMinimumItemWidth(200);
    pluginDelegate->setLeftMargin(20);
    pluginDelegate->setRightMargin(20);
    pluginDelegate->setSeparatorPixels(5);

    QFont title(parent->font());
    title.setPointSize(title.pointSize() + 2);
    title.setWeight(QFont::Bold);

    QFontMetrics titleMetrics(title);
    QFontMetrics currentMetrics(parent->font());

    pluginDelegate->setIconSize((pluginDelegate->getSeparatorPixels() * 2) + titleMetrics.height() + currentMetrics.height(),
                                (pluginDelegate->getSeparatorPixels() * 2) + titleMetrics.height() + currentMetrics.height());

    QObject::connect(pluginModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(emitChanged()));
    QObject::connect(pluginDelegate, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));
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
    link->setText(i18n("Automatic changes have been performed due to plugin dependencies"));
    dataLayout->addWidget(link);
    dataLayout->addWidget(details);
    layout->addLayout(dataLayout);
    setLayout(layout);

    QObject::connect(link, SIGNAL(leftClickedUrl()), this, SLOT(showDependencyDetails()));
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
    KMessageBox::information(0, message, i18n("Dependency Check"));

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
                                                             const KConfigGroup &configGroup,
                                                             PluginLoadMethod pluginLoadMethod,
                                                             AddMethod addMethod)
{
    QString myCategoryKey = categoryKey.toLower();

    if (!pluginInfoByCategory.contains(categoryName))
    {
        pluginInfoByCategory.insert(categoryName, KPluginInfo::List());
    }

    KConfigGroup providedConfigGroup;
    int addedPlugins = 0;
    bool alternateColor = pluginCount.contains(categoryName) ? ((pluginCount[categoryName] % 2) != 0) : false;
    foreach (KPluginInfo pluginInfo, pluginInfoList)
    {
        if (!pluginInfo.isHidden() &&
             ((myCategoryKey.isEmpty()) ||
              (pluginInfo.category().toLower() == myCategoryKey)))
        {
            if ((pluginLoadMethod == ReadConfigFile) && !pluginInfo.config().isValid())
                pluginInfo.load(configGroup);
            else if (pluginLoadMethod == ReadConfigFile)
            {
                providedConfigGroup = pluginInfo.config();
                pluginInfo.load();
            }

            pluginInfoByCategory[categoryName].append(pluginInfo);

            struct AdditionalInfo pluginAdditionalInfo;

            if (pluginInfo.isPluginEnabled())
                pluginAdditionalInfo.itemChecked = Qt::Checked;
            else
                pluginAdditionalInfo.itemChecked = Qt::Unchecked;

            pluginAdditionalInfo.alternateColor = alternateColor;

            pluginAdditionalInfo.configGroup = pluginInfo.config().isValid() ? providedConfigGroup : configGroup;
            pluginAdditionalInfo.addMethod = addMethod;

            additionalInfo.insert(pluginInfo, pluginAdditionalInfo);

            addedPlugins++;
            alternateColor = !alternateColor;
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
                additionalInfo[*static_cast<KPluginInfo*>(index.internalPointer())].itemChecked = Qt::Checked;
            else
                additionalInfo[*static_cast<KPluginInfo*>(index.internalPointer())].itemChecked = Qt::Unchecked;
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
        KPluginInfo pluginInfo = *static_cast<KPluginInfo*>(index.internalPointer());

        switch (role)
        {
            case PluginDelegate::Name:
                return pluginInfo.name();
            case PluginDelegate::Comment:
                return pluginInfo.comment();
            case PluginDelegate::Icon:
                return pluginInfo.icon();
            case PluginDelegate::Author:
                return pluginInfo.author();
            case PluginDelegate::Email:
                return pluginInfo.email();
            case PluginDelegate::Category:
                return pluginInfo.category();
            case PluginDelegate::InternalName:
                return pluginInfo.pluginName();
            case PluginDelegate::Version:
                return pluginInfo.version();
            case PluginDelegate::Website:
                return pluginInfo.website();
            case PluginDelegate::License:
                return pluginInfo.license();
            case PluginDelegate::Checked:
                return additionalInfo.value(*static_cast<KPluginInfo*>(index.internalPointer())).itemChecked;
        }
    }
    else // Is a category
    {
        switch (role)
        {
            case PluginDelegate::Checked:
                return additionalInfo.value(*static_cast<KPluginInfo*>(index.internalPointer())).itemChecked;

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
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    else // Is a category
        return Qt::ItemIsEnabled;
}

QModelIndex KPluginSelector::Private::PluginModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int currentPosition = 0;

    if ((row < 0) || (row >= rowCount()))
        return QModelIndex();

    foreach (QString category, pluginInfoByCategory.keys())
    {
        if (currentPosition == row)
            return createIndex(row, column, 0); // Is a category

        foreach (const KPluginInfo &pluginInfo, pluginInfoByCategory[category])
        {
            currentPosition++;

            if (currentPosition == row)
                return createIndex(row, column, const_cast<KPluginInfo *>(&pluginInfo)); // Is a plugin item
        }

        currentPosition++;
    }

    return QModelIndex();
}

int KPluginSelector::Private::PluginModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

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
        const KPluginInfo pluginInfo = *static_cast<KPluginInfo*>(index.internalPointer());

        return pluginInfo.kcmServices();
    }

    return QList<KService::Ptr>(); // We were asked for a category
}

KConfigGroup KPluginSelector::Private::PluginModel::configGroup(const QModelIndex &index) const
{
    return additionalInfo.value(*static_cast<KPluginInfo*>(index.internalPointer())).configGroup;
}

void KPluginSelector::Private::PluginModel::setParentComponents(const QModelIndex &index, const QStringList &parentComponents)
{
    additionalInfo[*static_cast<KPluginInfo*>(index.internalPointer())].parentComponents = parentComponents;
}

QStringList KPluginSelector::Private::PluginModel::parentComponents(const QModelIndex &index) const
{
    return additionalInfo.value(*static_cast<KPluginInfo*>(index.internalPointer())).parentComponents;
}

void KPluginSelector::Private::PluginModel::updateDependencies(const QString &dependency, const QString &pluginCausant, CheckWhatDependencies whatDependencies, QStringList &dependenciesPushed)
{
    QModelIndex theIndex;
    if (whatDependencies == DependenciesINeed)
    {
        for (int i = 0; i < rowCount(); i++)
        {
            theIndex = index(i, 0);

            if (data(theIndex, PluginDelegate::InternalName).toString() == dependency)
            {
                const KPluginInfo pluginInfo(*static_cast<const KPluginInfo*>(theIndex.internalPointer()));

                if (!data(theIndex, PluginDelegate::Checked).toBool())
                {
                    parent->dependenciesWidget->addDependency(pluginInfo.name(), pluginCausant, true);

                    setData(theIndex, true, PluginDelegate::Checked);
                    dependenciesPushed.append(pluginInfo.name());
                }

                foreach(const QString &indirectDependency, pluginInfo.dependencies())
                {
                    updateDependencies(indirectDependency, pluginInfo.name(), whatDependencies, dependenciesPushed);
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
                const KPluginInfo pluginInfo(*static_cast<const KPluginInfo*>(theIndex.internalPointer()));

                if (pluginInfo.dependencies().contains(dependency))
                {
                    if (data(theIndex, PluginDelegate::Checked).toBool())
                    {
                        parent->dependenciesWidget->addDependency(pluginInfo.name(), pluginCausant, false);

                        setData(theIndex, false, PluginDelegate::Checked);
                        dependenciesPushed.append(pluginInfo.name());
                    }

                    updateDependencies(pluginInfo.pluginName(), pluginCausant, whatDependencies, dependenciesPushed);
                }
            }
        }
    }
}

KPluginSelector::Private::PluginModel::AddMethod KPluginSelector::Private::PluginModel::addMethod(const KPluginInfo &pluginInfo) const
{
    return additionalInfo[pluginInfo].addMethod;
}

bool KPluginSelector::Private::PluginModel::alternateColor(const KPluginInfo &pluginInfo) const
{
    return additionalInfo[pluginInfo].alternateColor;
}


// =============================================================


KPluginSelector::KPluginSelector(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    QObject::connect(d, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    QObject::connect(d, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    d->listView = new Private::QListViewSpecialized();
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);

    d->listView->setModel(d->pluginModel);
    d->listView->setItemDelegate(d->pluginDelegate);

    d->listView->viewport()->installEventFilter(d->pluginDelegate);
    d->listView->installEventFilter(d->pluginDelegate);

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

    QList<KPluginInfo> pluginInfoList = KPluginInfo::fromFiles(desktopFileNames);

    if (pluginInfoList.isEmpty())
        return;

    Q_ASSERT(config);
    if (!config)
        config = KSharedConfig::openConfig(componentName);

    KConfigGroup *cfgGroup = new KConfigGroup(config, "KParts Plugins");
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup;

    d->pluginModel->appendPluginList(pluginInfoList, categoryName, categoryKey, *cfgGroup);
}

void KPluginSelector::addPlugins(const KComponentData &instance,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 const KSharedConfig::Ptr &config)
{
    addPlugins(instance.componentName(), categoryName, categoryKey, config);
}

void KPluginSelector::addPlugins(const QList<KPluginInfo> &pluginInfoList,
                                 PluginLoadMethod pluginLoadMethod,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 const KSharedConfig::Ptr &config)
{
    if (pluginInfoList.isEmpty())
        return;

    KConfigGroup *cfgGroup = new KConfigGroup(config ? config : KGlobal::config(), "Plugins");
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup;

    d->pluginModel->appendPluginList(pluginInfoList, categoryName, categoryKey, *cfgGroup, pluginLoadMethod, Private::PluginModel::ManuallyAdded);
}

void KPluginSelector::load()
{
    QModelIndex currentIndex;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            KPluginInfo currentPlugin(*static_cast<KPluginInfo*>(currentIndex.internalPointer()));

            currentPlugin.load(d->pluginModel->configGroup(currentIndex));

            d->pluginModel->setData(currentIndex, currentPlugin.isPluginEnabled(), Private::PluginDelegate::Checked);
        }
    }

    emit changed(false);
}

void KPluginSelector::save()
{
    QModelIndex currentIndex;
    KConfigGroup configGroup;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            KPluginInfo currentPlugin(*static_cast<KPluginInfo*>(currentIndex.internalPointer()));
            currentPlugin.setPluginEnabled(d->pluginModel->data(currentIndex, Private::PluginDelegate::Checked).toBool());

            configGroup = d->pluginModel->configGroup(currentIndex);

            currentPlugin.save(configGroup);

            configGroup.sync();
        }
    }

    d->dependenciesWidget->clearDependencies();
}

void KPluginSelector::defaults()
{
    QModelIndex currentIndex;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            KPluginInfo currentPlugin(*static_cast<KPluginInfo*>(currentIndex.internalPointer()));
            currentPlugin.defaults();
            d->pluginModel->setData(currentIndex, currentPlugin.isPluginEnabled(), Private::PluginDelegate::Checked);
        }
    }
}

void KPluginSelector::updatePluginsState()
{
    QModelIndex currentIndex;
    for (int i = 0; i < d->pluginModel->rowCount(); i++)
    {
        currentIndex = d->pluginModel->index(i, 0);
        if (currentIndex.internalPointer())
        {
            KPluginInfo currentPlugin(*static_cast<KPluginInfo*>(currentIndex.internalPointer()));

            // Only the items that were added "manually" will be updated, since the others
            // are not visible from the outside
            if (d->pluginModel->addMethod(currentPlugin) == Private::PluginModel::ManuallyAdded)
                currentPlugin.setPluginEnabled(d->pluginModel->data(currentIndex, Private::PluginDelegate::Checked).toBool());
        }
    }
}


// =============================================================


KPluginSelector::Private::PluginDelegate::PluginDelegate(KPluginSelector::Private *parent)
    : QItemDelegate(0), focusedElement(0), currentModuleProxyList(0), configDialog(0), parent(parent)
{
    iconLoader = new KIconLoader();
}

KPluginSelector::Private::PluginDelegate::~PluginDelegate()
{
    qDeleteAll(configDialogs);

    delete iconLoader;
}

void KPluginSelector::Private::PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy(option);
    const PluginModel *model = static_cast<const PluginModel*>(index.model());

    QRect theCheckRect = checkRect(index, optionCopy);
    QFontMetrics fontMetrics = painter->fontMetrics();

    QColor unselectedTextColor = optionCopy.palette.text().color();
    QPen currentPen = painter->pen();
    QPen linkPen = QPen(option.palette.color(QPalette::Link));

    QString details = i18n("More Options");
    QString about = i18n("About");

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
        const KPluginInfo info(*static_cast<KPluginInfo*>(index.internalPointer()));

        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(optionCopy.rect, optionCopy.palette.color(QPalette::Highlight));
        }
        else
        {
            if (model->alternateColor(info))
                painter->fillRect(optionCopy.rect, optionCopy.palette.color(QPalette::AlternateBase));
            else
                painter->fillRect(optionCopy.rect, optionCopy.palette.color(QPalette::Base));
        }

        QString display;
        QString secondaryDisplay = fontMetrics.elidedText(comment(index), Qt::ElideRight, optionCopy.rect.width() - leftMargin - rightMargin - iconPixmap.width() - separatorPixels * 2 - theCheckRect.width());

        QPen prevPen(painter->pen());
        painter->setPen(linkPen);

        painter->setFont(title);
        if (model->services(index).size()) // has configuration dialog
        {
            display = painter->fontMetrics().elidedText(name(index), Qt::ElideRight, optionCopy.rect.width() - leftMargin - rightMargin - iconPixmap.width() - separatorPixels * 2 - theCheckRect.width() - clickableLabelRect(optionCopy, details).width());

            if (clickableLabelRect(optionCopy, details).contains(relativeMousePosition) ||
                ((focusedElement == 1) && (option.state & QStyle::State_Selected)))
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

            if (clickableLabelRect(optionCopy, about).contains(relativeMousePosition) ||
                ((focusedElement == 1) && (option.state & QStyle::State_Selected)))
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

        if (option.state & QStyle::State_Selected)
        {
            painter->setPen(optionCopy.palette.color(QPalette::HighlightedText));
        }
        else
        {
            painter->setPen(prevPen);
        }

        painter->setFont(title);

        painter->drawText(leftMargin + separatorPixels * 2 + iconPixmap.width() + theCheckRect.width(), separatorPixels + optionCopy.rect.top(), painter->fontMetrics().width(display), painter->fontMetrics().height(), Qt::AlignLeft, display);

        painter->setFont(previousFont);

        painter->drawText(leftMargin + separatorPixels * 2 + iconPixmap.width() + theCheckRect.width(), optionCopy.rect.height() - separatorPixels - fontMetrics.height() + optionCopy.rect.top(), fontMetrics.width(secondaryDisplay), fontMetrics.height(), Qt::AlignLeft, secondaryDisplay);

        painter->drawPixmap(leftMargin + separatorPixels + theCheckRect.width(), calculateVerticalCenter(optionCopy.rect, iconPixmap.height()) + optionCopy.rect.top(), iconPixmap);

        QStyleOptionViewItem optionCheck(optionCopy);

        if (checkRect(index, optionCopy).contains(relativeMousePosition) ||
            ((focusedElement == 0) && (option.state & QStyle::State_Selected)))
        {
            optionCheck.state |= QStyle::State_MouseOver;
        }

        drawCheck(painter, optionCheck, checkRect(index, optionCheck), (Qt::CheckState) index.model()->data(index, Checked).toInt());
    }
    else // we are drawing a category
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
    }

    painter->restore();
}

QSize KPluginSelector::Private::PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    QFont title(option.font);
    title.setPointSize(title.pointSize() + 2);
    title.setWeight(QFont::Bold);

    QFontMetrics titleMetrics(title);
    QFontMetrics currentMetrics(option.font);

    if (index.internalPointer())
        return QSize(46, qMax((separatorPixels * 2) + iconHeight, (separatorPixels * 4) + titleMetrics.height() + currentMetrics.height()));

    return QSize(34, (separatorPixels * 2) + titleMetrics.height() + 2);
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

int KPluginSelector::Private::PluginDelegate::getSeparatorPixels() const
{
    return separatorPixels;
}

void KPluginSelector::Private::PluginDelegate::setSeparatorPixels(int separatorPixels)
{
    this->separatorPixels = separatorPixels;
}

bool KPluginSelector::Private::PluginDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if ((event->type() == QEvent::MouseButtonPress) ||
        (event->type() == QEvent::KeyPress))
    {
        event->accept();

        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);

        if (keyEvent && (keyEvent->key() != Qt::Key_Space) &&
                        (keyEvent->key() != Qt::Key_Up) &&
                        (keyEvent->key() != Qt::Key_Down) &&
                        (keyEvent->key() != Qt::Key_Tab))
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

        QWidget *viewport = qobject_cast<QWidget*>(watched);
        if (viewport)
        {
            QModelIndex currentIndex;

            QListViewSpecialized *listView = dynamic_cast<QListViewSpecialized*>(viewport->parent());
            if (!listView) // the keyboard event comes directly from the view, not the viewport
                listView = dynamic_cast<QListViewSpecialized*>(viewport);

            if ((eventReceived == MouseEvent) && listView)
            {
                currentIndex = listView->indexAt(viewport->mapFromGlobal(QCursor::pos()));

                focusedElement = 0;
            }
            else if ((eventReceived == KeyboardEvent) && listView)
            {
                currentIndex = listView->currentIndex();
            }

            if (keyEvent && keyEvent->key() == Qt::Key_Up)
            {
                if (currentIndex.row() && listView->model()->index(currentIndex.row() - 1, 0).internalPointer())
                    listView->setCurrentIndex(listView->model()->index(currentIndex.row() - 1, 0));
                else if (currentIndex.row() > 2)
                    listView->setCurrentIndex(listView->model()->index(currentIndex.row() - 2, 0));
                else
                    listView->setCurrentIndex(QModelIndex());

                focusedElement = 0;

                return true;
            }
            else if (keyEvent && keyEvent->key() == Qt::Key_Down)
            {
                if ((currentIndex.row() < listView->model()->rowCount()) && listView->model()->index(currentIndex.row() + 1, 0).internalPointer())
                    listView->setCurrentIndex(listView->model()->index(currentIndex.row() + 1, 0));
                else if (currentIndex.row() + 1 < listView->model()->rowCount())
                    listView->setCurrentIndex(listView->model()->index(currentIndex.row() + 2, 0));
                else
                    listView->setCurrentIndex(QModelIndex());

                focusedElement = 0;

                return true;
            }
            else if (keyEvent && keyEvent->key() == Qt::Key_Tab)
            {
                focusedElement = (focusedElement + 1) % 2;

                listView->update(listView->currentIndex());

                return true;
            }

            if (listView && currentIndex.isValid())
            {
                QStyleOptionViewItem optionViewItem(listView->viewOptions());
                optionViewItem.rect = listView->visualRect(currentIndex);

                if (currentIndex.internalPointer()) {
                    const KPluginInfo pluginInfo(*static_cast<const KPluginInfo*>(currentIndex.internalPointer()));
                    if (pluginInfo.kcmServices().size())
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
    if (!currentModuleProxyList)
        return;

    QList<KCModuleProxy*>::iterator it;
    for (it = currentModuleProxyList->begin(); it != currentModuleProxyList->end(); ++it)
    {
        (*it)->defaults();
    }
}


void KPluginSelector::Private::PluginDelegate::processUrl(const QString &url) const
{
    new KRun(KUrl(url), parent->parent);
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

    if (!index.internalPointer())
    {
        return;
    }

    const KPluginInfo pluginInfo(*static_cast<KPluginInfo*>(index.internalPointer()));

    if ((checkRect(index, option).contains(cursorPos) && (eventReceived == MouseEvent)) ||
        ((focusedElement == 0) && (eventReceived == KeyboardEvent)))
    {
        listView->model()->setData(index, !listView->model()->data(index, Checked).toBool(), Checked);

        parent->dependenciesWidget->userOverrideDependency(pluginInfo.name());

        if (listView->model()->data(index, Checked).toBool()) // Item was checked
            checkDependencies(model, pluginInfo, DependenciesINeed);
        else
            checkDependencies(model, pluginInfo, DependenciesNeedMe);
    }

    if ((clickableLabelRect(option, caption).contains(cursorPos) && (eventReceived == MouseEvent)) ||
        ((focusedElement == 1) && (eventReceived == KeyboardEvent)))
    {
        listView->setCurrentIndex(index);

        if (!configDialogs.contains(index.row()))
        {
            QList<KService::Ptr> services = model->services(index);

            configDialog = new KDialog(parent->parent);
            configDialog->setWindowTitle(pluginInfo.name());
            KTabWidget *newTabWidget = new KTabWidget(configDialog);
            bool configurable = false;

            tabWidgets.insert(index.row(), newTabWidget);

            foreach(KService::Ptr servicePtr, services)
            {
                if(!servicePtr->noDisplay())
                {
                    KCModuleInfo moduleinfo(servicePtr);
                    model->setParentComponents(index, moduleinfo.service()->property("X-KDE-ParentComponents").toStringList());
                    KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleinfo, newTabWidget);
                    if (currentModuleProxy->realModule())
                    {
                        newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                        configurable = true;
                    }

                    if (!modulesDialogs.contains(index.row()))
                        modulesDialogs.insert(index.row(), QList<KCModuleProxy*>() << currentModuleProxy);
                    else
                    {
                        modulesDialogs[index.row()].append(currentModuleProxy);
                    }
                }
            }

            if (!configurable)
                configDialog->setButtons(KDialog::Close);
            else
                configDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);

            QWidget *aboutWidget = new QWidget(newTabWidget);
            QVBoxLayout *layout = new QVBoxLayout;
            layout->setSpacing(0);
            aboutWidget->setLayout(layout);

            if (!pluginInfo.comment().isEmpty())
            {
                QLabel *description = new QLabel(i18n("Description:\n\t%1", pluginInfo.comment()), newTabWidget);
                layout->addWidget(description);
                layout->addSpacing(20);
            }

            if (!pluginInfo.author().isEmpty())
            {
                QLabel *author = new QLabel(i18n("Author:\n\t%1", pluginInfo.author()), newTabWidget);
                layout->addWidget(author);
                layout->addSpacing(20);
            }

            if (!pluginInfo.email().isEmpty())
            {
                QLabel *authorEmail = new QLabel(i18n("E-Mail:"), newTabWidget);
                KUrlLabel *sendEmail = new KUrlLabel("mailto:" + pluginInfo.email(), '\t' + pluginInfo.email());

                sendEmail->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                sendEmail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                sendEmail->setGlowEnabled(false);
                sendEmail->setUnderline(false);
                sendEmail->setFloatEnabled(true);
                sendEmail->setUseCursor(false);
                sendEmail->setHighlightedColor(option.palette.color(QPalette::Link));
                sendEmail->setSelectedColor(option.palette.color(QPalette::Link));

                QObject::connect(sendEmail, SIGNAL(leftClickedUrl(QString)), this, SLOT(processUrl(QString)));

                layout->addWidget(authorEmail);
                layout->addWidget(sendEmail);
                layout->addSpacing(20);
            }

            if (!pluginInfo.website().isEmpty())
            {
                QLabel *website = new QLabel(i18n("Website:"), newTabWidget);
                KUrlLabel *visitWebsite = new KUrlLabel(pluginInfo.website(), '\t' + pluginInfo.website());

                visitWebsite->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                visitWebsite->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                visitWebsite->setGlowEnabled(false);
                visitWebsite->setUnderline(false);
                visitWebsite->setFloatEnabled(true);
                visitWebsite->setUseCursor(false);
                visitWebsite->setHighlightedColor(option.palette.color(QPalette::Link));
                visitWebsite->setSelectedColor(option.palette.color(QPalette::Link));

                QObject::connect(visitWebsite, SIGNAL(leftClickedUrl(QString)), this, SLOT(processUrl(QString)));

                layout->addWidget(website);
                layout->addWidget(visitWebsite);
                layout->addSpacing(20);
            }

            if (!pluginInfo.version().isEmpty())
            {
                QLabel *version = new QLabel(i18n("Version:\n\t%1", pluginInfo.version()), newTabWidget);

                layout->addWidget(version);
                layout->addSpacing(20);
            }

            if (!pluginInfo.license().isEmpty())
            {
                QLabel *license = new QLabel(i18n("License:\n\t%1", pluginInfo.license()), newTabWidget);

                layout->addWidget(license);
                layout->addSpacing(20);
            }

            layout->insertStretch(-1);

            newTabWidget->addTab(aboutWidget, i18n("About"));
            configDialog->setMainWidget(newTabWidget);

            configDialogs.insert(index.row(), configDialog);
        }
        else
        {
            configDialog = configDialogs[index.row()];
        }

        currentModuleProxyList = modulesDialogs.contains(index.row()) ? &modulesDialogs[index.row()] : 0;

        QObject::connect(configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));

        if (configDialog->exec() == QDialog::Accepted)
        {
            foreach (KCModuleProxy *moduleProxy, modulesDialogs[index.row()])
            {
                moduleProxy->save();
                foreach (const QString &parentComponent, model->parentComponents(index))
                {
                    emit configCommitted(parentComponent.toLatin1());
                }
            }
        }
        else
        {
            foreach (KCModuleProxy *moduleProxy, modulesDialogs[index.row()])
            {
                moduleProxy->load();
            }
        }

        // Since the dialog is cached and the last tab selected will be kept selected, when closing the
        // dialog we set the selected tab to the first one again
        if (tabWidgets.contains(index.row()))
            tabWidgets[index.row()]->setCurrentIndex(0);

        QObject::disconnect(configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
    }
}

void KPluginSelector::Private::PluginDelegate::checkDependencies(PluginModel *model,
                                                                 const KPluginInfo &info,
                                                                 CheckWhatDependencies whatDependencies)
{
    QStringList dependenciesPushed;

    if (whatDependencies == DependenciesINeed)
    {
        foreach(const QString &dependency, info.dependencies())
        {
            model->updateDependencies(dependency, info.name(), whatDependencies, dependenciesPushed);
        }
    }
    else
    {
        model->updateDependencies(info.pluginName(), info.name(), whatDependencies, dependenciesPushed);
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
