/*
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kreplacetest.h"

#include <assert.h>
#include <stdlib.h>

#include <QApplication>
#include <QtCore/QEventLoop>
#include <QDebug>
#include <QPushButton>

#include <kreplace.h>
#include <kreplacedialog.h>

void KReplaceTest::enterLoop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(exitLoop()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void KReplaceTest::replace( const QString &pattern, const QString &replacement, long options )
{
    m_needEventLoop = false;
    // This creates a replace-next-prompt dialog if needed.
    delete m_replace;
    m_replace = new KReplace(pattern, replacement, options);

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_replace, SIGNAL(highlight(QString,int,int)),
            this, SLOT(slotHighlight(QString,int,int)));
    // Connect findNext signal - called when pressing the button in the dialog
    connect(m_replace, SIGNAL(findNext()),
            this, SLOT(slotReplaceNext()) );
    // Connect replace signal - called when doing a replacement
    connect(m_replace, SIGNAL(replace(QString,int,int,int)),
            this, SLOT(slotReplace(QString,int,int,int)) );

    // Go to initial position
    if ( (options & KFind::FromCursor) == 0 )
    {
        if ( m_text.isEmpty() )
            return;
        if ( m_replace->options() & KFind::FindBackwards ) {
            m_currentPos = --m_text.end();
        } else {
            m_currentPos = m_text.begin();
        }
    }

    // Launch first replacement
    slotReplaceNext();

    if ( m_needEventLoop )
        enterLoop();
}

void KReplaceTest::slotHighlight( const QString &str, int matchingIndex, int matchedLength )
{
    qDebug() << "slotHighlight Index:" << matchingIndex << " Length:" << matchedLength
             << " Substr:" << str.mid(matchingIndex, matchedLength)
             << endl;
    // Emulate the user saying yes
    // We need Qt::QueuedConnection (and the enterloop/exitloop)
    // otherwise we get an infinite loop (Match never returned,
    // so slotReplaceNext never returns)
    if ( m_replace->options() & KReplaceDialog::PromptOnReplace ) {
        QDialog* dlg = m_replace->replaceNextDialog(false);
        disconnect(dlg, SIGNAL(finished(int)), m_replace, 0); // hack to avoid _k_slotDialogClosed being called
        dlg->hide();

        QPushButton* button = dlg->findChild<QPushButton*>(m_buttonName);
        QMetaObject::invokeMethod(button, "click", Qt::QueuedConnection);

        m_needEventLoop = true;
    }
}


void KReplaceTest::slotReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength)
{
    Q_UNUSED(replacementIndex);
    Q_UNUSED(replacedLength);
    Q_UNUSED(matchedLength);
    //qDebug() << "index=" << replacementIndex << " replacedLength=" << replacedLength << " matchedLength=" << matchedLength << " text=" << text.left( 50 );
    *m_currentPos = text; // KReplace hacked the replacement into 'text' in already.
}

void KReplaceTest::slotReplaceNext()
{
    //qDebug();
    KFind::Result res = KFind::NoMatch;
    int backwards = m_replace->options() & KFind::FindBackwards;
    while ( res == KFind::NoMatch ) {
        if ( m_replace->needData() ) {
            m_replace->setData( *m_currentPos );
        }

        // Let KReplace inspect the text fragment, and display a dialog if a match is found
        res = m_replace->replace();

        if ( res == KFind::NoMatch ) {
            QStringList::iterator lastItem = backwards ? m_text.begin() : --m_text.end();
            if ( m_currentPos == lastItem )
                break;
            if ( m_replace->options() & KFind::FindBackwards ) {
                m_currentPos--;
            } else {
                m_currentPos++;
            }
        }
    }

#if 0 // commented out so that this test doesn't require interaction
    if ( res == KFind::NoMatch ) // i.e. at end
        if ( m_replace->shouldRestart() ) {
            if ( m_replace->options() & KFind::FindBackwards )
                m_currentPos = m_text.fromLast();
            else
                m_currentPos = m_text.begin();
            slotReplaceNext();
        }
#endif
    if ( res == KFind::NoMatch && m_needEventLoop )
        Q_EMIT exitLoop();
}

void KReplaceTest::print()
{
    QStringList::Iterator it = m_text.begin();
    for ( ; it != m_text.end() ; ++it )
        qDebug() << *it;
}

/* button is the button that we emulate pressing, when options includes PromptOnReplace.
   Valid possibilities are User1 (replace all) and User3 (replace) */
