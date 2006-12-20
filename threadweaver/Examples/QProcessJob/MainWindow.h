#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Job.h>

#include "ui_MainWindowUi.h"

using namespace ThreadWeaver;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow ( QWidget *parent = 0 );
private slots:
    void on_pushButtonStart_clicked();
    void on_pushButtonQuit_clicked();

    void update ( Job* );
private:
    Ui::MainWindow ui;

};

#endif
