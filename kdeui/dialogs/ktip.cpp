/*****************************************************************

Copyright (c) 2000-2003 Matthias Hoelzer-Kluepfel <mhk@kde.org>
                        Tobias Koenig <tokoe@kde.org>
                        Daniel Molkentin <molkentin@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "ktip.h"

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtGui/QCheckBox>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kaboutdata.h>
#include <kcolorscheme.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khbox.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <krandom.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <ktextbrowser.h>
#include <kconfiggroup.h>

class KTipDatabase::Private
{
  public:
    void loadTips( const QString &tipFile );
    void addTips( const QString &tipFile );

    QStringList tips;
    int currentTip;
};

void KTipDatabase::Private::loadTips( const QString &tipFile )
{
  tips.clear();
  addTips( tipFile );
}

/**
 * If you change something here, please update the script
 * preparetips, which depends on extracting exactly the same
 * text as done here.
 */
void KTipDatabase::Private::addTips( const QString &tipFile )
{
  QString fileName = KStandardDirs::locate( "data", tipFile );

  if ( fileName.isEmpty() ) {
    kDebug() << "KTipDatabase::addTips: can't find '" << tipFile << "' in standard dirs";
    return;
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug() << "KTipDatabase::addTips: can't open '" << fileName << "' for reading";
    return;
  }

  QByteArray data = file.readAll();
  QString content = QString::fromUtf8( data.data(), data.size() );
  const QRegExp rx( "\\n+" );

  int pos = -1;
  while ( ( pos = content.indexOf( "<html>", pos + 1, Qt::CaseInsensitive ) ) != -1 ) {
    /**
     * To make translations work, tip extraction here must exactly
     * match what is done by the preparetips script.
     */
    QString tip = content
           .mid( pos + 6, content.indexOf( "</html>", pos, Qt::CaseInsensitive ) - pos - 6 )
           .replace( rx, "\n" );

    if ( !tip.endsWith( '\n' ) )
      tip += '\n';

    if ( tip.startsWith( '\n' ) )
      tip = tip.mid( 1 );

    if ( tip.isEmpty() ) {
      kDebug() << "Empty tip found! Skipping! " << pos;
      continue;
    }

    tips.append( tip );
  }

  file.close();
}


KTipDatabase::KTipDatabase( const QString &_tipFile )
  : d( new Private )
{
  QString tipFile = _tipFile;

  if ( tipFile.isEmpty() )
    tipFile = KGlobal::mainComponent().aboutData()->appName() + "/tips";

  d->loadTips( tipFile );

  if ( !d->tips.isEmpty() )
    d->currentTip = KRandom::random() % d->tips.count();
}

KTipDatabase::KTipDatabase( const QStringList& tipsFiles )
  : d( new Private )
{
  if ( tipsFiles.isEmpty() || ( ( tipsFiles.count() == 1 ) && tipsFiles.first().isEmpty() ) ) {
    d->addTips( KGlobal::mainComponent().aboutData()->appName() + "/tips" );
  } else {
    for ( QStringList::ConstIterator it = tipsFiles.begin(); it != tipsFiles.end(); ++it )
      d->addTips( *it );
  }

  if ( !d->tips.isEmpty() )
    d->currentTip = KRandom::random() % d->tips.count();
}

KTipDatabase::~KTipDatabase()
{
    delete d;
}

void KTipDatabase::nextTip()
{
  if ( d->tips.isEmpty() )
    return;

  d->currentTip += 1;

  if ( d->currentTip >= (int) d->tips.count() )
    d->currentTip = 0;
}

void KTipDatabase::prevTip()
{
  if ( d->tips.isEmpty() )
    return;

  d->currentTip -= 1;

  if ( d->currentTip < 0 )
    d->currentTip = d->tips.count() - 1;
}

QString KTipDatabase::tip() const
{
  if ( d->tips.isEmpty() )
    return QString();

  return d->tips[ d->currentTip ];
}


class KTipDialog::Private
{
  public:
    Private( KTipDialog *_parent )
      : parent( _parent )
    {
    }

    void _k_nextTip();
    void _k_prevTip();
    void _k_showOnStart( bool );

    KTipDialog *parent;
    KTipDatabase *database;
    QColor baseColor;
    QColor blendedColor;
    QColor textColor;
    QCheckBox *tipOnStart;
    KTextBrowser *tipText;

    static KTipDialog *mInstance;
};

KTipDialog *KTipDialog::Private::mInstance = 0;

void KTipDialog::Private::_k_prevTip()
{
  database->prevTip();
  tipText->setHtml( QString::fromLatin1( "<qt text=\"%1\" bgcolor=\"%2\">%3</qt>" )
                  .arg( textColor.name() )
                  .arg( baseColor.name() )
                  .arg( i18n( database->tip().toUtf8() ) ) );
}

