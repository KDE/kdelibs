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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kglobal.h>
#include <qvector.h>

#include "kcompletion.h"
#include "kcompletion_private.h"

class KCompletionPrivate
{
public:
    // not a member to avoid #including kcompletion_private.h from kcompletion.h
    // list used for nextMatch() and previousMatch()
    KCompletionMatchesWrapper matches;
};

KCompletion::KCompletion()
{
    d = new KCompletionPrivate;

    myCompletionMode = KGlobalSettings::completionMode();
    myTreeRoot = new KCompTreeNode;
    myBeep       = true;
    myIgnoreCase = false;
    myHasMultipleMatches = false;
    myRotationIndex = 0;
    setOrder( Insertion );
}

KCompletion::~KCompletion()
{
    delete d;
    delete myTreeRoot;
}

void KCompletion::setOrder( CompOrder order )
{
    myOrder = order;
    d->matches.setSorting( order == Weighted );
}

void KCompletion::setIgnoreCase( bool ignoreCase )
{
    myIgnoreCase = ignoreCase;
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
    KCompletionMatchesWrapper list; // unsorted
    bool addWeight = (myOrder == Weighted);
    extractStringsFromNode( myTreeRoot, QString::null, &list, addWeight );

    return list.list();
}

bool KCompletion::isEmpty() const
{
  return (myTreeRoot->childrenCount() == 0);
}

void KCompletion::addItem( const QString& item )
{
    d->matches.clear();
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
//     qDebug("*** added: %s (%i)", item.latin1(), node->weight());
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
    d->matches.clear();
    myRotationIndex = 0;
    myLastString = QString::null;

    myTreeRoot->remove( item );
}


void KCompletion::clear()
{
    d->matches.clear();
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

    d->matches.clear();
    myRotationIndex = 0;
    myHasMultipleMatches = false;
    myLastMatch = myCurrentMatch;

    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if ( myCompletionMode == KGlobalSettings::CompletionShell &&
         string == myLastString ) {
        // Don't use d->matches since calling postProcessMatches()
        // on d->matches here would interfere with call to
        // postProcessMatch() during rotation
    
        findAllCompletions( string, &d->matches, myHasMultipleMatches );
        QStringList l = d->matches.list();
        postProcessMatches( &l );
        emit matches( l );

        if ( l.isEmpty() )
            doBeep( NoMatch );
    
        return QString::null;
    }

    QString completion;
    // in case-insensitive popup mode, we search all completions at once
    if ( myCompletionMode == KGlobalSettings::CompletionPopup ||
         myCompletionMode == KGlobalSettings::CompletionPopupAuto ) {
        findAllCompletions( string, &d->matches, myHasMultipleMatches );
        if ( !d->matches.isEmpty() )
            completion = d->matches.first();
    }
    else
        completion = findCompletion( string );

    if ( myHasMultipleMatches )
        emit multipleMatches();

    myLastString = string;
    myCurrentMatch = completion;

    postProcessMatch( &completion );

    if ( !string.isEmpty() ) { // only emit match when string is not empty
        //kdDebug(0) << "KCompletion: Match: " << completion << endl;
        emit match( completion );
    }

    if ( completion.isNull() )
        doBeep( NoMatch );

    return completion;
}


QStringList KCompletion::substringCompletion( const QString& string ) const
{
    // get all items in the tree, eventually in sorted order
    bool sorted = (myOrder == Weighted);
    KCompletionMatchesWrapper allItems( sorted );
    extractStringsFromNode( myTreeRoot, QString::null, &allItems, false );

    QStringList list = allItems.list();

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

    QStringList matches;
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
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    KCompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( myLastString, &matches, dummy );
    QStringList l = matches.list();
    postProcessMatches( &l );
    return l;
}

KCompletionMatches KCompletion::allWeightedMatches()
{
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    KCompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( myLastString, &matches, dummy );
    KCompletionMatches ret( matches );
    postProcessMatches( &ret );
    return ret;
}

QStringList KCompletion::allMatches( const QString &string )
{
    KCompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( string, &matches, dummy );
    QStringList l = matches.list();
    postProcessMatches( &l );
    return l;
}

KCompletionMatches KCompletion::allWeightedMatches( const QString &string )
{
    KCompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( string, &matches, dummy );
    KCompletionMatches ret( matches );
    postProcessMatches( &ret );
    return ret;
}

/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////


