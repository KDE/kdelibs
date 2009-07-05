/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*                                                               
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or   
*   (at your option) any later version.                                 
*                                                                       
*   This program is distributed in the hope that it will be useful,     
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
*   GNU General Public License for more details.                        
*                                                                       
*   You should have received a copy of the GNU General Public License   
*   along with this program; if not, write to the                       
*   Free Software Foundation, Inc.,                                     
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .      
*/

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

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
        Action::setHelperID("org.kde.auth.example");
        
        Action readAction = "org.kde.auth.example.action1";
        ActionReply reply;
        if(readAction.authorize())
        {
            reply = readAction.execute();
        }

        if(reply.failed())
            QMessageBox::information(this, "Errore", QString("KAuth returned an error code: %1").arg(reply.errorCode()));
        else
            ui->plainTextEdit->setPlainText(reply.data().value("text").toString());

        return;
    }

    ui->plainTextEdit->setPlainText(stream.readAll());

    file.close();
}

void MainWindow::on_actionOpenAsync_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("All (*.*)"));
    
    QFile file(filename);
    QTextStream stream(&file);
    
    if(!file.open(QIODevice::ReadOnly))
    {
        Action::setHelperID("org.kde.auth.example");
        
        Action action1 = "org.kde.auth.example.action1";
        Action action2 = "org.kde.auth.example.action2";
        
        connect(action1.watcher(), SIGNAL(actionPerformed(ActionReply)), this, SLOT(action1Executed(ActionReply)));
        connect(action2.watcher(), SIGNAL(actionPerformed(ActionReply)), this, SLOT(action2Executed(ActionReply)));
        
        if(action1.authorize() && action2.authorize())
        {            
           if(!Action::executeActions(QList<Action>() << action1 << action2))
               qDebug() << "executeActions returns false";
        }
    }
    
    ui->plainTextEdit->setPlainText(stream.readAll());
    
    file.close();
}

void MainWindow::action1Executed(ActionReply reply)
{
    qDebug() << "Action1 executed asynchronously";
}

void MainWindow::action2Executed(ActionReply reply)
{
    qDebug() << "Action2 executed asynchronously";
}
