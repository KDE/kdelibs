/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#ifndef _dom2_traversal_h_
#define _dom2_traversal_h_
#include "dom_node.h"


namespace DOM {
class Node;
class NodeFilter;
class NodeImpl;
 
/**
 * NodeIterators are used to step through a set of nodes, e.g. the set
 * of nodes in a NodeList, the document subtree governed by a
 * particular node, the results of a query, or any other set of nodes.
 * The set of nodes to be iterated is determined by the implementation
 * of the NodeIterator. DOM Level 2 specifies a single NodeIterator
 * implementation for document-order traversal of a document subtree.
 * Instances of these iterators are created by calling
 * DocumentTraversal.createNodeIterator().
 *
 *  Any Iterator that returns nodes may implement the <code>
 * NodeIterator </code> interface. Users and vendor libraries may also
 * choose to create Iterators that implement the <code> NodeIterator
 * </code> interface.
 *
 */
class NodeIterator
{
public:
    NodeIterator();
    NodeIterator(const NodeIterator &other);
    NodeIterator(Node n, NodeFilter *f=0);
    NodeIterator(Node n, long _whatToShow, NodeFilter *f=0);
    
    NodeIterator & operator = (const NodeIterator &other);

    ~NodeIterator();
    

    /**
     * Returns the next node in the set and advances the position of
     * the Iterator in the set. After a NodeIterator is created, the
     * first call to nextNode() returns the first node in the set.
     *
     * @return The next <code> Node </code> in the set being iterated
     * over, or <code> null </code> if there are no more members in
     * that set.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node nextNode (  );

    /**
     * Returns the previous node in the set and moves the position of
     * the Iterator backwards in the set.
     *
     * @return The previous <code> Node </code> in the set being
     * iterated over, or <code> null </code> if there are no more
     * members in that set.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node previousNode (  );

    void setWhatToShow(long _whatToShow);
    void setFilter(NodeFilter *_filter);
    void setExpandEntityReferences(bool value);

    /**
     * This function has to be called if you delete a node from the
     * document tree and you want the Iterator to react if there
     * are any changes concerning it.
     */
    void deleteNode(Node n);

    /**
     *  Move the Iterators referenceNode within the subtree. Does not set a new root node.
     */
    void moveReferenceNode(Node n);
    /**
     * Sets the iterators referenceNode and sets a new root node
     */
    void setReferenceNode(Node n);
    short isAccepted(Node n);
    Node getNextNode(Node n);
    Node getPreviousNode(Node n);
 protected:
    /**
     * These are the available values for the whatToShow parameter.
     * They are the same as the set of possible types for Node, and
     * their values are derived by using a bit position corresponding
     * to the value of NodeType for the equivalent node type.
     *
     */
    enum ShowCode {
        SHOW_ALL = 0x0000FFFF,
        SHOW_ELEMENT = 0x00000001,
        SHOW_ATTRIBUTE = 0x00000002,
        SHOW_TEXT = 0x00000004,
        SHOW_CDATA_SECTION = 0x00000008,
        SHOW_ENTITY_REFERENCE = 0x00000010,
        SHOW_ENTITY = 0x00000020,
        SHOW_PROCESSING_INSTRUCTION = 0x00000040,
        SHOW_COMMENT = 0x00000080,
        SHOW_DOCUMENT = 0x00000100,
        SHOW_DOCUMENT_TYPE = 0x00000200,
        SHOW_DOCUMENT_FRAGMENT = 0x00000400,
        SHOW_NOTATION = 0x00000800
    };

    /**
     * This attribute determines which node types are presented via
     * the Iterator.
     *
     */
    long whatToShow;

    /**
     * The filter used to screen nodes.
     *
     */
    NodeFilter *filter;

    /**
     * The value of this flag determines whether entity reference
     * nodes are expanded. To produce a view of the document that has
     * entity references expanded and does not expose the entity
     * reference node itself, use the whatToShow flags to hide the
     * entity reference node and set expandEntityReferences to true
     * when creating the iterator. To produce a view of the document
     * that has entity reference nodes but no entity expansion, use
     * the whatToShow flags to show the entity reference node and set
     * expandEntityReferences to true.
     *
     */
    bool expandEntityReferences;

    /**
     * internal, used to determine if the iterator is in front or to the back
     * of the referenceNode
     */
    bool inFront;
    /**
     * Internal, not specified by the dom
     * The current referenceNode
     */
    Node referenceNode;

    /**
     * Internal, not specified by the dom
     * The initial referenceNode for this Iterator
     */
    Node rootNode;
};


/**
 * Filters are objects that know how to "filter out" nodes. If an
 * Iterator or <code> TreeWalker </code> is given a filter, before it
 * returns the next node, it applies the filter. If the filter says to
 * accept the node, the Iterator returns it; otherwise, the Iterator
 * looks for the next node and pretends that the node that was
 * rejected was not there.
 *
 *  The DOM does not provide any filters. Filter is just an interface
 * that users can implement to provide their own filters.
 *
 *  Filters do not need to know how to iterate, nor do they need to
 * know anything about the data structure that is being iterated. This
 * makes it very easy to write filters, since the only thing they have
 * to know how to do is evaluate a single node. One filter may be used
 * with a number of different kinds of Iterators, encouraging code
 * reuse.
 *
 */
class NodeFilter
{
public:
    NodeFilter();
    NodeFilter(const NodeFilter &other);
    
