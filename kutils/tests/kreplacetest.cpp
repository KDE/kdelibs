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
#include <qeventloop.h>
#include <kpushbutton.h>
#include "../kreplace.h"
#include "../kreplacedialog.h"

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
    m_replace = new KReplace(pattern, replacement, options);

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_replace, SIGNAL( highlight( const QString &, int, int ) ),
            this, SLOT( slotHighlight( const QString &, int, int ) ) );
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
    kdDebug() << "slotHighlight Index:" << matchingIndex << " Length:" << matchedLength
              << " Substr:" << str.mid(matchingIndex, matchedLength)
              << endl;
    // Emulate the user saying yes
    // animateClick triggers a timer, hence the enterloop/exitloop
    // Calling slotReplace directly would lead to infinite loop anyway (Match never returned,
    // so slotReplaceNext never returns)
    if ( m_replace->options() & KReplaceDialog::PromptOnReplace ) {
        m_replace->replaceNextDialog( false )->actionButton( (KDialogBase::ButtonCode)m_button )->animateClick();
        m_needEventLoop = true;
    }
}


void KReplaceTest::slotReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength)
{
    kdDebug() << "slotReplace index=" << replacementIndex << " replacedLength=" << replacedLength << " matchedLength=" << matchedLength << " text=" << text.left( 50 ) << endl;
    *m_currentPos = text; // KReplace hacked the replacement into 'text' in already.
}

void KReplaceTest::slotReplaceNext()
{
    //kdDebug() << k_funcinfo << endl;
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
        kdDebug() << *it << endl;
}

/* button is the button that we emulate pressing, when options includes PromptOnReplace.
   Valid possibilities are User1 (replace all) and User3 (replace) */
static void testReplaceSimple( int options, int button = 0 )
{
    kdDebug() << "testReplaceSimple: " << options << endl;
    KReplaceTest test( QStringList() << QString( "hellohello" ), button );
    test.replace( "hello", "HELLO", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "HELLOHELLO" ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'HELLOHELLO'" << endl;
        exit(1);
    }
}

// Replacing "a" with "".
// input="aaaaaa", expected output=""
static void testReplaceBlank( int options, int button = 0 )
{
    kdDebug() << "testReplaceBlank: " << options << endl;
    KReplaceTest test( QStringList() << QString( "aaaaaa" ), button );
    test.replace( "a", "", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( !textLines[ 0 ].isEmpty() ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of ''" << endl;
        exit(1);
    }
}

// Replacing "" with "foo"
// input="bbbb", expected output="foobfoobfoobfoobfoo"
static void testReplaceBlankSearch( int options, int button = 0 )
{
    kdDebug() << "testReplaceBlankSearch: " << options << endl;
    KReplaceTest test( QStringList() << QString( "bbbb" ), button );
    test.replace( "", "foo", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "foobfoobfoobfoobfoo" ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'foobfoobfoobfoobfoo'" << endl;
        exit(1);
    }
}

static void testReplaceLonger( int options, int button = 0 )
{
    kdDebug() << "testReplaceLonger: " << options << endl;
    // Standard test of a replacement string longer than the matched string
    KReplaceTest test( QStringList() << QString( "aaaa" ), button );
    test.replace( "a", "bb", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "bbbbbbbb" ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'bbbbbbbb'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude( int options, int button = 0 )
{
    kdDebug() << "testReplaceLongerInclude: " << options << endl;
    // Similar test, where the replacement string includes the search string
    KReplaceTest test( QStringList() << QString( "a foo b" ), button );
    test.replace( "foo", "foobar", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "a foobar b" ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'a foobar b'" << endl;
        exit(1);
    }
}

static void testReplaceLongerInclude2( int options, int button = 0 )
{
    kdDebug() << "testReplaceLongerInclude2: " << options << endl;
    // Similar test, but with more chances of matches inside the replacement string
    KReplaceTest test( QStringList() << QString( "aaaa" ), button );
    test.replace( "a", "aa", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    if ( textLines[ 0 ] != "aaaaaaaa" ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of 'aaaaaaaa'" << endl;
        exit(1);
    }
}

// Test for the \0 backref
static void testReplaceBackRef( int options, int button = 0 )
{
    kdDebug() << "testReplaceBackRef: " << options << endl;
    KReplaceTest test( QStringList() << QString( "abc def" ), button );
    test.replace( "abc", "(\\0)", options );
    QStringList textLines = test.textLines();
    assert( textLines.count() == 1 );
    QString expected = options & KReplaceDialog::BackReference ? "(abc) def" : "(\\0) def";
    if ( textLines[ 0 ] != expected ) {
        kdError() << "ASSERT FAILED: replaced text is '" << textLines[ 0 ] << "' instead of '"<< expected << "'" << endl;
        exit(1);
    }
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kreplacetest", "KReplaceTest", "kreplace test", 0);
    KApplication app;

    testReplaceBlank( 0 );
    testReplaceBlank( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBlank( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceBlank( KFind::FindBackwards, 0 );
    testReplaceBlank( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBlank( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

    testReplaceBlankSearch( 0 );
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBlankSearch( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceBlankSearch( KFind::FindBackwards, 0 );
    testReplaceBlankSearch( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBlankSearch( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

    testReplaceSimple( 0 );
    testReplaceSimple( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceSimple( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceSimple( KFind::FindBackwards, 0 );
    testReplaceSimple( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceSimple( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

    testReplaceLonger( 0 );
    testReplaceLonger( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceLonger( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceLonger( KFind::FindBackwards, 0 );
    testReplaceLonger( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceLonger( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

    testReplaceLongerInclude( 0 );
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceLongerInclude( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceLongerInclude( KFind::FindBackwards, 0 );
    testReplaceLongerInclude( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceLongerInclude( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

    testReplaceLongerInclude2( 0 );
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceLongerInclude2( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceLongerInclude2( KFind::FindBackwards, 0 );
    testReplaceLongerInclude2( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceLongerInclude2( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

    testReplaceBackRef( 0 );
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBackRef( KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceBackRef( KFind::FindBackwards, 0 );
    testReplaceBackRef( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBackRef( KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBackRef( KReplaceDialog::BackReference | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards, 0 );
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User3 ); // replace
    testReplaceBackRef( KReplaceDialog::BackReference | KFind::FindBackwards | KReplaceDialog::PromptOnReplace, KDialogBase::User1 ); // replace all

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
    KReplaceTest test( QStringList::split( '\n', text, true ), 0 );

    test.replace( "GNU", "KDE", 0 );
    test.replace( "free", "*free*", 0 );
    test.replace( "This", "THIS*", KFind::FindBackwards );

    test.print();
    //return app.exec();
    return 0;
}
#include "kreplacetest.moc"
