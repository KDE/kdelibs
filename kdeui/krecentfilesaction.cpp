/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "krecentfilesaction.h"

#include <QtCore/QFile>

#include <kconfig.h>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "kmenu.h"

class KRecentFilesActionPrivate
{
public:
  KRecentFilesActionPrivate()
  {
    m_maxItems = 10;
  }

  int m_maxItems;
  QMap<QAction*, QString> m_shortNames;
  QMap<QAction*, KUrl> m_urls;
};


KRecentFilesAction::KRecentFilesAction( KActionCollection * parent, const QString& name )
  : KSelectAction(parent, name)
  , d(new KRecentFilesActionPrivate)
{
  init();
}

KRecentFilesAction::KRecentFilesAction( const QString & text, KActionCollection * parent, const QString& name )
  : KSelectAction(text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const KIcon & icon, const QString & text, KActionCollection * parent, const QString& name )
  : KSelectAction(icon, text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString & icon, const QString & text, KActionCollection * parent, const QString& name )
  : KSelectAction(KIcon(icon), text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const KShortcut& cut,
                                        KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction( text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  setShortcut( cut );

  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction( text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  setShortcut( cut );

  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KUrl&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIcon& pix,
                                        const KShortcut& cut,
                                        KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction(KIcon(pix), text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  setShortcut( cut );

  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix,
                                        const KShortcut& cut,
                                        KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction(KIcon(pix), text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  setShortcut( cut );

  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIcon& pix,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction(KIcon(pix), text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  setShortcut( cut );

  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KUrl&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction( KIcon(pix), text, parent, name)
  , d(new KRecentFilesActionPrivate)
{
  setShortcut( cut );

  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KUrl&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( KActionCollection* parent, const QString& name,
                                        int maxItems )
  : KSelectAction( parent, name )
  , d(new KRecentFilesActionPrivate)
{
  d->m_maxItems = maxItems;

  init();
}

void KRecentFilesAction::init()
{
  setMenu(new KMenu());
  setToolBarMode(MenuMode);

  connect(this, SIGNAL(triggered(QAction*)), SLOT(urlSelected(QAction*)));
}

KRecentFilesAction::~KRecentFilesAction()
{
  delete menu();
  delete d;
}

void KRecentFilesAction::urlSelected( QAction* action )
{
  emit urlSelected(d->m_urls[action]);
}

int KRecentFilesAction::maxItems() const
{
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems( int maxItems )
{
    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all excess items
    while( selectableActionGroup()->actions().count() > maxItems )
        delete removeAction(selectableActionGroup()->actions().last());
}

void KRecentFilesAction::addUrl( const KUrl& _url, const QString& name )
{
    /**
     * Create a deep copy here, because if _url is the parameter from
     * urlSelected() signal, we will delete it in the removeAction() call below.
     * but access it again in the addAction call... => crash
     */
    const KUrl url( _url );

    if ( url.isLocalFile() && !KGlobal::dirs()->relativeLocation("tmp", url.path()).startsWith("/"))
       return;
    const QString tmpName = name.isEmpty() ?  url.fileName() : name;
    const QString file = url.pathOrUrl();

    // remove file if already in list
    foreach (QAction* action, selectableActionGroup()->actions())
    {
      if ( d->m_urls[action].pathOrUrl().endsWith(file) )
      {
        removeAction(action)->deleteLater();
        break;
      }
    }
    // remove last item if already maxitems in list
    if( d->m_maxItems && selectableActionGroup()->actions().count() == d->m_maxItems )
    {
        // remove last item
        delete removeAction(selectableActionGroup()->actions().last());
    }

    // add file to list
    const QString title = tmpName + " [" + file + ']';
    QAction* action = new QAction(title, selectableActionGroup());
    addAction(action, url, tmpName);
}

void KRecentFilesAction::addAction(QAction* action, const KUrl& url, const QString& name)
{
  KSelectAction::addAction( action );

  d->m_shortNames.insert( action, name );
  d->m_urls.insert( action, url );
}

QAction* KRecentFilesAction::removeAction(QAction* action)
{
  KSelectAction::removeAction( action );

  d->m_shortNames.remove( action );
  d->m_urls.remove( action );

  return action;
}

void KRecentFilesAction::removeUrl( const KUrl& url )
{
  for (QMap<QAction*, KUrl>::ConstIterator it = d->m_urls.constBegin(); it != d->m_urls.constEnd(); ++it)
    if (it.value() == url) {
      delete removeAction(it.key());
      return;
    }
}

void KRecentFilesAction::clear()
{
    KSelectAction::clear();
    d->m_shortNames.clear();
    d->m_urls.clear();
}

void KRecentFilesAction::loadEntries( KConfig* config, const QString &groupname)
{
    clear();

    QString     key;
    QString     value;
    QString     nameKey;
    QString     nameValue;
    QString      title;
    QString     oldGroup;
    QStringList lst;
    KUrl        url;

    oldGroup = config->group();

    if (groupname.isEmpty())
      config->setGroup("RecentFiles");
    else
      config->setGroup( groupname );

    // read file list
    for( int i = 1 ; i <= d->m_maxItems ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = config->readPathEntry( key );
        url = KUrl( value );

        // Don't restore if file doesn't exist anymore
        if (url.isLocalFile() && !QFile::exists(url.path()))
          continue;

        // Don't restore where the url is already known (eg. broken config)
        if (d->m_urls.values().contains(url))
          continue;

        nameKey = QString( "Name%1" ).arg( i );
        nameValue = config->readPathEntry( nameKey, url.fileName() );
        title = nameValue + " [" + value + ']';
        if (!value.isNull())
        {
          addAction(new QAction(title, selectableActionGroup()), url, nameValue);
        }
    }

    config->setGroup( oldGroup );
}

void KRecentFilesAction::saveEntries( KConfig* config, const QString &groupname )
{
    QString     key;
    QString     value;
    QString     oldGroup;
    QStringList lst = items();

    oldGroup = config->group();

    QString group = groupname;
    if (groupname.isEmpty())
      group = "RecentFiles";
    config->deleteGroup( group );
    config->setGroup( group );

    // write file list
    for ( int i = 1 ; i <= selectableActionGroup()->actions().count() ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        // i - 1 because we started from 1
        value = d->m_urls[ selectableActionGroup()->actions()[ i - 1 ] ].pathOrUrl();
        config->writePathEntry( key, value );
        key = QString( "Name%1" ).arg( i );
        value = d->m_shortNames[ selectableActionGroup()->actions()[ i - 1 ] ];
        config->writePathEntry( key, value );
    }

    config->setGroup( oldGroup );
}

/* vim: et sw=2 ts=2
 */

#include "krecentfilesaction.moc"
