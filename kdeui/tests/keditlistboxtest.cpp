#include <kapplication.h>
#include <keditlistbox.h>
#include <kcombobox.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "keditlistboxtest" );

    KEditListBox::CustomEditor editor( new KComboBox( true, 0L, "test" ) );
    KEditListBox *box = new KEditListBox( QString::fromLatin1("KEditListBox"),
                                          editor );
    
    box->insertItem( QString::fromLatin1("Test") );
    box->insertItem( QString::fromLatin1("for") );
    box->insertItem( QString::fromLatin1("this") );
    box->insertItem( QString::fromLatin1("KEditListBox") );
    box->insertItem( QString::fromLatin1("Widget") );
    box->show();

    return app.exec();
}
