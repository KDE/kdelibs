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

#include "kpluginselector.h"
#include "kpluginselector_p.h"

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QBoxLayout>
#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QStyleOptionViewItemV4>

#include <kdebug.h>
#include <klineedit.h>
#include <kurllabel.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>
#include <kcategorydrawer.h>
#include <kcategorizedview.h>
#include <kcategorizedsortfilterproxymodel.h>

#define ICON_SIZE 32
#define MARGIN 5

static const char *details = I18N_NOOP("Settings");
static const char *about = I18N_NOOP("About");

KPluginSelector::Private::Private(KPluginSelector *parent)
    : QObject(parent)
    , parent(parent)
    , listView(0)
    , showIcons(false)
{
}

KPluginSelector::Private::~Private()
{
}

void KPluginSelector::Private::checkIfShowIcons(const QList<KPluginInfo> &pluginInfoList)
{
    foreach (const KPluginInfo &pluginInfo, pluginInfoList)
    {
        if (!KIconLoader::global()->iconPath(pluginInfo.icon(), KIconLoader::NoGroup, true).isNull())
        {
            showIcons = true;
            return;
        }
    }
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
    label->setPixmap(KIconLoader::global()->loadIcon("dialog-information", KIconLoader::Dialog));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(label);
    KUrlLabel *link = new KUrlLabel();
    link->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    link->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    link->setGlowEnabled(false);
    link->setUnderline(false);
    link->setFloatEnabled(true);
    link->setUseCursor(true);
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
    QString message = i18n("Automatic changes have been performed in order to satisfy plugin dependencies:\n");
    foreach(const QString &dependency, dependencyMap.keys())
    {
        if (dependencyMap[dependency].added)
            message += i18n("\n    %1 plugin has been automatically checked because of the dependency of %2 plugin", dependency, dependencyMap[dependency].pluginCausant);
        else
            message += i18n("\n    %1 plugin has been automatically unchecked because of its dependency on %2 plugin", dependency, dependencyMap[dependency].pluginCausant);
    }
    KMessageBox::information(this, message, i18n("Dependency Check"));

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


KPluginSelector::KPluginSelector(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    QObject::connect(d, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    QObject::connect(d, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    d->lineEdit = new KLineEdit(this);
    d->lineEdit->setClearButtonShown(true);
    d->lineEdit->setClickMessage(i18n("Search Plugins"));
    d->listView = new KCategorizedView(this);
    d->listView->setCategoryDrawer(new KCategoryDrawer);
    d->dependenciesWidget = new Private::DependenciesWidget(this);

    d->pluginModel = new Private::PluginModel(this);
    d->proxyModel = new Private::ProxyModel(d, this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->pluginModel);
    d->listView->setModel(d->proxyModel);
    d->listView->setItemDelegate(new Private::PluginDelegate(d->listView, this));

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, SIGNAL(textChanged(QString)), d->proxyModel, SLOT(invalidate()));

    layout->addWidget(d->lineEdit);
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
    kDebug( 702 ) << "cfgGroup = " << cfgGroup;

    d->checkIfShowIcons(pluginInfoList);

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey);
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
    kDebug( 702 ) << "cfgGroup = " << cfgGroup;

    d->checkIfShowIcons(pluginInfoList);

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey);
}

void KPluginSelector::load()
{
}

void KPluginSelector::save()
{
}

void KPluginSelector::defaults()
{
}

void KPluginSelector::updatePluginsState()
{
}

KPluginSelector::Private::PluginModel::PluginModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

KPluginSelector::Private::PluginModel::~PluginModel()
{
}

void KPluginSelector::Private::PluginModel::addPlugins(const QList<KPluginInfo> &pluginList, const QString &categoryName, const QString &categoryKey)
{
    QList<PluginEntry> listToAdd;

    foreach (const KPluginInfo &pluginInfo, pluginList) {
        PluginEntry pluginEntry;
        pluginEntry.category = categoryName;
        pluginEntry.pluginInfo = pluginInfo;

        if (!pluginEntryList.contains(pluginEntry) && !listToAdd.contains(pluginEntry) &&
             (!pluginInfo.property("X-KDE-PluginInfo-Category").isValid() ||
              !pluginInfo.property("X-KDE-PluginInfo-Category").toString().compare(categoryKey, Qt::CaseInsensitive))) {
            listToAdd << pluginEntry;
        }
    }

    if (listToAdd.count()) {
        beginInsertRows(QModelIndex(), pluginEntryList.count(), pluginEntryList.count() + listToAdd.count() - 1);
        pluginEntryList << listToAdd;
        endInsertRows();
    }
}

QList<KService::Ptr> KPluginSelector::Private::PluginModel::pluginServices(const QModelIndex &index) const
{
    return static_cast<PluginEntry*>(index.internalPointer())->pluginInfo.kcmServices();
}

QModelIndex KPluginSelector::Private::PluginModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column, (row < pluginEntryList.count()) ? (void*) &pluginEntryList.at(row)
                                                                    : 0);
}

QVariant KPluginSelector::Private::PluginModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
            return static_cast<PluginEntry*>(index.internalPointer())->pluginInfo.name();
        case CommentRole:
            return static_cast<PluginEntry*>(index.internalPointer())->pluginInfo.comment();
        case ServicesCountRole:
            return static_cast<PluginEntry*>(index.internalPointer())->pluginInfo.kcmServices().count();
