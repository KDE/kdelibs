/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 *
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
#ifndef _DOM_Document_h_
#define _DOM_Document_h_

#include <dom_node.h>

namespace DOM {

class DOMString;
class DocumentType;
class NodeList;
class CDATASection;
class Comment;
class DocumentFragment;
class Text;
class DOMImplementation;
class Element;
class Attr;
class EntityReference;
class ProcessingInstruction;
class DocumentImpl;


/**
 * The <code> DOMImplementation </code> interface provides a number of
 * methods for performing operations that are independent of any
 * particular instance of the document object model.
 *
 *  The DOM Level 1 does not specify a way of creating a document
 * instance, and hence document creation is an operation specific to
 * an implementation. Future Levels of the DOM specification are
 * expected to provide methods for creating documents directly.
 *
 */
class DOMImplementation
{
public:
    DOMImplementation();
    DOMImplementation(const DOMImplementation &other);

    DOMImplementation & operator = (const DOMImplementation &other);
    ~DOMImplementation();

    /**
     * Test if the DOM implementation implements a specific feature.
     *
     * @param feature The package name of the feature to test. In
     * Level 1, the legal values are "HTML" and "XML"
     * (case-insensitive).
     *
     * @param version This is the version number of the package name
     * to test. In Level 1, this is the string "1.0". If the version
     * is not specified, supporting any version of the feature will
     * cause the method to return <code> true </code> .
     *
     * @return <code> true </code> if the feature is implemented in
     * the specified version, <code> false </code> otherwise.
     *
     */
    bool hasFeature ( const DOMString &feature, const DOMString &version );
};


/**
 * The <code> Document </code> interface represents the entire HTML or
 * XML document. Conceptually, it is the root of the document tree,
 * and provides the primary access to the document's data.
 *
 *  Since elements, text nodes, comments, processing instructions,
 * etc. cannot exist outside the context of a <code> Document </code>
 * , the <code> Document </code> interface also contains the factory
 * methods needed to create these objects. The <code> Node </code>
 * objects created have a <code> ownerDocument </code> attribute which
 * associates them with the <code> Document </code> within whose
 * context they were created.
 *
 */
class Document : public Node
{
public:
    Document();
    /** 
     * don't create an implementation if false
     * use at own risk
     */
    Document(bool);
    Document(const Document &other);

    Document & operator = (const Document &other);

    ~Document();

    /**
     * The Document Type Declaration (see <code> DocumentType </code>
     * ) associated with this document. For HTML documents as well as
     * XML documents without a document type declaration this returns
     * <code> null </code> . The DOM Level 1 does not support editing
     * the Document Type Declaration, therefore <code> docType </code>
     * cannot be altered in any way.
     *
     */
    DocumentType doctype() const;

    /**
     * The <code> DOMImplementation </code> object that handles this
     * document. A DOM application may use objects from multiple
     * implementations.
     *
     */
    DOMImplementation implementation() const;

    /**
     * This is a convenience attribute that allows direct access to
     * the child node that is the root element of the document. For
     * HTML documents, this is the element with the tagName "HTML".
     *
     */
    Element documentElement() const;

    /**
     * Creates an element of the type specified. Note that the
     * instance returned implements the Element interface, so
     * attributes can be specified directly on the returned object.
     *
     * @param tagName The name of the element type to instantiate. For
     * XML, this is case-sensitive. For HTML, the <code> tagName
     * </code> parameter may be provided in any case, but it must be
     * mapped to the canonical uppercase form by the DOM
     * implementation.
     *
     * @return A new <code> Element </code> object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified name contains an
     * invalid character.
     *
     */
    Element createElement ( const DOMString &tagName );

    /**
     * Creates an empty <code> DocumentFragment </code> object.
     *
     * @return A new <code> DocumentFragment </code> .
     *
     */
    DocumentFragment createDocumentFragment (  );

    /**
     * Creates a <code> Text </code> node given the specified string.
     *
     * @param data The data for the node.
     *
     * @return The new <code> Text </code> object.
     *
     */
    Text createTextNode ( const DOMString &data );

    /**
     * Creates a <code> Comment </code> node given the specified
     * string.
     *
     * @param data The data for the node.
     *
     * @return The new <code> Comment </code> object.
     *
     */
    Comment createComment ( const DOMString &data );

    /**
     * Creates a <code> CDATASection </code> node whose value is the
     * specified string.
     *
     * @param data The data for the <code> CDATASection </code>
     * contents.
     *
     * @return The new <code> CDATASection </code> object.
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if this document is an HTML document.
     *
     */
    CDATASection createCDATASection ( const DOMString &data );

    /**
     * Creates a <code> ProcessingInstruction </code> node given the
     * specified name and data strings.
     *
     * @param target The target part of the processing instruction.
     *
     * @param data The data for the node.
     *
     * @return The new <code> ProcessingInstruction </code> object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if an invalid character is
     * specified.
     *
     *  NOT_SUPPORTED_ERR: Raised if this document is an HTML
     * document.
     *
     */
    ProcessingInstruction createProcessingInstruction ( const DOMString &target, const DOMString &data );

    /**
     * Creates an <code> Attr </code> of the given name. Note that the
     * <code> Attr </code> instance can then be set on an <code>
     * Element </code> using the <code> setAttribute </code> method.
     *
     * @param name The name of the attribute.
     *
     * @return A new <code> Attr </code> object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified name contains an
     * invalid character.
     *
     */
    Attr createAttribute ( const DOMString &name );

