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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kcmdlineargs.h>
#include <kapplication.h>
#include "../kreplace.h"
#include "../kreplacedialog.h"

#include "kreplacetest.h"
#include <kdebug.h>

void KReplaceTest::replace( const QString &pattern, const QString &replacement, long options )
{
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
   if ( (options & KReplaceDialog::FromCursor) == 0 )
   {
       if ( m_replace->options() & KFindDialog::FindBackwards )
           m_currentPos = m_text.fromLast();
       else
           m_currentPos = m_text.begin();
   }

   // Launch first replacement
   slotReplaceNext();
}

void KReplaceTest::slotHighlight( const QString &str, int matchingIndex, int matchedLength )
{
    kdDebug() << "slotHighlight Index:" << matchingIndex << " Length:" << matchedLength
              << " Substr:" << str.mid(matchingIndex, matchedLength)
              << endl;
}


void KReplaceTest::slotReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength)
{
    kdDebug() << "slotReplace index=" << replacementIndex << " replacedLength=" << replacedLength << " matchedLength=" << matchedLength << endl;
    *m_currentPos = text; // KReplace hacked the replacement into 'text' in already.
}

void KReplaceTest::slotReplaceNext()
{
    kdDebug() << k_funcinfo << endl;
    KFind::Result res = KFind::NoMatch;
    while ( res == KFind::NoMatch && m_currentPos != m_text.end() ) {
        if ( m_replace->needData() )
            m_replace->setData( *m_currentPos );

        // Let KReplace inspect the text fragment, and display a dialog if a match is found
        res = m_replace->replace();

        if ( res == KFind::NoMatch ) {
            if ( m_replace->options() & KFindDialog::FindBackwards )
                m_currentPos--;
            else
                m_currentPos++;
        }
    }

    if ( res == KFind::NoMatch ) // i.e. at end
        if ( m_replace->shouldRestart() ) {
            if ( m_replace->options() & KFindDialog::FindBackwards )
                m_currentPos = m_text.fromLast();
            else
                m_currentPos = m_text.begin();
            slotReplaceNext();
        }
}

void KReplaceTest::print()
{
    QStringList::Iterator it = m_text.begin();
    for ( ; it != m_text.end() ; ++it )
        kdDebug() << *it << endl;
}

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, "kreplacetest", 0, 0);
  KApplication app;

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
"    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,\n"
"    Boston, MA 02111-1307, USA.";
  KReplaceTest test( QStringList::split( '\n', text, true ) );

  test.replace( "GNU", "KDE", /*KReplaceDialog::PromptOnReplace*/ 0 );
  test.replace( "free", "*free*", 0 );
  test.replace( "This", "THIS*", KFindDialog::FindBackwards );

  test.print();
  //return app.exec();
  return 0;
}
#include "kreplacetest.moc"
