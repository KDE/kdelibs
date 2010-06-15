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
#include <kdialog.h>
#include <kurllabel.h>
#include <ktabwidget.h>
#include <kcmoduleinfo.h>
#include <kcmoduleproxy.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>
#include <kcategorydrawer.h>
#include <kcategorizedview.h>
#include <kcategorizedsortfilterproxymodel.h>
#include <kaboutapplicationdialog.h>

#define MARGIN 5

KPluginSelector::Private::Private(KPluginSelector *parent)
    : QObject(parent)
    , parent(parent)
    , listView(0)
    , categoryDrawer(new KCategoryDrawer)
    , showIcons(false)
{
}

KPluginSelector::Private::~Private()
{
    delete categoryDrawer;
}

void KPluginSelector::Private::updateDependencies(PluginEntry *pluginEntry, bool added)
{
    if (added) {
        QStringList dependencyList = pluginEntry->pluginInfo.dependencies();

        if (!dependencyList.count()) {
            return;
        }

        for (int i = 0; i < pluginModel->rowCount(); i++) {
            const QModelIndex index = pluginModel->index(i, 0);
            PluginEntry *pe = static_cast<PluginEntry*>(index.internalPointer());

            if ((pe->pluginInfo.pluginName() != pluginEntry->pluginInfo.pluginName()) &&
                dependencyList.contains(pe->pluginInfo.pluginName()) && !pe->checked) {
                dependenciesWidget->addDependency(pe->pluginInfo.name(), pluginEntry->pluginInfo.name(), added);
                const_cast<QAbstractItemModel*>(index.model())->setData(index, added, Qt::CheckStateRole);
                updateDependencies(pe, added);
            }
        }
    } else {
        for (int i = 0; i < pluginModel->rowCount(); i++) {
            const QModelIndex index = pluginModel->index(i, 0);
            PluginEntry *pe = static_cast<PluginEntry*>(index.internalPointer());

            if ((pe->pluginInfo.pluginName() != pluginEntry->pluginInfo.pluginName()) &&
                pe->pluginInfo.dependencies().contains(pluginEntry->pluginInfo.pluginName()) && pe->checked) {
                dependenciesWidget->addDependency(pe->pluginInfo.name(), pluginEntry->pluginInfo.name(), added);
                const_cast<QAbstractItemModel*>(index.model())->setData(index, added, Qt::CheckStateRole);
                updateDependencies(pe, added);
            }
        }
    }
}

int KPluginSelector::Private::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (listView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

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
    link->setText(i18n("Automatic changes have been performed due to plugin dependencies. Click here for further information"));
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
        message += i18np("%1 plugin automatically added due to plugin dependencies", "%1 plugins automatically added due to plugin dependencies", addedByDependencies);

    if (removedByDependencies && !message.isEmpty())
        message += i18n(", ");

    if (removedByDependencies)
        message += i18np("%1 plugin automatically removed due to plugin dependencies", "%1 plugins automatically removed due to plugin dependencies", removedByDependencies);

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
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    d->lineEdit = new KLineEdit(this);
    d->lineEdit->setClearButtonShown(true);
    d->lineEdit->setClickMessage(i18n("Search Plugins"));
    d->listView = new KCategorizedView(this);
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);
    d->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // bug 213068
    d->listView->setAlternatingRowColors(true);
    d->listView->setCategoryDrawer(d->categoryDrawer);
    d->dependenciesWidget = new Private::DependenciesWidget(this);

    d->pluginModel = new Private::PluginModel(d, this);
    d->proxyModel = new Private::ProxyModel(d, this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->pluginModel);
    d->listView->setModel(d->proxyModel);
    d->listView->setAlternatingRowColors(true);

    Private::PluginDelegate *pluginDelegate = new Private::PluginDelegate(d, this);
    d->listView->setItemDelegate(pluginDelegate);

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, SIGNAL(textChanged(QString)), d->proxyModel, SLOT(invalidate()));
    connect(pluginDelegate, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(pluginDelegate, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));

    layout->addWidget(d->lineEdit);
    layout->addWidget(d->listView);
    layout->addWidget(d->dependenciesWidget);
}

KPluginSelector::~KPluginSelector()
{
    delete d->listView; // depends on some other things in d, make sure this dies first.
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

    KConfigGroup cfgGroup(config, "KParts Plugins");
    kDebug( 702 ) << "cfgGroup = " << &cfgGroup;

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey, cfgGroup);
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

    KConfigGroup cfgGroup(config ? config : KGlobal::config(), "Plugins");
    kDebug( 702 ) << "cfgGroup = " << &cfgGroup;

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey, cfgGroup, pluginLoadMethod, true /* manually added */);
}

