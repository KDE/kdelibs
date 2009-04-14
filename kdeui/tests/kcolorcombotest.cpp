/*
    This file is part of the KDE Libraries

    Copyright (c) 2007 David Jarvie <software@astrojar.org.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kcolorcombotest.h"

#include <assert.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kconfig.h>

#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <khbox.h>
#include <QtGui/QBoxLayout>

#include <kcolorcombo.h>

KColorComboTest::KColorComboTest(QWidget* widget)
    : QWidget(widget)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    // Standard color list
    KHBox *hbox = new KHBox(this);
    hbox->setSpacing(-1);
    QLabel *lbl = new QLabel("&Standard colors:", hbox);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    mStandard = new KColorCombo(hbox);
    mStandard->setObjectName("StandardColors");
    lbl->setBuddy(mStandard);
    new QLabel("Preset to green (0,255,0)", hbox);
    vbox->addWidget(hbox);

    // Custom color list
    hbox = new KHBox(this);
    hbox->setSpacing(-1);
    lbl = new QLabel("&Reds, greens, blues:", hbox);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    mCustom = new KColorCombo(hbox);
    mCustom->setObjectName("CustomColors");
    lbl->setBuddy(mCustom);
    new QLabel("Preset to green (0,192,0)", hbox);
    vbox->addWidget(hbox);

    // Create an exit button
    hbox = new KHBox(this);
    mExit = new QPushButton("E&xit", hbox);
    QObject::connect(mExit, SIGNAL(clicked()), SLOT(quitApp()));

    vbox->addWidget(hbox);

    // Populate the custom list
    QList<QColor> standardList;
    standardList << Qt::red << Qt::green << Qt::blue << Qt::cyan << Qt::magenta << Qt::yellow << Qt::darkRed
                 << Qt::darkGreen << Qt::darkBlue << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow
                 << Qt::white << Qt::lightGray << Qt::gray << Qt::darkGray << Qt::black;
    QList<QColor> list;
    list << QColor(255,0,0) << QColor(192,0,0) << QColor(128,0,0) << QColor(64,0,0)
         << QColor(0,255,0) << QColor(0,192,0) << QColor(0,128,0) << QColor(0,64,0)
         << QColor(0,0,255) << QColor(0,0,192) << QColor(0,0,128) << QColor(0,0,64);
    mCustom->setColors(list);
    if (mCustom->colors() != list)
        kError() << "Custom combo: setColors() != colors()" << endl;
    mCustom->setColors(QList<QColor>());
    if (mCustom->colors() != standardList)
        kError() << "Custom combo: setColors(empty) != standard colors" << endl;
    mCustom->setColors(list);
    if (mCustom->colors() != list)
        kError() << "Custom combo: setColors() != colors()" << endl;

    if (mStandard->colors() != standardList)
        kError() << "Standard combo: colors()" << endl;

    QColor col = QColor(1,2,3);
    mStandard->setColor(col);
    if (mStandard->color() != col)
        kError() << "Standard combo: set custom color -> " << mStandard->color().red() << "," << mStandard->color().green() << "," << mStandard->color().blue() << endl;
    if (!mStandard->isCustomColor())
        kError() << "Standard combo: custom color: isCustomColor() -> false" << endl;
    mStandard->setColor(Qt::green);
    if (mStandard->color() != Qt::green)
        kError() << "Standard combo: color() -> " << mStandard->color().red() << "," << mStandard->color().green() << "," << mStandard->color().blue() << endl;
    if (mStandard->isCustomColor())
        kError() << "Standard combo: standard color: isCustomColor() -> true" << endl;

    col = QColor(1,2,3);
    mCustom->setColor(col);
    if (mCustom->color() != col)
        kError() << "Custom combo: set custom color -> " << mCustom->color().red() << "," << mCustom->color().green() << "," << mCustom->color().blue() << endl;
    if (!mCustom->isCustomColor())
        kError() << "Custom combo: custom color: isCustomColor() -> false" << endl;
    col = QColor(0,192,0);
    mCustom->setColor(col);
    if (mCustom->color() != col)
        kError() << "Custom combo: color() -> " << mCustom->color().red() << "," << mCustom->color().green() << "," << mCustom->color().blue() << endl;
    if (mCustom->isCustomColor())
        kError() << "Custom combo: standard color: isCustomColor() -> true" << endl;

}

KColorComboTest::~KColorComboTest()
{
}

void KColorComboTest::quitApp()
{
    kapp->closeAllWindows();
}

int main(int argc, char **argv)
{
    KAboutData about("kcolorcombotest", 0, ki18n("kcolorcombotest"), "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication a;

    KColorComboTest* t= new KColorComboTest;
    t->show();
    return a.exec();
}

#include "kcolorcombotest.moc"
