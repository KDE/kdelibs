/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kxmlguibuilder.h"
#include "kmenubar.h"
#include "kpopupmenu.h"
#include "ktoolbar.h"
#include "kstatusbar.h"
#include "kmainwindow.h"
#include "kaction.h"
#include "kglobalsettings.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qobjectlist.h>

class KXMLGUIBuilderPrivate
{
public:
    KXMLGUIBuilderPrivate() {
    }
  ~KXMLGUIBuilderPrivate() {
  }

    QWidget *m_widget;

    QString tagMainWindow;
    QString tagMenuBar;
    QString tagMenu;
    QString tagToolBar;
    QString tagStatusBar;

    QString tagSeparator;
    QString tagTearOffHandle;
    QString tagMenuTitle;

    QString attrName;
    QString attrLineSeparator;

    QString attrText1;
    QString attrText2;

    QString attrIcon;

    QString attrFullWidth;
    QString attrPosition;
    QString attrIndex;
    QString attrOffset;
    QString attrNewLine;
    QString attrIconText;
    QString attrIconSize;

    KInstance *m_instance;
    KXMLGUIClient *m_client;
};

KXMLGUIBuilder::KXMLGUIBuilder( QWidget *widget )
{
  d = new KXMLGUIBuilderPrivate;
  d->m_widget = widget;

  d->tagMainWindow = QString::fromLatin1( "mainwindow" );
  d->tagMenuBar = QString::fromLatin1( "menubar" );
  d->tagMenu = QString::fromLatin1( "menu" );
  d->tagToolBar = QString::fromLatin1( "toolbar" );
  d->tagStatusBar = QString::fromLatin1( "statusbar" );

  d->tagSeparator = QString::fromLatin1( "separator" );
  d->tagTearOffHandle = QString::fromLatin1( "tearoffhandle" );
  d->tagMenuTitle = QString::fromLatin1( "title" );

  d->attrName = QString::fromLatin1( "name" );
  d->attrLineSeparator = QString::fromLatin1( "lineseparator" );

  d->attrText1 = QString::fromLatin1( "text" );
  d->attrText2 = QString::fromLatin1( "Text" );

  d->attrIcon = QString::fromLatin1( "icon" );
  d->attrFullWidth = QString::fromLatin1( "fullWidth" );
  d->attrPosition = QString::fromLatin1( "position" );
  d->attrIconText = QString::fromLatin1( "iconText" );
  d->attrIconSize = QString::fromLatin1( "iconSize" );
  d->attrIndex = QString::fromLatin1( "index" );
  d->attrOffset = QString::fromLatin1( "offset" );
  d->attrNewLine = QString::fromLatin1( "newline" );

  d->m_instance = 0;
  d->m_client = 0;
}

KXMLGUIBuilder::~KXMLGUIBuilder()
{
  delete d;
}

QWidget *KXMLGUIBuilder::widget()
{
  return d->m_widget;
}

QStringList KXMLGUIBuilder::containerTags() const
{
  QStringList res;
  res << d->tagMenu << d->tagToolBar << d->tagMainWindow << d->tagMenuBar << d->tagStatusBar;

  return res;
}

