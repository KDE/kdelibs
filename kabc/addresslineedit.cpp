/*
    This file is part of libkabc.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
		  2002 Lubos Lunak <llunak@suse.cz>
                  2001 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Waldo Bastian <bastian@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#include "addresslineedit.h"

#include <qobject.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qevent.h>
#include <qdragobject.h>

#include <kcompletionbox.h>
#include <kstdaccel.h>
#include <kurldrag.h>

#include <kabc/stdaddressbook.h>
#include <kabc/distributionlist.h>
#include <kurldrag.h>
#include "ldapclient.h"

#include <kdebug.h>

//=============================================================================
//
//   Class  AddressLineEdit
//
//=============================================================================


using namespace KABC;

KCompletion * AddressLineEdit::s_completion = 0L;
bool AddressLineEdit::s_addressesDirty = false;
QTimer* AddressLineEdit::s_LDAPTimer = 0L;
LdapSearch* AddressLineEdit::s_LDAPSearch = 0L;
QString* AddressLineEdit::s_LDAPText = 0L;
AddressLineEdit* AddressLineEdit::s_LDAPLineEdit = 0L;

AddressLineEdit::AddressLineEdit(QWidget* parent,
		bool useCompletion,
		const char *name)
    : KLineEdit(parent,name)
{
  m_useCompletion = useCompletion;
  m_completionInitialized = false;
  m_smartPaste = false;

  init();

  // Whenever a new AddressLineEdit is created (== a new composer is created),
  // we set a dirty flag to reload the addresses upon the first completion.
  // The address completions are shared between all AddressLineEdits.
  // Is there a signal that tells us about addressbook updates?
  if (m_useCompletion)
    s_addressesDirty = true;
}


//-----------------------------------------------------------------------------
void AddressLineEdit::init()
{
  if ( !s_completion ) {
      s_completion = new KCompletion();
      s_completion->setOrder( KCompletion::Sorted );
      s_completion->setIgnoreCase( true );
  }

  if( m_useCompletion ) {
      if( !s_LDAPTimer ) {
        s_LDAPTimer = new QTimer;
        s_LDAPSearch = new LdapSearch;
        s_LDAPText = new QString;
      }
      connect( s_LDAPTimer, SIGNAL( timeout()), SLOT( slotStartLDAPLookup()));
      connect( s_LDAPSearch, SIGNAL( searchData( const QStringList& )),
        SLOT( slotLDAPSearchData( const QStringList& )));
  }

  if ( m_useCompletion && !m_completionInitialized )
  {
      setCompletionObject( s_completion, false ); // we handle it ourself
      connect( this, SIGNAL( completion(const QString&)),
               this, SLOT(slotCompletion() ));

      KCompletionBox *box = completionBox();
      connect( box, SIGNAL( highlighted( const QString& )),
               this, SLOT( slotPopupCompletion( const QString& ) ));
      connect( box, SIGNAL( userCancelled( const QString& )),
               SLOT( setText( const QString& )));

      m_completionInitialized = true; // don't connect muliple times. That's
                                      // ugly, tho, better have completionBox()
                                      // virtual in KDE 4
      // Why? This is only called once. Why should this be called more 
      // than once? And why was this protected?
      // And while I'm at it: who deletes all those static objects? (pfeiffer)
  }
}

//-----------------------------------------------------------------------------
AddressLineEdit::~AddressLineEdit()
{
}

//-----------------------------------------------------------------------------
void AddressLineEdit::setFont( const QFont& font )
{
    KLineEdit::setFont( font );
    if ( m_useCompletion )
        completionBox()->setFont( font );
}

//-----------------------------------------------------------------------------
void AddressLineEdit::keyPressEvent(QKeyEvent *e)
{
    bool accept = false;

    if (KStdAccel::shortcut(KStdAccel::SubstringCompletion).contains(KKey(e)))
    {
      doCompletion(true);
      accept = true;
    }
    else if (e->state()==ControlButton && e->key() == Key_Right)
    {
      if ((int)text().length() == cursorPosition()) // at End?
      {
        doCompletion(true);
	accept = true;
      }
    }
    else if (e->state()==ControlButton && e->key() == Key_V)
    {
      if (m_useCompletion)
         m_smartPaste = true;
      paste();
      m_smartPaste = false;
      accept = true;
    }

    if( !accept )
        KLineEdit::keyPressEvent( e );

    if( e->isAccepted())
    {
        if( m_useCompletion && s_LDAPTimer != NULL )
	{
            if( *s_LDAPText != text())
                stopLDAPLookup();
	    *s_LDAPText = text();
	    s_LDAPLineEdit = this;
	    s_LDAPTimer->start( 1000, true );
	}
	m_typedText = text();
    }
}

void AddressLineEdit::mouseReleaseEvent( QMouseEvent * e )
{
   if (m_useCompletion && (e->button() == MidButton))
   {
      m_smartPaste = true;
      KLineEdit::mouseReleaseEvent(e);
      m_smartPaste = false;
      return;
   }
   KLineEdit::mouseReleaseEvent(e);
}

void AddressLineEdit::insert(const QString &t)
{
    if (!m_smartPaste)
    {
       KLineEdit::insert(t);
       return;
    }
    QString newText = t.stripWhiteSpace();
    if (newText.isEmpty())
       return;

    // remove newlines in the to-be-pasted string as well as an eventual
    // mailto: protocol
    newText.replace( QRegExp("\r?\n"), " " );
    if ( newText.startsWith( "mailto:" ) )
        newText.remove( 0, 7 );
    else if (newText.contains(" at "))
    {
       // Anti-spam stuff
       newText.replace( QRegExp(" at "), "@" );
       newText.replace( QRegExp(" dot "), "." );
    }
    else if (newText.contains("(at)"))
    {
      newText.replace( QRegExp("\\s*\\(at\\)\\s*"), "@" );
    }

    QString contents = text();
    int start_sel = 0;
    int end_sel = 0;
    int pos = cursorPosition();
    if (getSelection(&start_sel, &end_sel))
    {
       // Cut away the selection.
       if (pos > end_sel)
          pos -= (end_sel - start_sel);
       else if (pos > start_sel)
          pos = start_sel;
       contents = contents.left(start_sel) + contents.right(end_sel+1);
    }

    int eot = contents.length();
    while ((eot > 0) && contents[eot-1].isSpace()) eot--;
    if (eot == 0)
    {
       contents = QString::null;
    }
    else if (pos >= eot)
    {
       if (contents[eot-1] == ',')
          eot--;
       contents.truncate(eot);
       contents += ", ";
       pos = eot+2;
    }

    contents = contents.left(pos)+newText+contents.mid(pos);
    setText(contents);
    setCursorPosition(pos+newText.length());
}

void AddressLineEdit::paste()
{
    if (m_useCompletion)
       m_smartPaste = true;
    KLineEdit::paste();
    m_smartPaste = false;
}

//-----------------------------------------------------------------------------
void AddressLineEdit::cursorAtEnd()
{
    setCursorPosition( text().length() );
}

//-----------------------------------------------------------------------------
void AddressLineEdit::enableCompletion(bool enable)
{
  m_useCompletion = enable;
}

//-----------------------------------------------------------------------------
void AddressLineEdit::doCompletion(bool ctrlT)
{
    if ( !m_useCompletion )
        return;

    QString s(m_typedText);
    QString prevAddr;
    int n = s.findRev(',');
    if (n>= 0)
    {
        prevAddr = s.left(n+1) + ' ';
        s = s.mid(n+1,255).stripWhiteSpace();
    }

    KCompletionBox *box = completionBox();

    if ( s.isEmpty() )
    {
        box->hide();
        return;
    }

    KGlobalSettings::Completion  mode = completionMode();

    if ( s_addressesDirty )
        loadAddresses();

    QString match;
    int curPos = cursorPosition();
    if ( mode != KGlobalSettings::CompletionNone )
    {
        match = s_completion->makeCompletion( s );
        if (match.isNull() && mode == KGlobalSettings::CompletionPopup)
          match = s_completion->makeCompletion( "\"" + s );
        if (match.isNull() && mode == KGlobalSettings::CompletionPopup)
          match = s_completion->makeCompletion( "$$" + s );
    }

    kdDebug() << "** completion for: " << s << " : " << match << endl;

    if ( ctrlT )
    {
        QStringList addresses = s_completion->items();
        QStringList::Iterator it = addresses.begin();
        QStringList completions;
        for (; it != addresses.end(); ++it)
        {
            if ((*it).find(s,0,false) >= 0)
                completions.append( *it );
        }

        if (completions.count() > 1) {
            m_previousAddresses = prevAddr;
            box->setItems( completions );
            box->setCancelledText( text() );
            box->popup();
        }
        else if (completions.count() == 1)
            setText(prevAddr + completions.first());
        else
            box->hide();

        cursorAtEnd();
        return;
    }

    switch ( mode )
    {
        case KGlobalSettings::CompletionPopup:
        {
            if ( !match.isNull() )
            {
                m_previousAddresses = prevAddr;
		QStringList items = s_completion->allMatches( s );
                items += s_completion->allMatches( "\"" + s );
		items += s_completion->substringCompletion( '<' + s );
		if( !s.contains( ' ' )) // one word, possibly given name
		    items += s_completion->allMatches( "$$" + s );
    		for( QStringList::Iterator it = items.begin();
		     it != items.end();
		     ++it )
		{ // remove the '$$whatever$' part
		    int pos = (*it).find( '$', 2 );
		    if( pos < 0 ) // ???
		        continue;
		    (*it)=(*it).mid( pos + 1 );
		}
		items = removeMailDupes( items );
		box->setItems( items );
                box->setCancelledText( text() );
                box->popup();
            }
            else
                box->hide();

            break;
        }

        case KGlobalSettings::CompletionShell:
        {
            if ( !match.isNull() && match != s )
            {
                setText( prevAddr + match );
                cursorAtEnd();
            }
            break;
        }

        case KGlobalSettings::CompletionMan: // Short-Auto in fact
        case KGlobalSettings::CompletionAuto:
        {
            if ( !match.isNull() && match != s )
            {
                QString adds = prevAddr + match;
                validateAndSet( adds, curPos, curPos, adds.length() );
            }
            break;
        }

        default: // fall through
        case KGlobalSettings::CompletionNone:
            break;
    }
}

//-----------------------------------------------------------------------------
void AddressLineEdit::slotPopupCompletion( const QString& completion )
{
    setText( m_previousAddresses + completion );
    cursorAtEnd();
}

//-----------------------------------------------------------------------------
void AddressLineEdit::loadAddresses()
{
    s_completion->clear();
    s_addressesDirty = false;

    QStringList adrs = addresses();
    for( QStringList::ConstIterator it = adrs.begin();
	 it != adrs.end();
	 ++it)
    	addAddress( *it );
}

void AddressLineEdit::addAddress( const QString& adr )
{
    s_completion->addItem( adr );
}

void AddressLineEdit::slotStartLDAPLookup()
{
    if( !s_LDAPSearch->isAvailable() || s_LDAPLineEdit != this )
	return;
    startLoadingLDAPEntries();
}

void AddressLineEdit::stopLDAPLookup()
{
    s_LDAPSearch->cancelSearch();
    s_LDAPLineEdit = NULL;
}

void AddressLineEdit::startLoadingLDAPEntries()
{
    QString s( *s_LDAPText );
    // TODO cache last?
    QString prevAddr;
    int n = s.findRev(',');
    if (n>= 0)
    {
        prevAddr = s.left(n+1) + ' ';
        s = s.mid(n+1,255).stripWhiteSpace();
    }
    if( s.length() == 0 )
	return;
    loadAddresses(); // TODO reuse these?
    s_LDAPSearch->startSearch( s );
}

void AddressLineEdit::slotLDAPSearchData( const QStringList& adrs )
{
    if( s_LDAPLineEdit != this )
        return;
    for( QStringList::ConstIterator it = adrs.begin();
	 it != adrs.end();
	 ++it )
	addAddress( *it );
    doCompletion( false );
}

QStringList AddressLineEdit::removeMailDupes( const QStringList& adrs )
{
    QStringList src = adrs;
    qHeapSort( src );
    QString last;
    for( QStringList::Iterator it = src.begin();
	 it != src.end();
	 )
    {
	if( *it == last )
	{
	    it = src.remove( it );
	    continue; // dupe
	}
	last = *it;
	++it;
    }
    return src;
}

//-----------------------------------------------------------------------------
void AddressLineEdit::dropEvent(QDropEvent *e)
{
  KURL::List uriList;
  if(KURLDrag::canDecode(e) && KURLDrag::decode( e, uriList ))
  {
    QString ct = text();
    KURL::List::Iterator it = uriList.begin();
    for (; it != uriList.end(); ++it)
    {
      if (!ct.isEmpty()) ct.append(", ");
      KURL u(*it);
      if ((*it).protocol() == "mailto") 
          ct.append( (*it).path() );
      else 
          ct.append( (*it).url() );
    }
    setText(ct);
  }
  else {
    if (m_useCompletion)
       m_smartPaste = true;
    QLineEdit::dropEvent(e);
    m_smartPaste = false;
  }
}


QStringList AddressLineEdit::addresses()
{
  QStringList result;

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  KABC::AddressBook::Iterator it;
  for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    QStringList emails = (*it).emails();
    QString n = (*it).prefix() + " " +
		(*it).givenName() + " " +
		(*it).additionalName() + " " +
	        (*it).familyName() + " " +
		(*it).suffix();
    n = n.simplifyWhiteSpace();

    QRegExp needQuotes("[^ 0-9A-Za-z\\x0080-\\xFFFF]");
    QString endQuote = "\" ";
    QString empty = "";
    QStringList::ConstIterator mit;
    QString addr, email;

    for ( mit = emails.begin(); mit != emails.end(); ++mit ) {
      email = *mit;
      if (!email.isEmpty()) {
	if (n.isEmpty() || (email.find( '<' ) != -1))
	  addr = empty;
	else { /* do we really need quotes around this name ? */
          if (n.find(needQuotes) != -1)
	    addr = '"' + n + endQuote;
	  else
	    addr = n + ' ';
	}

	if (!addr.isEmpty() && (email.find( '<' ) == -1)
	    && (email.find( '>' ) == -1)
	    && (email.find( ',' ) == -1))
	  addr += '<' + email + '>';
	else
	  addr += email;
	addr = addr.stripWhiteSpace();
	result.append( addr );
      }
    }
  }
  KABC::DistributionListManager manager( addressBook );
  manager.load();

  QStringList names = manager.listNames();
  QStringList::Iterator jt;
  for ( jt = names.begin(); jt != names.end(); ++jt)
    result.append( *jt );
  result.sort();

  return result;
}

#include "addresslineedit.moc"