void KPluginSelector::load()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());
        pluginEntry->pluginInfo.load(pluginEntry->cfgGroup);
        d->pluginModel->setData(index, pluginEntry->pluginInfo.isPluginEnabled(), Qt::CheckStateRole);
    }

    emit changed(false);
}

void KPluginSelector::save()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());
        pluginEntry->pluginInfo.setPluginEnabled(pluginEntry->checked);
        pluginEntry->pluginInfo.save(pluginEntry->cfgGroup);
        pluginEntry->cfgGroup.sync();
    }

    emit changed(false);
}

void KPluginSelector::defaults()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());
        d->pluginModel->setData(index, pluginEntry->pluginInfo.isPluginEnabledByDefault(), Qt::CheckStateRole);
    }

    emit changed(true);
}

bool KPluginSelector::isDefault() const
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());
        if (d->pluginModel->data(index, Qt::CheckStateRole).toBool() != pluginEntry->pluginInfo.isPluginEnabledByDefault()) {
            return false;
        }
    }

    return true;
}

void KPluginSelector::updatePluginsState()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());
        if (pluginEntry->manuallyAdded) {
            pluginEntry->pluginInfo.setPluginEnabled(pluginEntry->checked);
        }
    }
}

KPluginSelector::Private::PluginModel::PluginModel(KPluginSelector::Private *pluginSelector_d, QObject *parent)
    : QAbstractListModel(parent)
    , pluginSelector_d(pluginSelector_d)
{
}

KPluginSelector::Private::PluginModel::~PluginModel()
{
}

void KPluginSelector::Private::PluginModel::addPlugins(const QList<KPluginInfo> &pluginList, const QString &categoryName, const QString &categoryKey, const KConfigGroup &cfgGroup, PluginLoadMethod pluginLoadMethod, bool manuallyAdded)
{
    QList<PluginEntry> listToAdd;

    foreach (const KPluginInfo &pluginInfo, pluginList) {
        PluginEntry pluginEntry;
        pluginEntry.category = categoryName;
        pluginEntry.pluginInfo = pluginInfo;
        if (pluginLoadMethod == ReadConfigFile) {
            pluginEntry.pluginInfo.load(cfgGroup);
        }
        pluginEntry.checked = pluginInfo.isPluginEnabled();
        pluginEntry.manuallyAdded = manuallyAdded;
        if (cfgGroup.isValid()) {
            pluginEntry.cfgGroup = cfgGroup;
        } else {
            pluginEntry.cfgGroup = pluginInfo.config();
        }

        // this is where kiosk will set if a plugin is checkable or not (pluginName + "Enabled")
        pluginEntry.isCheckable = !pluginInfo.isValid() || !pluginEntry.cfgGroup.isEntryImmutable(pluginInfo.pluginName() + QLatin1String("Enabled"));

        if (!pluginEntryList.contains(pluginEntry) && !listToAdd.contains(pluginEntry) &&
             (!pluginInfo.property("X-KDE-PluginInfo-Category").isValid() ||
              !pluginInfo.property("X-KDE-PluginInfo-Category").toString().compare(categoryKey, Qt::CaseInsensitive)) &&
            (pluginInfo.service().isNull() || !pluginInfo.service()->noDisplay())) {
            listToAdd << pluginEntry;

            if (!pluginSelector_d->showIcons && !pluginInfo.icon().isEmpty()) {
                pluginSelector_d->showIcons = true;
            }
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
    Q_UNUSED(parent)

    return createIndex(row, column, (row < pluginEntryList.count()) ? (void*) &pluginEntryList.at(row)
                                                                    : 0);
}

QVariant KPluginSelector::Private::PluginModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return QVariant();
    }

    PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());

    switch (role) {
        case Qt::DisplayRole:
            return pluginEntry->pluginInfo.name();
        case PluginEntryRole:
            return QVariant::fromValue(pluginEntry);
        case ServicesCountRole:
            return pluginEntry->pluginInfo.kcmServices().count();
        case NameRole:
            return pluginEntry->pluginInfo.name();
        case CommentRole:
            return pluginEntry->pluginInfo.comment();
        case AuthorRole:
            return pluginEntry->pluginInfo.author();
        case EmailRole:
            return pluginEntry->pluginInfo.email();
        case WebsiteRole:
            return pluginEntry->pluginInfo.website();
        case VersionRole:
            return pluginEntry->pluginInfo.version();
        case LicenseRole:
            return pluginEntry->pluginInfo.license();
        case DependenciesRole:
            return pluginEntry->pluginInfo.dependencies();
        case IsCheckableRole:
            return pluginEntry->isCheckable;
        case Qt::DecorationRole:
            return pluginEntry->pluginInfo.icon();
        case Qt::CheckStateRole:
            return pluginEntry->checked;
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole: // fall through
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            return pluginEntry->category;
        default:
            return QVariant();
    }
}