QWidget *KXMLGUIBuilder::createContainer( QWidget *parent, int index, const QDomElement &element, int &id )
{
  id = -1;
  if ( element.tagName().lower() == d->tagMainWindow )
  {
    KMainWindow *mainwindow = 0;
    if ( d->m_widget->inherits( "KMainWindow" ) )
      mainwindow = static_cast<KMainWindow *>(d->m_widget);

    return mainwindow;
  }

  if ( element.tagName().lower() == d->tagMenuBar )
  {
    KMenuBar *bar;

    if ( d->m_widget->inherits( "KMainWindow" ) )
      bar = static_cast<KMainWindow *>(d->m_widget)->menuBar();
    else
      bar = new KMenuBar( d->m_widget );

    bar->show();
    return bar;
  }

  if ( element.tagName().lower() == d->tagMenu )
  {
    // Look up to see if we are inside a mainwindow. If yes, then
    // use it as parent widget (to get kaction to plug itself into the
    // mainwindow). Don't use a popupmenu as parent widget, otherwise
    // the popup won't be hidden if it is used as a standalone menu as well.
    // And we don't want to set the parent for a standalone popupmenu,
    // otherwise its shortcuts appear.
    QWidget* p = parent;
    while ( p && !p->inherits("KMainWindow") )
        p = p->parentWidget();

    KPopupMenu *popup = new KPopupMenu( p, element.attribute( d->attrName ).utf8());

    QString i18nText;
    QCString text = element.namedItem( d->attrText1 ).toElement().text().utf8();
    if ( text.isEmpty() ) // try with capital T
      text = element.namedItem( d->attrText2 ).toElement().text().utf8();

    if ( text.isEmpty() ) // still no luck
      i18nText = i18n( "No text!" );
    else
      i18nText = i18n( text );

    QString icon = element.attribute( d->attrIcon );
    QIconSet pix;

    if ( !icon.isEmpty() )
    {
      KInstance *instance = d->m_instance;
      if ( !instance )
        instance = KGlobal::instance();

      pix = SmallIconSet( icon, 16, instance );
    }

    if ( parent && parent->inherits( "KMenuBar" ) )
    {
      if ( !icon.isEmpty() )
        id = static_cast<KMenuBar *>(parent)->insertItem( pix, i18nText, popup, -1, index );
      else
        id = static_cast<KMenuBar *>(parent)->insertItem( i18nText, popup, -1, index );
    }
    else if ( parent && parent->inherits( "QPopupMenu" ) )
    {
      if ( !icon.isEmpty() )
        id = static_cast<QPopupMenu *>(parent)->insertItem( pix, i18nText, popup, -1, index );
      else
        id = static_cast<QPopupMenu *>(parent)->insertItem( i18nText, popup, -1, index );
    }

    return popup;
  }

  if ( element.tagName().lower() == d->tagToolBar )
  {
    bool honor = (element.attribute( d->attrName ) == "mainToolBar");

    QCString name = element.attribute( d->attrName ).utf8();

    KToolBar *bar = static_cast<KToolBar*>(d->m_widget->child( name, "KToolBar" ));
    if( !bar )
    {
       bar = new KToolBar( d->m_widget, name, honor, false );
    }

    if ( d->m_widget->inherits( "KMainWindow" ) )
    {
        if ( d->m_client && !d->m_client->xmlFile().isEmpty() )
            bar->setXMLGUIClient( d->m_client );
    }

    bar->loadState( element );

    return bar;
  }

  if ( element.tagName().lower() == d->tagStatusBar )
  {
    if ( d->m_widget->inherits( "KMainWindow" ) )
    {
      KMainWindow *mainWin = static_cast<KMainWindow *>(d->m_widget);
      mainWin->statusBar()->show();
      return mainWin->statusBar();
    }
    KStatusBar *bar = new KStatusBar( d->m_widget );
    return bar;
  }

  return 0L;
}

void KXMLGUIBuilder::removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id )
{
  // Warning parent can be 0L

  if ( container->inherits( "QPopupMenu" ) )
  {
    if ( parent )
    {
        if ( parent->inherits( "KMenuBar" ) )
            static_cast<KMenuBar *>(parent)->removeItem( id );
        else if ( parent->inherits( "QPopupMenu" ) )
            static_cast<QPopupMenu *>(parent)->removeItem( id );
    }

    delete container;
  }
  else if ( container->inherits( "KToolBar" ) )
  {
    KToolBar *tb = static_cast<KToolBar *>( container );

    tb->saveState( element );
    delete tb;
  }
  else if ( container->inherits( "KMenuBar" ) )
  {
    KMenuBar *mb = static_cast<KMenuBar *>( container );
    mb->hide();
    // Don't delete menubar - it can be reused by createContainer.
    // If you decide that you do need to delete the menubar, make
    // sure that QMainWindow::d->mb does not point to a deleted
    // menubar object.
  }
  else if ( container->inherits( "KStatusBar" ) )
  {
    if ( d->m_widget->inherits( "KMainWindow" ) )
        container->hide();
    else
      delete static_cast<KStatusBar *>(container);
  }
  else
     kdWarning() << "Unhandled container to remove : " << container->className() << endl;
}

