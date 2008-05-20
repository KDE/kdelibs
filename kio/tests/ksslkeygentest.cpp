#include <qapplication.h>
#include <ksslkeygen.h>
#include <kcomponentdata.h>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    KComponentData data(QByteArray("ksslkeygentest"));

    KSSLKeyGen wizard(0);
    wizard.setKeySize(0);
    return wizard.exec();
}
