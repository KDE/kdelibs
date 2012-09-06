// krazy:excludeall=i18ncheckarg
/*  This file is part of the KDE libraries
    Copyright (C) 2006 Chusslove Illich <caslav.ilic@gmx.net>

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

#include "klocalizedstringtest.h"

#include <locale.h>

#include <kdebug.h>
#include "qtest_kde.h"

#include <libintl.h>

#include "klocale.h"
#include "klocalizedstring.h"
#include "kconfiggroup.h"

#include <QtCore/QString>


void KLocalizedStringTest::initTestCase ()
{
    const QString kdelibs_fr = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/") + "fr/LC_MESSAGES/kdelibs4.mo");
    m_hasFrench = !kdelibs_fr.isEmpty();
    if (m_hasFrench) {
        setlocale(LC_ALL, "fr_FR.utf8");
        if (setlocale(LC_ALL, NULL) != QByteArray("fr_FR.utf8")) {
            kDebug() << "Setting locale to fr_FR.utf8 failed";
            m_hasFrench = false;
        }
    }

    if (m_hasFrench)
        KLocale::global()->setLanguage(QStringList() << "fr" << "en_US");
    KLocale::global()->setThousandsSeparator(QLatin1String(","));
    KLocale::global()->setDecimalSymbol(QLatin1String("."));
    QCOMPARE(KLocale::global()->isApplicationTranslatedInto("en_US"), true);

    if (m_hasFrench) {
        QCOMPARE(KLocale::global()->isApplicationTranslatedInto("fr"), true);
    }

}

void KLocalizedStringTest::correctSubs ()
{
    // Warm up.
    QCOMPARE(i18n("Daisies, daisies"),
             QString("Daisies, daisies"));

    // Placeholder in the middle.
    QCOMPARE(i18n("Fault in %1 unit", QString("AE35")),
             QString("Fault in AE35 unit"));
    // Placeholder at the start.
    QCOMPARE(i18n("%1, Tycho Magnetic Anomaly 1", QString("TMA-1")),
             QString("TMA-1, Tycho Magnetic Anomaly 1"));
    // Placeholder at the end.
    QCOMPARE(i18n("...odd things happening at %1", QString("Clavius")),
             QString("...odd things happening at Clavius"));
    QCOMPARE(i18n("Group %1", 1),
            QString("Group 1"));

    // Two placeholders.
    QCOMPARE(i18n("%1 and %2", QString("Bowman"), QString("Poole")),
             QString("Bowman and Poole"));
    // Two placeholders in inverted order.
    QCOMPARE(i18n("%2 and %1", QString("Poole"), QString("Bowman")),
             QString("Bowman and Poole"));

    // % which is not of placeholder.
    QCOMPARE(i18n("It's going to go %1% failure in 72 hours.", 100),
             QString("It's going to go 100% failure in 72 hours."));

    // Usual plural.
    QCOMPARE(i18np("%1 pod", "%1 pods", 1),
             QString("1 pod"));
    QCOMPARE(i18np("%1 pod", "%1 pods", 10),
             QString("10 pods"));

    // No plural-number in singular.
    QCOMPARE(i18np("A pod", "%1 pods", 1),
             QString("A pod"));
    QCOMPARE(i18np("A pod", "%1 pods", 10),
             QString("10 pods"));

    // No plural-number in singular or plural.
    QCOMPARE(i18np("A pod", "Few pods", 1),
             QString("A pod"));
    QCOMPARE(i18np("A pod", "Few pods", 10),
             QString("Few pods"));

    // First of two arguments as plural-number.
    QCOMPARE(i18np("A pod left on %2", "%1 pods left on %2",
                   1, QString("Discovery")),
             QString("A pod left on Discovery"));
    QCOMPARE(i18np("A pod left on %2", "%1 pods left on %2",
                   2, QString("Discovery")),
             QString("2 pods left on Discovery"));

    // Second of two arguments as plural-number.
    QCOMPARE(i18np("%1 has a pod left", "%1 has %2 pods left",
                   QString("Discovery"), 1),
             QString("Discovery has a pod left"));
    QCOMPARE(i18np("%1 has a pod left", "%1 has %2 pods left",
                   QString("Discovery"), 2),
             QString("Discovery has 2 pods left"));

    // No plural-number in singular or plural, but another argument present.
    QCOMPARE(i18np("A pod left on %2", "Some pods left on %2",
                   1, QString("Discovery")),
             QString("A pod left on Discovery"));
    QCOMPARE(i18np("A pod left on %2", "Some pods left on %2",
                   2, QString("Discovery")),
             QString("Some pods left on Discovery"));

    // Visual formatting.
    QCOMPARE(i18n("E = mc^2"),
             QString("E = mc^2"));
    QCOMPARE(i18n("E &lt; mc^2"),
             QString("E < mc^2"));
    QCOMPARE(i18n("E &lt; mc^2<br/>"),
             QString("E &lt; mc^2<br/>"));
    QCOMPARE(i18n("<b>E</b> &lt; mc^2"),
             QString("<b>E</b> &lt; mc^2"));
    QCOMPARE(i18n("<html>E &lt; mc^2</html>"),
             QString("<html>E &lt; mc^2</html>"));
    QCOMPARE(i18n("E ? <emphasis>mc^2</emphasis>"),
             QString("E ? *mc^2*"));
    QCOMPARE(i18n("E &lt; <emphasis>mc^2</emphasis>"),
             QString("E < *mc^2*"));
    QCOMPARE(i18n("<html>E &lt; <emphasis>mc^2</emphasis></html>"),
             QString("<html>E &lt; <i>mc^2</i></html>"));
    QCOMPARE(i18n("<b>E</b> &lt; <emphasis>mc^2</emphasis>"),
             QString("<b>E</b> &lt; <i>mc^2</i>"));
    QCOMPARE(i18n("<emphasis>E</emphasis> &lt; <b>mc^2</b>"),
             QString("<i>E</i> &lt; <b>mc^2</b>"));
    QCOMPARE(i18nc("@label", "E &lt; <emphasis>mc^2</emphasis>"),
             QString("E < *mc^2*"));
    QCOMPARE(i18nc("@info", "E &lt; <emphasis>mc^2</emphasis>"),
             QString("<html>E &lt; <i>mc^2</i></html>"));
    QCOMPARE(i18n("E = mc^&#x0032;"),
             QString("E = mc^2"));
    QCOMPARE(i18n("E = mc^&#0050;"),
             QString("E = mc^2"));

    // Number formatting.
    QCOMPARE(ki18n("%1").subs(42).toString(),
             QString("42"));
    QCOMPARE(ki18n("%1").subs(42, 5).toString(),
             QString("   42"));
    QCOMPARE(ki18n("%1").subs(42, -5, 10, QChar('_')).toString(),
             QString("42___"));
    QCOMPARE(ki18n("%1").subs(4.2, 5, 'f', 2).toString(),
             QString(" 4.20"));
}

void KLocalizedStringTest::correctButIllFormed()
{
    // ...and some ill-formed i18n, to test markup salvage.
    // (prevent debug info about bad markup).
    KConfig dc( "kdebugrc", KConfig::SimpleConfig );
    dc.group(QString::number(173)).writeEntry("InfoOutput", 4);
    dc.sync();
    QCOMPARE(i18n("E < %1 * mc^2", 10),
             QString("E < 10 * mc^2"));
    QCOMPARE(i18n("<emphasis>%1</emphasis> &lt; mc^2", QString("<E>")),
             QString("*<E>* &lt; mc^2"));
    QCOMPARE(i18n("<emphasis>%1</emphasis> &lt; <b>mc^2</b>", QString("<E>")),
             QString("<i><E></i> &lt; <b>mc^2</b>"));
}

void KLocalizedStringTest::wrongSubs ()
{
    #ifndef NDEBUG
    // Too many arguments.
    QVERIFY(i18n("Europa", 1)
            != QString("Europa"));

    // Too few arguments.
    QVERIFY(i18n("%1, %2 and %3", QString("Hunter"), QString("Kimball"))
            != QString("Hunter, Kimball and %3"));

    // Gaps in placheholder numbering.
    QVERIFY(ki18n("Beyond the %2").subs("infinity").toString()
            != QString("Beyond the infinity"));

    // Plural argument not supplied.
    QVERIFY(ki18np("1 pod", "%1 pods").toString()
            != QString("1 pod"));
    QVERIFY(ki18np("1 pod", "%1 pods").toString()
            != QString("%1 pods"));
    #endif
}

void KLocalizedStringTest::miscMethods ()
{
    KLocalizedString k;
    QVERIFY(k.isEmpty());
}

// Same as translateToFrench, but using libintl directly (bindtextdomain+dgettext).
// Useful for debugging. This changes global state, though, so it's skipped by default.
void KLocalizedStringTest::translateToFrenchLowlevel()
{
    if (!m_hasFrench) {
        QSKIP_PORTING("l10n/fr not installed", SkipAll);
    }
    QSKIP_PORTING("skipped by default to avoid changing global state", SkipAll);
    // fr_FR locale was set by initTestCase already.
    if (QFile::exists("/usr/share/locale/fr/LC_MESSAGES/kdelibs4.mo")) {
        bindtextdomain("kdelibs4", "/usr/share/locale");
        QCOMPARE(QString::fromUtf8(dgettext("kdelibs4", "Loadable modules")), QString::fromUtf8("Modules chargeables"));
    }
}

void KLocalizedStringTest::translateToFrench()
{
    if (!m_hasFrench) {
        QSKIP_PORTING("l10n/fr not installed", SkipAll);
    }
    QCOMPARE(i18n("Loadable modules"), QString::fromUtf8("Modules chargeables"));
    QCOMPARE(i18n("Job"), QString::fromUtf8("Tâche"));
}

void KLocalizedStringTest::translateQt()
{
    QString result = KLocale::global()->translateQt("QPrintPreviewDialog", "Landscape", 0);
    // When we use the default language, translateQt returns an empty string.
    QString expected = m_hasFrench ? QString("Paysage") : QString();
    QCOMPARE(result, expected);
    result = QCoreApplication::translate("QPrintPreviewDialog", "Landscape");
    QString expected2 = m_hasFrench ? QString("Paysage") : QString("Landscape");
    QCOMPARE(result, expected2);

    // So let's use translateRaw instead for the threaded test
    QString lang;
    KLocale::global()->translateRaw("Landscape", &lang, &result);
    QCOMPARE(lang, m_hasFrench ? QString("fr") : QString("en_US")); // it finds it in kdeqt.po
    QCOMPARE(result, m_hasFrench ? QString("Paysage") : QString("Landscape"));
}

#include <QThreadPool>
#include <qtconcurrentrun.h>
#include <qstandardpaths.h>
#include <QFutureSynchronizer>

void KLocalizedStringTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QFutureSynchronizer<void> sync;
    sync.addFuture(QtConcurrent::run(this, &KLocalizedStringTest::correctSubs));
    sync.addFuture(QtConcurrent::run(this, &KLocalizedStringTest::correctSubs));
    sync.addFuture(QtConcurrent::run(this, &KLocalizedStringTest::correctSubs));
    sync.addFuture(QtConcurrent::run(this, &KLocalizedStringTest::translateQt));
    sync.addFuture(QtConcurrent::run(this, &KLocalizedStringTest::translateQt));
    sync.addFuture(QtConcurrent::run(this, &KLocalizedStringTest::translateToFrench));
    KLocale::global()->removeCatalog("kdelibs4");
    sync.waitForFinished();
    QThreadPool::globalInstance()->setMaxThreadCount(1); // delete those threads
}

QTEST_KDEMAIN_CORE_WITH_COMPONENTNAME(KLocalizedStringTest, "kdelibs4" /*so that the .po exists*/)