QStringList KXMLGUIBuilder::customTags() const
{
  QStringList res;
  res << d->tagSeparator << d->tagTearOffHandle << d->tagMenuTitle;
  return res;
}

int KXMLGUIBuilder::createCustomElement( QWidget *parent, int index, const QDomElement &element )
{
  if ( element.tagName().lower() == d->tagSeparator )
  {
    if ( parent->inherits( "QPopupMenu" ) )
      return static_cast<QPopupMenu *>(parent)->insertSeparator( index );
    else if ( parent->inherits( "QMenuBar" ) )
       return static_cast<QMenuBar *>(parent)->insertSeparator( index );
    else if ( parent->inherits( "KToolBar" ) )
    {
      KToolBar *bar = static_cast<KToolBar *>( parent );

      bool isLineSep = false;

      QDomNamedNodeMap attributes = element.attributes();
      unsigned int i = 0;
      for (; i < attributes.length(); i++ )
      {
        QDomAttr attr = attributes.item( i ).toAttr();

        if ( attr.name().lower() == d->attrLineSeparator &&
             attr.value().lower() == QString::fromLatin1("true") )
        {
          isLineSep = true;
          break;
        }
      }

      int id = KAction::getToolButtonID();

      if ( isLineSep )
          bar->insertLineSeparator( index, id );
      else
          bar->insertSeparator( index, id );

      return id;
    }
  }
  else if ( element.tagName().lower() == d->tagTearOffHandle )
  {
    if ( parent->inherits( "QPopupMenu" )  && KGlobalSettings::insertTearOffHandle())
      return static_cast<QPopupMenu *>(parent)->insertTearOffHandle( -1, index );
  }
  else if ( element.tagName().lower() == d->tagMenuTitle )
  {
    if ( parent->inherits( "KPopupMenu" ) )
    {
      QString i18nText;
      QCString text = element.text().utf8();

      if ( text.isEmpty() )
        i18nText = i18n( "No text!" );
      else
        i18nText = i18n( text );

      QString icon = element.attribute( d->attrIcon );
      QPixmap pix;

      if ( !icon.isEmpty() )
      {
        KInstance *instance = d->m_instance;
        if ( !instance )
          instance = KGlobal::instance();

        pix = SmallIcon( icon, instance );
      }

      if ( !icon.isEmpty() )
        return static_cast<KPopupMenu *>(parent)->insertTitle( pix, i18nText, -1, index );
      else
        return static_cast<KPopupMenu *>(parent)->insertTitle( i18nText, -1, index );
    }
  }
  return 0;
}

void KXMLGUIBuilder::removeCustomElement( QWidget *parent, int id )
{
  if ( parent->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu *>(parent)->removeItem( id );
  else if ( parent->inherits( "QMenuBar" ) )
    static_cast<QMenuBar *>(parent)->removeItem( id );
  else if ( parent->inherits( "KToolBar" ) )
    static_cast<KToolBar *>(parent)->removeItem( id );
}

KXMLGUIClient *KXMLGUIBuilder::builderClient() const
{
  return d->m_client;
}

void KXMLGUIBuilder::setBuilderClient( KXMLGUIClient *client )
{
  d->m_client = client;
  if ( client )
      setBuilderInstance( client->instance() );
}

KInstance *KXMLGUIBuilder::builderInstance() const
{
  return d->m_instance;
}

void KXMLGUIBuilder::setBuilderInstance( KInstance *instance )
{
  d->m_instance = instance;
}

void KXMLGUIBuilder::finalizeGUI( KXMLGUIClient * )
{
    if ( !d->m_widget || !d->m_widget->inherits( "KMainWindow" ) )
        return;
#if 0
    KToolBar *toolbar = 0;
    QListIterator<KToolBar> it( ( (KMainWindow*)d->m_widget )->toolBarIterator() );
    while ( ( toolbar = it.current() ) ) {
        kdDebug() << "KXMLGUIBuilder::finalizeGUI toolbar=" << (void*)toolbar << endl;
        ++it;
        toolbar->positionYourself();
    }
#else
    static_cast<KMainWindow *>(d->m_widget)->finalizeGUI( false );
#endif
}

void KXMLGUIBuilder::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

