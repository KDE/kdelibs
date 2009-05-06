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
#include "krecentfilesaction_p.h"

#include <QtCore/QFile>
#ifdef Q_OS_WIN
#include <QtCore/QDir>
#endif

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kmenu.h"


KRecentFilesAction::KRecentFilesAction(QObject *parent)
  : KSelectAction(*new KRecentFilesActionPrivate, parent)
{
  Q_D(KRecentFilesAction);
  d->init();
}

KRecentFilesAction::KRecentFilesAction(const QString &text, QObject *parent)
  : KSelectAction(*new KRecentFilesActionPrivate, parent)
{
  Q_D(KRecentFilesAction);
  d->init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction(const KIcon &icon, const QString &text, QObject *parent)
  : KSelectAction(*new KRecentFilesActionPrivate, parent)
{
  Q_D(KRecentFilesAction);
  d->init();

  setIcon(icon);
  // Want to keep the ampersands
  setText(text);
}

void KRecentFilesActionPrivate::init()
{
  Q_Q(KRecentFilesAction);
  delete q->menu();
  q->setMenu(new KMenu());
  q->setToolBarMode(KSelectAction::MenuMode);
  m_noEntriesAction = q->menu()->addAction(i18n("No Entries"));
  m_noEntriesAction->setEnabled(false);
  clearSeparator = q->menu()->addSeparator();
  clearSeparator->setVisible(false);
  clearAction = q->menu()->addAction(i18n("Clear List"), q, SLOT(clear()));
  clearAction->setVisible(false);
  q->setEnabled(false);
  q->connect(q, SIGNAL(triggered(QAction*)), SLOT(_k_urlSelected(QAction*)));
}

KRecentFilesAction::~KRecentFilesAction()
{
}

void KRecentFilesActionPrivate::_k_urlSelected( QAction* action )
{
    Q_Q(KRecentFilesAction);
    emit q->urlSelected(m_urls[action]);
}

int KRecentFilesAction::maxItems() const
{
    Q_D(const KRecentFilesAction);
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems( int maxItems )
{
    Q_D(KRecentFilesAction);
    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all excess items
    while( selectableActionGroup()->actions().count() > maxItems )
        delete removeAction(selectableActionGroup()->actions().last());
}

void KRecentFilesAction::addUrl( const KUrl& _url, const QString& name )
{
    Q_D(KRecentFilesAction);
    /**
     * Create a deep copy here, because if _url is the parameter from
     * urlSelected() signal, we will delete it in the removeAction() call below.
     * but access it again in the addAction call... => crash
     */
    const KUrl url( _url );

    if ( url.isLocalFile() && KGlobal::dirs()->relativeLocation("tmp", url.toLocalFile()) != url.toLocalFile() )
       return;
    const QString tmpName = name.isEmpty() ?  url.fileName() : name;
#ifdef Q_OS_WIN
    const QString file = url.isLocalFile() ? QDir::toNativeSeparators( url.pathOrUrl() ) : url.pathOrUrl();
#else
    const QString file = url.pathOrUrl();
#endif

    // remove file if already in list
    foreach (QAction* action, selectableActionGroup()->actions())
    {
      if ( d->m_urls[action].pathOrUrl().endsWith(file) )
      {
        removeAction(action)->deleteLater();
        break;
      }
    }
    // remove oldest item if already maxitems in list
    if( d->m_maxItems && selectableActionGroup()->actions().count() == d->m_maxItems )
    {
        // remove oldest added item
        delete removeAction(selectableActionGroup()->actions().first());
    }

    d->m_noEntriesAction->setVisible(false);
    d->clearSeparator->setVisible(true);
    d->clearAction->setVisible(true);
    setEnabled(true);
    // add file to list
    const QString title = tmpName + " [" + file + ']';
    QAction* action = new QAction(title, selectableActionGroup());
    addAction(action, url, tmpName);
}

void KRecentFilesAction::addAction(QAction* action, const KUrl& url, const QString& name)
{
  Q_D(KRecentFilesAction);
  //kDebug (129) << "KRecentFilesAction::addAction(" << action << ")";

  action->setActionGroup(selectableActionGroup());

  // Keep in sync with createToolBarWidget()
  foreach (QToolButton* button, d->m_buttons)
    button->insertAction(button->actions().value(0), action);

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->insertAction(comboBox->actions().value(0), action);

  menu()->insertAction(menu()->actions().value(0), action);

  d->m_shortNames.insert( action, name );
  d->m_urls.insert( action, url );
}

QAction* KRecentFilesAction::removeAction(QAction* action)
{
  Q_D(KRecentFilesAction);
  KSelectAction::removeAction( action );

  d->m_shortNames.remove( action );
  d->m_urls.remove( action );

  return action;
}

void KRecentFilesAction::removeUrl( const KUrl& url )
{
  Q_D(KRecentFilesAction);
  for (QMap<QAction*, KUrl>::ConstIterator it = d->m_urls.constBegin(); it != d->m_urls.constEnd(); ++it)
    if (it.value() == url) {
      delete removeAction(it.key());
      return;
    }
}

KUrl::List KRecentFilesAction::urls() const
{
  Q_D(const KRecentFilesAction);
  return d->m_urls.values ();
}

void KRecentFilesAction::clear()
{
    clearEntries();
    emit recentListCleared();
}

void KRecentFilesAction::clearEntries()
{
    Q_D(KRecentFilesAction);
    KSelectAction::clear();
    d->m_shortNames.clear();
    d->m_urls.clear();
    d->m_noEntriesAction->setVisible(true);
    d->clearSeparator->setVisible(false);
    d->clearAction->setVisible(false);
    setEnabled(false);
}

void KRecentFilesAction::loadEntries( const KConfigGroup& _config)
{
    Q_D(KRecentFilesAction);
    clearEntries();

    QString     key;
    QString     value;
    QString     nameKey;
    QString     nameValue;
    QString      title;
    KUrl        url;

    KConfigGroup cg = _config;
    if ( cg.name().isEmpty())
        cg = KConfigGroup(cg.config(),"RecentFiles");

    bool thereAreEntries=false;
    // read file list
    for( int i = 1 ; i <= d->m_maxItems ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = cg.readPathEntry( key, QString() );
        if (value.isEmpty()) continue;
        url = KUrl( value );

        // Don't restore if file doesn't exist anymore
        if (url.isLocalFile() && !QFile::exists(url.toLocalFile()))
          continue;

        // Don't restore where the url is already known (eg. broken config)
        if (d->m_urls.values().contains(url))
          continue;

#ifdef Q_OS_WIN
        // convert to backslashes
        if ( url.isLocalFile() )
            value = QDir::toNativeSeparators( value );
#endif

        nameKey = QString( "Name%1" ).arg( i );
        nameValue = cg.readPathEntry( nameKey, url.fileName() );
        title = nameValue + " [" + value + ']';
        if (!value.isNull())
        {
          thereAreEntries=true;
          addAction(new QAction(title, selectableActionGroup()), url, nameValue);
        }
    }
    if (thereAreEntries)
    {
        d->m_noEntriesAction->setVisible(false);
        d->clearSeparator->setVisible(true);
        d->clearAction->setVisible(true);
        setEnabled(true);
    }
}

void KRecentFilesAction::saveEntries( const KConfigGroup &_cg )
{
    Q_D(KRecentFilesAction);
    QString     key;
    QString     value;
    QStringList lst = items();

    KConfigGroup cg = _cg;
    if (cg.name().isEmpty())
        cg = KConfigGroup(cg.config(),"RecentFiles");

    cg.deleteGroup();

    // write file list
    for ( int i = 1 ; i <= selectableActionGroup()->actions().count() ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        // i - 1 because we started from 1
        value = d->m_urls[ selectableActionGroup()->actions()[ i - 1 ] ].pathOrUrl();
        cg.writePathEntry( key, value );
        key = QString( "Name%1" ).arg( i );
        value = d->m_shortNames[ selectableActionGroup()->actions()[ i - 1 ] ];
        cg.writePathEntry( key, value );
    }

}

/* vim: et sw=2 ts=2
 */

#include "krecentfilesaction.moc"
