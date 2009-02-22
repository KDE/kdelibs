/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include "kfileplacesitem_p.h"
#include "kfileplacesmodel.h"

#include <QtCore/QDateTime>

#include <kbookmarkmanager.h>
#include <kiconloader.h>
#include <kdirlister.h>
#include <solid/storageaccess.h>
#include <solid/storagevolume.h>


KFilePlacesItem::KFilePlacesItem(KBookmarkManager *manager,
                                 const QString &address,
                                 const QString &udi)
    : m_manager(manager), m_lister(0), m_folderIsEmpty(true)
{
    m_bookmark = m_manager->findByAddress(address);

    if (udi.isEmpty() && m_bookmark.metaDataItem("ID").isEmpty()) {
        m_bookmark.setMetaDataItem("ID", generateNewId());
    } else if (udi.isEmpty()) {
        if (hasFullIcon(m_bookmark)) {
            // TODO if this is only for the trash, it would be much faster to just read trashrc
            m_lister = new KDirLister(this);
            m_lister->setAutoErrorHandlingEnabled(false, 0); // don't bother the user if trash:/ doesn't exist
            m_lister->setDelayedMimeTypes(true); // we don't need the mimetypes, so don't penalize other KDirLister users
            connect(m_lister, SIGNAL(completed()),
                    this, SLOT(onListerCompleted()));
            m_lister->openUrl(m_bookmark.url());
        }
    } else if (!udi.isEmpty()) {
        Solid::Device dev(udi);
        Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();
        if (access!=0) {
            connect(access, SIGNAL(accessibilityChanged(bool, const QString &)),
                    this, SLOT(onAccessibilityChanged()));
        }
    }
}

KFilePlacesItem::~KFilePlacesItem()
{
}

QString KFilePlacesItem::id() const
{
    if (isDevice()) {
        return bookmark().metaDataItem("UDI");
    } else {
        return bookmark().metaDataItem("ID");
    }
}

bool KFilePlacesItem::isDevice() const
{
    return !bookmark().metaDataItem("UDI").isEmpty();
}

KBookmark KFilePlacesItem::bookmark() const
{
    return m_bookmark;
}

void KFilePlacesItem::setBookmark(const KBookmark &bookmark)
{
    m_bookmark = bookmark;
}

Solid::Device KFilePlacesItem::device() const
{
    return Solid::Device(bookmark().metaDataItem("UDI"));
}

QVariant KFilePlacesItem::data(int role) const
{
    QVariant returnData;

    if (role!=KFilePlacesModel::HiddenRole && role!=Qt::BackgroundRole && isDevice()) {
        returnData = deviceData(role);
    } else {
        returnData = bookmarkData(role);
    }

    return returnData;
}

QVariant KFilePlacesItem::bookmarkData(int role) const
{
    KBookmark b = bookmark();

    if (b.isNull()) return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return b.text();
    case Qt::DecorationRole:
        return KIcon(iconNameForBookmark(b));
    case Qt::BackgroundRole:
        if (b.metaDataItem("IsHidden")=="true") {
            return Qt::lightGray;
        } else {
            return QVariant();
        }
    case KFilePlacesModel::UrlRole:
        return QUrl(b.url());
    case KFilePlacesModel::SetupNeededRole:
        return false;
    case KFilePlacesModel::HiddenRole:
        return b.metaDataItem("IsHidden")=="true";
    default:
        return QVariant();
    }
}

QVariant KFilePlacesItem::deviceData(int role) const
{
    Solid::Device d = device();

    if (d.isValid()) {
        const Solid::StorageAccess *access = d.as<Solid::StorageAccess>();
        const Solid::StorageVolume *volume = d.as<Solid::StorageVolume>();
        QStringList overlays;

        switch (role)
        {
        case Qt::DisplayRole:
            return d.product();
        case Qt::DecorationRole:
            if (access && access->isAccessible()) {
                overlays << "emblem-mounted";
            } else {
                overlays << QString(); // We have to guarantee the placement of the next emblem
            }
            if (volume && volume->usage()==Solid::StorageVolume::Encrypted) {
                overlays << "security-high";
            }
            return KIcon(d.icon(), 0, overlays);
        case KFilePlacesModel::UrlRole:
            if (access) {
                return QUrl(KUrl(access->filePath()));
            } else {
                return QVariant();
            }
        case KFilePlacesModel::SetupNeededRole:
            if (access) {
                return !access->isAccessible();
            } else {
                return QVariant();
            }
        default:
            return QVariant();
        }
    } else {
        return QVariant();
    }
}

KBookmark KFilePlacesItem::createBookmark(KBookmarkManager *manager,
                                          const QString &label,
                                          const KUrl &url,
                                          const QString &iconName)
{
    KBookmarkGroup root = manager->root();
    if (root.isNull())
        return KBookmark();
    QString empty_icon = iconName;
    if (url==KUrl("trash:/")) {
        if (empty_icon.endsWith("-full")) {
            empty_icon.chop(5);
        } else if (empty_icon.isEmpty()) {
            empty_icon = "user-trash";
        }
    }
    KBookmark bookmark = root.addBookmark(label, url, empty_icon);
    bookmark.setMetaDataItem("ID", generateNewId());

    return bookmark;
}

KBookmark KFilePlacesItem::createSystemBookmark(KBookmarkManager *manager,
                                          const QString &label,
                                          const KUrl &url,
                                          const QString &iconName)
{
    KBookmark bookmark = createBookmark(manager, label, url, iconName);
    if (!bookmark.isNull())
        bookmark.setMetaDataItem("isSystemItem", "true");
    return bookmark;
}


KBookmark KFilePlacesItem::createDeviceBookmark(KBookmarkManager *manager,
                                                const QString &udi)
{
    KBookmarkGroup root = manager->root();
    if (root.isNull())
        return KBookmark();
    KBookmark bookmark = root.createNewSeparator();
    bookmark.setMetaDataItem("UDI", udi);
    bookmark.setMetaDataItem("isSystemItem", "true");    
    return bookmark;
}

QString KFilePlacesItem::generateNewId()
{
    static int count = 0;

//    return QString::number(count++);
    
    return QString::number(QDateTime::currentDateTime().toTime_t())
      + '/' + QString::number(count++);
    

//    return QString::number(QDateTime::currentDateTime().toTime_t())
//         + '/' + QString::number(qrand());
}

void KFilePlacesItem::onAccessibilityChanged()
{
    emit itemChanged(id());
}

bool KFilePlacesItem::hasFullIcon(const KBookmark &bookmark) const
{
    return bookmark.url()==KUrl("trash:/");
}

QString KFilePlacesItem::iconNameForBookmark(const KBookmark &bookmark) const
{
    if (!m_folderIsEmpty && hasFullIcon(bookmark)) {
        return bookmark.icon()+"-full";
    } else {
        return bookmark.icon();
    }
}

void KFilePlacesItem::onListerCompleted()
{
    m_folderIsEmpty = m_lister->items().isEmpty();
    emit itemChanged(id());
}

#include "kfileplacesitem_p.moc"
