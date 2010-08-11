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

#include "klanguagebutton.moc"

#include <QtGui/QMenu>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QMenuItem>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfiggroup.h>

static void checkInsertPos( QMenu *popup, const QString &str, int &index )
{
  if ( index != -1 )
    return;

  int a = 0;
  const QList<QAction*> actions = popup->actions();
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
  KLanguageButtonPrivate( KLanguageButton *parent);
  ~KLanguageButtonPrivate()  { delete button; delete popup; }
  void setCurrentItem( QAction* );
  void clear();
  QAction *findAction(const QString &data) const;

  QPushButton *button;
  QStringList ids;
  QMenu *popup;
  QString current;
  const KLocale *locale;
  bool staticText : 1;
  bool showCodes : 1;
};

KLanguageButton::KLanguageButton( QWidget * parent )
  : QWidget( parent ),
    d( new KLanguageButtonPrivate(this) )
{
}

KLanguageButton::KLanguageButton( const QString & text, QWidget * parent )
  : QWidget( parent ),
    d( new KLanguageButtonPrivate(this) )
{
  setText(text);
}

KLanguageButtonPrivate::KLanguageButtonPrivate( KLanguageButton *parent )
  : button(new QPushButton(parent)),
    popup(new QMenu(parent)),
    locale(0),
    staticText(false),
    showCodes(false)
{
  QHBoxLayout *layout = new QHBoxLayout( parent );
  layout->setMargin(0);
  layout->addWidget( button );

  parent->setFocusProxy( button );
  parent->setFocusPolicy( button->focusPolicy() );

  button->setMenu( popup );

  QObject::connect( popup, SIGNAL(triggered(QAction*)), parent, SLOT(slotTriggered(QAction*)) );
  QObject::connect( popup, SIGNAL(hovered(QAction*)), parent, SLOT(slotHovered(QAction*)) );
}

KLanguageButton::~KLanguageButton()
{
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

void KLanguageButton::insertLanguage( const QString &languageCode, const QString &name, int index )
{
  QString text;
  bool showCodes = d->showCodes;
  if (name.isEmpty())
  {
    text = languageCode;
    const KLocale *locale = d->locale ? d->locale : KGlobal::locale();
    if (locale)
      text = locale->languageCodeToName(languageCode);
    else
      showCodes = false;
  }
  else
    text = name;
  if (showCodes)
    text += QLatin1String( " (" ) + languageCode + QLatin1Char(')');

  checkInsertPos( d->popup, text, index );
  QAction *a = new QAction(QIcon(), text, this);
  a->setData(languageCode);
  if ( index >= 0 && index < d->popup->actions().count()-1)
    d->popup->insertAction(d->popup->actions()[index], a);
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
    QString code = fpath.mid(fpath.lastIndexOf('/') + 1);
    KConfig entry(langlist[i], KConfig::SimpleConfig);
    KConfigGroup group(&entry, "KCM Locale");
    QString name = group.readEntry("Name", i18n("without name"));
    insertLanguage(code, name);
  }

  const KLocale *locale = d->locale ? d->locale : KGlobal::locale();
  setCurrentItem(locale ? locale->language() : KLocale::defaultLanguage());
}

void KLanguageButton::slotTriggered( QAction *a )
{
  //kDebug() << "slotTriggered" << index;
  if (!a)
    return;

  d->setCurrentItem( a );

  // Forward event from popup menu as if it was emitted from this widget:
  emit activated( d->current );
}

void KLanguageButton::slotHovered( QAction *a )
{
  //kDebug() << "slotHovered" << index;

  emit highlighted(a->data().toString());
}

int KLanguageButton::count() const
{
  return d->ids.count();
}

void KLanguageButton::clear()
{
  d->clear();
}

void KLanguageButtonPrivate::clear()
{
  ids.clear();
  popup->clear();

  if ( !staticText ) {
    button->setText( QString() );
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
