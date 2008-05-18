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

#define ICON_SIZE 32
#define MARGIN 5

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

void KPluginSelector::Private::updateDependencies(const KPluginInfo &pluginInfo, bool added)
{
    if (added) {
        QStringList dependencyList = pluginInfo.dependencies();

        if (!dependencyList.count()) {
            return;
        }

        for (int i = 0; i < pluginModel->rowCount(); i++) {
            QModelIndex index = pluginModel->index(i, 0);
            Private::PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());

            if ((pluginEntry->pluginInfo.pluginName() != pluginInfo.pluginName()) &&
                dependencyList.contains(pluginEntry->pluginInfo.pluginName())) {
                dependenciesWidget->addDependency(pluginEntry->pluginInfo.name(), pluginInfo.name(), added);
                const_cast<QAbstractItemModel*>(index.model())->setData(index, added, Qt::CheckStateRole);
                updateDependencies(pluginEntry->pluginInfo, added);
            }
        }
    } else {
        for (int i = 0; i < pluginModel->rowCount(); i++) {
            QModelIndex index = pluginModel->index(i, 0);
            Private::PluginEntry *pluginEntry = static_cast<PluginEntry*>(index.internalPointer());

            if ((pluginEntry->pluginInfo.pluginName() != pluginInfo.pluginName()) &&
                pluginEntry->pluginInfo.dependencies().contains(pluginInfo.pluginName())) {
                dependenciesWidget->addDependency(pluginEntry->pluginInfo.name(), pluginInfo.name(), added);
                const_cast<QAbstractItemModel*>(index.model())->setData(index, added, Qt::CheckStateRole);
                updateDependencies(pluginEntry->pluginInfo, added);
            }
        }
    }
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

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey, *cfgGroup);
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

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey, *cfgGroup, pluginLoadMethod, true /* manually added */);
}

void KPluginSelector::load()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        QModelIndex index = d->pluginModel->index(i, 0);
        Private::PluginEntry *pluginEntry = static_cast<Private::PluginEntry*>(index.internalPointer());
        pluginEntry->pluginInfo.load(pluginEntry->cfgGroup);
        d->pluginModel->setData(index, pluginEntry->pluginInfo.isPluginEnabled(), Qt::CheckStateRole);
    }

    emit changed(false);
}

void KPluginSelector::save()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        QModelIndex index = d->pluginModel->index(i, 0);
        Private::PluginEntry *pluginEntry = static_cast<Private::PluginEntry*>(index.internalPointer());
        pluginEntry->pluginInfo.setPluginEnabled(pluginEntry->checked);
        pluginEntry->pluginInfo.save(pluginEntry->cfgGroup);
        pluginEntry->cfgGroup.sync();
    }

    emit changed(false);
}

void KPluginSelector::defaults()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        QModelIndex index = d->pluginModel->index(i, 0);
        Private::PluginEntry *pluginEntry = static_cast<Private::PluginEntry*>(index.internalPointer());
        d->pluginModel->setData(index, pluginEntry->pluginInfo.isPluginEnabledByDefault(), Qt::CheckStateRole);
    }

    emit changed(true);
}

