#include "kcomboboxtest.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>


Widget::Widget()
 : QWidget()
{
    // Insert the widget container (parent widget) into
    // a layout manager (VERTICAL).
    QVBoxLayout *vbox = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
    // Resize the widget
    resize( 500, 100 );
 
    // Popuplate the select-only list box
    QStringList list;
    list << "Stone" << "Tree" << "Peables" << "Ocean" << "Sand" << "Chips" << "Computer" << "Mankind";
    list.sort();
 
    // Create and modify read-write widget
    rwc = new KComboBox( true, this, "rwcombobox" );
    QLabel* lblrw = new QLabel( rwc, "&Editable ComboBox", this, "rwcombolabel" );
    rwc->setDuplicatesEnabled( true );
    rwc->completionObject()->setItems( list );
    rwc->setInsertionPolicy( QComboBox::NoInsertion );
    rwc->insertStringList( list );
    rwc->setEditText( "file:/home/adawit/" );
    // rwc->setCompletionMode( KGlobalSettings::CompletionAuto );
 
    // Create a read-write combobox and reproduce konqueror's code
    konqc = new KComboBox( true, this, "konqc" );
    konqc->setMaxCount( 10 );
      KSimpleConfig historyConfig( "konq_history" );
      historyConfig.setGroup( "History" );
      KCompletion * s_pCompletion = new KCompletion;
      s_pCompletion->setOrder( KCompletion::Weighted );
      s_pCompletion->setItems( historyConfig.readListEntry( "CompletionItems" ) );
      s_pCompletion->setCompletionMode( KGlobalSettings::completionMode() );
      konqc->setCompletionObject( s_pCompletion, false );
    QLabel* lblkonq = new QLabel( konqc, "&Konqueror's ComboBox", this );
    QPixmap pix = SmallIcon("www");
    konqc->insertItem( pix, "http://www.kde.org" );
    konqc->setCurrentItem( konqc->count()-1 );
    kdDebug() << "setLocationBarURL setCurrentItem " << konqc->count()-1 << endl;

    // Create a read-only widget
    soc = new KComboBox( this, "socombobox" );
    QLabel* lblso = new QLabel( soc, "&Select-Only ComboBox", this, "socombolabel" );
    soc->setCompletionMode( KGlobalSettings::CompletionAuto );
    soc->completionObject()->setItems( list );
    soc->insertStringList( list );

    // Create an exit button
    QPushButton * push = new QPushButton( "E&xit", this );
    QObject::connect( push, SIGNAL( clicked() ), this, SLOT( slotDisplay() ) );
    QObject::connect( push, SIGNAL( clicked() ), kapp, SLOT( closeAllWindows() ) );

    // Insert the widgets into the layout manager.
    vbox->addWidget( lblrw );
    vbox->addWidget( rwc );
    vbox->addWidget( lblso );
    vbox->addWidget( soc );
    vbox->addWidget( lblkonq );
    vbox->addWidget( konqc );
    vbox->addWidget( push );

    rwc->setFocus();
}

void Widget::slotDisplay()
{
    kdDebug() << "rwc: " << rwc->currentText() << endl;
    kdDebug() << "konqc: " << konqc->currentText() << endl;
    kdDebug() << "soc: " << soc->currentText() << endl;
}

int main ( int argc, char **argv)
{
    KApplication a(argc, argv, "kcomboboxtest");

    // Make a central widget to contain the other widgets
    QWidget * w = new Widget();
    a.setMainWidget(w);
    w->show();
    return a.exec();
}

#include "kcomboboxtest.moc"
