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

#include <assert.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <QtCore/QEventLoop>
#include <kpushbutton.h>
#include "kreplace.h"
#include "kreplacedialog.h"

#include "kreplacetest.h"
#include <kdebug.h>
#include <stdlib.h>


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
    connect(m_replace, SIGNAL(highlight(QString, int, int)),
            this, SLOT(slotHighlight(QString, int, int)));
    // Connect findNext signal - called when pressing the button in the dialog
    connect(m_replace, SIGNAL( findNext() ),
            this, SLOT( slotReplaceNext() ) );
    // Connect replace signal - called when doing a replacement
    connect(m_replace, SIGNAL( replace(const QString &, int, int, int) ),
            this, SLOT( slotReplace(const QString &, int, int, int) ) );

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
    kDebug() << "slotHighlight Index:" << matchingIndex << " Length:" << matchedLength
              << " Substr:" << str.mid(matchingIndex, matchedLength)
              << endl;
    // Emulate the user saying yes
    // We need Qt::QueuedConnection (and the enterloop/exitloop)
    // otherwise we get an infinite loop (Match never returned,
    // so slotReplaceNext never returns)
    if ( m_replace->options() & KReplaceDialog::PromptOnReplace ) {
        KDialog* dlg = m_replace->replaceNextDialog(false);
        disconnect(dlg, SIGNAL(finished()), m_replace, 0); // hack to avoid _k_slotDialogClosed being called
        dlg->hide();

        QAbstractButton* button = dlg->button( (KDialog::ButtonCode)m_button );
        QMetaObject::invokeMethod(button, "click", Qt::QueuedConnection);

        m_needEventLoop = true;
    }
}


void KReplaceTest::slotReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength)
{
    //kDebug() << "index=" << replacementIndex << " replacedLength=" << replacedLength << " matchedLength=" << matchedLength << " text=" << text.left( 50 );
    *m_currentPos = text; // KReplace hacked the replacement into 'text' in already.
}

void KReplaceTest::slotReplaceNext()
{
    //kDebug();
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
        emit exitLoop();
}

void KReplaceTest::print()
{
    QStringList::Iterator it = m_text.begin();
    for ( ; it != m_text.end() ; ++it )
        kDebug() << *it;
}

/* button is the button that we emulate pressing, when options includes PromptOnReplace.
   Valid possibilities are User1 (replace all) and User3 (replace) */
static void testReplaceSimple( int options, int button = 0 )
{
    kDebug() << "testReplaceSimple: " << options;
    KReplaceTest test( QStringList() << QString( "hellohello" ), button );
    test.replace( "hello", "HELLO", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "HELLOHELLO" ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'HELLOHELLO'" << endl;
        exit(1);
    }
}

