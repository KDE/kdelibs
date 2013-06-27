#include <QApplication>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <QUrl>

int main( int argc, char **argv )
{
    //KCmdLineOptions opt;
    //opt.add("+[startDir]", qi18n("Directory to start in"), QByteArray());

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QUrl startDir;
    if (argc > 1)
        startDir = QUrl::fromLocalFile(argv[1]);
    QUrl u = KDirSelectDialog::selectDirectory(startDir);
    if ( u.isValid() )
        KMessageBox::information(NULL,
                                 QString("You selected the url: %1")
                                 .arg(u.toString()), "Selected URL");

    return 0;
}
