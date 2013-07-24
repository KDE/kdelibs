#include <QApplication>
#include <kmessagebox.h>

#include "passworddialog.h"

int main ( int argc, char** argv )
{
    QApplication::setApplicationName("kiopassdlgtest");
    QApplication app(argc, argv);

    QString usr, pass, comment, label;
    label = "Site:";
    comment = "<b>localhost</b>";
    int res = KIO::PasswordDialog::getNameAndPassword( usr, pass, 0L,
                                                       QString(), false,
                                                       QString(), comment,
                                                       label );
    if ( res == QDialog::Accepted )
        KMessageBox::information( 0L, QString("You entered:\n"
					   "  Username: %1\n"
                                           "  Password: %2").arg(usr).arg(pass),
                                	"Test Result");
    else
        KMessageBox::information( 0L, "Password dialog was canceled!",
                                      "Test Result");

    return 0;
}
