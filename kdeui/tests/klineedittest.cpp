#include <QPushButton>
#include <QTimer>
#include <QBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QRegExp>
#include <QRegExpValidator>

#include <klineedit.h>
#include <kcompletionbox.h>

#include "klineedittest.h"

KLineEditTest::KLineEditTest ( QWidget* widget )
              :QWidget( widget )
{
    QVBoxLayout* layout = new QVBoxLayout( this );

    QStringList list;
    list << "Tree" << "Suuupa" << "Stroustrup" << "Stone" << "Slick"
         << "Slashdot" << "Send" << "Peables" << "Mankind" << "Ocean"
         << "Chips" << "Computer" << "Sandworm" << "Sandstorm" << "Chops";
    list.sort();

    m_lineedit = new KLineEdit( this );
    m_lineedit->setObjectName( "klineedittest" );
    m_lineedit->completionObject()->setItems( list );
    m_lineedit->setSqueezedTextEnabled( true );
    m_lineedit->setClearButtonShown( true );
    connect( m_lineedit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()) );
    connect( m_lineedit, SIGNAL(returnPressed(QString)),
             SLOT(slotReturnPressed(QString)) );

    QHBoxLayout* restrictedHBox = new QHBoxLayout;
    m_restrictedLine = new QLineEdit(this);
    QRegExp regex(QString::fromUtf8("[aeiouyé]*"));
    QRegExpValidator* validator = new QRegExpValidator(regex, m_restrictedLine);
    m_restrictedLine->setValidator(validator);
    connect( m_restrictedLine, SIGNAL(invalidChar(int)), this, SLOT(slotInvalidChar(int)));
    connect( m_restrictedLine, SIGNAL(returnPressed()), SLOT(slotReturnPressed()) );
    connect( m_restrictedLine, SIGNAL(returnPressed(QString)),
             SLOT(slotReturnPressed(QString)) );
    restrictedHBox->addWidget(new QLabel("Vowels only:", this));
    restrictedHBox->addWidget(m_restrictedLine);
    m_invalidCharLabel = new QLabel(this);
    restrictedHBox->addWidget(m_invalidCharLabel);


    // horizontal button layout
    m_btnExit = new QPushButton( "E&xit", this );
    connect( m_btnExit, SIGNAL(clicked()), SLOT(quitApp()) );

    m_btnReadOnly = new QPushButton( "&Read Only", this );
    m_btnReadOnly->setCheckable (true);
    connect( m_btnReadOnly, SIGNAL(toggled(bool)), SLOT(slotReadOnly(bool)) );

    m_btnPassword = new QPushButton( "&Password", this );
    m_btnPassword->setCheckable (true);
    connect( m_btnPassword, SIGNAL(toggled(bool)), SLOT(slotPassword(bool)) );

    m_btnEnable = new QPushButton( "Dis&able", this );
    m_btnEnable->setCheckable (true);
    connect( m_btnEnable, SIGNAL(toggled(bool)), SLOT(slotEnable(bool)) );

    m_btnHide = new QPushButton( "Hi&de", this );
    connect( m_btnHide, SIGNAL(clicked()), SLOT(slotHide()) );

    m_btnPlaceholderText = new QPushButton( "Place Holder Text", this);
    m_btnPlaceholderText->setCheckable (true);
    connect( m_btnPlaceholderText, SIGNAL(toggled(bool)), SLOT(slotPlaceholderText(bool)) );

    m_btnClickMessage = new QPushButton( "Clicked Message - DEPRECATED", this);
    m_btnClickMessage->setCheckable (true);
    connect( m_btnClickMessage, SIGNAL(toggled(bool)), SLOT(slotClickMessage(bool)) );

    QPushButton *btnStyle = new QPushButton( "Stylesheet", this);
    connect( btnStyle, SIGNAL(clicked()), SLOT(slotSetStyleSheet()));

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget( m_btnExit );
    buttonLayout->addWidget( m_btnReadOnly );
    buttonLayout->addWidget( m_btnPassword );
    buttonLayout->addWidget( m_btnEnable );
    buttonLayout->addWidget( m_btnHide );
    buttonLayout->addWidget( m_btnClickMessage );
    buttonLayout->addWidget( m_btnPlaceholderText );
    buttonLayout->addWidget( btnStyle );

    layout->addWidget( m_lineedit );
    layout->addLayout( restrictedHBox );
    layout->addLayout( buttonLayout );
    setWindowTitle( "KLineEdit Unit Test" );
}

KLineEditTest::~KLineEditTest()
{
}

void KLineEditTest::quitApp()
{
    qApp->closeAllWindows();
}

void KLineEditTest::slotSetStyleSheet()
{
    m_lineedit->setStyleSheet("QLineEdit{ background-color:#baf9ce }");
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
    qDebug() << "Return pressed";
}

void KLineEditTest::slotReturnPressed( const QString& text )
{
    qDebug() << "Return pressed: " << text;
}

void KLineEditTest::resultOutput( const QString& text )
{
    qDebug() << "KlineEditTest Debug: " << text;
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

void KLineEditTest::slotPlaceholderText(bool click)
{
    if( click )
    {
        m_lineedit->setText(""); // Clear before to add message
        m_lineedit->setPlaceholderText("Click in this lineedit");
    }
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

void KLineEditTest::slotInvalidChar(int key)
{
    m_invalidCharLabel->setText(QString("Invalid char: %1").arg(key));
}

int main ( int argc, char **argv)
{
    QApplication a(argc, argv);
    KLineEditTest *t = new KLineEditTest();
    //t->lineEdit()->setTrapReturnKey( true );
    //t->lineEdit()->completionBox()->setTabHandling( false );
    t->lineEdit()->setSqueezedTextEnabled( true );
    t->lineEdit()->setText ("This is a really really really really really really "
                            "really really long line because I am a talkative fool!"
                            "I mean ... REALLY talkative. If you don't believe me, ask my cousin.");
    t->show();
    return a.exec();
}