// Replacing "a" with "".
// input="aaaaaa", expected output=""
static void testReplaceBlank( int options, int button = 0 )
{
    kDebug() << "testReplaceBlank: " << options;
    KReplaceTest test( QStringList() << QString( "aaaaaa" ), button );
    test.replace( "a", "", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( !textLines[ 0 ].isEmpty() ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of ''" << endl;
        exit(1);
    }
}

// Replacing "" with "foo"
// input="bbbb", expected output="foobfoobfoobfoobfoo"
static void testReplaceBlankSearch( int options, int button = 0 )
{
    kDebug() << "testReplaceBlankSearch: " << options;
    KReplaceTest test( QStringList() << QString( "bbbb" ), button );
    test.replace( "", "foo", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "foobfoobfoobfoobfoo" ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'foobfoobfoobfoobfoo'" << endl;
        exit(1);
    }
}

static void testReplaceLonger( int options, int button = 0 )
{
    kDebug() << "testReplaceLonger: " << options;
    // Standard test of a replacement string longer than the matched string
    KReplaceTest test( QStringList() << QString( "aaaa" ), button );
    test.replace( "a", "bb", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "bbbbbbbb" ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'bbbbbbbb'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude( int options, int button = 0 )
{
    kDebug() << "testReplaceLongerInclude: " << options;
    // Similar test, where the replacement string includes the search string
    KReplaceTest test( QStringList() << QString( "a foo b" ), button );
    test.replace( "foo", "foobar", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "a foobar b" ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'a foobar b'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude2( int options, int button = 0 )
{
    kDebug() << "testReplaceLongerInclude2: " << options;
    // Similar test, but with more chances of matches inside the replacement string
    KReplaceTest test( QStringList() << QString( "aaaa" ), button );
    test.replace( "a", "aa", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "aaaaaaaa" ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'aaaaaaaa'" << endl;
        exit(1);
    }
}

// Test for the \0 backref
static void testReplaceBackRef( int options, int button = 0 )
{
    KReplaceTest test( QStringList() << QString( "abc def" ), button );
    test.replace( "abc", "(\\0)", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    QString expected = options & KReplaceDialog::BackReference ? "(abc) def" : "(\\0) def";
    if ( textLines[ 0 ] != expected ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of '"<< expected << "'" << endl;
        exit(1);
    }
}

// Test for other backrefs
static void testReplaceBackRef1( int options, int button = 0 )
{
    KReplaceTest test( QStringList() << QString( "a1 b2 a3" ), button );
    test.replace( "([ab])([\\d])", "\\1 and \\2 in (\\0)", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    QString expected = "a and 1 in (a1) b and 2 in (b2) a and 3 in (a3)";
    if ( textLines[ 0 ] != expected ) {
        kError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of '"<< expected << "'" << endl;
        exit(1);
    }
}

static void testReplacementHistory( const QStringList& findHistory, const QStringList& replaceHistory )
{
    KReplaceDialog dlg( 0, 0, findHistory, replaceHistory );
    dlg.show();
    kDebug() << "testReplacementHistory:" << dlg.replacementHistory();
    assert( dlg.replacementHistory() == replaceHistory );
}

static void testReplacementHistory()
{
    QStringList findHistory;
    QStringList replaceHistory;
    findHistory << "foo" << "bar";
    replaceHistory << "FOO" << "BAR";
    testReplacementHistory( findHistory, replaceHistory );

    findHistory.clear();
    replaceHistory.clear();
    findHistory << "foo" << "bar";
    replaceHistory << QString() << "baz"; // #130831
    testReplacementHistory( findHistory, replaceHistory );
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kreplacetest", 0, qi18n("KReplaceTest"), 0, qi18n("kreplace test"));
    //KApplication::disableAutoDcopRegistration();
    KApplication app;

    testReplacementHistory(); // #130831

    testReplaceBlank( 0 );
    testReplaceBlank( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBlank( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceBlank( KFind::FindBackwards, 0 );
    testReplaceBlank( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBlank( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceBlankSearch( 0 );
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceBlankSearch( KFind::FindBackwards, 0 );
    testReplaceBlankSearch( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBlankSearch( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceSimple( 0 );
    testReplaceSimple( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceSimple( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceSimple( KFind::FindBackwards, 0 );
    testReplaceSimple( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceSimple( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceLonger( 0 );
    testReplaceLonger( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceLonger( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceLonger( KFind::FindBackwards, 0 );
    testReplaceLonger( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceLonger( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceLongerInclude( 0 );
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceLongerInclude( KFind::FindBackwards, 0 );
    testReplaceLongerInclude( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceLongerInclude( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceLongerInclude2( 0 );
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceLongerInclude2( KFind::FindBackwards, 0 );
    testReplaceLongerInclude2( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceLongerInclude2( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceBackRef( 0 );
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceBackRef( KFind::FindBackwards, 0 );
    testReplaceBackRef( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBackRef( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards, 0 );
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User3 ); // replace
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialog::User1 ); // replace all

    testReplaceBackRef1( KReplaceDialog::BackReference | KFind::RegularExpression, KDialog::User3 ); // replace
    testReplaceBackRef1( KReplaceDialog::BackReference | KFind::RegularExpression, KDialog::User1 ); // replace all

    QString text = "This file is part of the KDE project.\n"
                   "This library is free software; you can redistribute it and/or\n"
                   "modify it under the terms of the GNU Library General Public\n"
                   "License version 2, as published by the Free Software Foundation.\n"
                   "\n"
                   "    This library is distributed in the hope that it will be useful,\n"
                   "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                   "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
                   "    Library General Public License for more details.\n"
                   "\n"
                   "    You should have received a copy of the GNU Library General Public License\n"
                   "    along with this library; see the file COPYING.LIB.  If not, write to\n"
                   "    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
                   "    Boston, MA 02110-1301, USA.\n"
                   "More tests:\n"
                   "ThisThis This, This. This\n"
                   "aGNU\n"
                   "free";
    KReplaceTest test( text.split( '\n' ), 0 );

    test.replace( "GNU", "KDE", 0 );
    test.replace( "free", "*free*", 0 );
    test.replace( "This", "THIS*", KFind::FindBackwards );

    test.print();
    //return app.exec();
    return 0;
}
#include "kreplacetest.moc"
