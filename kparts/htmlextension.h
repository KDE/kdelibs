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

#include <QtCore/QSharedDataPointer>
#include <QtCore/QObject>

#include <kparts/kparts_export.h>

class QUrl;

namespace KParts
{

class ReadOnlyPart;
class HtmlExtensionPrivate;
class SelectorInterfacePrivate;

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

    /**
     * Returns the current base url of the part that implements this extension.
     *
     * This function is mostly used to resolve any relative URLs that might be
     * returned when querying the part for links.
     */
    virtual QUrl baseUrl() const = 0;

    /**
     * Returns true if portions of the content in the part that implements
     * this extension are selected.
     *
     * By default this function returns false.
     */
    virtual bool hasSelection() const;

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
class KPARTS_EXPORT SelectorInterface
{
public:
    class ElementPrivate;
    class Element;

    /**
     * Query methods.
     */
    enum QueryMethod {
        None = 0x00,                   /*!< Quering is not possible. */
        EntireContent = 0x01,          /*!< Query or can query the entire content. */
        SelectedContent = 0x02         /*!< Query or can query only the user selected content, if any. */
    };
    Q_DECLARE_FLAGS(QueryMethods, QueryMethod)

    /**
     * Destructor
     */
    virtual ~SelectorInterface() {}

    /**
     * Returns the supported query methods.
     * 
     * By default this function returns None.
     * 
     * @see QueryMethod
     */
    virtual QueryMethods supportedQueryMethods() const;

    /**
     * Returns the first (in document order) element in this fragment matching
     * the given CSS selector @p query and querying method @p method.
     * 
     * Note that since the returned item is static snapshot, i.e. not live, it
     * will not be updated when the document changes.
     * 
     * If the quering method specified by @p method is not supported or cannot be
     * handled, then a null element is returned.
     * 
     * @see supportedQueryMethods
     * @see QueryMethod
     */
    virtual Element querySelector(const QString& query, QueryMethod method) const = 0;

    /**
     * Returns all (in document order) elements in this fragment matching the
     * given CSS selector @p query and querying method @p method.
     * 
     * Note that since the returned list is static snapshot, i.e. not live, it 
     * will not be updated when the document changes.
     * 
     * If the quering method specified by @p method is not supported or cannot be
     * handled, then an empty list is returned.
     * 
     * @see supportedQueryMethods
     * @see QueryMethod
     */
    virtual QList<Element> querySelectorAll(const QString& query, QueryMethod method) const = 0;

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
         * Returns true if the element is null ; otherwise returns false.
         */
        bool isNull() const;

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

        /**
         * Returns true if the attribute with the given @p name exists.
         */
        bool hasAttribute(const QString& name) const;

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

/**
 * @short An interface for modifying the settings of browser engines.
 *
 *  This interface provides a generic means for querying or changing the
 *  settings of browser engines that implement it.
 *
 *  To use this class simply cast an instance of the HTMLExtension object
 *  using qobject_cast<KParts::HtmlSettingsInterface>.
 *
 *  Example:
 *  <code>
 *  KParts::HTMLExtension* extension = KParts::HTMLExtension::childObject(part);
 *  KParts::HtmlSettingsInterface* settings = qobject_cast&lt;KParts::HtmlSettingsInterface&gt;(extension);
 *  const bool autoLoadImages = settings->attribute(KParts::AutoLoadImages);
 *  </code>
 *
 *  @since 4.8.1
 */
class KPARTS_EXPORT HtmlSettingsInterface
{
public:
    /**
     * Settings attribute types.
     */
    enum HtmlSettingsType {
        AutoLoadImages,
        DnsPrefetchEnabled,
        JavaEnabled,
        JavascriptEnabled,
        MetaRefreshEnabled,
        PluginsEnabled,
        PrivateBrowsingEnabled,
        OfflineStorageDatabaseEnabled,
        OfflineWebApplicationCacheEnabled,
        LocalStorageEnabled,
        UserDefinedStyleSheetURL
    };

    /**
     * This enum specifies whether Java/JavaScript execution is allowed.
     *
     * @since 4.8.2
     */
    enum JavaScriptAdvice {
        JavaScriptDunno=0,
        JavaScriptAccept,
        JavaScriptReject
    };

    /**
     * This enum specifies the policy for window.open
     *
     * @since 4.8.2
     */
    enum JSWindowOpenPolicy {
        JSWindowOpenAllow=0,
        JSWindowOpenAsk,
        JSWindowOpenDeny,
        JSWindowOpenSmart
    };

    /**
     * This enum specifies the policy for window.status and .defaultStatus
     *
     * @since 4.8.2
     */
    enum JSWindowStatusPolicy {
        JSWindowStatusAllow=0,
        JSWindowStatusIgnore
    };

    /**
     * This enum specifies the policy for window.moveBy and .moveTo
     *
     * @since 4.8.2
     */
    enum JSWindowMovePolicy {
        JSWindowMoveAllow=0,
        JSWindowMoveIgnore
    };

    /**
     * This enum specifies the policy for window.resizeBy and .resizeTo
     *
     * @since 4.8.2
     */
    enum JSWindowResizePolicy {
        JSWindowResizeAllow=0,
        JSWindowResizeIgnore
    };

    /**
     * This enum specifies the policy for window.focus
     *
     * @since 4.8.2
     */
    enum JSWindowFocusPolicy {
        JSWindowFocusAllow=0,
        JSWindowFocusIgnore
    };

    /**
     * Destructor
     */
    virtual ~HtmlSettingsInterface() {}

    /**
     * Returns the value of the browser engine's attribute @p type.
     */
    virtual QVariant htmlSettingsProperty(HtmlSettingsType type) const = 0;

    /**
     * Sets the value of the browser engine's attribute @p type to @p value.
     */
    virtual bool setHtmlSettingsProperty(HtmlSettingsType type, const QVariant& value) = 0;

   /**
    * A convenience function that returns the javascript advice for @p text.
    *
    * If text is not either "accept" or "reject", this function returns
    * @ref JavaScriptDunno.
    *
    *  @since 4.8.2
    */
    static JavaScriptAdvice textToJavascriptAdvice(const QString& text);

   /**
    * A convenience function Returns the text for the given JavascriptAdvice @p advice.
    *
    * If @p advice is not either JavaScriptAccept or JavaScriptReject, this
    * function returns a NULL string.
    *
    *  @since 4.8.2
    */
    static const char* javascriptAdviceToText(JavaScriptAdvice advice);

   /**
    * A convenience function that splits @p text into @p domain, @p javaAdvice
    * and @p jScriptAdvice.
    *
    * If @p text is empty or does not contain the proper delimiter (':'), this
    * function will set @p domain to @p text and the other two parameters to
    * JavaScriptDunno.
    *
    *  @since 4.8.2
    */
    static void splitDomainAdvice(const QString& text,
                                  QString& domain,
                                  JavaScriptAdvice& javaAdvice,
                                  JavaScriptAdvice& javaScriptAdvice);
};

} // namespace KParts

inline void qSwap( KParts::SelectorInterface::Element & lhs, KParts::SelectorInterface::Element & rhs )
{
    lhs.swap( rhs );
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KParts::SelectorInterface::QueryMethods)

Q_DECLARE_TYPEINFO(KParts::SelectorInterface::Element, Q_MOVABLE_TYPE);

Q_DECLARE_INTERFACE(KParts::SelectorInterface,
                    "org.kde.KParts.SelectorInterface")
Q_DECLARE_INTERFACE(KParts::HtmlSettingsInterface,
                    "org.kde.KParts.HtmlSettingsInterface")


#endif /* KPARTS_HTMLEXTENSION_H */
