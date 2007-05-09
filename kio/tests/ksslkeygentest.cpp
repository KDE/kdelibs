#include <qapplication.h>
#include <ksslkeygen.h>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    KSSLKeyGen wizard(0);
    return wizard.exec();
}
