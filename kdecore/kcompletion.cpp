/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <klocale.h>
#include <knotifyclient.h>

#include "kcompletion.h"
#include "kcompletion_private.h"


KCompletion::KCompletion()
{
    myCompletionMode = KGlobalSettings::completionMode();
    myTreeRoot = new KCompTreeNode;
    mySorting    = false;
    myBeep       = true;
    myIgnoreCase = false;
    myHasMultipleMatches = false;
    myRotationIndex = 0;
}

KCompletion::~KCompletion()
{
    delete myTreeRoot;
}


void KCompletion::setItems( const QStringList& items )
{
    myMatches.clear();
    myRotationIndex = 0;
    myLastString = QString::null;

    QStringList::ConstIterator it;
    for ( it = items.begin(); it != items.end(); ++it )
        addItemInternal( *it );
}


QStringList KCompletion::items() const
{
    QStringList list;
    extractStringsFromNode( myTreeRoot, QString::null, &list );

    return list;
}


void KCompletion::addItem( const QString& item )
{
    myMatches.clear();
    myRotationIndex = 0;
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
    myMatches.clear();
    myRotationIndex = 0;
    myLastString = QString::null;

    myTreeRoot->remove( item );
}


void KCompletion::clear()
{
    myMatches.clear();
    myRotationIndex = 0;
    myLastString = QString::null;

    delete myTreeRoot;
    myTreeRoot = new KCompTreeNode;
}


QString KCompletion::makeCompletion( const QString& string )
{
    if ( myCompletionMode == KGlobalSettings::CompletionNone )
        return QString::null;

    kdDebug(0) << "KCompletion: completing: " << string << endl;

    myMatches.clear();
    myRotationIndex = 0;
    myHasMultipleMatches = false;
    myLastMatch = myCurrentMatch;

    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if ( myCompletionMode == KGlobalSettings::CompletionShell &&
	 string == myLastString ) {
        myMatches = findAllCompletions( string );
	postProcessMatches( &myMatches );
	emit matches( myMatches );
    }

    QString completion = findCompletion( string );
    if ( myHasMultipleMatches )
        emit multipleMatches();

    myLastString = string;
    myCurrentMatch = completion;

    if ( !string.isEmpty() ) { // only emit match when string != ""
	kdDebug(0) << "KCompletion: Match: " << completion << endl;

	postProcessMatch( &completion );
        emit match( completion );
    }
	else
        postProcessMatch( &completion );


    if ( completion.isNull() )
        doBeep( NoMatch );

    return completion;
}


void KCompletion::setCompletionMode( KGlobalSettings::Completion mode )
{
    myCompletionMode = mode;
}



/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////


QString KCompletion::nextMatch()
{
    QString completion;
    myLastMatch = myCurrentMatch;

    if ( myMatches.isEmpty() ) {
	myMatches = findAllCompletions( myLastString );
	completion = myMatches.first();
	myCurrentMatch = completion;
	postProcessMatch( &completion );
	emit match( completion );
	return completion;
    }

    myLastMatch = myMatches[ myRotationIndex++ ];

    if ( myRotationIndex == myMatches.count() -1 )
	doBeep( Rotation ); // indicate last matching item -> rotating

    else if ( myRotationIndex == myMatches.count() )
	myRotationIndex = 0;

    completion = myMatches[ myRotationIndex ];
    myCurrentMatch = completion;
    postProcessMatch( &completion );
    emit match( completion );
    return completion;
}



QString KCompletion::previousMatch()
{
    QString completion;
    myLastMatch = myCurrentMatch;

    if ( myMatches.isEmpty() ) {
	myMatches = findAllCompletions( myLastString );
	completion = myMatches.last();
	myCurrentMatch = completion;
	postProcessMatch( &completion );
	emit match( completion );
	return completion;
    }

    myLastMatch = myMatches[ myRotationIndex ];
    if ( myRotationIndex == 1 )
	doBeep( Rotation ); // indicate first item -> rotating

    else if ( myRotationIndex == 0 )
	myRotationIndex = myMatches.count();

    myRotationIndex--;

    completion = myMatches[ myRotationIndex ];
    myCurrentMatch = completion;
    postProcessMatch( &completion );
    emit match( completion );
    return completion;
}



// tries to complete "string" from the tree-root
QString KCompletion::findCompletion( const QString& string )
{
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

    // if multiple matches and auto-completion mode
    // -> find the first complete match
    if ( node && node->childrenCount() > 1 ) {
	myHasMultipleMatches = true;
	
	if ( myCompletionMode == KGlobalSettings::CompletionAuto ||
	     myCompletionMode == KGlobalSettings::CompletionMan ) {

	    myRotationIndex = 1;
	    while ( (node = node->firstChild()) ) {
		if ( !node->isNull() )
		    completion += *node;
		else
		    break;
	    }
	}

	else
	    doBeep( PartialMatch ); // partial match -> beep
    }

    return completion;
}


const QStringList& KCompletion::findAllCompletions( const QString& string )
{
    kdDebug(0) << "*** finding all completions for " << string << endl;
    myMatches.clear();
    myRotationIndex = 0;

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
	myHasMultipleMatches = true;
        extractStringsFromNode( node, completion, &myMatches );
    }

    return myMatches;
}


void KCompletion::extractStringsFromNode( const KCompTreeNode *node,
					  const QString& beginning,
					  QStringList *matches ) const
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


void KCompletion::doBeep( BeepMode mode )
{
    if ( !myBeep )
	return;

    QString text, event;

    switch ( mode ) {
    case Rotation:
	event = QString::fromLatin1("KCompletion: rotation");
	text = i18n("You reached the end of the list\nof matching items.\n");
	break;
    case PartialMatch:
	if ( myCompletionMode == KGlobalSettings::CompletionShell ) {
	    event = QString::fromLatin1("KCompletion: partial match");
	    text = i18n("The completion is ambiguous, more than one\nmatch is available.\n");
	}
	break;
    case NoMatch:
	if ( myCompletionMode == KGlobalSettings::CompletionShell ||
	     myCompletionMode == KGlobalSettings::CompletionMan ) {
	    event = QString::fromLatin1("KCompletion: no match");
	    text = i18n("There is no matching item available.\n");
	}
	break;
    }

    if ( !text.isEmpty() )
	KNotifyClient::event( event, text );
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

	// FIXME, first (slow) sorted insertion implementation
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
