#include <QApplication>
#include <keditlistbox.h>
#include <kcombobox.h>

int main( int argc, char **argv )
{
    QApplication::setApplicationName("keditlistboxtest");

    QApplication app(argc, argv);


    KEditListBox::CustomEditor editor( new KComboBox( true, 0 ) );
    KEditListBox *box = new KEditListBox( QLatin1String("KEditListBox"),
                                          editor );

    box->insertItem( QLatin1String("Test") );
    box->insertItem( QLatin1String("for") );
    box->insertItem( QLatin1String("this") );
    box->insertItem( QLatin1String("KEditListBox") );
    box->insertItem( QLatin1String("Widget") );
    box->show();

    return app.exec();
}