QString KCompletion::nextMatch()
{
    QString completion;
    myLastMatch = myCurrentMatch;

    if ( d->matches.isEmpty() ) {
        findAllCompletions( myLastString, &d->matches, myHasMultipleMatches );
        completion = d->matches.first();
        myCurrentMatch = completion;
        myRotationIndex = 0;
        postProcessMatch( &completion );
        emit match( completion );
        return completion;
    }

    QStringList matches = d->matches.list();
    myLastMatch = matches[ myRotationIndex++ ];

    if ( myRotationIndex == matches.count() -1 )
        doBeep( Rotation ); // indicate last matching item -> rotating

    else if ( myRotationIndex == matches.count() )
        myRotationIndex = 0;

    completion = matches[ myRotationIndex ];
    myCurrentMatch = completion;
    postProcessMatch( &completion );
    emit match( completion );
    return completion;
}



QString KCompletion::previousMatch()
{
    QString completion;
    myLastMatch = myCurrentMatch;

    if ( d->matches.isEmpty() ) {
        findAllCompletions( myLastString, &d->matches, myHasMultipleMatches );
        completion = d->matches.last();
        myCurrentMatch = completion;
        myRotationIndex = 0;
        postProcessMatch( &completion );
        emit match( completion );
        return completion;
    }

    QStringList matches = d->matches.list();
    myLastMatch = matches[ myRotationIndex ];
    if ( myRotationIndex == 1 )
        doBeep( Rotation ); // indicate first item -> rotating

    else if ( myRotationIndex == 0 )
        myRotationIndex = matches.count();

    myRotationIndex--;

    completion = matches[ myRotationIndex ];
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
    for( int i = 0; i < string.length(); i++ ) {
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


void KCompletion::findAllCompletions(const QString& string,
                                     KCompletionMatchesWrapper *matches,
                                     bool& hasMultipleMatches) const
{
    //kdDebug(0) << "*** finding all completions for " << string << endl;

    if ( string.isEmpty() )
        return;

    if ( myIgnoreCase ) { // case insensitive completion
        extractStringsFromNodeCI( myTreeRoot, QString::null, string, matches );
        hasMultipleMatches = (matches->count() > 1);
        return;
    }

    QChar ch;
    QString completion;
    const KCompTreeNode *node = myTreeRoot;

    // start at the tree-root and try to find the search-string
    for( int i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
        node = node->find( ch );

        if ( node )
            completion += ch;
        else
            return; // no completion -> return empty list
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
        matches->append( node->weight(), completion );

    else {
        // node has more than one child
        // -> recursively find all remaining completions
        hasMultipleMatches = true;
        extractStringsFromNode( node, completion, matches );
    }
}


void KCompletion::extractStringsFromNode( const KCompTreeNode *node,
                                          const QString& beginning,
                                          KCompletionMatchesWrapper *matches,
                                          bool addWeight ) const
{
    if ( !node || !matches )
        return;

    // kDebug() << "Beginning: " << beginning << endl;
    const KCompTreeChildren *list = node->children();
    QString string;
    QString w;

    // loop thru all children
    for ( KCompTreeNode *cur = list->begin(); cur ; cur = cur->next) {
        string = beginning;
        node = cur;
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
            matches->append( node->weight(), string );
        }

        // recursively find all other strings.
        if ( node && node->childrenCount() > 1 )
            extractStringsFromNode( node, string, matches, addWeight );
    }
}

void KCompletion::extractStringsFromNodeCI( const KCompTreeNode *node,
                                            const QString& beginning,
                                            const QString& restString,
                                            KCompletionMatchesWrapper *matches ) const
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
            event = QLatin1String("Textcompletion: rotation");
            text = i18n("You reached the end of the list\nof matching items.\n");
            break;
        case PartialMatch:
            if ( myCompletionMode == KGlobalSettings::CompletionShell ||
                 myCompletionMode == KGlobalSettings::CompletionMan ) {
                event = QLatin1String("Textcompletion: partial match");
                text = i18n("The completion is ambiguous, more than one\nmatch is available.\n");
            }
            break;
        case NoMatch:
            if ( myCompletionMode == KGlobalSettings::CompletionShell ) {
                event = QLatin1String("Textcompletion: no match");
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
    KCompTreeNode *cur = myChildren.begin();
    while (cur) {
        KCompTreeNode * next = cur->next;
        delete myChildren.remove(cur);
        cur = next;
    }
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
            KCompTreeNode * prev = 0;
            KCompTreeNode * cur = myChildren.begin();
            while ( cur ) {
                if ( ch > *cur ) {
                    prev = cur;
                    cur = cur->next;
                } else
                    break;
            }
            if (prev)
                myChildren.insert( prev, child );
            else
                myChildren.prepend(child);
        }

        else
            myChildren.append( child );
    }

    // implicit weighting: the more often an item is inserted, the higher
    // priority it gets.
    child->confirm();

    return child;
}


