/*
 * Copyright 2013  Alex Merry <alex.merry@kdemail.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the X Consortium shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from the X Consortium.
 */

#include "testview.h"
#include <QSignalSpy>
#include <QScopedPointer>
#include <QTest>

class ViewTest : public QWidget
{
    Q_OBJECT

private Q_SLOTS:
    void init() {
        view = new TestView(this);
    }
    void cleanup() {
        delete view;
        view = 0;
    }

    void testParent();
    void testDefaultVideoWidget();
    void testSetVideoWidget();
    void testSetButtons();

private:
    TestView *view;
};

using namespace KMediaPlayer;

void ViewTest::testParent() {
    QCOMPARE(view->parent(), this);
}

void ViewTest::testDefaultVideoWidget() {
    QCOMPARE(view->videoWidget(), static_cast<QWidget*>(0));
}

void ViewTest::testSetVideoWidget() {
    QScopedPointer<QWidget> widget(new QWidget(this));

    view->setVideoWidgetWrapper(widget.data());
    QCOMPARE(view->videoWidget(), widget.data());

    view->setVideoWidgetWrapper(0);
    QCOMPARE(view->videoWidget(), static_cast<QWidget*>(0));
}

void ViewTest::testSetButtons() {
    QSignalSpy spy(view, SIGNAL(buttonsChanged(int)));

    int expButtons = (int)(View::Stop | View::Pause);
    view->setButtons(expButtons);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), expButtons);
    QCOMPARE(view->buttons(), expButtons);

    expButtons |= (int)View::Play;
    view->toggleButton((int)(View::Play));
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), expButtons);
    QCOMPARE(view->buttons(), expButtons);

    expButtons = (int)(View::Stop | View::Pause);
    view->toggleButton((int)(View::Play));
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), expButtons);
    QCOMPARE(view->buttons(), expButtons);

    expButtons = (int)View::Stop;
    view->hideButton((int)(View::Pause));
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), expButtons);
    QCOMPARE(view->buttons(), expButtons);

    expButtons |= (int)View::Seeker;
    view->showButton((int)(View::Seeker));
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), expButtons);
    QCOMPARE(view->buttons(), expButtons);

    // showing an already-visible button
    view->showButton((int)(View::Stop));
    QCOMPARE(view->buttons(), expButtons);

    // hiding an already-hidden button
    view->hideButton((int)(View::Play));
    QCOMPARE(view->buttons(), expButtons);
}


QTEST_MAIN(ViewTest)

#include "viewtest.moc"
#include "moc_testview.cpp"

