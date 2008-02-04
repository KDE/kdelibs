/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    Permission to use, copy, modify, and distribute this software
    and its documentation for any purpose and without fee is hereby
    granted, provided that the above copyright notice appear in all
    copies and that both that the copyright notice and this
    permission notice and warranty disclaimer appear in supporting
    documentation, and that the name of the author not be used in
    advertising or publicity pertaining to distribution of the
    software without specific, written prior permission.

    The author disclaim all warranties with regard to this
    software, including all implied warranties of merchantability
    and fitness.  In no event shall the author be liable for any
    special, indirect or consequential damages or any damages
    whatsoever resulting from loss of use, data or profits, whether
    in an action of contract, negligence or other tortious action,
    arising out of or in connection with the use or performance of
    this software.

*/

#include <qtest_kde.h>
#include <kfadewidgeteffect.h>
#include <QtCore/QPointer>

class KFadeWidgetEffectTest : public QObject
{
    Q_OBJECT
    private slots:
        void initTestCase();
        void createEffect();
        void startEffect();
        void cleanupTestCase();

    private:
        QWidget *parent, *w;
        QPointer<KFadeWidgetEffect> fade;
};

void KFadeWidgetEffectTest::initTestCase()
{
    parent = new QWidget;
    w = new QWidget(parent);
    //parent->show();
}

void KFadeWidgetEffectTest::createEffect()
{
    fade = new KFadeWidgetEffect(w);
}

void KFadeWidgetEffectTest::startEffect()
{
    fade->start(250);
    QVERIFY(QTest::kWaitForSignal(fade, SIGNAL(destroyed(QObject *)), 2000));
}

void KFadeWidgetEffectTest::cleanupTestCase()
{
    QVERIFY(!fade);
    delete parent;
}

QTEST_KDEMAIN(KFadeWidgetEffectTest, GUI)

#include "kfadewidgeteffecttest.moc"
