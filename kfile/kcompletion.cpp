/* This file is part of the KDE libraries
    Copyright (C) 1999 Carsten Pfeiffer <pfeiffer@kde.org>

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


#include <kapp.h>
#include "kcompletion.h"

template class QList<KCompFork>;

KCompletion::KCompletion()
{
    setCompletionMode( KGlobal::completionMode() );
    myTreeRoot = new KCompTreeNode;
    myForkList.setAutoDelete( false );
    mySorting = true; // don't sort the items by default (FIXME, -> false)
    myBeep = true;
}

KCompletion::~KCompletion()
{
    delete myTreeRoot;
}


void KCompletion::setItemList( const QStringList& items )
{
    myLastMatches.clear();
    myForkList.clear();
    myLastString = QString::null;

    QStringList::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it )
        addItemInternal( *it );
}


void KCompletion::addItem( const QString& item )
{
    myLastMatches.clear();
    myForkList.clear();
    myLastString = QString::null;

    addItemInternal( item );
}


void KCompletion::addItemInternal( const QString& item )
{
    QChar ch;
    KCompTreeNode *node = myTreeRoot;

    for ( uint i = 0; i < item.length(); i++ ) {
        ch = item.at( i );
	node = node->insert( ch, mySorting );
    }

    node->insert( 0x0, true ); // add 0x0-item as delimiter
}


void KCompletion::removeItem( const QString& item )
{
    myLastMatches.remove( item );
    myForkList.clear();
    myLastString = QString::null;

    myTreeRoot->remove( item );
}


void KCompletion::clear()
{
    myLastMatches.clear();
    myForkList.clear();
    myLastString = QString::null;

    delete myTreeRoot;
    myTreeRoot = new KCompTreeNode;
}


QString KCompletion::makeCompletion( const QString& string )
{
    if ( myCompletionMode == KGlobal::CompletionNone )
        return QString::null;

    debug("KCompletion: completing: %s", string.ascii());

    QStringList tmp = findAllCompletions( string );

    myLastString = string;
    QString completion = QString::null;

    if ( !string.isEmpty() ) { // only emit single match when string != ""
      myIterator = tmp.begin();

	if ( myIterator != tmp.end() )
	    completion = *myIterator;
	
	debug("KCompletion: Match: %s",
	      completion.isNull() ? "(null)" : completion.ascii());

        emit match( completion );
    }

    if ( completion.isNull() )
        doBeep();
    
    return completion;
}


QString KCompletion::nextMatch()
{
  /* to come....
  QString string = findCompletion( myForkList.next() );
  if ( string.isNull() ) // rotation
    string = findAllCompletions( myLastString ).first(); // FIXME

  return string;
  */

    if ( myLastMatches.isEmpty() ) {
        doBeep();
	return QString::null;
    }

    if ( myIterator == myLastMatches.fromLast() )
	myIterator = myLastMatches.begin();
    else
	++myIterator;

    emit match( *myIterator );
    return *myIterator;

}


QString KCompletion::previousMatch()
{
  /* to come...
  QString string = findCompletion( myForkList.previous() );
  if ( string.isNull() ) // rotation
    string = findAllCompletions( myLastString ).first(); // FIXME

  return string;
  */

    if ( myLastMatches.isEmpty() ) {
        doBeep();
	return QString::null;
    }

    if ( myIterator == myLastMatches.begin() )
	myIterator = myLastMatches.fromLast();
    else
	--myIterator;

    emit match( *myIterator );
    return *myIterator;

}



/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////



// finds the completion of a given KCompFork, respecting indices for
// previous and next usage.
QString KCompletion::findCompletion( KCompFork *fork )
{
    ASSERT( fork != 0L );

    QChar ch;
    QString string = fork->string;
    QString completion;
    const KCompTreeNode *node = fork->node;
    for( uint i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
	node = node->find( ch );

       if ( node )
	    completion += ch;
       else
	    break;
    }

    // debug("* first Part: %s (now %c)", completion.ascii(), node->latin1());

    // ok, now we have the last node of the to be completed string
    // follow it until 0x0

    while ( node && node->childrenCount() == 1 ) {
        node = node->child();
	if ( !node->isNull() )
	    completion += *node;
	// debug("-> %s", completion.ascii());
    }

    return completion;
}


// ------------------------------------------------ //


