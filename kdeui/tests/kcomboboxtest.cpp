#include "kcomboboxtest.h"
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>

Widget::Widget() : QObject()
{
}

void Widget::slotActivated( int index ) {
    kdDebug() << "slotActivated combo:" << sender()->name() << " index:" << index << endl;
}

int main ( int argc, char **argv)
{
    KApplication a(argc, argv, "kcomboboxtest");

    Widget* obj = new Widget;

    // Make a central widget to contain the other widgets
    QWidget * w = new QWidget();
    // Insert the widget container (parent widget) into
    // a layout manager (VERTICAL).
    QVBoxLayout *vbox = new QVBoxLayout( w, KDialog::marginHint(), KDialog::spacingHint() );
    // Resize the widget
    w->resize( 500, 100 );

    // Popuplate the select-only list box
    QStringList list;
    list << "Stone" << "Tree" << "Peables" << "Ocean" << "Sand" << "Chips" << "Computer" << "Mankind";
    list.sort();

    // Create and modify read-write widget
    KComboBox *rwc = new KComboBox( true, w, "rwcombobox" );
    QLabel* lblrw = new QLabel( rwc, "&Editable ComboBox", w, "rwcombolabel" );
    rwc->setDuplicatesEnabled( true );
    rwc->completionObject()->setItems( list );
    rwc->setInsertionPolicy( QComboBox::NoInsertion );
    rwc->insertStringList( list );
    rwc->setEditText( "file:/home/adawit/" );
    // rwc->setCompletionMode( KGlobalSettings::CompletionAuto );
    QObject::connect( rwc, SIGNAL( activated( int ) ), obj, SLOT( slotActivated( int ) ) );

    // Create a read-write combobox and reproduce konqueror's code
    KComboBox *konqc = new KComboBox( true, w, "konqc" );
    konqc->setMaxCount( 10 );
      KSimpleConfig historyConfig( "konq_history" );
      historyConfig.setGroup( "Location Bar" );
      KCompletion * s_pCompletion = new KCompletion;
//      s_pCompletion->setOrder( KCompletion::Weighted );
      s_pCompletion->setItems( historyConfig.readListEntry( "ComboContents" ) );
      s_pCompletion->setCompletionMode( KGlobalSettings::completionMode() );
      konqc->setCompletionObject( s_pCompletion );
    QLabel* lblkonq = new QLabel( konqc, "&Konqueror's ComboBox", w );
    QPixmap pix = SmallIcon("www");
    konqc->insertItem( pix, "http://www.kde.org" );
    konqc->setCurrentItem( konqc->count()-1 );
    kdDebug() << "setLocationBarURL setCurrentItem " << konqc->count()-1 << endl;
    QObject::connect( konqc, SIGNAL( activated( int ) ), obj, SLOT( slotActivated( int ) ) );

    // Create a read-only widget
    KComboBox *soc = new KComboBox( w, "socombobox" );
    QLabel* lblso = new QLabel( soc, "&Select-Only ComboBox", w, "socombolabel" );
    soc->setCompletionMode( KGlobalSettings::CompletionAuto );
    soc->completionObject()->setItems( list );
    soc->insertStringList( list );
    QObject::connect( soc, SIGNAL( activated( int ) ), obj, SLOT( slotActivated( int ) ) );

    // Create an exit button
    QPushButton * push = new QPushButton( "E&xit", w );
    QObject::connect( push, SIGNAL( clicked() ), &a, SLOT( closeAllWindows() ) );

    // Insert the widgets into the layout manager.
    vbox->addWidget( lblrw );
    vbox->addWidget( rwc );
    vbox->addWidget( lblso );
    vbox->addWidget( soc );
    vbox->addWidget( lblkonq );
    vbox->addWidget( konqc );
    vbox->addWidget( push );


    a.setMainWidget(w);
    rwc->setFocus();
    w->show();
    //w->setIcon( *(new QPixmap("/opt/kde2/share/icons/hicolor/16x16/apps/samba.png" )));
    return a.exec();
}
#include "kcomboboxtest.moc"
