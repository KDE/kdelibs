#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <khbox.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kglobalsettings.h>
#include <kcompletionbox.h>

#include "klineedittest.h"

KLineEditTest::KLineEditTest (QWidget* widget, const char* name )
              :QWidget( widget, name )
{
    QVBoxLayout* layout = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

    QStringList list;
    list << "Tree" << "Suuupa" << "Stroustrup" << "Stone" << "Slick"
         << "Slashdot" << "Send" << "Peables" << "Mankind" << "Ocean"
         << "Chips" << "Computer" << "Sandworm" << "Sandstorm" << "Chops";
    list.sort();

    m_lineedit = new KLineEdit( this );
    m_lineedit->setObjectName( "klineedittest" );
    m_lineedit->completionObject()->setItems( list );
    m_lineedit->setFixedSize(500,30);
    m_lineedit->setEnableSqueezedText( true );
    connect( m_lineedit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
    connect( m_lineedit, SIGNAL( returnPressed(const QString&) ),
             SLOT( slotReturnPressed(const QString&) ) );

    KHBox *hbox = new KHBox (this);
    m_btnExit = new QPushButton( "E&xit", hbox );
    m_btnExit->setFixedSize(100,30);
    connect( m_btnExit, SIGNAL( clicked() ), SLOT( quitApp() ) );

    m_btnReadOnly = new QPushButton( "&Read Only", hbox );
    m_btnReadOnly->setToggleButton (true);
    m_btnReadOnly->setFixedSize(100,30);
    connect( m_btnReadOnly, SIGNAL( toggled(bool) ), SLOT( slotReadOnly(bool) ) );

    m_btnEnable = new QPushButton( "Dis&able", hbox );
    m_btnEnable->setToggleButton (true);
    m_btnEnable->setFixedSize(100,30);
    connect( m_btnEnable, SIGNAL( toggled(bool) ), SLOT( slotEnable(bool) ) );

    m_btnHide = new QPushButton( "Hi&de", hbox );
     m_btnHide->setFixedSize(100,30);
    connect( m_btnHide, SIGNAL( clicked() ), SLOT( slotHide() ) );

    layout->addWidget( m_lineedit );
    layout->addWidget( hbox );
    setCaption( "KLineEdit Unit Test" );
}

KLineEditTest::~KLineEditTest()
{
}

void KLineEditTest::quitApp()
{
    kapp->closeAllWindows();
}

void KLineEditTest::show()
{
  if (m_lineedit->isHidden())
    m_lineedit->show();

  m_btnHide->setEnabled( true );

  QWidget::show();
}

void KLineEditTest::slotReturnPressed()
{
    kdDebug() << "Return pressed" << endl;
}

void KLineEditTest::slotReturnPressed( const QString& text )
{
    kdDebug() << "Return pressed: " << text << endl;
}

void KLineEditTest::resultOutput( const QString& text )
{
    kdDebug() << "KlineEditTest Debug: " << text << endl;
}

void KLineEditTest::slotReadOnly( bool ro )
{
    m_lineedit->setReadOnly (ro);
    QString text = (ro) ? "&Read Write" : "&Read Only";
    m_btnReadOnly->setText (text);
}

void KLineEditTest::slotEnable (bool enable)
{
    m_lineedit->setEnabled (!enable);
    QString text = (enable) ? "En&able":"Dis&able";
    m_btnEnable->setText (text);
}

void KLineEditTest::slotHide()
{
    m_lineedit->hide();
    m_btnHide->setEnabled( false );
    m_lineedit->setText( "My dog ate the homework, whaaaaaaaaaaaaaaaaaaaaaaa"
                          "aaaaaaaaaaaaaaaaaaaaaaaaa! I want my mommy!" );
    QTimer::singleShot( 1000, this, SLOT(show()) );
}

int main ( int argc, char **argv)
{
    KAboutData aboutData( "klineedittest", "klineedittest", "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addStdCmdLineOptions();

    KApplication a;
    KLineEditTest *t = new KLineEditTest();
    //t->lineEdit()->setTrapReturnKey( true );
    //t->lineEdit()->completionBox()->setTabHandling( false );
    t->lineEdit()->setEnableSqueezedText( true );
    t->lineEdit()->setText ("This is a really really really really really really "
                            "really really long line because I am a talkative fool!");
    a.setMainWidget(t);
    t->show();
    return a.exec();
}

#include "klineedittest.moc"
