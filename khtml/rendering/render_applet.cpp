/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
 *
 * $Id$
 */
#include <config.h>
#include <klocale.h>

#include "render_applet.h"
#include "../khtmlview.h"
#include "../khtml_part.h"

#include <qlabel.h>
#include <qscrollview.h>

#include <java/kjavaappletwidget.h>
#include <misc/htmltags.h>

using namespace khtml;

RenderApplet::RenderApplet(QScrollView *view,
                           QMap<QString, QString> args, HTMLElementImpl *applet)
    : RenderWidget(view)
{
    // init RenderObject attributes
    m_inline = true;   // our object is Inline
    m_applet = applet;

    KJavaAppletContext *context = 0;
    KHTMLView *_view = dynamic_cast<KHTMLView*>(view);
    if ( _view ) {
        KHTMLPart *part = _view->part();
        context = part->createJavaContext();
    }

    if ( context ) {
        setQWidget( new KJavaAppletWidget(context, view->viewport()) );
        processArguments(args);
        ((KJavaAppletWidget*) m_widget)->create();
    }

    m_layoutPerformed = FALSE;
}

RenderApplet::~RenderApplet()
{
}

void RenderApplet::layout()
{
    if(m_layoutPerformed)
        return;

    KJavaAppletWidget *tmp = ((KJavaAppletWidget*) m_widget);
    if ( tmp ) {
        NodeImpl *child = m_applet->firstChild();

        while(child) {

            if(child->id() == ID_PARAM) {
                HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>(child);
                tmp->setParameter( p->name(), p->value());
            }
            child = child->nextSibling();
        }

        tmp->showApplet();
    }

    m_layoutPerformed = TRUE;
}

void RenderApplet::processArguments(QMap<QString, QString> args)
{
    KJavaAppletWidget *tmp = (KJavaAppletWidget*) m_widget;

    if ( tmp ) {
        tmp->setBaseURL( args[QString::fromLatin1("baseURL") ] );
        tmp->setAppletClass( args[QString::fromLatin1("code") ] );

        m_width = args[QString::fromLatin1("width") ].toInt();
        m_minWidth = m_maxWidth = m_width;
        m_height = args[QString::fromLatin1("height") ].toInt();
        tmp->resize( m_width, m_height );

        if( !args[QString::fromLatin1("codeBase") ].isEmpty() )
            tmp->setCodeBase( args[QString::fromLatin1("codeBase") ] );
        if( !args[QString::fromLatin1("name") ].isNull() )
            tmp->setAppletName( args[QString::fromLatin1("name") ] );
        else
            tmp->setAppletName( args[QString::fromLatin1("code") ] );

        if( !args[QString::fromLatin1("archive") ].isEmpty() )
            tmp->setJARFile( args[QString::fromLatin1("archive") ] );
    }
}


RenderEmptyApplet::RenderEmptyApplet(QScrollView *view, QSize sz)
  : RenderWidget( view )
{
    // init RenderObject attributes
    m_inline = true;   // our object is Inline

    setQWidget(
        new QLabel(i18n("Java Applet is not loaded. (Java interpreter disabled)"),
                   view->viewport())
        );

    ((QLabel*)m_widget)->setAlignment( Qt::AlignCenter );

    m_width = sz.width();
    m_minWidth = m_maxWidth = m_width;
    m_height = sz.height();
    m_widget->resize( sz );
}

