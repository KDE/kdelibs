/* This file is part of the KDE libraries
    Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

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
    myBeep       = true;
    myIgnoreCase = false;
    myHasMultipleMatches = false;
    myRotationIndex = 0;
    myOrder = Insertion;
}

KCompletion::~KCompletion()
{
    delete myTreeRoot;
}


void KCompletion::setItems( const QStringList& items )
{
    clear();
    insertItems( items );
}


void KCompletion::insertItems( const QStringList& items )
{
    bool weighted = (myOrder == Weighted);
    QStringList::ConstIterator it;
    if ( weighted ) { // determine weight
	for ( it = items.begin(); it != items.end(); ++it )
	    addWeightedItem( *it );
    }
    else {
	for ( it = items.begin(); it != items.end(); ++it )
	    addItem( *it, 0 );
    }
}


QStringList KCompletion::items() const
{
    QStringList list;
    bool addWeight = (myOrder == Weighted);
    extractStringsFromNode( myTreeRoot, QString::null, &list, addWeight );

    return list;
}


void KCompletion::addItem( const QString& item )
{
    myMatches.clear();
    myRotationIndex = 0;
    myLastString = QString::null;

    addItem( item, 0 );
}

void KCompletion::addItem( const QString& item, uint weight )
{
    if ( item.isEmpty() )
	return;

    KCompTreeNode *node = myTreeRoot;
    uint len = item.length();

    bool sorted = (myOrder == Sorted);
    bool weighted = ((myOrder == Weighted) && weight > 1);

    // knowing the weight of an item, we simply add this weight to all of its
    // nodes.

    for ( uint i = 0; i < len; i++ ) {
	node = node->insert( item.at(i), sorted );
	if ( weighted )
	    node->confirm( weight -1 ); // node->insert() sets weighting to 1
    }

    // add 0x0-item as delimiter with evtl. weight
    node = node->insert( 0x0, true );
    if ( weighted )
	node->confirm( weight -1 );
}

void KCompletion::addWeightedItem( const QString& item )
{
    if ( myOrder != Weighted ) {
	addItem( item, 0 );
	return;
    }

    uint len = item.length();
    uint weight = 0;

    // find out the weighting of this item (appended to the string as ":num")
    int index = item.findRev(':');
    if ( index > 0 ) {
	bool ok;
	weight = item.mid( index + 1 ).toUInt( &ok );
	if ( !ok )
	    weight = 0;

	len = index; // only insert until the ':'
    }

    addItem( item.left( len ), weight );
    return;
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

    //kdDebug(0) << "KCompletion: completing: " << string << endl;

    myMatches.clear();
    myRotationIndex = 0;
    myHasMultipleMatches = false;
    myLastMatch = myCurrentMatch;

    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if ( myCompletionMode == KGlobalSettings::CompletionShell &&
	 string == myLastString ) {
	// Don't use myMatches since calling postProcessMatches()
	// on myMatches here would interfere with call to
	// postProcessMatch() during rotation
	
	QStringList l = findAllCompletions( string );
	postProcessMatches( &l );
	emit matches( l );

	if ( l.isEmpty() )
	    doBeep( NoMatch );
	
	return QString::null;
    }

    QString completion;
    // in case-insensitive popup mode, we search all completions at once
    if ( myCompletionMode == KGlobalSettings::CompletionPopup ) {
        myMatches = findAllCompletions( string );
        if ( !myMatches.isEmpty() )
            completion = myMatches.first();
    }
    else
        completion = findCompletion( string );

    if ( myHasMultipleMatches )
        emit multipleMatches();

    myLastString = string;
    myCurrentMatch = completion;

    postProcessMatch( &completion );

    if ( !string.isEmpty() ) { // only emit match when string != ""
	//kdDebug(0) << "KCompletion: Match: " << completion << endl;
        emit match( completion );
    }

    if ( completion.isNull() )
        doBeep( NoMatch );

    return completion;
}


QStringList KCompletion::substringCompletion( const QString& string ) const
{
    QStringList matches;
    QStringList list = items(); // ### maybe cache this!

    // subStringMatches is invoked manually, via a shortcut, so we should
    // beep here, if necessary.
    if ( list.isEmpty() ) {
        doBeep( NoMatch );
        return list;
    }

    if ( string.isEmpty() ) { // shortcut
        postProcessMatches( &list );
        return list;
    }

    QStringList::ConstIterator it = list.begin();

    for( ; it != list.end(); ++it ) {
        QString item = *it;
        if ( item.find( string, 0, false ) != -1 ) { // always case insensitive
            postProcessMatch( &item );
            matches.append( item );
        }
    }

    if ( matches.isEmpty() )
        doBeep( NoMatch );

    return matches;
}


void KCompletion::setCompletionMode( KGlobalSettings::Completion mode )
{
    myCompletionMode = mode;
}


QStringList KCompletion::allMatches()
{
    // Don't use myMatches since calling postProcessMatches()
    // on myMatches here would interfere with call to
    // postProcessMatch() during rotation
    QStringList l = findAllCompletions( myLastString );
    postProcessMatches( &l );
    return l;
}

QStringList KCompletion::allMatches( const QString &string )
{
    // Don't use myMatches since calling postProcessMatches()
    // on myMatches here would interfere with call to
    // postProcessMatch() during rotation
    QStringList l = findAllCompletions( string );
    postProcessMatches( &l );
    return l;
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
	node = node->find( ch );

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
	
	if ( myCompletionMode == KGlobalSettings::CompletionAuto ) {
	    myRotationIndex = 1;
	    if (myOrder != Weighted) {
		while ( (node = node->firstChild()) ) {
		    if ( !node->isNull() )
			completion += *node;
	    	    else
			break;
    		}
	    }
	    else {
		// don't just find the "first" match, but the one with the
		// highest priority
		
		const KCompTreeNode* temp_node = 0L;
		while(1) {
		    int count = node->childrenCount();
		    temp_node = node->firstChild();
		    uint weight = temp_node->weight();
		    const KCompTreeNode* hit = temp_node;
		    for( int i = 1; i < count; i++ ) {
			temp_node = node->childAt(i);
			if( temp_node->weight() > weight ) {
			    hit = temp_node;
			    weight = hit->weight();
			}
		    }
		    // 0x0 has the highest priority -> we have the best match
		    if ( hit->isNull() )
			break;

		    node = hit;
		    completion += *node;
    		}
	    }
	}

	else
	    doBeep( PartialMatch ); // partial match -> beep
    }

    return completion;
}


const QStringList& KCompletion::findAllCompletions( const QString& string )
{
    //kdDebug(0) << "*** finding all completions for " << string << endl;
    myMatches.clear();
    myRotationIndex = 0;

    if ( string.isEmpty() )
        return myMatches;

    if ( myIgnoreCase ) { // case insensitive completion
        extractStringsFromNodeCI(myTreeRoot, QString::null, string,&myMatches);
        myHasMultipleMatches = (myMatches.count() > 1);
        return myMatches;
    }

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

    while ( node->childrenCount() == 1 ) {
        node = node->firstChild();
	if ( !node->isNull() )
	    completion += *node;
	// kdDebug() << completion << node->latin1();
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
					  QStringList *matches,
					  bool addWeight ) const
{
    if ( !node || !matches )
        return;

    // kDebug() << "Beginning: " << beginning << endl;
    KCompTreeChildren::ConstIterator it;
    const KCompTreeChildren *list = node->children();
    QString string;
    QString w;

    // loop thru all children
    for ( it = list->begin(); it != list->end(); ++it ) {
        string = beginning;
        node = *it;
	if ( !node->isNull() )
	    string += *node;

	while ( node && node->childrenCount() == 1 ) {
	    node = node->firstChild();
	    if ( node->isNull() )
		break;
	    string += *node;
	}

	if ( node && node->isNull() ) { // we found a leaf
	    if ( addWeight ) {
		// add ":num" to the string to store the weighting
		string += ':';
		w.setNum( node->weight() );
		string.append( w );
	    }
	    matches->append( string );
	}

	// recursively find all other strings.
	if ( node && node->childrenCount() > 1 )
	    extractStringsFromNode( node, string, matches, addWeight );
    }
}

void KCompletion::extractStringsFromNodeCI( const KCompTreeNode *node,
                                                   const QString& beginning,
                                                   const QString& restString,
                                                   QStringList *matches )
{
    if ( restString.isEmpty() ) {
        extractStringsFromNode( node, beginning, matches, false /*noweight*/ );
        return;
    }

    QChar ch1 = restString.at(0);
    QString newRest = restString.mid(1);
    KCompTreeNode *child1, *child2;

    child1 = node->find( ch1 ); // the correct match
    if ( child1 )
        extractStringsFromNodeCI( child1, beginning + *child1, newRest,
                                  matches );

    // append the case insensitive matches, if available
    if ( ch1.isLetter() ) {
        // find out if we have to lower or upper it. Is there a better way?
        QChar ch2 = ch1.lower();
        if ( ch1 == ch2 )
            ch2 = ch1.upper();
        if ( ch1 != ch2 ) {
            child2 = node->find( ch2 );
            if ( child2 )
                extractStringsFromNodeCI( child2, beginning + *child2, newRest,
                                          matches );
        }
    }
}


void KCompletion::doBeep( BeepMode mode ) const
{
    if ( !myBeep )
	return;

    QString text, event;

    switch ( mode ) {
    case Rotation:
	event = QString::fromLatin1("Textcompletion: rotation");
	text = i18n("You reached the end of the list\nof matching items.\n");
	break;
    case PartialMatch:
	if ( myCompletionMode == KGlobalSettings::CompletionShell ||
	     myCompletionMode == KGlobalSettings::CompletionMan ) {
	    event = QString::fromLatin1("Textcompletion: partial match");
	    text = i18n("The completion is ambiguous, more than one\nmatch is available.\n");
	}
	break;
    case NoMatch:
	if ( myCompletionMode == KGlobalSettings::CompletionShell ) {
	    event = QString::fromLatin1("Textcompletion: no match");
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
    KCompTreeNode *child = find( ch );
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

    // implicit weighting: the more often an item is inserted, the higher
    // priority it gets.
    child->confirm();

    return child;
}


// Recursively removes a string from the tree (untested :-)
void KCompTreeNode::remove( const QString& string )
{
    KCompTreeNode *child = 0L;

    if ( string.isEmpty() ) {
        child = find( 0x0 );
        delete child;
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

#include "kcompletion.moc"
