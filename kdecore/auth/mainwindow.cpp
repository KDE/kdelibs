#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Action.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("All (*.*)"));

    QFile file(filename);
    QTextStream stream(&file);

    if(!file.open(QIODevice::ReadOnly))
    {
        Action readAction = "it.gigabytes.auth.read";
        ActionReply reply;
        if(readAction.authorize())
        {
            readAction.arguments()["path"] = filename;
            reply = readAction.execute();
            // Asynchronous alternative:
            // readAction.execute(SLOT(readActionFinished(ActionReply)));
            // return;
        }

        if(reply != ActionReply::OK)
            QMessageBox::information(this, "Errore", file.errorString());
        else
            ui->plainTextEdit->setPlainText(reply.value("text").toString());

        return;
    }

    ui->plainTextEdit->setPlainText(stream.readAll());

    file.close();
}