    NodeFilter & operator = (const NodeFilter &other);

    virtual ~NodeFilter();
    /**
     * The following constants are returned by the acceptNode()
     * method:
     *
     */
    enum acceptNode {
        FILTER_ACCEPT = 1,
        FILTER_REJECT = 2,
        FILTER_SKIP = 3
    };

    /**
     * Test whether a specified node is visible in the logical view of
     * a TreeWalker or NodeIterator. This function will be called by
     * the implementation of TreeWalker and NodeIterator; it is not
     * intended to be called directly from user code.
     *
     * @param n The node to check to see if it passes the filter or
     * not.
     *
     * @return a constant to determine whether the node is accepted,
     * rejected, or skipped, as defined <a
     * href="#Traversal-NodeFilter-acceptNode-constants"> above </a> .
     *
     */
    virtual short acceptNode ( const Node &n );
};


/**
 * <code> TreeWalker </code> objects are used to navigate a document
 * tree or subtree using the view of the document defined by its
 * <code> whatToShow </code> flags and any filters that are defined
 * for the <code> TreeWalker </code> . Any function which performs
 * navigation using a <code> TreeWalker </code> will automatically
 * support any view defined by a <code> TreeWalker </code> .
 *
 *  Omitting nodes from the logical view of a subtree can result in a
 * structure that is substantially different from the same subtree in
 * the complete, unfiltered document. Nodes that are siblings in the
 * TreeWalker view may be children of different, widely separated
 * nodes in the original view. For instance, consider a Filter that
 * skips all nodes except for Text nodes and the root node of a
 * document. In the logical view that results, all text nodes will be
 * siblings and appear as direct children of the root node, no matter
 * how deeply nested the structure of the original document.
 *
 */
class TreeWalker
{
public:
    TreeWalker();
    TreeWalker(const TreeWalker &other);
    TreeWalker(Node n, NodeFilter *f=0);
    TreeWalker(Node n, long _whatToShow, NodeFilter *f=0);
    TreeWalker & operator = (const TreeWalker &other);

               
    ~TreeWalker();
    /**
     * These are the available values for the <code> whatToShow
     * </code> parameter. They are the same as the set of possible
     * types for <code> Node </code> , and their values are derived by
     * using a bit position corresponding to the value of NodeType for
     * the equivalent node type.
     *
     */
    enum ShowCode {
        SHOW_ALL = 0x0000FFFF,
        SHOW_ELEMENT = 0x00000001,
        SHOW_ATTRIBUTE = 0x00000002,
        SHOW_TEXT = 0x00000004,
        SHOW_CDATA_SECTION = 0x00000008,
        SHOW_ENTITY_REFERENCE = 0x00000010,
        SHOW_ENTITY = 0x00000020,
        SHOW_PROCESSING_INSTRUCTION = 0x00000040,
        SHOW_COMMENT = 0x00000080,
        SHOW_DOCUMENT = 0x00000100,
        SHOW_DOCUMENT_TYPE = 0x00000200,
        SHOW_DOCUMENT_FRAGMENT = 0x00000400,
        SHOW_NOTATION = 0x00000800
    };

   

    /**
     * see @ref currentNode
     */
    void setCurrentNode( const Node _currentNode );

    /**
     * Moves to and returns the parent node of the current node. If
     * there is no parent node, or if the current node is the root
     * node from which this TreeWalker was created, retains the
     * current position and returns null.
     *
     * @return The new parent node, or null if the current node has no
     * parent in the TreeWalker's logical view.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node parentNode (  );

    /**
     * Moves the <code> TreeWalker </code> to the first child of the
     * current node, and returns the new node. If the current node has
     * no children, returns <code> null </code> , and retains the
     * current node.
     *
     * @return The new node, or <code> null </code> if the current
     * node has no children.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node firstChild (  );

    /**
     * Moves the <code> TreeWalker </code> to the last child of the
     * current node, and returns the new node. If the current node has
     * no children, returns <code> null </code> , and retains the
     * current node.
     *
     * @return The new node, or <code> null </code> if the current
     * node has no children.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node lastChild (  );

    /**
     * Moves the <code> TreeWalker </code> to the previous sibling of
     * the current node, and returns the new node. If the current node
     * has no previous sibling, returns <code> null </code> , and
     * retains the current node.
     *
     * @return The new node, or <code> null </code> if the current
     * node has no previous sibling.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node previousSibling (  );

    /**
     * Moves the <code> TreeWalker </code> to the next sibling of the
     * current node, and returns the new node. If the current node has
     * no next sibling, returns <code> null </code> , and retains the
     * current node.
     *
     * @return The new node, or <code> null </code> if the current
     * node has no next sibling.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node nextSibling (  );

    /**
     * Moves the <code> TreeWalker </code> to the previous node in
     * document order relative to the current node, and returns the
     * new node. If the current node has no previous node, returns
     * <code> null </code> , and retains the current node.
     *
     * @return The new node, or <code> null </code> if the current
     * node has no previous node.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node previousNode (  );

    /**
     * Moves the <code> TreeWalker </code> to the next node in
     * document order relative to the current node, and returns the
     * new node. If the current node has no next node, returns <code>
     * null </code> , and retains the current node.
     *
     * @return The new node, or <code> null </code> if the current
     * node has no next node.
     *
     * @exception Exceptions from user code
     * Any exceptions raised by a user-written Filter will propagate
     * through.
     *
     */
    Node nextNode (  );