void KPluginSelector::updatePluginsState()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        QModelIndex index = d->pluginModel->index(i, 0);
        Private::PluginEntry *pluginEntry = static_cast<Private::PluginEntry*>(index.internalPointer());
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

        if (!pluginEntryList.contains(pluginEntry) && !listToAdd.contains(pluginEntry) &&
             (!pluginInfo.property("X-KDE-PluginInfo-Category").isValid() ||
              !pluginInfo.property("X-KDE-PluginInfo-Category").toString().compare(categoryKey, Qt::CaseInsensitive))) {
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
        case PluginInfoRole:
            return QVariant::fromValue(pluginEntry->pluginInfo);
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

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    int iconSize = option.rect.height() - MARGIN * 2;
    if (pluginSelector_d->showIcons) {
        KIcon icon(index.model()->data(index, Qt::DecorationRole).toString());
        painter->drawPixmap(QRect(MARGIN + option.rect.left() + xOffset, MARGIN + option.rect.top(), iconSize, iconSize), icon.pixmap(iconSize, iconSize), QRect(0, 0, iconSize, iconSize));
    } else {
        iconSize = -MARGIN;
    }

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

    if (!pluginSelector_d->showIcons) {
        i--;
    }

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(qMax(fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()),
                      option.fontMetrics.width(index.model()->data(index, CommentRole).toString())) +
                      pluginSelector_d->showIcons ? ICON_SIZE : 0 + MARGIN * i + pushButton->sizeHint().width() * j,
                 qMax(ICON_SIZE + MARGIN * 2, fmTitle.height() + option.fontMetrics.height() + MARGIN * 2));
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
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);

    setBlockedEventTypes(aboutPushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);

    setBlockedEventTypes(configurePushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);

    widgetList << enabledCheckBox << configurePushButton << aboutPushButton;

    return widgetList;
}

void KPluginSelector::Private::PluginDelegate::updateItemWidgets(const QList<QWidget*> widgets,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QPersistentModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    QCheckBox *checkBox = static_cast<QCheckBox*>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(MARGIN, option.rect.height() / 2 - checkBox->sizeHint().height() / 2);
    checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());

    KPushButton *aboutPushButton = static_cast<KPushButton*>(widgets[2]);
    QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
    aboutPushButton->resize(aboutPushButtonSizeHint);
    aboutPushButton->move(option.rect.width() - MARGIN - aboutPushButtonSizeHint.width(), option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);

    KPushButton *configurePushButton = static_cast<KPushButton*>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(option.rect.width() - MARGIN * 2 - configurePushButtonSizeHint.width() - aboutPushButtonSizeHint.width(), option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);

    configurePushButton->setVisible(index.model()->data(index, ServicesCountRole).toBool());
}

void KPluginSelector::Private::PluginDelegate::slotStateChanged(bool state)
{
    if (!focusedIndex().isValid())
        return;

    const QModelIndex index = focusedIndex();

    pluginSelector_d->dependenciesWidget->clearDependencies();

    KPluginInfo pluginInfo = index.model()->data(index, PluginInfoRole).value<KPluginInfo>();
    pluginSelector_d->updateDependencies(pluginInfo, state);

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

    const QString name = model->data(index, NameRole).toString();
    const QString comment = model->data(index, CommentRole).toString();
    const QString author = model->data(index, AuthorRole).toString();
    const QString email = model->data(index, EmailRole).toString();
    const QString website = model->data(index, WebsiteRole).toString();
    const QString version = model->data(index, VersionRole).toString();
    const QString license = model->data(index, LicenseRole).toString();

    QString message = i18n("Name:\n%1", name);

    if (!comment.isEmpty()) {
        message += i18n("\n\nComment:\n%1", comment);
    }

    if (!author.isEmpty()) {
        message += i18n("\n\nAuthor:\n%1", author);
    }

    if (!email.isEmpty()) {
        message += i18n("\n\nE-Mail:\n%1", email);
    }

    if (!website.isEmpty()) {
        message += i18n("\n\nWebsite:\n%1", website);
    }

    if (!version.isEmpty()) {
        message += i18n("\n\nVersion:\n%1", version);
    }

    if (!license.isEmpty()) {
        message += i18n("\n\nLicense:\n%1", license);
    }

    KMessageBox::information(itemView(), message, i18n("About Plugin \"%1\"", name));
}

void KPluginSelector::Private::PluginDelegate::slotConfigureClicked()
{
    const QModelIndex index = focusedIndex();
    const QAbstractItemModel *model = index.model();

    KPluginInfo pluginInfo = model->data(index, PluginInfoRole).value<KPluginInfo>();

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
