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

    // All the other widgets
    KComboBox *rwc = new KComboBox( true, w, "rwcombobox", true );
    QLabel* lblrw = new QLabel( rwc, i18n("&Editable ComboBox"), w, "rwcombolabel" );
    KComboBox *soc = new KComboBox( w, "socombobox", true );
    QLabel* lblso = new QLabel( soc, i18n("&Select-Only ComboBox"), w, "socombolabel" );
    QPushButton * push = new QPushButton( "E&xit", w );

    // Set up the editable combo box.
    rwc->setEnableMultipleInsertion( false );
    QObject::connect( rwc, SIGNAL( returnPressed( const QString& ) ), rwc->completionObject(), SLOT( addItem( const QString& ) ) );


    // Set up select-only combo box.
    soc->setCompletionMode( KGlobal::CompletionAuto );
    soc->setEnableMultipleInsertion( false );
    // Popuplate the select-only list box
    QStringList list;
    list <<i18n("Stone") <<i18n("Tree") <<i18n("Peables") <<i18n("Ocean") <<i18n("Sand") <<i18n("Chips") <<i18n("Computer") <<i18n("Mankind");
    list.sort();
    // This is to test a feature that does not currently work.
    soc->completionObject()->setItems( list );
    soc->insertStringList( list );

    // Insert the widgets into the layout manager.
    vbox->addWidget( lblrw );
    vbox->addWidget( rwc );
    vbox->addWidget( lblso );
    vbox->addWidget( soc );
    vbox->addWidget( push );

    // Set focus on the editable box
    rwc->setFocus();

    // Insert widget into the app object, show and exit when requetsed ...
    QObject::connect( push, SIGNAL( clicked() ), &a, SLOT( closeAllWindows() ) );
    a.setMainWidget(w);
    w->show();
    return a.exec();
}