    /**
     * Sets which node types are to be presented via the TreeWalker
     */
    void setWhatToShow(long _whatToShow);
    void setFilter(NodeFilter *_filter);
    void setExpandEntityReferences(bool value);
    Node getCurrentNode();

    Node getParentNode(Node n);
    Node getFirstChild(Node n);
    Node getLastChild(Node n);
    Node getPreviousSibling(Node n);
    Node getNextSibling(Node n);

    short isAccepted(Node n);
    
protected:
    /**
     * This attribute determines which node types are presented via
     * the TreeWalker.
     *
     */
    long whatToShow;

    /**
     * The filter used to screen nodes.
     *
     */
    NodeFilter *filter;

    /**
     * The value of this flag determines whether entity reference
     * nodes are expanded. To produce a view of the document that has
     * entity references expanded and does not expose the entity
     * reference node itself, use the whatToShow flags to hide the
     * entity reference node and set expandEntityReferences to true
     * when creating the iterator. To produce a view of the document
     * that has entity reference nodes but no entity expansion, use
     * the whatToShow flags to show the entity reference node and set
     * expandEntityReferences to true.
     *
     * This is not implemented (allways true)
     */
    bool expandEntityReferences;

    /**
     * The current node.
     *
     *  The value must not be null. Attempting to set it to null will
     * raise a NOT_SUPPORTED_ERR exception. When setting a node, the
     * whatToShow flags and any Filter associated with the TreeWalker
     * are not checked. The currentNode may be set to any Node of any
     * type.
     *
     */
    Node currentNode;

    Node rootNode;
};


// ### see my comments to DocumentRange for this class. Lars

/**
 * <code> DocumentTraversal </code> contains methods that creates
 * Iterators to traverse a node and its children in document order
 * (depth first, pre-order traversal, which is equivalent to the order
 * in which the start tags occur in the text representation of the
 * document).
 *
 */
class DocumentTraversal
{
public:
    DocumentTraversal();
    DocumentTraversal(const DocumentTraversal &other);

    DocumentTraversal & operator = (const DocumentTraversal &other);

    ~DocumentTraversal();

    /**
     *
     *
     * @param root The node which will be iterated together with its
     * children. The iterator is initially positioned just before this
     * node. The whatToShow flags and the filter, if any, are not
     * considered when setting this position.
     *
     * @param whatToShow This flag specifies which node types may
     * appear in the logical view of the tree presented by the
     * Iterator. See the description of Iterator for the set of
     * possible values. These flags can be combined using OR.
     *
     *  These flags can be combined using <code> OR </code> .
     *
     * @param filter The Filter to be used with this TreeWalker, or
     * null to indicate no filter.
     *
     * @param entityReferenceExpansion The value of this flag
     * determines whether entity reference nodes are expanded.
     *
     * @return The newly created <code> NodeIterator </code> .
     *
     */
    NodeIterator createNodeIterator ( const Node &root, long whatToShow, 
				      const NodeFilter &filter, bool entityReferenceExpansion );

    /**
     * Create a new TreeWalker over the subtree rooted by the
     * specified node.
     *
     * @param root The node which will serve as the root for the
     * <code> TreeWalker </code> . The currentNode of the TreeWalker
     * is set to this node. The whatToShow flags and the NodeFilter
     * are not considered when setting this value; any node type will
     * be accepted as the root. The root must not be null.
     *
     * @param whatToShow This flag specifies which node types may
     * appear in the logical view of the tree presented by the
     * Iterator. See the description of TreeWalker for the set of
     * possible values. These flags can be combined using OR.
     *
     *  These flags can be combined using <code> OR </code> .
     *
     * @param filter The Filter to be used with this TreeWalker, or
     * null to indicate no filter.
     *
     * @param entityReferenceExpansion The value of this flag
     * determines whether entity reference nodes are expanded.
     *
     * @return The newly created <code> TreeWalker </code> .
     *
     * @exception DOMException
     * Raises the exception NOT_SUPPORTED_ERR if the specified root
     * node is null.
     *
     */
    TreeWalker createTreeWalker ( const Node &root, long whatToShow, 
				  const NodeFilter &filter, bool entityReferenceExpansion );
};


}; // namespace

#endif
