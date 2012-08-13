/*
 *   kwizardtest - a test program for the KWizard dialog
 *   Copyright (C) 1998  Thomas Tanghus (tanghus@kde.org)
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
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QBoxLayout>
#include <k3wizard.h>

int main(int argc, char **argv)
{
  QApplication::setApplicationName("test");
  QApplication a(argc, argv);
  K3Wizard *wiz = new K3Wizard(0, "kwizardtest", false);
  QObject::connect((QObject*) wiz->cancelButton(), SIGNAL(clicked()),
		   &a, SLOT(quit()));
  QObject::connect((QObject*) wiz->finishButton(), SIGNAL(clicked()),
		   &a, SLOT(quit()));
  for(int i = 1; i < 11; i++)
  {
    QWidget *p = new QWidget;
    QString msg = QString("This is page %1 out of 10").arg(i);
    QLabel *label = new QLabel(msg, p);
    QHBoxLayout *layout = new QHBoxLayout(p, 5);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(300, 200);
    layout->addWidget(label);
    QString title = QString("%1. page").arg(i);
    wiz->addPage(p, title);
    wiz->setFinishEnabled(p, (i==10));
  }

  wiz->show();
  return a.exec();
}



