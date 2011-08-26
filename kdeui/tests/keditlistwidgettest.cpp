#include <qapplication.h>
#include <kdialog.h>
#include <keditlistwidget.h>
#include <kcombobox.h>
#include <QDebug>

int main( int argc, char **argv )
{
    QApplication app(argc, argv);

#if 0
    KEditListWidget::CustomEditor editor( new KComboBox( true, 0 ) );
    KEditListWidget *box = new KEditListWidget( editor );

    box->insertItem( QLatin1String("Test") );
    box->insertItem( QLatin1String("for") );
    box->insertItem( QLatin1String("this") );
    box->insertItem( QLatin1String("KEditListWidget") );
    box->insertItem( QLatin1String("Widget") );
    box->show();

#else

    // code from kexi
    QStringList list; list << "one" << "two";
    KDialog dialog;
    dialog.setObjectName("stringlist_dialog");
    dialog.setModal(true);
    dialog.setWindowTitle("Edit List of Items");
    dialog.setButtons(KDialog::Ok | KDialog::Cancel);

    KEditListWidget *edit = new KEditListWidget(&dialog);
    edit->setObjectName("editlist");
    dialog.setMainWidget(edit);
    edit->insertStringList(list);

    if (dialog.exec() == QDialog::Accepted) {
        list = edit->items();
        qDebug() << list;
    }
#endif

    return app.exec();
}
