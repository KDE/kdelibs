#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include "kdualcolortest.h"

KDualColorWidget::KDualColorWidget( QWidget *parent )
  : QWidget(parent)
{
  mLabel = new QLabel( "<qt>Testing, testing, 1, 2, 3...</qt>", this );

  KDualColorButton *colorButton;
  colorButton = new KDualColorButton( mLabel->palette().color( QPalette::Text ),
                                      mLabel->palette().color( QPalette::Background ), this );

  connect( colorButton, SIGNAL( foregroundColorChanged( const QColor& ) ),
           this, SLOT( slotForegroundColorChanged( const QColor& ) ) );
  connect( colorButton, SIGNAL( backgroundColorChanged( const QColor& ) ),
           this, SLOT( slotBackgroundColorChanged( const QColor& ) ) );
  connect( colorButton, SIGNAL( selectionChanged( KDualColorButton::Selection ) ),
           this, SLOT( slotSelectionChanged( KDualColorButton::Selection ) ) );

  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 5 );
  layout->addWidget( colorButton, 0 );
  layout->addWidget( mLabel, 1 );

  resize( sizeHint() );
}

void KDualColorWidget::slotForegroundColorChanged( const QColor &color )
{
  qDebug( "Foreground Color Changed." );

  QPalette palette = mLabel->palette();
  palette.setColor( QPalette::Text, color );
  mLabel->setPalette( palette );
}

void KDualColorWidget::slotBackgroundColorChanged( const QColor &color )
{
  qDebug( "Background Color Changed." );

  QPalette palette = mLabel->palette();
  palette.setColor( QPalette::Background, color );
  setPalette( palette );
}

void KDualColorWidget::slotSelectionChanged( KDualColorButton::Selection selection )
{
  if ( selection == KDualColorButton::Foreground )
    qDebug( "Foreground Button Selected." );
  else
    qDebug( "Background Button Selected." );
}

int main( int argc, char **argv )
{
  KAboutData about( "KDualColorTest", "KDualColorTest", "version" );
  KCmdLineArgs::init( argc, argv, &about );

  KApplication app;

  KDualColorWidget widget;
  widget.show();

  return app.exec();
}

#include "kdualcolortest.moc"