bool KPluginSelector::Private::PluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        static_cast<PluginEntry*>(index.internalPointer())->checked = value.toBool();
        ret = true;
    }

    if (ret) {
        emit dataChanged(index, index);
    }

    return ret;
}

int KPluginSelector::Private::PluginModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return pluginEntryList.count();
}

KPluginSelector::Private::ProxyModel::ProxyModel(KPluginSelector::Private *pluginSelector_d, QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , pluginSelector_d(pluginSelector_d)
{
    sort(0);
}

KPluginSelector::Private::ProxyModel::~ProxyModel()
{
}

bool KPluginSelector::Private::ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    if (!pluginSelector_d->lineEdit->text().isEmpty()) {
        const QModelIndex index = sourceModel()->index(sourceRow, 0);
        const KPluginInfo pluginInfo = static_cast<PluginEntry*>(index.internalPointer())->pluginInfo;
        return pluginInfo.name().contains(pluginSelector_d->lineEdit->text(), Qt::CaseInsensitive) ||
               pluginInfo.comment().contains(pluginSelector_d->lineEdit->text(), Qt::CaseInsensitive);
    }

    return true;
}

bool KPluginSelector::Private::ProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return static_cast<PluginEntry*>(left.internalPointer())->pluginInfo.name().compare(static_cast<PluginEntry*>(right.internalPointer())->pluginInfo.name(), Qt::CaseInsensitive) < 0;
}

KPluginSelector::Private::PluginDelegate::PluginDelegate(KPluginSelector::Private *pluginSelector_d, QObject *parent)
    : KWidgetItemDelegate(pluginSelector_d->listView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new KPushButton)
    , pluginSelector_d(pluginSelector_d)
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
    bool disabled = !index.model()->data(index, IsCheckableRole).toBool();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    int iconSize = option.rect.height() - MARGIN * 2;
    if (pluginSelector_d->showIcons) {
        QPixmap pixmap = KIconLoader::global()->loadIcon(index.model()->data(index, Qt::DecorationRole).toString(),
                                                         KIconLoader::Desktop, iconSize, disabled ? KIconLoader::DisabledState : KIconLoader::DefaultState);

        painter->drawPixmap(QRect(pluginSelector_d->dependantLayoutValue(MARGIN + option.rect.left() + xOffset, iconSize, option.rect.width()), MARGIN + option.rect.top(), iconSize, iconSize), pixmap, QRect(0, 0, iconSize, iconSize));
    } else {
        iconSize = -MARGIN;
    }

    QRect contentsRect(pluginSelector_d->dependantLayoutValue(MARGIN * 2 + iconSize + option.rect.left() + xOffset, option.rect.width() - MARGIN * 3 - iconSize - xOffset, option.rect.width()), MARGIN + option.rect.top(), option.rect.width() - MARGIN * 3 - iconSize - xOffset, option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();
    if (index.model()->data(index, ServicesCountRole).toBool()) {
        lessHorizontalSpace += MARGIN + pushButton->sizeHint().width();
    }

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    if (pluginSelector_d->listView->layoutDirection() == Qt::RightToLeft) {
        contentsRect.translate(lessHorizontalSpace, 0);
    }

    painter->save();
    if (disabled) {
        QPalette pal(option.palette);
        pal.setCurrentColorGroup(QPalette::Disabled);
        painter->setPen(pal.text().color());
    }

    painter->save();
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(index.model()->data(index, CommentRole).toString(), Qt::ElideRight, contentsRect.width()));

    painter->restore();
    painter->restore();
}

QSize KPluginSelector::Private::PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 5;
    int j = 1;
    if (index.model()->data(index, ServicesCountRole).toBool()) {
        i = 6;
        j = 2;
    }

    if (!pluginSelector_d->showIcons) {
        i--;
    }

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(qMax(fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()),
                      option.fontMetrics.width(index.model()->data(index, CommentRole).toString())) +
                      pluginSelector_d->showIcons ? KIconLoader::SizeMedium : 0 + MARGIN * i + pushButton->sizeHint().width() * j,
                 qMax(KIconLoader::SizeMedium + MARGIN * 2, fmTitle.height() + option.fontMetrics.height() + MARGIN * 2));
}

