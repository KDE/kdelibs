#include "TestBlendColors.h"

#include <kgraphicsutils.h>

void TestBlendColors::test() {
    QColor color1(10, 10, 100);
    QColor color2(10, 10, 160);
    QColor blended = KGraphicsUtils::blendColor(color1, color2);
    QCOMPARE(blended, color2); // no transparacny.

    QColor previous;
    // check that when altering the alpha of color2 to be less and less transparant this
    // means we are moving more and more towards color2
    for(int i= 10; i <= 255; i+=10) {
        color2.setAlpha(i);
        blended = KGraphicsUtils::blendColor(color1, color2);
        if(previous.isValid()) {
            QCOMPARE(previous.red(), 10);
            QCOMPARE(previous.green(), 10);
            QVERIFY(previous.blue() <= blended.blue());
        }
        previous = blended;
    }

    // only the alpha of color 2 alters the output
    color2.setAlpha(255); //opaque
    color1.setAlpha(80); //opaque
    blended = KGraphicsUtils::blendColor(color2, color2);
    QCOMPARE(blended.red(), color2.red());
    QCOMPARE(blended.green(), color2.green());
    QCOMPARE(blended.blue(), color2.blue());

    // merge from itself to itself gives; TADA; itself again ;)
    color2.setAlpha(127);
    blended = KGraphicsUtils::blendColor(color2, color2);
    QCOMPARE(blended.red(), color2.red());
    QCOMPARE(blended.green(), color2.green());
    QCOMPARE(blended.blue(), color2.blue());
}

QTEST_MAIN(TestBlendColors)
#include "TestBlendColors.moc"
