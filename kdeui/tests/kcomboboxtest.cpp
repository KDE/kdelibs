#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <kapp.h>
#include <kcombobox.h>
#include <klocale.h>

int main ( int argc, char **argv)
{
    KApplication a(argc, argv, "kcomboboxtest");

    // Make a central widget to contain the other widgets
    QWidget * w = new QWidget();
    // Insert the widget container (parent widget) into
    // a layout manager (VERTICAL).
    QVBoxLayout *vbox = new QVBoxLayout( w );
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

    // Create a read-only widget
    KComboBox *soc = new KComboBox( w, "socombobox" );
    QLabel* lblso = new QLabel( soc, "&Select-Only ComboBox", w, "socombolabel" );
    soc->setCompletionMode( KGlobalSettings::CompletionAuto );
    soc->completionObject()->setItems( list );
    soc->insertStringList( list );

    // Create an exit button
    QPushButton * push = new QPushButton( "E&xit", w );
    QObject::connect( push, SIGNAL( clicked() ), &a, SLOT( closeAllWindows() ) );

    // Insert the widgets into the layout manager.
    vbox->addWidget( lblrw );
    vbox->addWidget( rwc );
    vbox->addWidget( lblso );
    vbox->addWidget( soc );
    vbox->addWidget( push );

    a.setMainWidget(w);
    rwc->setFocus();
    w->show();
    return a.exec();
}
