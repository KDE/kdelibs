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
#include <kdebug.h>
#include "kcompletion.h"

template class QList<KCompFork>;


KCompletion::KCompletion()
{
    myCompletionMode = KGlobal::completionMode();
    myTreeRoot = new KCompTreeNode;
    myForkList.setAutoDelete( true );
    mySorting    = true; // don't sort the items by default (FIXME, -> false)
    myBackwards  = false;
    myBeep       = true;
    myIgnoreCase = false;
    myItemIndex  = 0;
}

KCompletion::~KCompletion()
{
    delete myTreeRoot;
}


void KCompletion::setItems( const QStringList& items )
{
    myForkList.clear();
    myLastString = QString::null;

    QStringList::ConstIterator it;
    for ( it = items.begin(); it != items.end(); ++it )
        addItemInternal( *it );
}


QStringList KCompletion::items() const
{
    QStringList list;
    extractStringsFromNode( myTreeRoot, QString::null, &list, true );

    return list;
}


void KCompletion::addItem( const QString& item )
{
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
    myForkList.clear();
    myLastString = QString::null;

    myTreeRoot->remove( item );
}


void KCompletion::clear()
{
    myForkList.clear();
    myLastString = QString::null;

    delete myTreeRoot;
    myTreeRoot = new KCompTreeNode;
}


QString KCompletion::makeCompletion( const QString& string )
{
    if ( myCompletionMode == KGlobal::CompletionNone )
        return QString::null;

    kDebugInfo(250, "KCompletion: completing: %s", debugString( string ));

    if ( string.isEmpty() ) // reset for rotation
	myForkList.clear();
    
    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if ( myCompletionMode == KGlobal::CompletionShell &&
	 string == myLastString ) {
        myMatches = findAllCompletions( string );
	emit matches( myMatches );
    }

    QString completion = findCompletion( string );
    if ( !myForkList.isEmpty() )
        emit multipleMatches();

    myLastString = string;

    if ( !string.isEmpty() ) { // only emit match when string != ""
	debug("KCompletion: Match: %s", debugString( completion ));

        emit match( completion );
    }

    if ( completion.isNull() )
        doBeep();

    return completion;
}


void KCompletion::setCompletionMode( KGlobal::Completion mode )
{
    myCompletionMode = mode;
    myForkList.clear(); // we would get in trouble otherwise
}



/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////


QString KCompletion::nextMatch()
{
    QString completion;

    if ( myForkList.count() == 0 ) {
        kDebugInfo(250, "KCompletion::nextMatch(): no forks available");
	completion = findCompletion( myLastString );
	emit match( completion );
	return completion;
    }

    KCompFork *fork = myForkList.current();
    if ( !fork )
        fork = myForkList.last();

    if ( fork->string.find( QString::fromLatin1("kfiledialog" )) == 0 )
      kDebugInfo(250, "  fork: %s, index: %i (children: %i)", fork->string.ascii(), fork->index, fork->node->childrenCount());
    fork->index++;

    // if no further child of this fork -> jump to previous fork(s)
    while ( fork->index >= (fork->node->childrenCount()) ) {
        fork->index = fork->node->childrenCount();
        KCompFork *fork2 = myForkList.prev();
	if ( fork2 ) {
	    fork = fork2;
	    fork->index++;
    if ( fork->string.find( QString::fromLatin1("kfiledialog" )) == 0 )
	    kDebugInfo(250, "             ++ (fork: %s), %i",fork->string.ascii(), fork->index);
	}
	
	else { // no previous fork available -> rotation
    if ( fork->string.find( QString::fromLatin1("kfiledialog")) == 0 )
kDebugInfo(250, "   -> fork: %s, index: %i (children: %i)", fork->string.ascii(), fork->index, fork->node->childrenCount());
	    completion = findCompletion( myLastString );
	    emit match( completion );
	    return completion;
	}
    }

    int index = fork->index;
    const KCompTreeNode *node = fork->node;
    ASSERT( node != 0L && node->childrenCount() > index );
    node = node->childAt( index );

    completion = fork->string;

    while ( !node->isNull() ) {
        completion += *node;

	if ( node->childrenCount() > 1 )
	    (void) myForkList.append( completion, node, 0 );
	
	node = node->firstChild();
    }

    if ( fork->string.find( QString::fromLatin1("kfiledialog")) == 0 )
    kDebugInfo(250, " -> completed: %s", completion.ascii());
    myItemIndex++;

    emit match( completion );
    return completion;
}



QString KCompletion::previousMatch()
{
    QString completion;

    if ( myForkList.count() == 0 || myItemIndex == 0 ) {
      kDebugInfo(250, "     myItemIndex: %i", myItemIndex);
        myBackwards = true;
        kDebugInfo(250, "KCompletion::previousMatch(): no forks available");
	completion = findCompletion( myLastString );
	myBackwards = false;

	emit match( completion );
	return completion;
    }

    KCompFork *fork = myForkList.current();
    if ( !fork )
        fork = myForkList.first();

    if ( fork->string.find( QString::fromLatin1("kfiledialog")) == 0 )
kDebugInfo(250, "  fork: %s, index: %i (children: %i)", fork->string.ascii(), fork->index, fork->node->childrenCount());
    fork->index--;

    // if we have traveled all children, go to the previous fork(s)
    while ( fork->index < 0 ) {
        fork->index = -1;
        KCompFork *fork2 = myForkList.prev();
	if ( fork2 ) {
	    fork = fork2;
	    fork->index--;
    if ( fork->string.find( QString::fromLatin1("kfiledialog")) == 0 )
	    kDebugInfo(250, "             -- (fork: %s, %i)",fork->string.ascii(),
		  fork->index);
	}

	else { // no further fork available -> rotation
	    myBackwards = true;
	    completion = findCompletion( myLastString );
	    myBackwards = false;
	    emit match( completion );
	    return completion;
	}
    }

    //    completion = findCompletion( fork );
    if ( fork->string.find( QString::fromLatin1("kfiledialog")) == 0 )
kDebugInfo(250, "  fork: %s, index: %i (children: %i)", fork->string.ascii(), fork->index, fork->node->childrenCount());

    int index = fork->index;
    const KCompTreeNode *node = fork->node;
    ASSERT( node != 0L && node->childrenCount() > index );
    node = node->childAt( index );

    completion = fork->string;

    while ( !node->isNull() ) {
        completion += *node;

	if ( node->childrenCount() > 1 ) {
	    int index = node->childrenCount() -1;
	    myForkList.append( completion, node, index );
	}

	node = node->lastChild();
    }

    kDebugInfo(250, " -> completed: %s", completion.ascii());
    myItemIndex--;

    emit match( completion );
    return completion;
}