//         case Qt::CheckStateRole:
//             return false;
        case Qt::DecorationRole:
            return static_cast<PluginEntry*>(index.internalPointer())->pluginInfo.icon();
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole: // fall through
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            return static_cast<PluginEntry*>(index.internalPointer())->category;
        default:
            return QVariant();
    }
}

int KPluginSelector::Private::PluginModel::rowCount(const QModelIndex &parent) const
{
    return pluginEntryList.count();
}

// Qt::ItemFlags KPluginSelector::Private::PluginModel::flags(const QModelIndex &index) const
// {
//     return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
// }

KPluginSelector::Private::ProxyModel::ProxyModel(KPluginSelector::Private *q, QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , q(q)
{
    sort(0);
}

KPluginSelector::Private::ProxyModel::~ProxyModel()
{
}

bool KPluginSelector::Private::ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!q->lineEdit->text().isEmpty()) {
        const QModelIndex index = sourceModel()->index(sourceRow, 0);
        const KPluginInfo pluginInfo = static_cast<PluginEntry*>(index.internalPointer())->pluginInfo;
        return pluginInfo.name().contains(q->lineEdit->text(), Qt::CaseInsensitive) ||
               pluginInfo.comment().contains(q->lineEdit->text(), Qt::CaseInsensitive);
    }

    return true;
}

bool KPluginSelector::Private::ProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return static_cast<PluginEntry*>(left.internalPointer())->pluginInfo.name().compare(static_cast<PluginEntry*>(right.internalPointer())->pluginInfo.name(), Qt::CaseInsensitive) < 0;
}

KPluginSelector::Private::PluginDelegate::PluginDelegate(QAbstractItemView *itemView, QObject *parent)
    : KWidgetItemDelegate(itemView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new KPushButton)
{
    pushButton->setIcon(KIcon("configure")); // only for getting size matters
}

KPluginSelector::Private::PluginDelegate::~PluginDelegate()
{
    delete checkBox;
    delete pushButton;
}

void KPluginSelector::Private::PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    int xOffset = checkBox->sizeHint().width();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    const int iconSize = option.rect.height() - MARGIN * 2;

    KIcon icon(index.model()->data(index, Qt::DecorationRole).toString());
    painter->drawPixmap(QRect(MARGIN + option.rect.left() + xOffset, MARGIN + option.rect.top(), iconSize, iconSize), icon.pixmap(iconSize, iconSize), QRect(0, 0, iconSize, iconSize));

    QRect contentsRect(MARGIN * 2 + iconSize + option.rect.left() + xOffset, MARGIN + option.rect.top(), option.rect.width() - MARGIN * 3 - iconSize - xOffset, option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();
    if (index.model()->data(index, ServicesCountRole).toBool()) {
        lessHorizontalSpace += MARGIN + pushButton->sizeHint().width();
    }

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    painter->save();
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(index.model()->data(index, CommentRole).toString(), Qt::ElideRight, contentsRect.width()));

    painter->restore();

    KWidgetItemDelegate::paintWidgets(painter, option, index);
}

QSize KPluginSelector::Private::PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 5;
    int j = 1;
    if (index.model()->data(index, ServicesCountRole).toBool()) {
        i = 6;
        j = 2;
    }

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(qMax(fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()),
                      option.fontMetrics.width(index.model()->data(index, CommentRole).toString())) +
                      ICON_SIZE + MARGIN * i + pushButton->sizeHint().width() * j,
                 qMax(ICON_SIZE + MARGIN * 2, fmTitle.height() + option.fontMetrics.height() + MARGIN * 2));
}

QList<QWidget*> KPluginSelector::Private::PluginDelegate::createItemWidgets() const
{
    QList<QWidget*> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;

    KPushButton *aboutPushButton = new KPushButton;
    aboutPushButton->setIcon(KIcon("dialog-information"));

    KPushButton *configurePushButton = new KPushButton;
    configurePushButton->setIcon(KIcon("configure"));

    widgetList << enabledCheckBox << configurePushButton << aboutPushButton;

    return widgetList;
}

void KPluginSelector::Private::PluginDelegate::updateItemWidgets(const QList<QWidget*> widgets,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    QCheckBox *checkBox = static_cast<QCheckBox*>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(MARGIN, option.rect.height() / 2 - widgets[0]->sizeHint().height() / 2);

    KPushButton *aboutPushButton = static_cast<KPushButton*>(widgets[2]);
    aboutPushButton->resize(aboutPushButton->sizeHint());
    aboutPushButton->move(option.rect.width() - MARGIN - aboutPushButton->sizeHint().width(), option.rect.height() / 2 - widgets[2]->sizeHint().height() / 2);

    KPushButton *configurePushButton = static_cast<KPushButton*>(widgets[1]);
    configurePushButton->resize(configurePushButton->sizeHint());
    configurePushButton->move(option.rect.width() - MARGIN * 2 - configurePushButton->sizeHint().width() - aboutPushButton->sizeHint().width(), option.rect.height() / 2 - widgets[1]->sizeHint().height() / 2);

    configurePushButton->setVisible(index.model()->data(index, ServicesCountRole).toBool());
}

QFont KPluginSelector::Private::PluginDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

#include "kpluginselector_p.moc"
#include "kpluginselector.moc"
