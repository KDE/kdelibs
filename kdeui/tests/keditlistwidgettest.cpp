#include <qapplication.h>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

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
    QDialog dialog;
    dialog.setObjectName("stringlist_dialog");
    dialog.setModal(true);
    dialog.setWindowTitle("Edit List of Items");

    KEditListWidget *edit = new KEditListWidget(&dialog);
    edit->setObjectName("editlist");
    edit->insertStringList(list);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(&dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(edit);
    layout->addWidget(buttonBox);
    dialog.setLayout(layout);

    if (dialog.exec() == QDialog::Accepted) {
        list = edit->items();
        qDebug() << list;
    }
#endif

    return app.exec();
}