const QStringList& KCompletion::findAllCompletions( const QString& string )
{
    if ( string == myLastCompletion )
        return myMatches;

    myMatches.clear();

    if ( string.isEmpty() )
        return myMatches;

    QChar ch;
    QString completion;
    const KCompTreeNode *node = myTreeRoot;

    // start at the tree-root and try to find the search-string
    for( uint i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
	node = node->find( ch );

	if ( node )
	    completion += ch;
	else
	    return myMatches; // no completion -> return empty list
    }
	
    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while ( node && node->childrenCount() == 1 ) {
        node = node->child();
	if ( !node->isNull() )
	    completion += *node;
	// debug("-> %s, %c", completion.ascii(), node->latin1());
    }

    myLastCompletion = completion;

    // only manual completion is interested in a partial match (or
    // auto-completion, when there is just one single match)
    if ( myCompletionMode == KGlobal::CompletionEOL || !node ||
	 (node && node->isNull()) )
        myMatches.append( completion );

    // debug("* last non-ambiguous completion: %s (now %c)", completion.ascii(), node ? node->latin1() : 0 );


    // ok, node is now either 0L, 0x0 (= leaf) or has more than one child
    // (= partial match) -> recursively find all remaining completions
    if ( node && node->childrenCount() > 0 ) {
        myForkList.append( completion, node );
        extractStringsFromNode( completion, node );
        doBeep(); // partial match -> beep
    }

    return myMatches;
}


void KCompletion::extractStringsFromNode( const QString& beginning,
					  const KCompTreeNode *node )
{
    if ( !node )
        return;

    // debug("Beginning: %s", beginning.ascii());
    KCompTreeChildren::ConstIterator it;
    const KCompTreeChildren *list = node->children();
    QString string;

    // loop thru all children
    for ( it = list->begin(); it != list->end(); ++it ) {
        string = beginning;
        node = *it;
	string += *node;

	while ( node && node->childrenCount() == 1 ) {
	    node = node->child();

	    if ( !node->isNull() )
	        string += *node;

	    else { // we found a leaf
	        myMatches.append( string );
		// debug( " -> found match: %s", string.ascii() );
	    }
	}

	// recursively find all other strings.
	if ( node && node->childrenCount() > 1 ) {
	    myForkList.append( string, node );
	    extractStringsFromNode( string, node );
	}
    }
}


void KCompletion::doBeep()
{
    if ( myBeep && myCompletionMode == KGlobal::CompletionEOL )
        kapp->beep();
}

/////////////////////////////////
/////////


// Implements the tree. Every node is a QChar and has a list of children, which
// are Nodes as well.
// QChar( 0x0 ) is used as the delimiter of a string; the last child of each
// inserted string is 0x0.

KCompTreeNode::KCompTreeNode()
  : QChar()
{
}

KCompTreeNode::KCompTreeNode( const QChar& ch )
  : QChar( ch )
{
}


KCompTreeNode::~KCompTreeNode()
{
    // delete all children
    KCompTreeChildren::Iterator it;
    for ( it = myChildren.begin(); it != myChildren.end(); ++it )
        delete *it;
}


// Adds a child-node "ch" to this node. If such a node is already existant,
// it will not be created. Returns the new/existing node.
KCompTreeNode * KCompTreeNode::insert( const QChar& ch, bool sorted )
{
    KCompTreeNode *child = find( ch );
    if ( !child ) {
        child = new KCompTreeNode( ch );

	// FIXME, first (slow) sorted insertion
	if ( sorted ) {
	    KCompTreeChildren::Iterator it = myChildren.begin();
	    while ( it != myChildren.end() ) {
	        if ( ch > *(*it) )
		    ++it;
		else
		    break;
	    }
	    myChildren.insert( it, child );
	}

	else
	    myChildren.append( child );
    }

    return child;
}


// Recursively removes a string from the tree (untested :-)
void KCompTreeNode::remove( const QString& string )
{
    KCompTreeNode *child = 0L;

    if ( string.isEmpty() ) {
        child = find( 0x0 );
	myChildren.remove( child );
	return;
    }

    QChar ch = string.at(0);
    child = find( ch );
    if ( child ) {
        child->remove( string.right( string.length() -1 ) );
	if ( child->myChildren.count() == 0 ) {
	    delete child;
	    myChildren.remove( child );
	}
    }
}


// Returns a child of this node matching ch, if available.
// Otherwise, returns 0L
KCompTreeNode * KCompTreeNode::find( const QChar& ch ) const
{
    KCompTreeChildren::ConstIterator it;
    for ( it = myChildren.begin(); it != myChildren.end(); ++it )
        if ( *(*it) == ch )
	    return *it;

    return 0L;
}


#include "kcompletion.moc"
