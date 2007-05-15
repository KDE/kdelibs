/* -*- C++ -*-

This file declares the mainwindows for the QProcessJob example.

$ Author: Mirko Boehm $
$ Copyright: (C) 2006, 2007 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <Job.h>

#include "ui_MainWindowUi.h"

using namespace ThreadWeaver;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow ( QWidget *parent = 0 );
private Q_SLOTS:
    void on_pushButtonStart_clicked();
    void on_pushButtonQuit_clicked();

    void update ( ThreadWeaver::Job* );
private:
    Ui::MainWindow ui;

};

#endif