void KTipDialog::Private::_k_nextTip()
{
  database->nextTip();
  tipText->setHtml( QString::fromLatin1( "<qt text=\"%1\" bgcolor=\"%2\">%3</qt>" )
                  .arg( textColor.name() )
                  .arg( baseColor.name() )
                  .arg( i18n( database->tip().toUtf8() ) ) );
}

void KTipDialog::Private::_k_showOnStart( bool on )
{
  parent->setShowOnStart( on );
}


KTipDialog::KTipDialog( KTipDatabase *database, QWidget *parent )
  : KDialog( parent ),
    d( new Private( this ) )
{
  setButtons( KDialog::None );
  setCaption( i18n( "Tip of the Day" ) );

  /**
   * Parent is 0L when TipDialog is used as a mainWidget. This should
   * be the case only in ktip, so let's use the ktip layout.
   */
  bool isTipDialog = (parent != 0);

  QImage img;
  int h, s, v;

  d->blendedColor = KGlobalSettings::activeTitleColor();
  d->blendedColor.getHsv( &h, &s, &v );
  d->blendedColor.setHsv( h, int(s * (71 / 76.0)), int(v * (67 / 93.0)) );

  if ( !isTipDialog ) {
    img = QImage(KStandardDirs::locate( "data", "kdewizard/pics/wizard_small.png" ) );
    // colorize and check to figure the correct color
    KIconEffect::colorize( img, d->blendedColor, 1.0 );
    QRgb colPixel( img.pixel( 0, 0 ) );

    d->blendedColor = QColor( qRed( colPixel ), qGreen( colPixel ), qBlue( colPixel ) );
  }

  KColorScheme colorScheme(QPalette::Active, KColorScheme::View);

  d->baseColor = colorScheme.background(KColorScheme::AlternateBackground).color();
  d->baseColor.getHsv( &h, &s, &v );
  d->baseColor.setHsv( h, int(s * (10 / 6.0)), int(v * (93 / 99.0)) );

  d->textColor = colorScheme.foreground().color();

  d->database = database;

  setWindowIcon(KIcon("ktip"));

  QFrame *frame = new QFrame( this );
  setMainWidget( frame );
  QVBoxLayout *vbox = new QVBoxLayout( frame );
  vbox->setMargin( 0 );
  vbox->setSpacing( spacingHint() );

  if ( isTipDialog ) {
    QHBoxLayout *pl = new QHBoxLayout();
    pl->setMargin( 0 );
    pl->setSpacing( 0 );

    vbox->addLayout( pl );

    QLabel *bulb = new QLabel( this );
    bulb->setPixmap( KStandardDirs::locate( "data", "kdeui/pics/ktip-bulb.png" ) );
    pl->addWidget( bulb );

    QLabel *titlePane = new QLabel( this );

    QBrush brush;
    brush.setTexture( QPixmap( KStandardDirs::locate( "data", "kdeui/pics/ktip-background.png" ) ) );

    QPalette palette = titlePane->palette();
    palette.setBrush( QPalette::Window, brush );
    titlePane->setPalette( palette );
    titlePane->setText( i18n( "Did you know...?\n" ) );
    titlePane->setFont( QFont( KGlobalSettings::generalFont().family(), 20, QFont::Bold ) );
    titlePane->setAlignment( Qt::AlignCenter );
    titlePane->setAutoFillBackground(true);
    pl->addWidget( titlePane, 100 );
  }

  KHBox *hbox = new KHBox;
  hbox->setSpacing( 0 );
  hbox->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  QPalette pal;
  pal.setColor( QPalette::Window, d->blendedColor );
  hbox->setPalette( pal );
  hbox->setAutoFillBackground( true );
  vbox->addWidget( hbox );

  KHBox *tl = new KHBox( hbox );
  tl->setMargin( 7 );
  QPalette palette = tl->palette();
  palette.setColor( QPalette::Window, d->blendedColor );
  tl->setAutoFillBackground( true );
  tl->setPalette( palette );

  KHBox *topLeft = new KHBox( tl );
  topLeft->setMargin( 15 );
  palette = topLeft->palette();
  palette.setColor( QPalette::Window, d->baseColor );
  topLeft->setPalette( palette );
  topLeft->setAutoFillBackground( true );

  d->tipText = new KTextBrowser( topLeft );

  d->tipText->setOpenExternalLinks( true );

  d->tipText->setWordWrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere );

  QStringList paths;
  paths << KGlobal::dirs()->resourceDirs("icon")
        << KGlobal::dirs()->findResourceDir("data", "kdewizard/pics")+"kdewizard/pics/";

  d->tipText->setSearchPaths( paths );

  d->tipText->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  d->tipText->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  palette = d->tipText->palette();
  palette.setColor( QPalette::Active, QPalette::Link, d->blendedColor );
  palette.setColor( QPalette::Inactive, QPalette::Link, d->blendedColor );
  d->tipText->setPalette( palette );

  if ( !isTipDialog ) {
    QLabel *label = new QLabel( hbox );
    label->setPixmap( QPixmap::fromImage( img ) );
    label->setAlignment( Qt::AlignRight | Qt::AlignBottom );

    QPalette palette = label->palette();
    palette.setColor( QPalette::Window, d->blendedColor );
    label->setPalette( palette );

    resize( 550, 230 );
    QSize sh = size();

    QRect rect = KGlobalSettings::splashScreenDesktopGeometry();

    move( rect.x() + ( rect.width() - sh.width() ) / 2,
          rect.y() + ( rect.height() - sh.height() ) / 2 );
  }

  KSeparator* sep = new KSeparator( Qt::Horizontal );
  vbox->addWidget( sep );

  QHBoxLayout *hbox2 = new QHBoxLayout();
  hbox->setMargin( 4 );
  hbox->setSpacing( 0 );

  vbox->addLayout( hbox2 );

  d->tipOnStart = new QCheckBox( i18n( "&Show tips on startup" ) );
  hbox2->addWidget( d->tipOnStart, 1 );

  KPushButton *prev = new KPushButton( KStandardGuiItem::back( KStandardGuiItem::UseRTL ) );
  prev->setText( i18n( "&Previous" ) );
  hbox2->addWidget( prev );

  KPushButton *next = new KPushButton( KStandardGuiItem::forward( KStandardGuiItem::UseRTL ));
  next->setText( i18nc( "Opposite to Previous", "&Next" ) );
  hbox2->addWidget( next );

  KPushButton *ok = new KPushButton( KStandardGuiItem::close());
  ok->setDefault( true );
  hbox2->addWidget( ok );

  KConfigGroup config( KGlobal::config(), "TipOfDay" );
  d->tipOnStart->setChecked( config.readEntry( "RunOnStart", true ) );

  connect( next, SIGNAL( clicked() ), this, SLOT( _k_nextTip() ) );
  connect( prev, SIGNAL( clicked() ), this, SLOT( _k_prevTip() ) );
  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( d->tipOnStart, SIGNAL( toggled( bool ) ), this, SLOT( _k_showOnStart( bool ) ) );

  ok->setFocus();

  d->_k_nextTip();
}