QList<QWidget*> KPluginSelector::Private::PluginDelegate::createItemWidgets() const
{
    QList<QWidget*> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(slotStateChanged(bool)));
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(emitChanged()));

    KPushButton *aboutPushButton = new KPushButton;
    aboutPushButton->setIcon(KIcon("dialog-information"));
    connect(aboutPushButton, SIGNAL(clicked(bool)), this, SLOT(slotAboutClicked()));

    KPushButton *configurePushButton = new KPushButton;
    configurePushButton->setIcon(KIcon("configure"));
    connect(configurePushButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureClicked()));

    setBlockedEventTypes(enabledCheckBox, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                            << QEvent::KeyPress << QEvent::KeyRelease);

    setBlockedEventTypes(aboutPushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                            << QEvent::KeyPress << QEvent::KeyRelease);
                            
    setBlockedEventTypes(configurePushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                            << QEvent::KeyPress << QEvent::KeyRelease);

    widgetList << enabledCheckBox << configurePushButton << aboutPushButton;

    return widgetList;
}

void KPluginSelector::Private::PluginDelegate::updateItemWidgets(const QList<QWidget*> widgets,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QPersistentModelIndex &index) const
{
    QCheckBox *checkBox = static_cast<QCheckBox*>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(pluginSelector_d->dependantLayoutValue(MARGIN, checkBox->sizeHint().width(), option.rect.width()), option.rect.height() / 2 - checkBox->sizeHint().height() / 2);

    KPushButton *aboutPushButton = static_cast<KPushButton*>(widgets[2]);
    QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
    aboutPushButton->resize(aboutPushButtonSizeHint);
    aboutPushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - MARGIN - aboutPushButtonSizeHint.width(), aboutPushButtonSizeHint.width(), option.rect.width()), option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);

    KPushButton *configurePushButton = static_cast<KPushButton*>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - MARGIN * 2 - configurePushButtonSizeHint.width() - aboutPushButtonSizeHint.width(), configurePushButtonSizeHint.width(), option.rect.width()), option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);

    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        aboutPushButton->setVisible(false);
        configurePushButton->setVisible(false);
    } else {
        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        checkBox->setEnabled(index.model()->data(index, IsCheckableRole).toBool());
        configurePushButton->setVisible(index.model()->data(index, ServicesCountRole).toBool());
        configurePushButton->setEnabled(index.model()->data(index, Qt::CheckStateRole).toBool());
    }
}

void KPluginSelector::Private::PluginDelegate::slotStateChanged(bool state)
{
    if (!focusedIndex().isValid())
        return;

    const QModelIndex index = focusedIndex();

    pluginSelector_d->dependenciesWidget->clearDependencies();

    PluginEntry *pluginEntry = index.model()->data(index, PluginEntryRole).value<PluginEntry*>();
    pluginSelector_d->updateDependencies(pluginEntry, state);

    const_cast<QAbstractItemModel*>(index.model())->setData(index, state, Qt::CheckStateRole);
}

void KPluginSelector::Private::PluginDelegate::emitChanged()
{
    emit changed(true);
}

void KPluginSelector::Private::PluginDelegate::slotAboutClicked()
{
    const QModelIndex index = focusedIndex();
    const QAbstractItemModel *model = index.model();

    // Try to retrieve the plugin information from the KComponentData object of the plugin.
    // If there is no valid information, go and fetch it from the service itself (the .desktop
    // file).

    PluginEntry *entry = index.model()->data(index, PluginEntryRole).value<PluginEntry*>();
    KService::Ptr entryService = entry->pluginInfo.service();
    if (entryService) {
        KPluginLoader loader(*entryService);
        KPluginFactory *factory = loader.factory();
        if (factory) {
            const KAboutData *aboutData = factory->componentData().aboutData();
            if (!aboutData->programName().isEmpty()) { // Be sure the about data is not completely empty
                KAboutApplicationDialog aboutPlugin(aboutData, itemView());
                aboutPlugin.exec();
                return;
            }
        }
    }

    const QString name = model->data(index, NameRole).toString();
    const QString comment = model->data(index, CommentRole).toString();
    const QString author = model->data(index, AuthorRole).toString();
    const QString email = model->data(index, EmailRole).toString();
    const QString website = model->data(index, WebsiteRole).toString();
    const QString version = model->data(index, VersionRole).toString();
    const QString license = model->data(index, LicenseRole).toString();

    KAboutData aboutData(name.toUtf8(), name.toUtf8(), ki18n(name.toUtf8()), version.toUtf8(), ki18n(comment.toUtf8()), KAboutLicense::byKeyword(license).key(), ki18n(QByteArray()), ki18n(QByteArray()), website.toLatin1());
    aboutData.setProgramIconName(index.model()->data(index, Qt::DecorationRole).toString());
    const QStringList authors = author.split(',');
    const QStringList emails = email.split(',');
    int i = 0;
    if (authors.count() == emails.count()) {
        foreach (const QString &author, authors) {
            if (!author.isEmpty()) {
                aboutData.addAuthor(ki18n(author.toUtf8()), ki18n(QByteArray()), emails[i].toUtf8(), 0);
            }
            i++;
        }
    }
    KAboutApplicationDialog aboutPlugin(&aboutData, itemView());
    aboutPlugin.exec();
}