// tries to complete "string" from the tree-root
QString KCompletion::findCompletion( const QString& string )
{
    QChar ch;
    myForkList.clear();
    QString completion;
    const KCompTreeNode *node = myTreeRoot;
    myItemIndex = myBackwards ? -1 : 0;

    // start at the tree-root and try to find the search-string
    for( uint i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
	node = node->find( ch, myIgnoreCase );

	if ( node )
	    completion += ch;
	else
	    return QString::null; // no completion
    }
	
    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while ( node->childrenCount() == 1 ) {
        node = node->firstChild();
	if ( !node->isNull() )
	    completion += *node;
    }

    // multiple matches -> create a KCompFork and find the first complete match
    // in auto-completion mode
    if ( node && node->childrenCount() > 1 ) {
        int index = myBackwards ? node->childrenCount()-1 : -1;
	if ( myCompletionMode != KGlobal::CompletionShell && !myBackwards )
	    index = 0;
        myForkList.append( completion, node, index );
	
	if ( myCompletionMode == KGlobal::CompletionAuto ||
	     myCompletionMode == KGlobal::CompletionMan || myBackwards )
	    completion = findCompletion( myForkList.last() );
	else
	    doBeep(); // partial match -> beep
    }

    return completion;
}


// finds the first full completion of a given KCompFork, respecting indices for
// previous and next usage.
// also respects the myBackwards-flag for rotation, and returns the first
// available completion in reverse order
QString KCompletion::findCompletion( KCompFork *fork )
{
    ASSERT( fork != 0L );

    int index = fork->index;
    const KCompTreeNode *node = fork->node;
    ASSERT( node != 0L && node->childrenCount() > index );

    if ( myBackwards )
        node = node->lastChild();
    else
        node = node->childAt( index );

    QString completion = fork->string;

    while ( !node->isNull() ) {
        completion += *node;

	if ( node->childrenCount() > 1 ) {
	    int index = myBackwards ? node->childrenCount() -1 : 0;
	    myForkList.append( completion, node, index );
	}

	if ( myBackwards )
	    node = node->lastChild();
	else
	    node = node->firstChild();
    }

    return completion;
}


const QStringList& KCompletion::findAllCompletions( const QString& string )
{
    kDebugInfo(250, "*** finding all completions for %s", string.ascii() );
    myMatches.clear();

    if ( string.isEmpty() )
        return myMatches;

    QChar ch;
    QString completion;
    const KCompTreeNode *node = myTreeRoot;

    // start at the tree-root and try to find the search-string
    for( uint i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
	node = node->find( ch, myIgnoreCase );

	if ( node )
	    completion += ch;
	else
	    return myMatches; // no completion -> return empty list
    }
	
    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while ( node->childrenCount() == 1 ) {
        node = node->firstChild();
	if ( !node->isNull() )
	    completion += *node;
	// debug("-> %s, %c", completion.ascii(), node->latin1());
    }


    // there is just one single match)
    if ( node->childrenCount() == 0 )
        myMatches.append( completion );

    else {
        // node has more than one child
        // -> recursively find all remaining completions
        extractStringsFromNode( node, completion, &myMatches );
    }

    return myMatches;
}


void KCompletion::extractStringsFromNode( const KCompTreeNode *node,
					  const QString& beginning,
					  QStringList *matches,
					  bool getAllItems ) const
{
    if ( !node || !matches )
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
	    node = node->firstChild();

	    if ( !node->isNull() )
	        string += *node;

	    else { // we found a leaf
	        matches->append( string );
		// debug( " -> found match: %s", debugString( string ));
	    }
	}

	// recursively find all other strings.
	if ( node && node->childrenCount() > 1 )
	    extractStringsFromNode( node, string, matches );
    }
}


void KCompletion::doBeep()
{
    if ( myBeep &&
	 (myCompletionMode == KGlobal::CompletionShell ||
	  myCompletionMode == KGlobal::CompletionMan) )
        kapp->beep();
}

/////////////////////////////////
/////////


// Implements the tree. Every node is a QChar and has a list of children, which
// are Nodes as well.
// QChar( 0x0 ) is used as the delimiter of a string; the last child of each
// inserted string is 0x0.


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
    KCompTreeNode *child = find( ch, false );
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
        child = find( 0x0, false );
	myChildren.remove( child );
	return;
    }

    QChar ch = string.at(0);
    child = find( ch, false );
    if ( child ) {
        child->remove( string.right( string.length() -1 ) );
	if ( child->myChildren.count() == 0 ) {
	    delete child;
	    myChildren.remove( child );
	}
    }
}


#include "kcompletion.moc"
