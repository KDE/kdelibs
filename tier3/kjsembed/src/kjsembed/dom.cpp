/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "dom.h"

#include <QtXml/QDomAttr>
#include <QtCore/QDebug>

using namespace KJSEmbed;

const KJS::ClassInfo DomNodeBinding::info = { "QDomNode", &ValueBinding::info, 0, 0 };
DomNodeBinding::DomNodeBinding( KJS::ExecState *exec, const QDomNode &value )
    : ValueBinding(exec, "QDomNode", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
}

namespace DomNodeNS
{
START_VALUE_METHOD( nodeType, QDomNode )
    result = KJS::jsNumber((int)value.nodeType());
END_VALUE_METHOD

START_VALUE_METHOD( nodeName, QDomNode )
    result = KJS::jsString( value.nodeName());
END_VALUE_METHOD

START_VALUE_METHOD( nodeValue, QDomNode )
    result = KJS::jsString( value.nodeValue());
END_VALUE_METHOD

START_VALUE_METHOD( appendChild, QDomNode )
    QDomNode newNode = KJSEmbed::extractValue<QDomNode>( exec, args, 0);
    QDomNode node = value.appendChild(newNode);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( attributes, QDomNode )
    QDomNamedNodeMap map = value.attributes();
    result = KJSEmbed::createValue(exec, "QDomNamedNodeMap", map);
END_VALUE_METHOD

START_VALUE_METHOD( childNodes, QDomNode )
    QDomNodeList nodes = value.childNodes();
    result = KJSEmbed::createValue(exec, "QDomNodeList", nodes);
END_VALUE_METHOD

START_VALUE_METHOD( clear, QDomNode )
    value.clear();
END_VALUE_METHOD

START_VALUE_METHOD( cloneNode, QDomNode )
    bool deep = KJSEmbed::extractBool( exec, args, 0, true);
    QDomNode node = value.cloneNode(deep);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( firstChild, QDomNode )
    QDomNode node = value.firstChild();
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( firstChildElement, QDomNode )
    QString name = KJSEmbed::extractQString(exec, args, 0);
    QDomElement node = value.firstChildElement( name );
    result = KJSEmbed::createValue( exec, "QDomElement", node );
END_VALUE_METHOD

START_VALUE_METHOD( hasAttributes, QDomNode )
    result = KJS::jsBoolean( value.hasAttributes() );
END_VALUE_METHOD

START_VALUE_METHOD( hasChildNodes, QDomNode )
    result = KJS::jsBoolean( value.hasChildNodes() );
END_VALUE_METHOD

START_VALUE_METHOD( insertBefore, QDomNode )
    QDomNode first = KJSEmbed::extractValue<QDomNode>(exec, args, 0);
    QDomNode second = KJSEmbed::extractValue<QDomNode>(exec, args, 1);
    QDomNode node = value.insertBefore( first, second );
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( insertAfter, QDomNode )
    QDomNode first = KJSEmbed::extractValue<QDomNode>(exec, args, 0);
    QDomNode second = KJSEmbed::extractValue<QDomNode>(exec, args, 1);
    QDomNode node = value.insertAfter( first, second );
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( isAttr, QDomNode )
    result = KJS::jsBoolean( value.isAttr() );
END_VALUE_METHOD

START_VALUE_METHOD( isCDATASection, QDomNode )
    result = KJS::jsBoolean( value.isCDATASection() );
END_VALUE_METHOD

START_VALUE_METHOD( isCharacterData, QDomNode )
    result = KJS::jsBoolean( value.isCharacterData() );
END_VALUE_METHOD

START_VALUE_METHOD( isComment, QDomNode )
    result = KJS::jsBoolean( value.isComment() );
END_VALUE_METHOD

START_VALUE_METHOD( isDocument, QDomNode )
    result = KJS::jsBoolean( value.isDocument() );
END_VALUE_METHOD

START_VALUE_METHOD( isDocumentFragment, QDomNode )
    result = KJS::jsBoolean( value.isDocumentFragment() );
END_VALUE_METHOD

START_VALUE_METHOD( isDocumentType, QDomNode )
    result = KJS::jsBoolean( value.isDocumentType() );
END_VALUE_METHOD

START_VALUE_METHOD( isElement, QDomNode )
    result = KJS::jsBoolean( value.isElement() );
END_VALUE_METHOD

START_VALUE_METHOD( isEntity, QDomNode )
    result = KJS::jsBoolean( value.isEntity() );
END_VALUE_METHOD

START_VALUE_METHOD( isEntityReference, QDomNode )
    result = KJS::jsBoolean( value.isEntityReference() );
END_VALUE_METHOD

START_VALUE_METHOD( isNotation, QDomNode )
    result = KJS::jsBoolean( value.isNotation() );
END_VALUE_METHOD

START_VALUE_METHOD( isNull, QDomNode )
    result = KJS::jsBoolean( value.isNull() );
END_VALUE_METHOD

START_VALUE_METHOD( isProcessingInstruction, QDomNode )
    result = KJS::jsBoolean( value.isProcessingInstruction() );
END_VALUE_METHOD

START_VALUE_METHOD( isSupported, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QString arg1 = KJSEmbed::extractQString(exec, args, 1);
    result = KJS::jsBoolean( value.isSupported( arg0, arg1 ) );
END_VALUE_METHOD

START_VALUE_METHOD( isText, QDomNode )
    result = KJS::jsBoolean( value.isText() );
END_VALUE_METHOD

START_VALUE_METHOD( lastChild, QDomNode )
    QDomNode node = value.lastChild();
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( lastChildElement, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomElement node = value.lastChildElement( arg0 );
    result = KJSEmbed::createValue(exec, "QDomElement", node );
END_VALUE_METHOD

START_VALUE_METHOD( localName, QDomNode )
    result = KJS::jsString( value.localName() );
END_VALUE_METHOD

START_VALUE_METHOD( namedItem, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomNode node = value.namedItem( arg0 );
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( namespaceURI, QDomNode )
    result = KJS::jsString( value.namespaceURI() );
END_VALUE_METHOD

START_VALUE_METHOD( nextSibling, QDomNode )
    QDomNode node = value.nextSibling();
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( nextSiblingElement, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomElement node = value.nextSiblingElement( arg0 );
    result = KJSEmbed::createValue(exec, "QDomElement", node );
END_VALUE_METHOD

START_VALUE_METHOD( normalize, QDomNode )
    value.normalize();
END_VALUE_METHOD

START_VALUE_METHOD( ownerDocument, QDomNode )
    QDomDocument doc = value.ownerDocument();
    result = KJSEmbed::createValue( exec, "QDomDocument", doc);
END_VALUE_METHOD

START_VALUE_METHOD( parentNode, QDomNode )
    QDomNode parent = value.parentNode();
    result = KJSEmbed::createValue(exec, "QDomNode", parent );
END_VALUE_METHOD

START_VALUE_METHOD( prefix, QDomNode )
    result = KJS::jsString( value.prefix() );
END_VALUE_METHOD

START_VALUE_METHOD( previousSibling, QDomNode )
    QDomNode node = value.previousSibling();
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( previousSiblingElement, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QDomNode node = value.previousSiblingElement( arg0 );
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( removeChild, QDomNode )
    QDomNode arg0 = KJSEmbed::extractValue<QDomNode>(exec, args, 0);
    QDomNode node = value.removeChild( arg0 );
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( replaceChild, QDomNode )
    QDomNode arg0 = KJSEmbed::extractValue<QDomNode>(exec, args, 0);
    QDomNode arg1 =KJSEmbed::extractValue<QDomNode>(exec, args, 1);
    QDomNode node = value.replaceChild(arg0,arg1);
    result = KJSEmbed::createValue(exec, "QDomNode", node );
END_VALUE_METHOD

START_VALUE_METHOD( setPrefix, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.setPrefix(arg0);
END_VALUE_METHOD

START_VALUE_METHOD( setNodeValue, QDomNode )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.setNodeValue(arg0);
END_VALUE_METHOD

START_VALUE_METHOD( toAttr, QDomNode )
    QDomAttr attr = value.toAttr(  );
    result = KJSEmbed::createValue(exec, "QDomAttr", attr );
END_VALUE_METHOD

START_VALUE_METHOD( toElement, QDomNode )
    QDomElement elem = value.toElement(  );
    result = KJSEmbed::createValue(exec, "QDomElement", elem );
END_VALUE_METHOD
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

const KJS::ClassInfo DomDocumentBinding::info = { "QDomDocument", &ValueBinding::info, 0, 0 };
DomDocumentBinding::DomDocumentBinding( KJS::ExecState *exec, const QDomDocument &value )
    : ValueBinding(exec, "QDomDocument", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomDocument::methods() );
}

namespace DomDocumentNS
{
QString parserErrorTemplate = "XML Parse error '%1' at %2,%3";

START_VALUE_METHOD( setContent, QDomDocument )
    QString xml = KJSEmbed::extractQString( exec, args, 0);
    QString message;
    int row = 0;
    int col = 0;
    bool success = value.setContent(xml, &message, &row, &col );
    result = KJS::jsBoolean( success );
    if( !success )
    {
        KJS::throwError(exec, KJS::SyntaxError, parserErrorTemplate.arg(message).arg(row).arg(col) );
    }
END_VALUE_METHOD

START_VALUE_METHOD( toString, QDomDocument )
    int indent = KJSEmbed::extractInt( exec, args, 0, 1);
    result = KJS::jsString(value.toString(indent));
END_VALUE_METHOD

START_VALUE_METHOD( documentElement, QDomDocument )
    QDomElement elem = value.documentElement();
    result = KJSEmbed::createValue(exec, "QDomElement", elem);
END_VALUE_METHOD

START_VALUE_METHOD( elementById, QDomDocument )
    QString id = KJSEmbed::extractQString(exec, args, 0);
    QDomElement elem = value.elementById(id);
    result = KJSEmbed::createValue(exec, "QDomElement", elem);
END_VALUE_METHOD

START_VALUE_METHOD( createAttribute, QDomDocument )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomAttr attr = value.createAttribute( name );
    result = KJSEmbed::createValue(exec, "QDomAttr", attr);
END_VALUE_METHOD

START_VALUE_METHOD( createAttributeNS, QDomDocument )
    QString nsURI = KJSEmbed::extractQString( exec, args, 0);
    QString qName = KJSEmbed::extractQString( exec, args, 1);
    QDomAttr attr = value.createAttributeNS( nsURI, qName );
    result = KJSEmbed::createValue(exec, "QDomAttr", attr);
END_VALUE_METHOD

START_VALUE_METHOD( createCDATASection, QDomDocument )
    QString cdatatxt = KJSEmbed::extractQString( exec, args, 0);
    QDomCDATASection  cdata = value.createCDATASection( cdatatxt );
    result = KJSEmbed::createValue(exec, "QDomCDATASection", cdata);
END_VALUE_METHOD

START_VALUE_METHOD( createComment, QDomDocument )
    QString commenttxt = KJSEmbed::extractQString( exec, args, 0);
    QDomComment comment = value.createComment( commenttxt );
    result = KJSEmbed::createValue(exec, "QDomComment", comment);
END_VALUE_METHOD

START_VALUE_METHOD( createDocumentFragment, QDomDocument )
    QDomDocumentFragment fragment = value.createDocumentFragment();
    result = KJSEmbed::createValue(exec, "QDomDocumentFragment", fragment);
END_VALUE_METHOD

START_VALUE_METHOD( createElement, QDomDocument )
    QString tagName = KJSEmbed::extractQString( exec, args, 0);
    QDomElement elem = value.createElement( tagName );
    result = KJSEmbed::createValue(exec, "QDomElement", elem);
END_VALUE_METHOD

START_VALUE_METHOD( createElementNS, QDomDocument )
    QString nsURI = KJSEmbed::extractQString( exec, args, 0);
    QString qName = KJSEmbed::extractQString( exec, args, 1);
    QDomElement elem = value.createElementNS( nsURI, qName );
    result = KJSEmbed::createValue(exec, "QDomElement", elem);
END_VALUE_METHOD

START_VALUE_METHOD( createEntityReference, QDomDocument )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomEntityReference ref = value.createEntityReference( name );
    result = KJSEmbed::createValue(exec, "QDomEntityReference", ref);
END_VALUE_METHOD

START_VALUE_METHOD( createProcessingInstruction, QDomDocument )
    QString target = KJSEmbed::extractQString( exec, args, 0);
    QString data = KJSEmbed::extractQString( exec, args, 1);
    QDomProcessingInstruction inst = value.createProcessingInstruction(target, data );
    result = KJSEmbed::createValue(exec, "QDomProcessingInstruction", inst);
END_VALUE_METHOD

START_VALUE_METHOD( createTextNode, QDomDocument )
    QString texttext = KJSEmbed::extractQString( exec, args, 0);
    QDomText text = value.createTextNode( texttext );
    result = KJSEmbed::createValue(exec, "QDomText", text);
END_VALUE_METHOD

START_VALUE_METHOD( importNode, QDomDocument )
    QDomNode import = KJSEmbed::extractValue<QDomNode>(exec, args, 0);
    bool deep = KJSEmbed::extractBool( exec, args, 1);
    QDomNode node = value.importNode( import, deep );
    result = KJSEmbed::createValue(exec, "QDomNode",  node);
END_VALUE_METHOD

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

const KJS::ClassInfo DomElementBinding::info = { "QDomElement", &ValueBinding::info, 0, 0 };
DomElementBinding::DomElementBinding( KJS::ExecState *exec, const QDomElement &value )
    : ValueBinding(exec, "QDomElement", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomElement::methods() );
}

namespace DomElementNS {
START_VALUE_METHOD( toString, QDomElement )
    result = KJS::jsString( value.text( ) );
END_VALUE_METHOD

START_VALUE_METHOD( tagName, QDomElement )
    result = KJS::jsString( value.tagName( ) );
END_VALUE_METHOD

START_VALUE_METHOD( setTagName, QDomElement )
    QString tag = KJSEmbed::extractQString(exec, args, 0);
    value.setTagName(tag);
END_VALUE_METHOD

START_VALUE_METHOD( attribute, QDomElement )
    QString tag = KJSEmbed::extractQString(exec, args, 0);
    QString defaultValue = KJSEmbed::extractQString(exec, args, 1, QString());
    result = KJS::jsString( value.attribute(tag,defaultValue) );
END_VALUE_METHOD

START_VALUE_METHOD( setAttribute, QDomElement )
    QString tag = KJSEmbed::extractQString(exec, args, 0);
    QString newValue = KJSEmbed::extractQString(exec, args, 1);
    value.setAttribute(tag,newValue);
END_VALUE_METHOD

START_VALUE_METHOD( hasAttribute, QDomElement )
    QString attr = KJSEmbed::extractQString(exec, args, 0);
    result = KJS::jsBoolean( value.hasAttribute(attr) );
END_VALUE_METHOD

START_VALUE_METHOD( removeAttribute, QDomElement )
    QString attr = KJSEmbed::extractQString(exec, args, 0);
    value.removeAttribute( attr );
END_VALUE_METHOD

START_VALUE_METHOD( setAttributeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString attr = KJSEmbed::extractQString(exec, args, 1);
    QString val = KJSEmbed::extractQString(exec, args, 3);
    value.setAttributeNS( ns, attr, val );
END_VALUE_METHOD

START_VALUE_METHOD( attributeNS, QDomElement )
    QString nsURI = KJSEmbed::extractQString(exec, args, 0);
    QString localName = KJSEmbed::extractQString(exec, args, 1);
    QString defValue = KJSEmbed::extractQString( exec, args, 1, QString() );
    result = KJS::jsString( value.attributeNS( nsURI, localName, defValue ));
END_VALUE_METHOD

START_VALUE_METHOD( hasAttributeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString attr = KJSEmbed::extractQString(exec, args, 1);
    result = KJS::jsBoolean( value.hasAttributeNS(ns, attr) );
END_VALUE_METHOD

START_VALUE_METHOD( removeAttributeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString attr = KJSEmbed::extractQString(exec, args, 1);
    value.removeAttributeNS( ns, attr );
END_VALUE_METHOD

START_VALUE_METHOD( elementsByTagName, QDomElement )
    QString name = KJSEmbed::extractQString(exec, args, 0);
    QDomNodeList nodes = value.elementsByTagName(name);
    result = KJSEmbed::createValue(exec, "QDomNodeList", nodes);
END_VALUE_METHOD

START_VALUE_METHOD( elementsByTagNameNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString name = KJSEmbed::extractQString(exec, args, 1);
    QDomNodeList nodes = value.elementsByTagNameNS( ns, name );
    result = KJSEmbed::createValue(exec, "QDomNodeList", nodes);
END_VALUE_METHOD

START_VALUE_METHOD( attributeNode, QDomElement )
    QString name = KJSEmbed::extractQString(exec, args, 0);
    QDomAttr attr = value.attributeNode( name );
    result = KJSEmbed::createValue(exec, "QDomAttr", attr);
END_VALUE_METHOD

START_VALUE_METHOD( attributeNodeNS, QDomElement )
    QString ns = KJSEmbed::extractQString(exec, args, 0);
    QString name = KJSEmbed::extractQString(exec, args, 1);
    QDomAttr attr = value.attributeNodeNS( ns, name );
    result = KJSEmbed::createValue(exec, "QDomAttr", attr);
END_VALUE_METHOD

START_VALUE_METHOD( removeAttributeNode, QDomElement )
    QDomAttr attr = KJSEmbed::extractValue<QDomAttr>( exec, args, 0);
    QDomAttr newAttr = value.removeAttributeNode( attr );
    result = KJSEmbed::createValue(exec, "QDomAttr", newAttr);
END_VALUE_METHOD

START_VALUE_METHOD( setAttributeNode, QDomElement )
    QDomAttr attr = KJSEmbed::extractValue<QDomAttr>( exec, args, 0);
    QDomAttr newAttr = value.setAttributeNode( attr );
    result = KJSEmbed::createValue(exec, "QDomAttr", newAttr);
END_VALUE_METHOD

START_VALUE_METHOD( setAttributeNodeNS, QDomElement )
    QDomAttr attr = KJSEmbed::extractValue<QDomAttr>( exec, args, 0);
    QDomAttr newAttr = value.setAttributeNodeNS( attr );
    result = KJSEmbed::createValue(exec, "QDomAttr", newAttr);
END_VALUE_METHOD
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

const KJS::ClassInfo DomAttrBinding::info = { "QDomAttr", &ValueBinding::info, 0, 0 };
DomAttrBinding::DomAttrBinding( KJS::ExecState *exec, const QDomAttr &value )
    : ValueBinding(exec, "QDomAttr", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomAttr::methods() );
}

namespace AttrElementNS {
START_VALUE_METHOD( name, QDomAttr )
    result = KJS::jsString( value.value( ) );
END_VALUE_METHOD

START_VALUE_METHOD( specified, QDomAttr )
    result = KJS::jsBoolean( value.specified( ) );
END_VALUE_METHOD

START_VALUE_METHOD( ownerElement, QDomAttr )
    QDomElement owner = value.ownerElement();
    result = KJSEmbed::createValue(exec, "QDomElement", owner);
END_VALUE_METHOD

START_VALUE_METHOD( value, QDomAttr )
    result = KJS::jsString( value.value( ) );
END_VALUE_METHOD

START_VALUE_METHOD( setValue, QDomAttr )
    QString newValue = KJSEmbed::extractQString(exec, args, 0);
    value.setValue( newValue );
END_VALUE_METHOD

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

const KJS::ClassInfo DomNodeListBinding::info = { "QDomNodeList", &ValueBinding::info, 0, 0 };
DomNodeListBinding::DomNodeListBinding( KJS::ExecState *exec, const QDomNodeList &value )
    : ValueBinding(exec, "QDomNodeList", value )
{
    StaticBinding::publish( exec, this, DomNodeList::methods() );
}

namespace NodeListNS {
START_VALUE_METHOD( count, QDomNodeList )
    result = KJS::jsNumber( value.count( ) );
END_VALUE_METHOD

START_VALUE_METHOD( length, QDomNodeList )
    result = KJS::jsNumber( value.length( ) );
END_VALUE_METHOD

START_VALUE_METHOD( item, QDomNodeList )
    int idx = KJSEmbed::extractInt( exec, args, 0);
    QDomNode node = value.item(idx);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

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

const KJS::ClassInfo DomDocumentTypeBinding::info = { "QDomDocumentType", &ValueBinding::info, 0, 0 };
DomDocumentTypeBinding::DomDocumentTypeBinding( KJS::ExecState *exec, const QDomDocumentType &value )
    : ValueBinding(exec, "QDomDocumentType", value )
{
    StaticBinding::publish( exec, this, DomNode::methods() );
    StaticBinding::publish( exec, this, DomDocumentType::methods() );
}

namespace DomDocumentTypeNS {
START_VALUE_METHOD( internalSubset, QDomDocumentType )
    result = KJS::jsString( value.internalSubset( ) );
END_VALUE_METHOD

START_VALUE_METHOD( name, QDomDocumentType )
    result = KJS::jsString( value.name( ) );
END_VALUE_METHOD

START_VALUE_METHOD( publicId, QDomDocumentType )
    result = KJS::jsString( value.publicId( ) );
END_VALUE_METHOD

START_VALUE_METHOD( systemId, QDomDocumentType )
    result = KJS::jsString( value.systemId( ) );
END_VALUE_METHOD

START_VALUE_METHOD( entities, QDomDocumentType )
    result = KJSEmbed::createValue(exec, "QDomNamedNodeMap", value.entities( ) );
END_VALUE_METHOD

START_VALUE_METHOD( notations, QDomDocumentType )
    result = KJSEmbed::createValue(exec, "QDomNamedNodeMap", value.notations( ) );
END_VALUE_METHOD
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

const KJS::ClassInfo DomNamedNodeMapBinding::info = { "QDomNamedNodeMap", &ValueBinding::info, 0, 0 };
DomNamedNodeMapBinding::DomNamedNodeMapBinding( KJS::ExecState *exec, const QDomNamedNodeMap &value )
    : ValueBinding(exec, "QDomNamedNodeMap", value )
{
    StaticBinding::publish( exec, this, DomNamedNodeMap::methods() );
}

namespace NamedNodeMapNS {
START_VALUE_METHOD( contains, QDomNamedNodeMap )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    result = KJS::jsBoolean( value.contains(name) );
END_VALUE_METHOD

START_VALUE_METHOD( count, QDomNamedNodeMap )
    result = KJS::jsNumber(value.count());
END_VALUE_METHOD

START_VALUE_METHOD( item, QDomNamedNodeMap )
    int idx = KJSEmbed::extractInt( exec, args, 0);
    QDomNode node = value.item(idx);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( length, QDomNamedNodeMap )
    result = KJS::jsNumber( value.length( ) );
END_VALUE_METHOD

START_VALUE_METHOD( namedItem, QDomNamedNodeMap )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomNode node = value.namedItem(name);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( namedItemNS, QDomNamedNodeMap )
    QString nsuri = KJSEmbed::extractQString( exec, args, 0);
    QString localName = KJSEmbed::extractQString( exec, args, 1);
    QDomNode node = value.namedItemNS(nsuri, localName);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( removeNamedItem, QDomNamedNodeMap )
    QString name = KJSEmbed::extractQString( exec, args, 0);
    QDomNode node = value.removeNamedItem( name );
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( removeNamedItemNS, QDomNamedNodeMap )
    QString nsuri = KJSEmbed::extractQString( exec, args, 0);
    QString localName = KJSEmbed::extractQString( exec, args, 1);
    QDomNode node = value.removeNamedItemNS(nsuri, localName);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( setNamedItem, QDomNamedNodeMap )
    QDomNode newNode = KJSEmbed::extractValue<QDomNode>(exec,args,0);
    QDomNode node = value.setNamedItem(newNode);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

START_VALUE_METHOD( setNamedItemNS, QDomNamedNodeMap )
    QDomNode newNode = KJSEmbed::extractValue<QDomNode>(exec,args,0);
    QDomNode node = value.setNamedItemNS(newNode);
    result = KJSEmbed::createValue(exec, "QDomNode", node);
END_VALUE_METHOD

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

const KJS::ClassInfo DomTextBinding::info = { "QDomText", &ValueBinding::info, 0, 0 };
DomTextBinding::DomTextBinding( KJS::ExecState *exec, const QDomText &value )
    : ValueBinding(exec, "QDomText", value )
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

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