void KPluginSelector::Private::PluginDelegate::slotConfigureClicked()
{
    const QModelIndex index = focusedIndex();
    const QAbstractItemModel *model = index.model();

    PluginEntry *pluginEntry = model->data(index, PluginEntryRole).value<PluginEntry*>();
    KPluginInfo pluginInfo = pluginEntry->pluginInfo;

    KDialog configDialog(itemView());
    configDialog.setWindowTitle(model->data(index, NameRole).toString());
    // The number of KCModuleProxies in use determines whether to use a tabwidget
    KTabWidget *newTabWidget = 0;
    // Widget to use for the setting dialog's main widget,
    // either a KTabWidget or a KCModuleProxy
    QWidget * mainWidget = 0;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = &configDialog;

    foreach (const KService::Ptr &servicePtr, pluginInfo.kcmServices()) {
        if(!servicePtr->noDisplay()) {
            KCModuleInfo moduleInfo(servicePtr);
            KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleInfo, moduleProxyParentWidget);
            if (currentModuleProxy->realModule()) {
                moduleProxyList << currentModuleProxy;
                if (mainWidget && !newTabWidget) {
                    // we already created one KCModuleProxy, so we need a tab widget.
                    // Move the first proxy into the tab widget and ensure this and subsequent
                    // proxies are in the tab widget
                    newTabWidget = new KTabWidget(&configDialog);
                    moduleProxyParentWidget = newTabWidget;
                    mainWidget->setParent( newTabWidget );
                    KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy*>(mainWidget);
                    if (moduleProxy) {
                        newTabWidget->addTab(mainWidget, moduleProxy->moduleInfo().moduleName());
                        mainWidget = newTabWidget;
                    } else {
                        delete newTabWidget;
                        newTabWidget = 0;
                        moduleProxyParentWidget = &configDialog;
                        mainWidget->setParent(0);
                    }
                }

                if (newTabWidget) {
                    newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                } else {
                    mainWidget = currentModuleProxy;
                }
            } else {
                delete currentModuleProxy;
            }
        }
    }

    // it could happen that we had services to show, but none of them were real modules.
    if (moduleProxyList.count()) {
        configDialog.setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);

        QWidget *showWidget = new QWidget(&configDialog);
        QVBoxLayout *layout = new QVBoxLayout;
        showWidget->setLayout(layout);
        layout->addWidget(mainWidget);
        layout->insertSpacing(-1, KDialog::marginHint());
        configDialog.setMainWidget(showWidget);

        connect(&configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));

        if (configDialog.exec() == QDialog::Accepted) {
            foreach (KCModuleProxy *moduleProxy, moduleProxyList) {
                QStringList parentComponents = moduleProxy->moduleInfo().service()->property("X-KDE-ParentComponents").toStringList();
                moduleProxy->save();
                foreach (const QString &parentComponent, parentComponents) {
                    emit configCommitted(parentComponent.toLatin1());
                }
            }
        } else {
            foreach (KCModuleProxy *moduleProxy, moduleProxyList) {
                moduleProxy->load();
            }
        }

        qDeleteAll(moduleProxyList);
        moduleProxyList.clear();
    }
}

void KPluginSelector::Private::PluginDelegate::slotDefaultClicked()
{
    foreach (KCModuleProxy *moduleProxy, moduleProxyList) {
        moduleProxy->defaults();
    }
}

QFont KPluginSelector::Private::PluginDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

#include "kpluginselector_p.moc"
#include "kpluginselector.moc"
