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
#include <klibloader.h>
#include <klocale.h>

#include "render_applet.h"

#include <qlabel.h>
#include <qscrollview.h>

#include <java/kjavaappletwidget.h>
#include <misc/htmltags.h>

/**
 * We use single applet context to run all applets in all pages.
 * If all appelts are deleted we delete the context too. It has its
 * drawback: deleting context stops Java process and next applet have to
 * restart it again.
 * TODO: Implement contex manager which will create different context
 * for different URLs.
 */
static KJavaAppletContext *context = 0;
static int context_counter = 0;

using namespace khtml;

RenderApplet::RenderApplet(QScrollView *view,
                           QMap<QString, QString> args, HTMLElementImpl *applet)
    : RenderWidget(view)
{
    if( context == 0 ) {
        context = new KJavaAppletContext();
        context_counter = 0;
    }

    m_applet = applet;
    setQWidget(new KJavaAppletWidget(context, view->viewport()));
    context_counter++;

    processArguments(args);

    ((KJavaAppletWidget*) m_widget)->create();

    m_layoutPerformed = FALSE;
}

RenderApplet::~RenderApplet()
{
  context_counter--;
  if( context_counter == 0 )
  {
    delete context;
    context = 0;
  }
}

void RenderApplet::layout()
{
    if(m_layoutPerformed)
        return;

    KJavaAppletWidget *tmp = ((KJavaAppletWidget*) m_widget);

    NodeImpl *child = m_applet->firstChild();

    while(child)
    {
        if(child->id() == ID_PARAM)
        {
            HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>(child);
            tmp->setParameter( p->name(), p->value());
        }
        child = child->nextSibling();
    }

    tmp->show();

    m_layoutPerformed = TRUE;
}

void RenderApplet::processArguments(QMap<QString, QString> args)
{
    KJavaAppletWidget *tmp = (KJavaAppletWidget*) m_widget;

    tmp->setBaseURL( args[QString::fromLatin1("baseURL") ] );
    tmp->setAppletClass( args[QString::fromLatin1("code") ] );

    m_width = args[QString::fromLatin1("width") ].toInt();
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


RenderEmptyApplet::RenderEmptyApplet(QScrollView *view, QSize sz)
  : RenderWidget( view )
{
  setQWidget(
    new QLabel(i18n("Java Applet is not loaded. (Java interpreter disabled)"),
               view->viewport())
	);

  ((QLabel*)m_widget)->setAlignment( Qt::AlignCenter );

  m_width = sz.width();
  m_height = sz.height();
  m_widget->resize( sz );
}

