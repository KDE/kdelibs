#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kapp.h>
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
    connect( lineedit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
    lineedit->setFixedSize(500,30);
    lineedit->setFocus();

    button = new QPushButton( "E&xit", this );
    button->setFixedSize(100,30);
    QObject::connect( button, SIGNAL( clicked() ), SLOT( quitApp() ) );

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
