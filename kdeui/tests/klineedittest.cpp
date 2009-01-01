#include <QtGui/QPushButton>
#include <QtCore/QTimer>
#include <QtGui/QBoxLayout>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kglobalsettings.h>
#include <kcompletionbox.h>
#include <khbox.h>

#include "klineedittest.h"

KLineEditTest::KLineEditTest ( QWidget* widget )
              :QWidget( widget )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    QStringList list;
    list << "Tree" << "Suuupa" << "Stroustrup" << "Stone" << "Slick"
         << "Slashdot" << "Send" << "Peables" << "Mankind" << "Ocean"
         << "Chips" << "Computer" << "Sandworm" << "Sandstorm" << "Chops";
    list.sort();

    m_lineedit = new KLineEdit( this );
    m_lineedit->setObjectName( "klineedittest" );
    m_lineedit->completionObject()->setItems( list );
    m_lineedit->setFixedSize(500,30);
    m_lineedit->setSqueezedTextEnabled( true );
    m_lineedit->setClearButtonShown( true );
    connect( m_lineedit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
    connect( m_lineedit, SIGNAL( returnPressed(const QString&) ),
             SLOT( slotReturnPressed(const QString&) ) );

    KHBox *hbox = new KHBox (this);
    m_btnExit = new QPushButton( "E&xit", hbox );
    m_btnExit->setFixedSize(100,30);
    connect( m_btnExit, SIGNAL( clicked() ), SLOT( quitApp() ) );

    m_btnReadOnly = new QPushButton( "&Read Only", hbox );
    m_btnReadOnly->setCheckable (true);
    m_btnReadOnly->setFixedSize(100,30);
    connect( m_btnReadOnly, SIGNAL( toggled(bool) ), SLOT( slotReadOnly(bool) ) );

    m_btnPassword = new QPushButton( "&Password", hbox );
    m_btnPassword->setCheckable (true);
    m_btnPassword->setFixedSize(100,30);
    connect( m_btnPassword, SIGNAL( toggled(bool) ), SLOT( slotPassword(bool) ) );

    m_btnEnable = new QPushButton( "Dis&able", hbox );
    m_btnEnable->setCheckable (true);
    m_btnEnable->setFixedSize(100,30);
    connect( m_btnEnable, SIGNAL( toggled(bool) ), SLOT( slotEnable(bool) ) );

    m_btnHide = new QPushButton( "Hi&de", hbox );
     m_btnHide->setFixedSize(100,30);
    connect( m_btnHide, SIGNAL( clicked() ), SLOT( slotHide() ) );

	m_btnClickMessage = new QPushButton( "Clicked Message", hbox);
	m_btnClickMessage->setCheckable (true);
	m_btnClickMessage->setFixedSize(100,30);
	connect( m_btnClickMessage, SIGNAL( toggled(bool) ), SLOT( slotClickMessage(bool) ) );
	
    layout->addWidget( m_lineedit );
    layout->addWidget( hbox );
    setWindowTitle( "KLineEdit Unit Test" );
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
    kDebug() << "Return pressed";
}

void KLineEditTest::slotReturnPressed( const QString& text )
{
    kDebug() << "Return pressed: " << text;
}

void KLineEditTest::resultOutput( const QString& text )
{
    kDebug() << "KlineEditTest Debug: " << text;
}

void KLineEditTest::slotReadOnly( bool ro )
{
    m_lineedit->setReadOnly (ro);
    QString text = (ro) ? "&Read Write" : "&Read Only";
    m_btnReadOnly->setText (text);
}

void KLineEditTest::slotPassword( bool pw )
{
    m_lineedit->setPasswordMode (pw);
    QString text = (pw) ? "&Normal Text" : "&Password";
    m_btnPassword->setText (text);
}

void KLineEditTest::slotEnable (bool enable)
{
    m_lineedit->setEnabled (!enable);
    QString text = (enable) ? "En&able":"Dis&able";
    m_btnEnable->setText (text);
}

void KLineEditTest::slotClickMessage(bool click)
{
	if( click )
	{
		m_lineedit->setText(""); // Clear before to add message
		m_lineedit->setClickMessage ("Click in this lineedit");
	}
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
    KAboutData aboutData( "klineedittest", 0, ki18n("klineedittest"), "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addStdCmdLineOptions();

    KApplication a;
    KLineEditTest *t = new KLineEditTest();
    //t->lineEdit()->setTrapReturnKey( true );
    //t->lineEdit()->completionBox()->setTabHandling( false );
    t->lineEdit()->setSqueezedTextEnabled( true );
    t->lineEdit()->setText ("This is a really really really really really really "
                            "really really long line because I am a talkative fool!");
    t->show();
    return a.exec();
}

#include "klineedittest.moc"
