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
         << "Slashdot" << "Sand" << "Peables" << "Mankind" << "Ocean"
         << "Chips" << "Computer";
    list.sort();

    lineedit = new KLineEdit( this, "klineedittest" );
    lineedit->completionObject()->setItems( list );
    lineedit->setFixedSize(500,30);
    lineedit->setFocus();
    connect( lineedit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
    connect( lineedit, SIGNAL( returnPressed(const QString&) ), 
             SLOT( slotReturnPressed(const QString&) ) );

    button = new QPushButton( "E&xit", this );
    button->setFixedSize(100,30);
    connect( button, SIGNAL( clicked() ), SLOT( quitApp() ) );

    layout->addWidget( lineedit );
    layout->addWidget( button );
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