// Iteratively removes a string from the tree. The nicer recursive
// version apparently was a little memory hungry (see #56757)
void KCompTreeNode::remove( const QString& str )
{
    QString string = str;
    string += QChar(0x0);

    QVector<KCompTreeNode *> deletables( string.length() + 1 );

    KCompTreeNode *child = 0L;
    KCompTreeNode *parent = this;
    deletables.replace( 0, parent );
    
    int i = 0;
    for ( ; i < string.length(); i++ )
    {
        child = parent->find( string.at( i ) );
        if ( child )
            deletables.replace( i + 1, child );
        else
            break;

        parent = child;
    }

    for ( ; i >= 1; i-- )
    {
        parent = deletables.at( i - 1 );
        child = deletables.at( i );
        if ( child->myChildren.count() == 0 )
            delete parent->myChildren.remove( child );
    }
}

QStringList KCompletionMatchesWrapper::list() const 
{
    if ( sortedList && dirty ) {
        sortedList->sort();
        dirty = false;

        stringList.clear();

        // high weight == sorted last -> reverse the sorting here
        QList<KSortableItem<QString> >::const_iterator it;
        for ( it = sortedList->begin(); it != sortedList->end(); ++it )
            stringList.prepend( (*it).value() );
    }

    return stringList;
}

KCompletionMatches::KCompletionMatches( bool sort_P )
    : _sorting( sort_P )
{
}

KCompletionMatches::KCompletionMatches( const KCompletionMatchesWrapper& matches )
    : _sorting( matches.sorting())
{
    if( matches.sortedList != 0L )
        KCompletionMatchesList::operator=( *matches.sortedList );
    else {
        QStringList l = matches.list();
        for( QStringList::ConstIterator it = l.begin();
             it != l.end();
             ++it )
            prepend( KSortableItem<QString, int>( 1, *it ) );
    }
}

KCompletionMatches::~KCompletionMatches()
{
}

QStringList KCompletionMatches::list( bool sort_P ) const
{
    if( _sorting && sort_P )
        const_cast< KCompletionMatches* >( this )->sort();
    QStringList stringList;
    // high weight == sorted last -> reverse the sorting here
    for ( ConstIterator it = begin(); it != end(); ++it )
        stringList.prepend( (*it).value() );
    return stringList;
}

void KCompletionMatches::removeDuplicates()
{
    Iterator it1, it2;
    for ( it1 = begin(); it1 != end(); ++it1 ) {
        for ( (it2 = it1), ++it2; it2 != end();) {
            if( (*it1).value() == (*it2).value()) {
                // use the max height
                (*it1).first = kMax( (*it1).index(), (*it2).index());
                it2 = remove( it2 );
                continue;
            }
            ++it2;
        }
    }
}

void KCompTreeNodeList::append(KCompTreeNode *item)
{
    m_count++;
    if (!last) {
        last = item;
        last->next = 0;
        first = item;
        return;
    }
    last->next = item;
    item->next = 0;
    last = item;
}

void KCompTreeNodeList::prepend(KCompTreeNode *item)
{
    m_count++;
    if (!last) {
        last = item;
        last->next = 0;
        first = item;
        return;
    }
    item->next = first;
    first = item;
}

void KCompTreeNodeList::insert(KCompTreeNode *after, KCompTreeNode *item)
{
    if (!after) {
        append(item);
        return;
    }

    m_count++;

    item->next = after->next;
    after->next = item;

    if (after == last)
        last = item;
}

KCompTreeNode *KCompTreeNodeList::remove(KCompTreeNode *item)
{
    if (!first || !item)
        return 0;
    KCompTreeNode *cur = 0;

    if (item == first)
        first = first->next;
    else {
        cur = first;
        while (cur && cur->next != item) cur = cur->next;
        if (!cur)
            return 0;
        cur->next = item->next;
    }
    if (item == last)
        last = cur;
    m_count--;
    return item;
}

KCompTreeNode *KCompTreeNodeList::at(uint index) const
{
    KCompTreeNode *cur = first;
    while (index-- && cur) cur = cur->next;
    return cur;
}

KZoneAllocator KCompTreeNode::alloc(8192);

void KCompletion::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KCompletionBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcompletion.moc"