static void testReplaceSimple( int options, const QString &buttonName = QString() )
{
    qDebug() << "testReplaceSimple: " << options;
    KReplaceTest test( QStringList() << QLatin1String( "hellohello" ), buttonName );
    test.replace( QLatin1String("hello"), QLatin1String("HELLO"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != QLatin1String("HELLOHELLO") ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'HELLOHELLO'" << endl;
        exit(1);
    }
}

// Replacing "a" with "".
// input="aaaaaa", expected output=""
static void testReplaceBlank( int options, const QString &buttonName = QString() )
{
    qDebug() << "testReplaceBlank: " << options;
    KReplaceTest test( QStringList() << QLatin1String( "aaaaaa" ), buttonName );
    test.replace( QLatin1String("a"), QString(), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( !textLines[ 0 ].isEmpty() ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of ''" << endl;
        exit(1);
    }
}

// Replacing "" with "foo"
// input="bbbb", expected output="foobfoobfoobfoobfoo"
static void testReplaceBlankSearch( int options, const QString &buttonName = QString() )
{
    qDebug() << "testReplaceBlankSearch: " << options;
    KReplaceTest test( QStringList() << QLatin1String( "bbbb" ), buttonName );
    test.replace( QString(), QLatin1String("foo"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != QLatin1String("foobfoobfoobfoobfoo") ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'foobfoobfoobfoobfoo'" << endl;
        exit(1);
    }
}

static void testReplaceLonger( int options, const QString &buttonName = QString() )
{
    qDebug() << "testReplaceLonger: " << options;
    // Standard test of a replacement string longer than the matched string
    KReplaceTest test( QStringList() << QLatin1String( "aaaa" ), buttonName );
    test.replace( QLatin1String("a"), QLatin1String("bb"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != QLatin1String("bbbbbbbb") ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'bbbbbbbb'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude( int options, const QString &buttonName = QString() )
{
    qDebug() << "testReplaceLongerInclude: " << options;
    // Similar test, where the replacement string includes the search string
    KReplaceTest test( QStringList() << QLatin1String( "a foo b" ), buttonName );
    test.replace( QLatin1String("foo"), QLatin1String("foobar"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != QLatin1String("a foobar b") ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'a foobar b'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude2( int options, const QString &buttonName = QString() )
{
    qDebug() << "testReplaceLongerInclude2: " << options;
    // Similar test, but with more chances of matches inside the replacement string
    KReplaceTest test( QStringList() << QLatin1String( "aaaa" ), buttonName );
    test.replace( QLatin1String("a"), QLatin1String("aa"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != QLatin1String("aaaaaaaa") ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'aaaaaaaa'" << endl;
        exit(1);
    }
}

// Test for the \0 backref
static void testReplaceBackRef( int options, const QString &buttonName = QString() )
{
    KReplaceTest test( QStringList() << QLatin1String( "abc def" ), buttonName );
    test.replace( QLatin1String("abc"), QLatin1String("(\\0)"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    QString expected = options & KReplaceDialog::BackReference ? QLatin1String("(abc) def") : QLatin1String("(\\0) def");
    if ( textLines[ 0 ] != expected ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of '"<< expected << "'" << endl;
        exit(1);
    }
}

// Test for other backrefs
static void testReplaceBackRef1( int options, const QString &buttonName = QString() )
{
    KReplaceTest test( QStringList() << QLatin1String( "a1 b2 a3" ), buttonName );
    test.replace( QLatin1String("([ab])([\\d])"), QLatin1String("\\1 and \\2 in (\\0)"), options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    QString expected = QLatin1String("a and 1 in (a1) b and 2 in (b2) a and 3 in (a3)");
    if ( textLines[ 0 ] != expected ) {
        qCritical() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of '"<< expected << "'" << endl;
        exit(1);
    }
}

static void testReplacementHistory( const QStringList& findHistory, const QStringList& replaceHistory )
{
    KReplaceDialog dlg( 0, 0, findHistory, replaceHistory );
    dlg.show();
    qDebug() << "testReplacementHistory:" << dlg.replacementHistory();
    assert( dlg.replacementHistory() == replaceHistory );
}

static void testReplacementHistory()
{
    QStringList findHistory;
    QStringList replaceHistory;
    findHistory << QLatin1String("foo") << QLatin1String("bar");
    replaceHistory << QLatin1String("FOO") << QLatin1String("BAR");
    testReplacementHistory( findHistory, replaceHistory );

    findHistory.clear();
    replaceHistory.clear();
    findHistory << QLatin1String("foo") << QLatin1String("bar");
    replaceHistory << QString() << QLatin1String("baz"); // #130831
    testReplacementHistory( findHistory, replaceHistory );
}

int main( int argc, char **argv )
{
    QApplication::setApplicationName(QLatin1String("kreplacetest"));
    QApplication app(argc, argv);

    testReplacementHistory(); // #130831

    testReplaceBlank( 0 );
    testReplaceBlank( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBlank( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceBlank( KFind::FindBackwards );
    testReplaceBlank( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBlank( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceBlankSearch( 0 );
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceBlankSearch( KFind::FindBackwards );
    testReplaceBlankSearch( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBlankSearch( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceSimple( 0 );
    testReplaceSimple( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceSimple( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceSimple( KFind::FindBackwards );
    testReplaceSimple( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceSimple( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceLonger( 0 );
    testReplaceLonger( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceLonger( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceLonger( KFind::FindBackwards );
    testReplaceLonger( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceLonger( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceLongerInclude( 0 );
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceLongerInclude( KFind::FindBackwards );
    testReplaceLongerInclude( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceLongerInclude( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceLongerInclude2( 0 );
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceLongerInclude2( KFind::FindBackwards );
    testReplaceLongerInclude2( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceLongerInclude2( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceBackRef( 0 );
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceBackRef( KFind::FindBackwards );
    testReplaceBackRef( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBackRef( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards );
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("replaceButton") ); // replace
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards | KReplaceDialog::PromptOnReplace, QLatin1String("allButton") ); // replace all

    testReplaceBackRef1( KReplaceDialog::BackReference | KFind::RegularExpression, QLatin1String("replaceButton") ); // replace
    testReplaceBackRef1( KReplaceDialog::BackReference | KFind::RegularExpression, QLatin1String("allButton") ); // replace all

    QString text = QLatin1String("This file is part of the KDE project.\n") +
                   QLatin1String("This library is free software; you can redistribute it and/or\n") +
                   QLatin1String("modify it under the terms of the GNU Library General Public\n") +
                   QLatin1String("License version 2, as published by the Free Software Foundation.\n") +
                   QLatin1Char('\n') +
                   QLatin1String("    This library is distributed in the hope that it will be useful,\n") +
                   QLatin1String("    but WITHOUT ANY WARRANTY; without even the implied warranty of\n") +
                   QLatin1String("    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n") +
                   QLatin1String("    Library General Public License for more details.\n") +
                   QLatin1Char('\n') +
                   QLatin1String("    You should have received a copy of the GNU Library General Public License\n") +
                   QLatin1String("    along with this library; see the file COPYING.LIB.  If not, write to\n") +
                   QLatin1String("    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n") +
                   QLatin1String("    Boston, MA 02110-1301, USA.\n") +
                   QLatin1String("More tests:\n") +
                   QLatin1String("ThisThis This, This. This\n") +
                   QLatin1String("aGNU\n") +
                   QLatin1String("free");
    KReplaceTest test( text.split( QLatin1Char('\n') ), QLatin1String("0") );

    test.replace( QLatin1String("GNU"), QLatin1String("KDE"), 0 );
    test.replace( QLatin1String("free"), QLatin1String("*free*"), 0 );
    test.replace( QLatin1String("This"), QLatin1String("THIS*"), KFind::FindBackwards );

    test.print();
    //return app.exec();
    return 0;
}
