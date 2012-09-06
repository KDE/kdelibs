/* This file is part of the KDE project
   Copyright (C) 2012 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KPARTS_LISTINGEXTENSION_H
#define KPARTS_LISTINGEXTENSION_H

#include <QtCore/QObject>

#include <kparts/kparts_export.h>


class KFileItemList;

namespace KParts
{

class ReadOnlyPart;

/**
 * @short an extension for filtering listings.
 *
 * This extension is intended to be implemented by parts that provide listing
 * services, e.g. file management parts and is intended to provide a generic
 * API for filtering any listing through keywords, wildcard characters and/or
 * content-type.
 *
 * Examples:
 *
 * To show items that only match the term "kde"
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext && (ext->supportedFilterModes() & KParts::ListingFilterExtension::SubString)) {
 *       ext->setFilter(KParts::ListingFilterExtension::SubString, QLatin1String("kde"));
 *    }
 * \endcode
 *
 * To show items that only match "text/html"
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext && (ext->supportedFilterModes() & KParts::ListingFilterExtension::MimeType)) {
 *        ext->setFilter(KParts::ListingFilterExtension::MimeType, QLatin1String("text/html"));
 *    }
 * \endcode
 *
 * To show items that only match the wildcard string "*.txt"
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext && (ext->supportedFilterModes() & KParts::ListingFilterExtension::WildCard)) {
 *        ext->setFilter(KParts::ListingFilterExtension::WildCard, QLatin1String("*.txt"));
 *    }
 * \endcode
 *
 * To show items that match multiple mime types, e.g. text/html & application/xml:
 *
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext &&
 *        (ext->supportedFilterModes() & KParts::ListingFilterExtension::MimeType) &&
 *        ext->supportsMultipleFilters(KParts::ListingFilterExtension::MimeType)) {
 *        QStringList mimeTypes = ext->filter(KParts::ListingFilterExtension::MimeType).toStringList();
 *        mimeTypes << QLatin1String("text/html") << QLatin1String("application/xml");
 *        ext->setFilter(KParts::ListingFilterExtension::MimeType, mimeTypes);
 *    }
 * \endcode
 *
 * @since 4.9.2
 */
class KPARTS_EXPORT ListingFilterExtension : public QObject
{
    Q_OBJECT

public:
    /**
     * Supported file filtering modes modes.
     */
    enum FilterMode  {
        None = 0x00,
        MimeType = 0x01,        /*!< Filter by mime type, e.g. "text/plain". */
        SubString = 0x02,       /*!< Filter by matching any part of a file or directory name, e.g. "Documents" */
        WildCard = 0x04         /*!< Filter by using wildcard matches, e.g. "*.txt" */
    };

    Q_DECLARE_FLAGS(FilterModes, FilterMode)

    /*! Constructor */
    ListingFilterExtension(KParts::ReadOnlyPart* parent);

    /*! Destructor */
    virtual ~ListingFilterExtension();

    /**
     * Queries @p obj for a child object which inherits from this class.
     */
    static ListingFilterExtension *childObject(QObject* obj);

    /**
     * Returns the OR'ed value of the file filter modes supported by the part
     * that implements this extension.
     *
     * By default this function returns None.
     */
    virtual FilterModes supportedFilterModes() const;

    /**
     * Returns true if the part that implements this extension allows
     * the use of multiple filters for the given filtering @p mode.
     *
     * By default this function returns false.
     */
    virtual bool supportsMultipleFilters(FilterMode mode) const;

    /**
     * Returns the currently set filters for the given @p mode.
     *
     * @param mode the desired filter mode as specified in @ref FilterMode.
     */
    virtual QVariant filter(FilterMode mode) const = 0;

    /**
     * Sets the file @p filter that should be applied by the part that
     * implements this extension for the given filtering @p mode.
     *
     * To remove a filter for a given filter mode, simply call this function with
     * the desired mode and the @p filter parameter set to a NULL variant.
     *
     * The second parameter can be
     *
     * @param mode the desired filter mode as specified in @ref FilterMode.
     * @param filters a list of filter texts based on the selected mode.
     */
    virtual void setFilter(FilterMode mode, const QVariant& filter) = 0;

private:
    class ListingFilterExtensionPrivate;
    ListingFilterExtension* const d;
};

/**
 * @short an extension for receiving listing change notification.
 *
 * This extension is intended for implementation by parts that provide listing
 * services, e.g. file management and is intended to notify about changes to
 * a given listing. For example, if file management part implemented this extension
 * it would emit @ref itemsDeleted and @ref itemsAdded signal whenever new files
 * or folders are deleted and added to a directory respectively.
 *
 * @since 4.9.2
 */
class KPARTS_EXPORT ListingNotificationExtension : public QObject
{
    Q_OBJECT

public:
    /**
     * Supported notification event types.
     */
    enum NotificationEventType {
        None = 0x00,
        ItemsAdded = 0x01,      /*!< New items added to the listing. */
        ItemsDeleted = 0x02     /*!< Items deleted from the listing. */
    };

    Q_DECLARE_FLAGS(NotificationEventTypes, NotificationEventType)

    /*! Constructor */
    ListingNotificationExtension(KParts::ReadOnlyPart* parent);

    /*! Destructor */
    virtual ~ListingNotificationExtension();

    /**
     * Returns the OR'ed value of the notification types supported by the part
     * that implements this extension.
     *
     * By default this function returns None.
     */
    virtual NotificationEventTypes supportedNotificationEventTypes() const;

    /**
      * Queries @p obj for a child object which inherits from this class.
      */
    static ListingNotificationExtension *childObject(QObject* obj);

Q_SIGNALS:
    /**
      * This signal is emitted when one of the notification events listed
      * in @ref NotificationEventType occur.
      */
    void listingEvent(KParts::ListingNotificationExtension::NotificationEventType, const KFileItemList&);

private:
    class ListingNotificationExtensionPrivate;
    ListingNotificationExtension* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KParts::ListingFilterExtension::FilterModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(KParts::ListingNotificationExtension::NotificationEventTypes)

#endif /* KPARTS_LISTINGEXTENSION_H */
