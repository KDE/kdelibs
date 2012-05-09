/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2009 Dawit Alemayehu <adawit@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *
 */
#ifndef KWEBPLUGINFACTORY_H
#define KWEBPLUGINFACTORY_H

#include <kdewebkit_export.h>

#include <QtWebKit/QWebPluginFactory>

namespace KParts {
    class ReadOnlyPart;
}

/**
 * @short A QWebPluginFactory with integration into the KDE environment.
 *
 * This class will attempt to find a KPart to satisfy a plugin request.
 *
 * @author Michael Howell <mhowell123@gmail.com>
 * @author Dawit Alemayehu <adawit@kde.org>
 *
 * @see QWebPluginFactory
 * @since 4.4
 */
class KDEWEBKIT_EXPORT KWebPluginFactory : public QWebPluginFactory
{
    Q_OBJECT
public:
    /**
     * Constructs a KWebPluginFactory with parent @p parent.
     */
    KWebPluginFactory(QObject *parent);

    /**
     * Destroys the KWebPage.
     */
    ~KWebPluginFactory();

    /**
     * @reimp
     *
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWebPluginFactory::create
     * @internal
     */
    virtual QObject *create(const QString &mimeType,
                            const QUrl &url,
                            const QStringList &argumentNames,
                            const QStringList &argumentValues) const;

    /**
     * @reimp
     *
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWebPluginFactory::plugins
     * @internal
     */
    virtual QList<Plugin> plugins() const;

protected:
    /**
     * Sets @p mimeType to the content type guessed from @p url.
     *
     * Note that attempting to guess mime-type will not always produce the
     * correct content-type. This is especially true for the HTTP protocol
     * since the URL present might be for a cgi script URL instead of a static
     * URL that directly points to the content.
     *
     * If @p mimeType is not NULL, this function will set it to the content
     * type determined from @p url.
     *
     * @since 4.8.3
     */
    void extractGuessedMimeType(const QUrl& url, QString* mimeType) const;

    /**
     * Returns true if the given mime-type is excluded from being used to create
     * a web plugin using KDE's trader.
     *
     * Currently this function only returns true for mimetypes 'x-java',
     * 'x-shockwave-flash', and 'futuresplash' in the 'application' category
     * and everything under the 'inode' category.
     *
     * @since 4.8.3
     */
    bool excludedMimeType(const QString& mimeType) const;

    /**
     * Returns an instance of the service associated with @p mimeType.
     *
     * This function uses KDE's trader to create an instance of the service
     * associated with the given parameters. The parameters are the <param>
     * tags of the HTML object. The name and the value attributes of these
     * tags are specified by the @p argumentNames and @p argumentValues
     * respectively.
     *
     * The @p parentWidget and @p parent parameters specify the widget to use
     * as the parent of the newly created part and the parent for the part
     * itself respectively.
     *
     * The parameters for this function mirror that of @ref QWebPluginFactory::create.
     *
     * @see QWebPluginFactory::create
     * @since 4.8.3
     */
    KParts::ReadOnlyPart* createPartInstanceFrom(const QString& mimeType,
                                                 const QStringList &argumentNames,
                                                 const QStringList &argumentValues,
                                                 QWidget* parentWidget = 0,
                                                 QObject* parent = 0) const;
private:
    class KWebPluginFactoryPrivate;
    KWebPluginFactoryPrivate* const d;
};

#endif // KWEBPLUGINFACTORY_H
