/* This file is part of the KDE project
 *  Copyright (C) 2002 Stephan Kulow <coolo@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
*/

#ifndef __khtml_Iface_h__
#define __khtml_Iface_h__

#include <dcopobject.h>
#include <dcopref.h>

class KHTMLPart;

/**
 * DCOP interface for KHTML
 */
class KHTMLPartIface : public DCOPObject
{
    K_DCOP

public:

    KHTMLPartIface( KHTMLPart * );
    virtual ~KHTMLPartIface();

k_dcop:

    bool closeURL();

    /**
     * Enable/disable Javascript support. Note that this will
     * in either case permanently override the default usersetting.
     * If you want to have the default UserSettings, don't call this
     * method.
     */
    void setJScriptEnabled( bool enable );

    /**
     * Returns @p true if Javascript support is enabled or @p false
     * otherwise.
     */
    bool jScriptEnabled() const;

    /**
     * Enable/disable the automatic forwarding by <meta http-equiv="refresh" ....>
     */
    void setMetaRefreshEnabled( bool enable );

    /**
     * Returns @p true if automtaic forwarding is enabled.
     */
    bool metaRefreshEnabled() const;

    /**
     * Enables or disables Drag'n'Drop support. A drag operation is started if
     * the users drags a link.
     */
    void setDNDEnabled( bool b );

    /**
     * Returns whether Dragn'n'Drop support is enabled or not.
     */
    bool dndEnabled() const;

    /**
     * Enables/disables Java applet support. Note that calling this function
     * will permanently override the User settings about Java applet support.
     * Not calling this function is the only way to let the default settings
     * apply.
     */
    void setJavaEnabled( bool enable );

    /**
     * Return if Java applet support is enabled/disabled.
     */
    bool javaEnabled() const;


    /**
     * Enables or disables plugins via, default is enabled
     */
    void setPluginsEnabled( bool enable );

    /**
     * Returns trie if plugins are enabled/disabled.
     */
    bool pluginsEnabled() const;

    /**
     * Specifies whether images contained in the document should be loaded
     * automatically or not.
     *
     * @note Request will be ignored if called before @ref begin().
     */
    void setAutoloadImages( bool enable );

    /**
     * Returns whether images contained in the document are loaded automatically
     * or not.
     * @note that the returned information is unrelieable as long as no begin()
     * was called.
     */
    bool autoloadImages() const;

    /**
     * Security option.
     *
     * Specify whether only local references ( stylesheets, images, scripts, subdocuments )
     * should be loaded. ( default false - everything is loaded, if the more specific
     * options allow )
     */
    void setOnlyLocalReferences(bool enable);

    /**
     * Returnd whether references should be loaded ( default false )
     **/
    bool onlyLocalReferences() const;

    /**
     * Sets the encoding the page uses.
     *
     * This can be different from the charset. The widget will try to reload
     * the current page in the new encoding, if url() is not empty.
     */
    bool setEncoding( const QString &name );

    /**
     * Returns the encoding the page currently uses.
     *
     * Note that the encoding might be different from the charset.
     */
    QString encoding() const;

    /**
     * Sets a user defined style sheet to be used on top of the HTML 4
     * default style sheet.
     *
     * This gives a wide range of possibilities to
     * change the layout of the page.
     */
    void setUserStyleSheet(const QString &styleSheet);

    /**
     * Sets the fixed font style.
     *
     * @param name The font name to use for fixed text, e.g.
     * the <tt>&lt;pre&gt;</tt> tag.
     */
    void setFixedFont( const QString &name );

    /**
     * Finds the anchor named @p name.
     *
     * If the anchor is found, the widget
     * scrolls to the closest position. Returns @p if the anchor has
     * been found.
     */
    bool gotoAnchor( const QString &name );

    /**
     * Activate the node that currently has the focus
     * (emulates pressing Return)
     */
    void activateNode();

    /**
     * Returns the text the user has marked.
     */
    QString selectedText() const;

    /**
     * Marks all text in the document as selected.
     */
    void selectAll();

    /**
     * Last-modified date (in raw string format), if received in the [HTTP] headers.
     */
    QString lastModified() const;

    void debugRenderTree();
    void viewDocumentSource();
    void viewFrameSource();
    void saveBackground(const QString &url);
    void saveDocument(const QString &url);
    QString evalJS(const QString &script);

private:
    KHTMLPart *part;
};

#endif