KTipDialog::~KTipDialog()
{
  if ( Private::mInstance == this )
    Private::mInstance = 0L;
  delete d;
}

/**
 * use the one with a parent window as parameter instead of this one
 * or you will get focus problems
 */
void KTipDialog::showTip( const QString &tipFile, bool force )
{
  showTip( 0, tipFile, force );
}

void KTipDialog::showTip( QWidget *parent, const QString &tipFile, bool force )
{
  showMultiTip( parent, QStringList( tipFile ), force );
}

void KTipDialog::showMultiTip( QWidget *parent, const QStringList &tipFiles, bool force )
{
  KConfigGroup configGroup( KGlobal::config(), "TipOfDay" );

  const bool runOnStart = configGroup.readEntry( "RunOnStart", true );

  if ( !force ) {
    if ( !runOnStart )
      return;

    // showing the tooltips on startup suggests the tooltip
    // will be shown *each time* on startup, not $random days later
    // TODO either remove or uncomment this code, but make the situation clear
    /*bool hasLastShown = configGroup.hasKey( "TipLastShown" );
    if ( hasLastShown ) {
      const int oneDay = 24 * 60 * 60;
      QDateTime lastShown = configGroup.readEntry( "TipLastShown", QDateTime() );

      // Show tip roughly once a week
      if ( lastShown.secsTo( QDateTime::currentDateTime() ) < (oneDay + (KRandom::random() % (10 * oneDay))) )
        return;
    }

    configGroup.writeEntry( "TipLastShown", QDateTime::currentDateTime() );

    if ( !hasLastShown )
      return; // Don't show tip on first start*/
  }

  if ( !Private::mInstance )
    Private::mInstance = new KTipDialog( new KTipDatabase( tipFiles ), parent );
  else
      // The application might have changed the RunOnStart option in its own
      // configuration dialog, so we should update the checkbox.
      Private::mInstance->d->tipOnStart->setChecked( runOnStart );

  Private::mInstance->show();
  Private::mInstance->raise();
}

void KTipDialog::setShowOnStart( bool on )
{
  KConfigGroup config( KGlobal::config(), "TipOfDay" );
  config.writeEntry( "RunOnStart", on );
}

bool KTipDialog::eventFilter( QObject *object, QEvent *event )
{
  if ( object == d->tipText && event->type() == QEvent::KeyPress &&
       (((QKeyEvent *)event)->key() == Qt::Key_Return ||
       ((QKeyEvent *)event)->key() == Qt::Key_Space ))
    accept();

  /**
   * If the user presses Return or Space, we close the dialog as if the
   * default button was pressed even if the KTextBrowser has the keyboard
   * focus. This could have the bad side-effect that the user cannot use the
   * keyboard to open urls in the KTextBrowser, so we just let it handle
   * the key event _additionally_. (Antonio)
   */

  return QWidget::eventFilter( object, event );
}


#include "ktip.moc"
