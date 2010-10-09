/* This file is part of the KDE project
   Copyright (C) 2010 David Faure <faure@kde.org>

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

#ifndef KPARTS_HTMLEXTENSION_H
#define KPARTS_HTMLEXTENSION_H

#include <QSharedDataPointer>
#include <QtCore/QObject>
#include <kurl.h>
#include <kparts/kparts_export.h>

namespace KParts
{

class ReadOnlyPart;
class HtmlExtensionPrivate;

/**
 * @short an extension for KParts to provide HTML-related features
 *
 * Use qobject_cast to cast the extension to interesting interfaces, like
 * qobject_cast<KParts::SelectorInterface>.
 *
 * @since 4.6
 */
class KPARTS_EXPORT HtmlExtension : public QObject
{
    Q_OBJECT
public:
    HtmlExtension(KParts::ReadOnlyPart* parent);
    ~HtmlExtension();

    /**
     * Queries @p obj for a child object which inherits from this
     * HtmlExtension class.
     */
    static HtmlExtension *childObject( QObject *obj );

    virtual KUrl baseUrl() const = 0;

private:
    // for future extensions
    HtmlExtensionPrivate* const d;
};

/**
 * Optional base class for HtmlExtension-derived classes
 * Provides DOM Selector like API: querySelector and querySelectorAll,
 * in order to find specific elements in an HTML document.
 *
 * Example:
 * <code>
 * const QList<SelectorInterface::Element> elements = selectorInterface->querySelectorAll("head > link[rel=\"alternate\"]");
 * </code>
 */
class SelectorInterface
{
public:
    class ElementPrivate;
    class Element;

    virtual ~SelectorInterface() {}
    /**
     * Returns the first (in document order) element in this fragment
     * matching the given CSS selector @p query.
     */
    virtual Element querySelector(const QString& query) const = 0;
    /**
     * Returns all (in document order) elements in this fragment matching the
     * given CSS selector @p query. Note that the returned list is
     * static and not live, and will not be updated when the document
     * changes
     */
    virtual QList<Element> querySelectorAll(const QString& query) const = 0;

    class KPARTS_EXPORT Element {
    public:
        /**
         * Constructor
         */
        Element();
        /**
         * Copy constructor
         */
        Element(const Element& other);
        /**
         * Destructor
         */
        ~Element();

        /**
         * Sets the tag name of this element.
         */
        void setTagName(const QString& tag);
        /**
         * Returns the tag name of this element.
         */
        QString tagName() const;
        /**
         * Adds an attribute with the given name and value.
         * If an attribute with the same name exists, its value is replaced by value.
         */
        void setAttribute(const QString& name, const QString& value);

        /**
         * Returns the list of attributes in this element.
         */
        QStringList attributeNames() const;

        /**
         * Returns the attribute with the given name. If the attribute does not exist, defaultValue is returned.
         */
        QString attribute(const QString& name, const QString& defaultValue = QString()) const;

        // No namespace support yet, could be added with attributeNS, setAttributeNS

        /**
         * Swaps the contents of @p other with the contents of this.
         */
        void swap( Element& other ) {
            d.swap( other.d );
        }

        /**
         * Assignment operator
         */
        Element& operator=(const Element& other) {
            if ( this != &other ) {
                Element copy( other );
                swap( copy );
            }
            return *this;
        }

    private:
        QSharedDataPointer<ElementPrivate> d;
    };

};

} // namespace KParts

inline void qSwap( KParts::SelectorInterface::Element & lhs, KParts::SelectorInterface::Element & rhs )
{
    lhs.swap( rhs );
}

Q_DECLARE_TYPEINFO(KParts::SelectorInterface::Element, Q_MOVABLE_TYPE);

Q_DECLARE_INTERFACE(KParts::SelectorInterface,
                    "org.kde.KParts.SelectorInterface")

#endif /* KPARTS_HTMLEXTENSION_H */
