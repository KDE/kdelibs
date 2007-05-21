/*
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
 *           (c) 2007      David Jarvie <software@astrojar.org.uk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#define INCLUDE_MENUITEM_DEF
#include "klanguagebutton.moc"

#include <QtGui/QMenu>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QMenuItem>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

static void checkInsertPos( QMenu *popup, const QString &str, int &index )
{
  if ( index == -1 )
    return;

  int a = 0;
  QList<QAction*> actions = popup->actions();
  int b = actions.count();

  while ( a < b )
  {
    int w = ( a + b ) / 2;
    QAction *ac = actions[ w ];
    int j = str.localeAwareCompare( ac->text() );
    if ( j > 0 )
      a = w + 1;
    else
      b = w;
  }

  index = a; // it doesn't really matter ... a == b here.

  Q_ASSERT( a == b );
}

class KLanguageButtonPrivate
{
public:
  KLanguageButtonPrivate()
      : locale(0),
        staticText(false),
        showCodes(false) {}
  void setCurrentItem( QAction* );
  void init( KLanguageButton* );
  QAction *findAction(const QString &data) const;

  QPushButton *button;
  QStringList ids;
  QMenu *popup;
  QString current;
  const KLocale *locale;
  bool staticText;
  bool showCodes;
};

KLanguageButton::KLanguageButton( QWidget * parent )
  : QWidget( parent ),
    d( new KLanguageButtonPrivate )
{
  d->init(this);
}

KLanguageButton::KLanguageButton( const QString & text, QWidget * parent )
  : QWidget( parent ),
    d( new KLanguageButtonPrivate )
{
  d->init(this);

  setText(text);
}

void KLanguageButtonPrivate::init( KLanguageButton *parent )
{
  popup = new QMenu( parent );

  QHBoxLayout *layout = new QHBoxLayout( parent );
  layout->setMargin(0);
  layout->setSpacing(0);
  button = new QPushButton( parent ); // HPB don't touch this!!
  layout->addWidget( button );

  QObject::connect( popup, SIGNAL(triggered(QAction*)), parent, SLOT(slotTriggered(QAction*)) );
  QObject::connect( popup, SIGNAL(hovered(QAction*)), parent, SLOT(slotHovered(QAction*)) );

  parent->clear();
}

KLanguageButton::~KLanguageButton()
{
  delete d->button;
  delete d;
}

void KLanguageButton::setText(const QString & text)
{
  d->staticText = true;
  d->button->setText(text);
}

void KLanguageButton::setLocale( const KLocale *locale )
{
  d->locale = locale;
}

void KLanguageButton::showLanguageCodes( bool show )
{
  d->showCodes = show;
}

void KLanguageButton::insertLanguage( const QString &languageCode, int index )
{
  QString text = languageCode;
  const KLocale *locale = d->locale ? d->locale : KGlobal::locale();
  if (locale) {
    text = locale->twoAlphaToLanguageName(languageCode);
    if (d->showCodes)
      text += QLatin1String( " (" ) + languageCode + QLatin1String( ")" );
  }
  checkInsertPos( d->popup, text, index );
  QAction *a = new QAction(QIcon(), text, this);
  a->setData(languageCode);
  if ( index >= 0 && index < d->popup->actions().count()-1)
    d->popup->insertAction(a, d->popup->actions()[index]);
  else
    d->popup->addAction(a);
  d->ids.append(languageCode);
}

void KLanguageButton::insertSeparator( int index )
{
  if ( index >= 0 && index < d->popup->actions().count()-1)
    d->popup->insertSeparator(d->popup->actions()[index]);
  else
    d->popup->addSeparator();
}

void KLanguageButton::loadAllLanguages()
{
  QStringList langlist = KGlobal::dirs()->findAllResources("locale",
                                  QString::fromLatin1("*/entry.desktop"));
  langlist.sort();
  for (int i = 0, count = langlist.count(); i < count; ++i)
  {

    QString fpath = langlist[i].left(langlist[i].length() - 14);
    QString code = fpath.mid(fpath.lastIndexOf('/'));
    insertLanguage(code);
  }

  const KLocale *locale = d->locale ? d->locale : KGlobal::locale();
  setCurrentItem(locale ? locale->language() : KLocale::defaultLanguage());
}

void KLanguageButton::slotTriggered( QAction *a )
{
  //kDebug() << "slotTriggered" << index << endl;
  if (!a)
    return;

  d->setCurrentItem( a );

  // Forward event from popup menu as if it was emitted from this widget:
  emit activated( d->current );
}

void KLanguageButton::slotHovered( QAction *a )
{
  //kDebug() << "slotHovered" << index << endl;

  emit highlighted(a->data().toString());
}

int KLanguageButton::count() const
{
  return d->ids.count();
}

void KLanguageButton::clear()
{
  d->ids.clear();
  d->popup->clear();
  d->button->setMenu( d->popup );

  if ( !d->staticText ) {
    d->button->setText( QString() );
  }
}

bool KLanguageButton::contains( const QString &languageCode ) const
{
  return d->ids.contains( languageCode );
}

QString KLanguageButton::current() const
{
  return d->current.isEmpty() ? QLatin1String("en") : d->current;
}

QAction *KLanguageButtonPrivate::findAction(const QString& data) const
{
  foreach(QAction *a, popup->actions()) {
    if (!a->data().toString().compare(data))
      return a;
  }
  return 0;
}

void KLanguageButton::setCurrentItem( const QString & languageCode )
{
  if (!d->ids.count())
    return;
  QAction *a;
  if (d->ids.indexOf(languageCode) < 0)
    a = d->findAction(d->ids[0]);
  else
    a = d->findAction(languageCode);
  if (a)
    d->setCurrentItem(a);
}

void KLanguageButtonPrivate::setCurrentItem( QAction *a )
{
  if (!a->data().isValid())
    return;
  current = a->data().toString();

  if ( !staticText ) {
    button->setText( a->text() );
  }
}