    /**
     * Creates an EntityReference object.
     *
     * @param name The name of the entity to reference.
     *
     * @return The new <code> EntityReference </code> object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified name contains an
     * invalid character.
     *
     *  NOT_SUPPORTED_ERR: Raised if this document is an HTML
     * document.
     *
     */
    EntityReference createEntityReference ( const DOMString &name );

    /**
     * Returns a <code> NodeList </code> of all the <code> Element
     * </code> s with a given tag name in the order in which they
     * would be encountered in a preorder traversal of the <code>
     * Document </code> tree.
     *
     * @param tagname The name of the tag to match on. The special
     * value "*" matches all tags.
     *
     * @return A new <code> NodeList </code> object containing all the
     * matched <code> Element </code> s.
     *
     */
    NodeList getElementsByTagName ( const DOMString &tagname );

protected:
    Document( DocumentImpl *i);
    
    friend class Node;
};

class DocumentFragmentImpl;

/**
 * <code> DocumentFragment </code> is a "lightweight" or "minimal"
 * <code> Document </code> object. It is very common to want to be
 * able to extract a portion of a document's tree or to create a new
 * fragment of a document. Imagine implementing a user command like
 * cut or rearranging a document by moving fragments around. It is
 * desirable to have an object which can hold such fragments and it is
 * quite natural to use a Node for this purpose. While it is true that
 * a <code> Document </code> object could fulfil this role, a <code>
 * Document </code> object can potentially be a heavyweight object,
 * depending on the underlying implementation. What is really needed
 * for this is a very lightweight object. <code> DocumentFragment
 * </code> is such an object.
 *
 *  Furthermore, various operations -- such as inserting nodes as
 * children of another <code> Node </code> -- may take <code>
 * DocumentFragment </code> objects as arguments; this results in all
 * the child nodes of the <code> DocumentFragment </code> being moved
 * to the child list of this node.
 *
 *  The children of a <code> DocumentFragment </code> node are zero or
 * more nodes representing the tops of any sub-trees defining the
 * structure of the document. <code> DocumentFragment </code> nodes do
 * not need to be well-formed XML documents (although they do need to
 * follow the rules imposed upon well-formed XML parsed entities,
 * which can have multiple top nodes). For example, a <code>
 * DocumentFragment </code> might have only one child and that child
 * node could be a <code> Text </code> node. Such a structure model
 * represents neither an HTML document nor a well-formed XML document.
 *
 *  When a <code> DocumentFragment </code> is inserted into a <code>
 * Document </code> (or indeed any other <code> Node </code> that may
 * take children) the children of the <code> DocumentFragment </code>
 * and not the <code> DocumentFragment </code> itself are inserted
 * into the <code> Node </code> . This makes the <code>
 * DocumentFragment </code> very useful when the user wishes to create
 * nodes that are siblings; the <code> DocumentFragment </code> acts
 * as the parent of these nodes so that the user can use the standard
 * methods from the <code> Node </code> interface, such as <code>
 * insertBefore() </code> and <code> appendChild() </code> .
 *
 */
class DocumentFragment : public Node
{
    friend class Document;

public:
    DocumentFragment();
    DocumentFragment(const DocumentFragment &other);

    DocumentFragment & operator = (const DocumentFragment &other);

    ~DocumentFragment();

protected:
    DocumentFragment(DocumentFragmentImpl *i);
};

class NamedNodeMap;
class DOMString;

/**
 * Each <code> Document </code> has a <code> doctype </code> attribute
 * whose value is either <code> null </code> or a <code> DocumentType
 * </code> object. The <code> DocumentType </code> interface in the
 * DOM Level 1 Core provides an interface to the list of entities that
 * are defined for the document, and little else because the effect of
 * namespaces and the various XML scheme efforts on DTD representation
 * are not clearly understood as of this writing.
 *
 *  The DOM Level 1 doesn't support editing <code> DocumentType
 * </code> nodes.
 *
 */
class DocumentType : public Node
{
public:
    DocumentType();
    DocumentType(const DocumentType &other);

    DocumentType & operator = (const DocumentType &other);

    ~DocumentType();

    /**
     * The name of DTD; i.e., the name immediately following the
     * <code> DOCTYPE </code> keyword.
     *
     */
    DOMString name() const;

    /**
     * A <code> NamedNodeMap </code> containing the general entities,
     * both external and internal, declared in the DTD. Duplicates are
     * discarded. For example in: &lt;!DOCTYPE ex SYSTEM "ex.dtd" [
     * &lt;!ENTITY foo "foo"> &lt;!ENTITY bar "bar"> &lt;!ENTITY % baz
     * "baz"> ]> &lt;ex/> the interface provides access to <code> foo
     * </code> and <code> bar </code> but not <code> baz </code> .
     * Every node in this map also implements the <code> Entity
     * </code> interface.
     *
     *  The DOM Level 1 does not support editing entities, therefore
     * <code> entities </code> cannot be altered in any way.
     *
     */
    NamedNodeMap entities() const;

    /**
     * A <code> NamedNodeMap </code> containing the notations declared
     * in the DTD. Duplicates are discarded. Every node in this map
     * also implements the <code> Notation </code> interface.
     *
     *  The DOM Level 1 does not support editing notations, therefore
     * <code> notations </code> cannot be altered in any way.
     *
     */
    NamedNodeMap notations() const;
};

}; //namespace
#endif
