#include <QDomDocument>
#include <QDomText>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDebug>

#include "dom.h"

using namespace KJSEmbed;

DomNodeBinding::DomNodeBinding( KJS::ExecState *exec, const QDomNode &value )
    : ScalarBinding(exec, "QDomNode", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
}

namespace DomNodeNS
{
START_SCALAR_METHOD( nodeType, QDomNode )
    result = KJS::Number((int)value.nodeType());
END_SCALAR_METHOD

START_SCALAR_METHOD( nodeName, QDomNode )
    result = KJS::String( value.nodeName());
END_SCALAR_METHOD

START_SCALAR_METHOD( nodeValue, QDomNode )
    result = KJS::String( value.nodeValue());
END_SCALAR_METHOD

START_SCALAR_METHOD( appendChild, QDomNode )
    QDomNode newNode = KJSEmbed::extractScalar<QDomNode>( exec, args, 0);
    QDomNode node = value.appendChild(newNode);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( attributes, QDomNode )
    QDomNamedNodeMap map = value.attributes();
    result = KJSEmbed::createScalar(exec, "QDomNamedNodeMap", map);
END_SCALAR_METHOD

START_SCALAR_METHOD( childNodes, QDomNode )
    QDomNodeList nodes = value.childNodes();
    result = KJSEmbed::createScalar(exec, "QDomNodeList", nodes);
END_SCALAR_METHOD

START_SCALAR_METHOD( clear, QDomNode )
    value.clear();
END_SCALAR_METHOD

START_SCALAR_METHOD( cloneNode, QDomNode )
    bool deep = KJSEmbed::extractBool( exec, args, 0, true);
    QDomNode node = value.cloneNode(deep);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( firstChild, QDomNode )
    QDomNode node = value.firstChild();
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( firstChildElement, QDomNode )
    QString name = KJSEmbed::extractQString(exec, args, 0);
    QDomElement node = value.firstChildElement( name );
    result = KJSEmbed::createScalar( exec, "QDomElement", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( hasAttributes, QDomNode )
    result = KJS::Boolean( value.hasAttributes() );
END_SCALAR_METHOD

START_SCALAR_METHOD( hasChildNodes, QDomNode )
    result = KJS::Boolean( value.hasChildNodes() );
END_SCALAR_METHOD

START_SCALAR_METHOD( insertBefore, QDomNode )
    QDomNode first = KJSEmbed::extractScalar<QDomNode>(exec, args, 0);
    QDomNode second = KJSEmbed::extractScalar<QDomNode>(exec, args, 1);
    QDomNode node = value.insertBefore( first, second );
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( insertAfter, QDomNode )
    QDomNode first = KJSEmbed::extractScalar<QDomNode>(exec, args, 0);
    QDomNode second = KJSEmbed::extractScalar<QDomNode>(exec, args, 1);
    QDomNode node = value.insertAfter( first, second );
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( isAttr, QDomNode )
    result = KJS::Boolean( value.isAttr() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isCDATASection, QDomNode )
    result = KJS::Boolean( value.isCDATASection() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isCharacterData, QDomNode )
    result = KJS::Boolean( value.isCharacterData() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isComment, QDomNode )
    result = KJS::Boolean( value.isComment() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isDocument, QDomNode )
    result = KJS::Boolean( value.isDocument() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isDocumentFragment, QDomNode )
    result = KJS::Boolean( value.isDocumentFragment() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isDocumentType, QDomNode )
    result = KJS::Boolean( value.isDocumentType() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isElement, QDomNode )
    result = KJS::Boolean( value.isElement() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isEntity, QDomNode )
    result = KJS::Boolean( value.isEntity() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isEntityReference, QDomNode )
    result = KJS::Boolean( value.isEntityReference() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isNotation, QDomNode )
    result = KJS::Boolean( value.isNotation() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isNull, QDomNode )
    result = KJS::Boolean( value.isNull() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isProcessingInstruction, QDomNode )
    result = KJS::Boolean( value.isProcessingInstruction() );
END_SCALAR_METHOD

START_SCALAR_METHOD( isSupported, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QString arg1 = KJSEmbed::extractQString(exec, args, 1);
    result = KJS::Boolean( value.isSupported( arg0, arg1 ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( isText, QDomNode )
    result = KJS::Boolean( value.isText() );
END_SCALAR_METHOD

START_SCALAR_METHOD( lastChild, QDomNode )
    QDomNode node = value.lastChild();
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( lastChildElement, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomElement node = value.lastChildElement( arg0 );
    result = KJSEmbed::createScalar(exec, "QDomElement", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( localName, QDomNode )
    result = KJS::String( value.localName() );
END_SCALAR_METHOD

START_SCALAR_METHOD( namedItem, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomNode node = value.namedItem( arg0 );
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( namespaceURI, QDomNode )
    result = KJS::String( value.namespaceURI() );
END_SCALAR_METHOD

START_SCALAR_METHOD( nextSibling, QDomNode )
    QDomNode node = value.nextSibling();
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( nextSiblingElement, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomElement node = value.nextSiblingElement( arg0 );
    result = KJSEmbed::createScalar(exec, "QDomElement", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( normalize, QDomNode )
    value.normalize();
END_SCALAR_METHOD

START_SCALAR_METHOD( ownerDocument, QDomNode )
    QDomDocument doc = value.ownerDocument();
    result = KJSEmbed::createScalar( exec, "QDomDocument", doc);
END_SCALAR_METHOD

START_SCALAR_METHOD( parentNode, QDomNode )
    QDomNode parent = value.parentNode();
    result = KJSEmbed::createScalar(exec, "QDomNode", parent );
END_SCALAR_METHOD

START_SCALAR_METHOD( prefix, QDomNode )
    result = KJS::String( value.prefix() );
END_SCALAR_METHOD

START_SCALAR_METHOD( previousSibling, QDomNode )
    QDomNode node = value.previousSibling();
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( previousSiblingElement, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomNode node = value.previousSiblingElement( arg0 );
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( removeChild, QDomNode )
    QDomNode arg0 = KJSEmbed::extractScalar<QDomNode>(exec, args, 0);
    QDomNode node = value.removeChild( arg0 );
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( replaceChild, QDomNode )
    QDomNode arg0 = KJSEmbed::extractScalar<QDomNode>(exec, args, 0);
    QDomNode arg1 =KJSEmbed::extractScalar<QDomNode>(exec, args, 1);
    QDomNode node = value.replaceChild(arg0,arg1);
    result = KJSEmbed::createScalar(exec, "QDomNode", node );
END_SCALAR_METHOD

START_SCALAR_METHOD( setPrefix, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.setPrefix(arg0);
END_SCALAR_METHOD

START_SCALAR_METHOD( setNodeValue, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.setNodeValue(arg0);
END_SCALAR_METHOD

START_SCALAR_METHOD( toAttr, QDomNode )
    QDomAttr attr = value.toAttr(  );
    result = KJSEmbed::createScalar(exec, "QDomAttr", attr );
END_SCALAR_METHOD

START_SCALAR_METHOD( toElement, QDomNode )
    QDomElement elem = value.toElement(  );
    result = KJSEmbed::createScalar(exec, "QDomElement", elem );
END_SCALAR_METHOD
}

START_METHOD_LUT( DomNode )
    {"appendChild", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::appendChild},
    {"attributes", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::attributes},
    {"childNodes", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::childNodes},
    {"clear", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::clear},
    {"cloneNode", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::cloneNode},
    {"firstChild", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::firstChild},
    {"firstChildElement", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::firstChildElement},
    {"hasAttributes", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::hasAttributes},
    {"hasChildNodes", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::hasChildNodes},
    {"insertBefore", 2, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::insertBefore},
    {"insertAfter", 2, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::insertAfter},
    {"isAttr", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isAttr},
    {"isCDATASection", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isCDATASection},
    {"isCharacterData", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isCharacterData},
    {"isComment", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isComment},
    {"isDocument", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isDocument},
    {"isDocumentFragment", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isDocumentFragment},
    {"isDocumentType", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isDocumentType},
    {"isElement", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isElement},
    {"isEntity", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isEntity},
    {"isEntityReference", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isEntityReference},
    {"isNotation", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isNotation},
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isNull},
    {"isProcessingInstruction", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isProcessingInstruction},
    {"isSupported", 2, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isSupported},
    {"isText", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::isText},
    {"lastChild", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::lastChild},
    {"lastChildElement", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::lastChildElement},
    {"localName", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::localName},
    {"namedItem", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::namedItem},
    {"namespaceURI", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::namespaceURI},
    {"nextSibling", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::nextSibling},
    {"nextSiblingElement", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::nextSiblingElement},
    {"nodeType", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::nodeType},
    {"nodeName", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::nodeName},
    {"nodeValue", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::nodeValue},
    {"normalize", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::normalize},
    {"ownerDocument", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::ownerDocument},
    {"parentNode", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::parentNode},
    {"prefix", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::prefix},
    {"previousSibling", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::previousSibling},
    {"previousSiblingElement", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::previousSiblingElement},
    {"removeChild", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::removeChild},
    {"replaceChild", 2, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::replaceChild},
    {"setPrefix", 1, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::setPrefix},
    {"setNodeValue", 2, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::setNodeValue},
    {"toAttr", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::toAttr},
    {"toElement", 0, KJS::DontDelete|KJS::ReadOnly, &DomNodeNS::toElement}
END_METHOD_LUT

START_ENUM_LUT( DomNode )
    {"ElementNode", QDomNode::ElementNode},
    {"AttributeNode", QDomNode::AttributeNode},
    {"TextNode", QDomNode::TextNode},
    {"CDATASectionNode", QDomNode::CDATASectionNode},
    {"EntityReferenceNode", QDomNode::EntityReferenceNode},
    {"EntityNode", QDomNode::EntityNode},
    {"ProcessingInstructionNode", QDomNode::ProcessingInstructionNode},
    {"CommentNode", QDomNode::CommentNode},
    {"DocumentNode", QDomNode::DocumentNode},
    {"DocumentTypeNode", QDomNode::DocumentTypeNode},
    {"DocumentFragmentNode", QDomNode::DocumentFragmentNode},
    {"NotationNode", QDomNode::NotationNode},
    {"BaseNode", QDomNode::BaseNode},
    {"CharacterDataNode", QDomNode::CharacterDataNode}
END_ENUM_LUT

NO_STATICS( DomNode )

START_CTOR( DomNode, QDomNode, 0 )
    return new KJSEmbed::DomNodeBinding(exec, QDomNode( ) );
END_CTOR


DomDocumentBinding::DomDocumentBinding( KJS::ExecState *exec, const QDomDocument &value )
    : ScalarBinding(exec, "QDomDocument", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomDocument::methods() );
}

namespace DomDocumentNS
{
QString parserErrorTemplate = "XML Parse error '%1' at %2,%3";

START_SCALAR_METHOD( setContent, QDomDocument )
    QString xml = KJSEmbed::extractQString( exec, args, 0);
    QString message;
    int row = 0;
    int col = 0;
    bool success = value.setContent(xml, &message, &row, &col );
    result = KJS::Boolean( success );
    if( !success )
    {
        KJS::throwError(exec, KJS::GeneralError, parserErrorTemplate.arg(message).arg(row).arg(col) );
        // throwError(exec, parserErrorTemplate.arg(message).arg(row).arg(col) );
    }
END_SCALAR_METHOD

START_SCALAR_METHOD( toString, QDomDocument )
    int indent = KJSEmbed::extractInt( exec, args, 0, 1);
    result = KJS::String(value.toString(indent));
END_SCALAR_METHOD

START_SCALAR_METHOD( documentElement, QDomDocument )
    QDomElement elem = value.documentElement();
    result = KJSEmbed::createScalar(exec, "QDomElement", elem);
END_SCALAR_METHOD

START_SCALAR_METHOD( elementById, QDomDocument )
    QString id = KJSEmbed::extractQString(exec, args, 0);
    QDomElement elem = value.elementById(id);
    result = KJSEmbed::createScalar(exec, "QDomElement", elem);
END_SCALAR_METHOD

START_SCALAR_METHOD( createAttribute, QDomDocument )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomAttr attr = value.createAttribute( name );
    result = KJSEmbed::createScalar(exec, "QDomAttr", attr);
END_SCALAR_METHOD

START_SCALAR_METHOD( createAttributeNS, QDomDocument )
    QString nsURI = KJSEmbed::extractQString( exec, args, 0);
    QString qName = KJSEmbed::extractQString( exec, args, 1);
    QDomAttr attr = value.createAttributeNS( nsURI, qName );
    result = KJSEmbed::createScalar(exec, "QDomAttr", attr);
END_SCALAR_METHOD

START_SCALAR_METHOD( createCDATASection, QDomDocument )
    QString cdatatxt = KJSEmbed::extractQString( exec, args, 0);
    QDomCDATASection  cdata = value.createCDATASection( cdatatxt );
    result = KJSEmbed::createScalar(exec, "QDomCDATASection", cdata);
END_SCALAR_METHOD

START_SCALAR_METHOD( createComment, QDomDocument )
    QString commenttxt = KJSEmbed::extractQString( exec, args, 0);
    QDomComment comment = value.createComment( commenttxt );
    result = KJSEmbed::createScalar(exec, "QDomComment", comment);
END_SCALAR_METHOD

START_SCALAR_METHOD( createDocumentFragment, QDomDocument )
    QDomDocumentFragment fragment = value.createDocumentFragment();
    result = KJSEmbed::createScalar(exec, "QDomDocumentFragment", fragment);
END_SCALAR_METHOD

START_SCALAR_METHOD( createElement, QDomDocument )
    QString tagName = KJSEmbed::extractQString( exec, args, 0);
    QDomElement elem = value.createElement( tagName );
    result = KJSEmbed::createScalar(exec, "QDomElement", elem);
END_SCALAR_METHOD

START_SCALAR_METHOD( createElementNS, QDomDocument )
    QString nsURI = KJSEmbed::extractQString( exec, args, 0);
    QString qName = KJSEmbed::extractQString( exec, args, 1);
    QDomElement elem = value.createElementNS( nsURI, qName );
    result = KJSEmbed::createScalar(exec, "QDomElement", elem);
END_SCALAR_METHOD

START_SCALAR_METHOD( createEntityReference, QDomDocument )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomEntityReference ref = value.createEntityReference( name );
    result = KJSEmbed::createScalar(exec, "QDomEntityReference", ref);
END_SCALAR_METHOD

START_SCALAR_METHOD( createProcessingInstruction, QDomDocument )
    QString target = KJSEmbed::extractQString( exec, args, 0);
    QString data = KJSEmbed::extractQString( exec, args, 1);
    QDomProcessingInstruction inst = value.createProcessingInstruction(target, data );
    result = KJSEmbed::createScalar(exec, "QDomProcessingInstruction", inst);
END_SCALAR_METHOD

START_SCALAR_METHOD( createTextNode, QDomDocument )
    QString texttext = KJSEmbed::extractQString( exec, args, 0);
    QDomText text = value.createTextNode( texttext );
    result = KJSEmbed::createScalar(exec, "QDomText", text);
END_SCALAR_METHOD

START_SCALAR_METHOD( importNode, QDomDocument )
    QDomNode import = KJSEmbed::extractScalar<QDomNode>(exec, args, 0);
    bool deep = KJSEmbed::extractBool( exec, args, 1);
    QDomNode node = value.importNode( import, deep );
    result = KJSEmbed::createScalar(exec, "QDomNode",  node);
END_SCALAR_METHOD

}

START_METHOD_LUT( DomDocument )
    {"setContent", 1, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::setContent},
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::toString},
    {"documentElement", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::documentElement},
    {"elementById", 1, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::elementById},
    {"createAttribute", 1, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createAttribute },
    {"createAttributeNS", 2, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createAttributeNS },
    {"createCDATASection", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createCDATASection},
    {"createComment", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createComment},
    {"createDocumentFragment", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createDocumentFragment},
    {"createElement", 1, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createElement},
    {"createElementNS", 2, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createElementNS},
    {"createEntityReference", 1, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createEntityReference},
    {"createProcessingInstruction", 2, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createProcessingInstruction},
    {"createTextNode", 1, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::createTextNode},
    {"importNode", 2, KJS::DontDelete|KJS::ReadOnly, &DomDocumentNS::importNode}
END_METHOD_LUT

NO_ENUMS( DomDocument )
NO_STATICS( DomDocument )

START_CTOR( DomDocument, QDomDocument, 1 )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    return new KJSEmbed::DomDocumentBinding(exec, QDomDocument(name) );
END_CTOR

DomElementBinding::DomElementBinding( KJS::ExecState *exec, const QDomElement &value )
    : ScalarBinding(exec, "QDomElement", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomElement::methods() );
}

namespace DomElementNS {
START_SCALAR_METHOD( toString, QDomElement )
    result = KJS::String( value.text( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( tagName, QDomElement )
    result = KJS::String( value.tagName( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( setTagName, QDomElement )
    QString tag = KJSEmbed::extractQString(exec, args, 0);
    value.setTagName(tag);
END_SCALAR_METHOD

START_SCALAR_METHOD( attribute, QDomElement )
    QString tag = KJSEmbed::extractQString(exec, args, 0);
    QString defaultValue = KJSEmbed::extractQString(exec, args, 1, QString::null);
    result = KJS::String( value.attribute(tag,defaultValue) );
END_SCALAR_METHOD

START_SCALAR_METHOD( setAttribute, QDomElement )
    QString tag = KJSEmbed::extractQString(exec, args, 0);
    QString newValue = KJSEmbed::extractQString(exec, args, 1);
    value.setAttribute(tag,newValue);
END_SCALAR_METHOD

START_SCALAR_METHOD( hasAttribute, QDomElement )
    QString attr = KJSEmbed::extractQString(exec, args, 0);
    result = KJS::Boolean( value.hasAttribute(attr) );
END_SCALAR_METHOD

START_SCALAR_METHOD( removeAttribute, QDomElement )
    QString attr = KJSEmbed::extractQString(exec, args, 0);
    value.removeAttribute( attr );
END_SCALAR_METHOD

START_SCALAR_METHOD( setAttributeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString attr = KJSEmbed::extractQString(exec, args, 1);
    QString val = KJSEmbed::extractQString(exec, args, 3);
    value.setAttributeNS( ns, attr, val );
END_SCALAR_METHOD

START_SCALAR_METHOD( attributeNS, QDomElement )
    QString nsURI = KJSEmbed::extractQString(exec, args, 0);
    QString localName = KJSEmbed::extractQString(exec, args, 1);
    QString defValue = KJSEmbed::extractQString( exec, args, 1, QString::null );
    result = KJS::String( value.attributeNS( nsURI, localName, defValue ));
END_SCALAR_METHOD

START_SCALAR_METHOD( hasAttributeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString attr = KJSEmbed::extractQString(exec, args, 1);
    result = KJS::Boolean( value.hasAttributeNS(ns, attr) );
END_SCALAR_METHOD

START_SCALAR_METHOD( removeAttributeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString attr = KJSEmbed::extractQString(exec, args, 1);
    value.removeAttributeNS( ns, attr );
END_SCALAR_METHOD

START_SCALAR_METHOD( elementsByTagName, QDomElement )
    QString name = KJSEmbed::extractQString(exec, args, 0);
    QDomNodeList nodes = value.elementsByTagName(name);
    result = KJSEmbed::createScalar(exec, "QDomNodeList", nodes);
END_SCALAR_METHOD

START_SCALAR_METHOD( elementsByTagNameNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString name = KJSEmbed::extractQString(exec, args, 1);
    QDomNodeList nodes = value.elementsByTagNameNS( ns, name );
    result = KJSEmbed::createScalar(exec, "QDomNodeList", nodes);
END_SCALAR_METHOD

START_SCALAR_METHOD( attributeNode, QDomElement )
    QString name = KJSEmbed::extractQString(exec, args, 0);
    QDomAttr attr = value.attributeNode( name );
    result = KJSEmbed::createScalar(exec, "QDomAttr", attr);
END_SCALAR_METHOD

START_SCALAR_METHOD( attributeNodeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString name = KJSEmbed::extractQString(exec, args, 1);
    QDomAttr attr = value.attributeNodeNS( ns, name );
    result = KJSEmbed::createScalar(exec, "QDomAttr", attr);
END_SCALAR_METHOD

START_SCALAR_METHOD( removeAttributeNode, QDomElement )
    QDomAttr attr = KJSEmbed::extractScalar<QDomAttr>( exec, args, 0);
    QDomAttr newAttr = value.removeAttributeNode( attr );
    result = KJSEmbed::createScalar(exec, "QDomAttr", newAttr);
END_SCALAR_METHOD

START_SCALAR_METHOD( setAttributeNode, QDomElement )
    QDomAttr attr = KJSEmbed::extractScalar<QDomAttr>( exec, args, 0);
    QDomAttr newAttr = value.setAttributeNode( attr );
    result = KJSEmbed::createScalar(exec, "QDomAttr", newAttr);
END_SCALAR_METHOD

START_SCALAR_METHOD( setAttributeNodeNS, QDomElement )
    QDomAttr attr = KJSEmbed::extractScalar<QDomAttr>( exec, args, 0);
    QDomAttr newAttr = value.setAttributeNodeNS( attr );
    result = KJSEmbed::createScalar(exec, "QDomAttr", newAttr);
END_SCALAR_METHOD
}

NO_ENUMS( DomElement )
NO_STATICS( DomElement )

START_METHOD_LUT( DomElement )
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::toString},
    {"text", 0, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::toString},
    {"tagName", 0, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::tagName},
    {"setTagName", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::setTagName},
    {"setAttribute", 2, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::setAttribute},
    {"attribute", 2, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::attribute},
    {"hasAttribute", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::hasAttribute},
    {"removeAttribute", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::removeAttribute},
    {"setAttributeNS", 3, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::setAttributeNS},
    {"attributeNS", 3, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::attributeNS},
    {"hasAttributeNS", 2, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::hasAttributeNS},
    {"removeAttributeNS", 2, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::removeAttributeNS},
    {"elementsByTagName", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::elementsByTagName},
    {"elementsByTagNameNS", 2, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::elementsByTagNameNS},
    {"attributeNode", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::attributeNode},
    {"attributeNodeNS", 2, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::attributeNodeNS},
    {"removeAttributeNode", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::removeAttributeNode},
    {"setAttributeNode", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::setAttributeNode},
    {"setAttributeNodeNS", 1, KJS::DontDelete|KJS::ReadOnly, &DomElementNS::setAttributeNodeNS}
END_METHOD_LUT

START_CTOR( DomElement, QDomElement, 0 )
    return new KJSEmbed::DomElementBinding(exec, QDomElement( ) );
END_CTOR

DomAttrBinding::DomAttrBinding( KJS::ExecState *exec, const QDomAttr &value )
    : ScalarBinding(exec, "QDomAttr", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomAttr::methods() );
}

namespace AttrElementNS {
START_SCALAR_METHOD( name, QDomAttr )
    result = KJS::String( value.value( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( specified, QDomAttr )
    result = KJS::Boolean( value.specified( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( ownerElement, QDomAttr )
    QDomElement owner = value.ownerElement();
    result = KJSEmbed::createScalar(exec, "QDomElement", owner);
END_SCALAR_METHOD

START_SCALAR_METHOD( value, QDomAttr )
    result = KJS::String( value.value( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( setValue, QDomAttr )
    QString newValue = KJSEmbed::extractQString(exec, args, 0);
    value.setValue( newValue );
END_SCALAR_METHOD

}

NO_ENUMS( DomAttr )
NO_STATICS( DomAttr )

START_METHOD_LUT( DomAttr )
    {"name", 0, KJS::DontDelete|KJS::ReadOnly, &AttrElementNS::name},
    {"specified", 0, KJS::DontDelete|KJS::ReadOnly, &AttrElementNS::specified},
    {"ownerElement", 0, KJS::DontDelete|KJS::ReadOnly, &AttrElementNS::ownerElement},
    {"value", 0, KJS::DontDelete|KJS::ReadOnly, &AttrElementNS::value},
    {"setValue", 1, KJS::DontDelete|KJS::ReadOnly, &AttrElementNS::setValue}
END_METHOD_LUT

START_CTOR( DomAttr, QDomAttr, 0 )
    return new KJSEmbed::DomAttrBinding(exec, QDomAttr( ) );
END_CTOR

DomNodeListBinding::DomNodeListBinding( KJS::ExecState *exec, const QDomNodeList &value )
    : ScalarBinding(exec, "QDomNodeList", value )
{
    StaticBinding::publish( exec, this, DomNodeList::methods() );
}

namespace NodeListNS {
START_SCALAR_METHOD( count, QDomNodeList )
    result = KJS::Number( value.count( ) );
END_SCALAR_METHOD	

START_SCALAR_METHOD( length, QDomNodeList )
    result = KJS::Number( value.length( ) );
END_SCALAR_METHOD	

START_SCALAR_METHOD( item, QDomNodeList )
    int idx = KJSEmbed::extractInt( exec, args, 0);
    QDomNode node = value.item(idx);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD	

}

NO_ENUMS( DomNodeList )
NO_STATICS( DomNodeList )

START_METHOD_LUT( DomNodeList )
    {"count", 0, KJS::DontDelete|KJS::ReadOnly, &NodeListNS::count},
    {"length", 0, KJS::DontDelete|KJS::ReadOnly, &NodeListNS::length},
    {"item", 1, KJS::DontDelete|KJS::ReadOnly, &NodeListNS::item}
END_METHOD_LUT

START_CTOR( DomNodeList, QDomNodeList, 0 )
    return new KJSEmbed::DomNodeListBinding(exec, QDomNodeList( ) );
END_CTOR

DomDocumentTypeBinding::DomDocumentTypeBinding( KJS::ExecState *exec, const QDomDocumentType &value )
    : ScalarBinding(exec, "QDomDocumentType", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomDocumentType::methods() );
}

namespace DomDocumentTypeNS {
START_SCALAR_METHOD( internalSubset, QDomDocumentType )
    result = KJS::String( value.internalSubset( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( name, QDomDocumentType )
    result = KJS::String( value.name( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( publicId, QDomDocumentType )
    result = KJS::String( value.publicId( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( systemId, QDomDocumentType )
    result = KJS::String( value.systemId( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( entities, QDomDocumentType )
    result = KJSEmbed::createScalar(exec, "QDomNamedNodeMap", value.entities( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( notations, QDomDocumentType )
    result = KJSEmbed::createScalar(exec, "QDomNamedNodeMap", value.notations( ) );
END_SCALAR_METHOD
}

NO_ENUMS( DomDocumentType )
NO_STATICS( DomDocumentType )

START_METHOD_LUT( DomDocumentType )
    {"entities", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentTypeNS::entities},
    {"notations", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentTypeNS::notations},
    {"internalSubset", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentTypeNS::internalSubset},
    {"name", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentTypeNS::name},
    {"publicId", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentTypeNS::publicId},
    {"systemId", 0, KJS::DontDelete|KJS::ReadOnly, &DomDocumentTypeNS::systemId}
END_METHOD_LUT

START_CTOR( DomDocumentType, QDomDocumentType, 0 )
    return new KJSEmbed::DomDocumentTypeBinding(exec, QDomDocumentType( ) );
END_CTOR

DomNamedNodeMapBinding::DomNamedNodeMapBinding( KJS::ExecState *exec, const QDomNamedNodeMap &value )
    : ScalarBinding(exec, "QDomNamedNodeMap", value )
{
    StaticBinding::publish( exec, this, DomNamedNodeMap::methods() );
}

namespace NamedNodeMapNS {
START_SCALAR_METHOD( contains, QDomNamedNodeMap )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    result = KJS::Boolean( value.contains(name) );
END_SCALAR_METHOD	

START_SCALAR_METHOD( count, QDomNamedNodeMap )
    result = KJS::Number(value.count());
END_SCALAR_METHOD

START_SCALAR_METHOD( item, QDomNamedNodeMap )
    int idx = KJSEmbed::extractInt( exec, args, 0);
    QDomNode node = value.item(idx);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD	

START_SCALAR_METHOD( length, QDomNamedNodeMap )
    result = KJS::Number( value.length( ) );
END_SCALAR_METHOD

START_SCALAR_METHOD( namedItem, QDomNamedNodeMap )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomNode node = value.namedItem(name);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD	

START_SCALAR_METHOD( namedItemNS, QDomNamedNodeMap )
    QString nsuri = KJSEmbed::extractQString( exec, args, 0);
    QString localName = KJSEmbed::extractQString( exec, args, 1);
    QDomNode node = value.namedItemNS(nsuri, localName);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( removeNamedItem, QDomNamedNodeMap )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomNode node = value.removeNamedItem( name );
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( removeNamedItemNS, QDomNamedNodeMap )
    QString nsuri = KJSEmbed::extractQString( exec, args, 0);
    QString localName = KJSEmbed::extractQString( exec, args, 1);
    QDomNode node = value.removeNamedItemNS(nsuri, localName);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

START_SCALAR_METHOD( setNamedItem, QDomNamedNodeMap )
    QDomNode newNode = KJSEmbed::extractScalar<QDomNode>(exec,args,0);
    QDomNode node = value.setNamedItem(newNode);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD	

START_SCALAR_METHOD( setNamedItemNS, QDomNamedNodeMap )
    QDomNode newNode = KJSEmbed::extractScalar<QDomNode>(exec,args,0);
    QDomNode node = value.setNamedItemNS(newNode);
    result = KJSEmbed::createScalar(exec, "QDomNode", node);
END_SCALAR_METHOD

}

NO_ENUMS( DomNamedNodeMap )
NO_STATICS( DomNamedNodeMap )

START_METHOD_LUT( DomNamedNodeMap )
    {"contains", 0, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::contains},
    {"count", 0, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::count},
    {"item", 1, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::item},
    {"length", 0, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::length},
    {"namedItem", 1, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::namedItem},
    {"namedItemNS", 2, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::namedItemNS},
    {"removeNamedItem", 1, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::removeNamedItem},
    {"removeNamedItemNS", 2, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::removeNamedItemNS},
    {"setNamedItem", 1, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::setNamedItem},
    {"setNamedItemNS", 1, KJS::DontDelete|KJS::ReadOnly, &NamedNodeMapNS::setNamedItemNS}
END_METHOD_LUT

START_CTOR( DomNamedNodeMap, QDomNamedNodeMap, 0 )
    return new KJSEmbed::DomNamedNodeMapBinding(exec, QDomNamedNodeMap( ) );
END_CTOR

DomTextBinding::DomTextBinding( KJS::ExecState *exec, const QDomText &value )
    : ScalarBinding(exec, "QDomText", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomText::methods() );
}

NO_ENUMS( DomText )
NO_STATICS( DomText )
NO_METHODS( DomText )

START_CTOR( DomText, QDomText, 0 )
    return new KJSEmbed::DomTextBinding(exec, QDomText( ) );
END_CTOR

