#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kglobalsettings.h>

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

    m_lineedit = new KLineEdit( this, "klineedittest" );
    m_lineedit->completionObject()->setItems( list );
    m_lineedit->setFixedSize(500,30);
    m_lineedit->setFocus();
    m_lineedit->setText ("Whateverkdlsjfldskjf;alskj;ldskjf;lsdjfl;sdkjf;lsdjf;lsdjflkasjfd;lsdjfldskfj;ldskjf;lsdkjf;lsdkjf;sdlfks;dlkfj;lsdkfj;lsdkjflsdkjflsdkjf;lsdkjfs;dlkfs;dlkfsldkjf;lks");
    connect( m_lineedit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
    connect( m_lineedit, SIGNAL( returnPressed(const QString&) ), 
             SLOT( slotReturnPressed(const QString&) ) );

    m_btnExit = new QPushButton( "E&xit", this );
    m_btnExit->setFixedSize(100,30);
    connect( m_btnExit, SIGNAL( clicked() ), SLOT( quitApp() ) );
    
    m_btnReadOnly = new QPushButton( "&ReadOnly", this );
    m_btnReadOnly->setToggleButton (true);
    m_btnReadOnly->setFixedSize(100,30);
    connect( m_btnReadOnly, SIGNAL( toggled(bool) ), SLOT( readOnly(bool) ) );

    layout->addWidget( m_lineedit );
    layout->addWidget( m_btnExit );
    layout->addWidget( m_btnReadOnly );
    setCaption( "KLineEdit Unit Test" );
}

void KLineEditTest::quitApp()
{
    kapp->closeAllWindows();
}

void KLineEditTest::slotReturnPressed()
{
    kdDebug() << "Return pressed" << endl;
}

void KLineEditTest::slotReturnPressed( const QString& text )
{
    kdDebug() << "-- Return pressed " << text << endl;
}

void KLineEditTest::resultOutput( const QString& text )
{
    kdDebug() << "KlineEditTest Debug: " << text << endl;
}

void KLineEditTest::readOnly (bool ro)
{
    m_lineedit->setReadOnly (ro);
    if (ro)
      m_btnReadOnly->setText ("&Read Write");
    else
      m_btnReadOnly->setText ("&Read Only");
}

KLineEditTest::~KLineEditTest()
{
}

int main ( int argc, char **argv)
{
    KApplication *a = new KApplication (argc, argv, "klineedittest");
    KLineEditTest *t = new KLineEditTest();
    t->lineEdit()->setTrapReturnKey( true );
    a->setMainWidget(t);
    t->show();
    return a->exec();
}

#include "klineedittest.moc"
