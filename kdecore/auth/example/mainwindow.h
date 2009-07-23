/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>

#include <ActionReply.h>

using namespace KAuth;

namespace Ui
{
class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QProgressBar *progressBar;
    QPushButton *pushButton;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindowClass *ui;

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_longAction_triggered();
    void stopLongAction();
    void longActionPerformed(ActionReply reply);
};

#endif // MAINWINDOW_H
