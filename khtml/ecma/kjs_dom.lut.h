/* Automatically generated from kjs_dom.cpp using ../../kjs/create_hash_table. DO NOT EDIT ! */

namespace KJS {

const struct HashEntry DOMNodeProtoTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "insertBefore", DOMNode::InsertBefore, DontDelete|Function, 2, 0 },
   { 0, 0, 0, 0, 0 },
   { "hasAttributes", DOMNode::HasAttributes, DontDelete|Function, 0, &DOMNodeProtoTableEntries[13] },
   { "removeChild", DOMNode::RemoveChild, DontDelete|Function, 1, &DOMNodeProtoTableEntries[11] },
   { "replaceChild", DOMNode::ReplaceChild, DontDelete|Function, 2, &DOMNodeProtoTableEntries[12] },
   { "hasChildNodes", DOMNode::HasChildNodes, DontDelete|Function, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "dispatchEvent", DOMNode::DispatchEvent, DontDelete|Function, 1, 0 },
   { "addEventListener", DOMNode::AddEventListener, DontDelete|Function, 3, 0 },
   { "appendChild", DOMNode::AppendChild, DontDelete|Function, 1, &DOMNodeProtoTableEntries[14] },
   { "cloneNode", DOMNode::CloneNode, DontDelete|Function, 1, 0 },
   { "removeEventListener", DOMNode::RemoveEventListener, DontDelete|Function, 3, 0 },
   { "contains", DOMNode::Contains, DontDelete|Function, 1, 0 }
};

const struct HashTable DOMNodeProtoTable = { 2, 15, DOMNodeProtoTableEntries, 11 };

}; // namespace

namespace KJS {

const struct HashEntry DOMNodeTableEntries[] = {
   { "offsetTop", DOMNode::OffsetTop, DontDelete|ReadOnly, 0, 0 },
   { "onload", DOMNode::OnLoad, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "onmouseout", DOMNode::OnMouseOut, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "ownerDocument", DOMNode::OwnerDocument, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[64] },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "nodeName", DOMNode::NodeName, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[54] },
   { "onselect", DOMNode::OnSelect, DontDelete, 0, 0 },
   { "childNodes", DOMNode::ChildNodes, DontDelete|ReadOnly, 0, 0 },
   { "previousSibling", DOMNode::PreviousSibling, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[53] },
   { "onunload", DOMNode::OnUnload, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "lastChild", DOMNode::LastChild, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "clientHeight", DOMNode::ClientHeight, DontDelete|ReadOnly, 0, 0 },
   { "onblur", DOMNode::OnBlur, DontDelete, 0, 0 },
   { "onfocus", DOMNode::OnFocus, DontDelete, 0, 0 },
   { "onmove", DOMNode::OnMove, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "onreset", DOMNode::OnReset, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "firstChild", DOMNode::FirstChild, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[60] },
   { "nodeValue", DOMNode::NodeValue, DontDelete, 0, 0 },
   { "onresize", DOMNode::OnResize, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "parentNode", DOMNode::ParentNode, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[55] },
   { 0, 0, 0, 0, 0 },
   { "attributes", DOMNode::Attributes, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[58] },
   { "onkeydown", DOMNode::OnKeyDown, DontDelete, 0, 0 },
   { "onkeyup", DOMNode::OnKeyUp, DontDelete, 0, 0 },
   { "ondblclick", DOMNode::OnDblClick, DontDelete, 0, &DOMNodeTableEntries[62] },
   { "parentElement", DOMNode::ParentElement, DontDelete|ReadOnly, 0, 0 },
   { "onchange", DOMNode::OnChange, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "onmousedwn", DOMNode::OnMouseDown, DontDelete, 0, &DOMNodeTableEntries[63] },
   { 0, 0, 0, 0, 0 },
   { "nodeType", DOMNode::NodeType, DontDelete|ReadOnly, 0, 0 },
   { "nextSibling", DOMNode::NextSibling, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[59] },
   { "onkeypress", DOMNode::OnKeyPress, DontDelete, 0, &DOMNodeTableEntries[56] },
   { 0, 0, 0, 0, 0 },
   { "onmouseup", DOMNode::OnMouseUp, DontDelete, 0, 0 },
   { "onclick", DOMNode::OnClick, DontDelete, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "onmouseover", DOMNode::OnMouseOver, DontDelete, 0, 0 },
   { "onabort", DOMNode::OnAbort, DontDelete, 0, 0 },
   { "ondragdrop", DOMNode::OnDragDrop, DontDelete, 0, 0 },
   { "onerror", DOMNode::OnError, DontDelete, 0, &DOMNodeTableEntries[57] },
   { "onmousemove", DOMNode::OnMouseMove, DontDelete, 0, 0 },
   { "onsubmit", DOMNode::OnSubmit, DontDelete, 0, 0 },
   { "offsetLeft", DOMNode::OffsetLeft, DontDelete|ReadOnly, 0, 0 },
   { "offsetWidth", DOMNode::OffsetWidth, DontDelete|ReadOnly, 0, &DOMNodeTableEntries[61] },
   { "offsetHeight", DOMNode::OffsetHeight, DontDelete|ReadOnly, 0, 0 },
   { "offsetParent", DOMNode::OffsetParent, DontDelete|ReadOnly, 0, 0 },
   { "clientWidth", DOMNode::ClientWidth, DontDelete|ReadOnly, 0, 0 },
   { "scrollLeft", DOMNode::ScrollLeft, DontDelete|ReadOnly, 0, 0 },
   { "scrollTop", DOMNode::ScrollTop, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMNodeTable = { 2, 65, DOMNodeTableEntries, 53 };

}; // namespace

namespace KJS {

const struct HashEntry DOMAttrTableEntries[] = {
   { "specified", DOMAttr::Specified, DontDelete|ReadOnly, 0, 0 },
   { "value", DOMAttr::ValueProperty, DontDelete|ReadOnly, 0, 0 },
   { "name", DOMAttr::Name, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "ownerElement", DOMAttr::OwnerElement, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMAttrTable = { 2, 5, DOMAttrTableEntries, 5 };

}; // namespace

namespace KJS {

const struct HashEntry DOMDocumentProtoTableEntries[] = {
   { "createCDATASection", DOMDocument::CreateCDATASection, DontDelete|Function, 1, 0 },
   { 0, 0, 0, 0, 0 },
   { "createDocumentFragment", DOMDocument::CreateDocumentFragment, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[30] },
   { 0, 0, 0, 0, 0 },
   { "getElementsByTagName", DOMDocument::GetElementsByTagName, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[25] },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "createElement", DOMDocument::CreateElement, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[23] },
   { "createTreeWalker", DOMDocument::CreateTreeWalker, DontDelete|Function, 4, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "createAttribute", DOMDocument::CreateAttribute, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[24] },
   { "createTextNode", DOMDocument::CreateTextNode, DontDelete|Function, 1, 0 },
   { "createEntityReference", DOMDocument::CreateEntityReference, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[26] },
   { "createProcessingInstruction", DOMDocument::CreateProcessingInstruction, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[28] },
   { 0, 0, 0, 0, 0 },
   { "createComment", DOMDocument::CreateComment, DontDelete|Function, 1, &DOMDocumentProtoTableEntries[27] },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "createNodeIterator", DOMDocument::CreateNodeIterator, DontDelete|Function, 3, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "createElementNS", DOMDocument::CreateElementNS, DontDelete|Function, 2, &DOMDocumentProtoTableEntries[29] },
   { "createAttributeNS", DOMDocument::CreateAttributeNS, DontDelete|Function, 2, 0 },
   { "getElementsByTagNameNS", DOMDocument::GetElementsByTagNameNS, DontDelete|Function, 2, 0 },
   { "getElementById", DOMDocument::GetElementById, DontDelete|Function, 1, 0 },
   { "createRange", DOMDocument::CreateRange, DontDelete|Function, 0, 0 },
   { "createEvent", DOMDocument::CreateEvent, DontDelete|Function, 1, 0 },
   { "styleSheets", DOMDocument::StyleSheets, DontDelete|Function, 0, 0 },
   { "getOverrideStyle", DOMDocument::GetOverrideStyle, DontDelete|Function, 2, 0 }
};

const struct HashTable DOMDocumentProtoTable = { 2, 31, DOMDocumentProtoTableEntries, 23 };

}; // namespace

namespace KJS {

const struct HashEntry DOMDocumentTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { "doctype", DOMDocument::DocType, DontDelete|ReadOnly, 0, &DOMDocumentTableEntries[3] },
   { "documentElement", DOMDocument::DocumentElement, DontDelete|ReadOnly, 0, 0 },
   { "implementation", DOMDocument::Implementation, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMDocumentTable = { 2, 4, DOMDocumentTableEntries, 3 };

}; // namespace

namespace KJS {

const struct HashEntry DOMElementProtoTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { "removeAttributeNode", DOMElement::RemoveAttributeNode, DontDelete|Function, 1, 0 },
   { 0, 0, 0, 0, 0 },
   { "getAttribute", DOMElement::GetAttribute, DontDelete|Function, 1, &DOMElementProtoTableEntries[11] },
   { "setAttribute", DOMElement::SetAttribute, DontDelete|Function, 2, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "removeAttribute", DOMElement::RemoveAttribute, DontDelete|Function, 1, 0 },
   { "getAttributeNode", DOMElement::GetAttributeNode, DontDelete|Function, 1, 0 },
   { "setAttributeNode", DOMElement::SetAttributeNode, DontDelete|Function, 2, &DOMElementProtoTableEntries[12] },
   { "hasAttribute", DOMElement::HasAttribute, DontDelete|Function, 1, 0 },
   { "getElementsByTagName", DOMElement::GetElementsByTagName, DontDelete|Function, 1, 0 },
   { "normalize", DOMElement::Normalize, DontDelete|Function, 0, 0 }
};

const struct HashTable DOMElementProtoTable = { 2, 13, DOMElementProtoTableEntries, 11 };

}; // namespace

namespace KJS {

const struct HashEntry DOMElementTableEntries[] = {
   { "style", DOMElement::Style, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "tagName", DOMElement::TagName, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMElementTable = { 2, 3, DOMElementTableEntries, 3 };

}; // namespace

namespace KJS {

const struct HashEntry DOMDOMImplementationProtoTableEntries[] = {
   { "hasFeature", DOMDOMImplementation::HasFeature, DontDelete|Function, 2, 0 },
   { 0, 0, 0, 0, 0 },
   { "createCSSStyleSheet", DOMDOMImplementation::CreateCSSStyleSheet, DontDelete|Function, 2, 0 }
};

const struct HashTable DOMDOMImplementationProtoTable = { 2, 3, DOMDOMImplementationProtoTableEntries, 3 };

}; // namespace

namespace KJS {

const struct HashEntry DOMDocumentTypeTableEntries[] = {
   { "name", DOMDocumentType::Name, DontDelete|ReadOnly, 0, 0 },
   { "notations", DOMDocumentType::Notations, DontDelete|ReadOnly, 0, 0 },
   { "entities", DOMDocumentType::Entities, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMDocumentTypeTable = { 2, 3, DOMDocumentTypeTableEntries, 3 };

}; // namespace

namespace KJS {

const struct HashEntry DOMNamedNodeMapProtoTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { "setNamedItem", DOMNamedNodeMap::SetNamedItem, DontDelete|Function, 1, &DOMNamedNodeMapProtoTableEntries[5] },
   { 0, 0, 0, 0, 0 },
   { "removeNamedItem", DOMNamedNodeMap::RemoveNamedItem, DontDelete|Function, 1, 0 },
   { "getNamedItem", DOMNamedNodeMap::GetNamedItem, DontDelete|Function, 1, 0 },
   { "item", DOMNamedNodeMap::Item, DontDelete|Function, 1, 0 }
};

const struct HashTable DOMNamedNodeMapProtoTable = { 2, 6, DOMNamedNodeMapProtoTableEntries, 5 };

}; // namespace

namespace KJS {

const struct HashEntry DOMProcessingInstructionTableEntries[] = {
   { "sheet", DOMProcessingInstruction::Sheet, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "target", DOMProcessingInstruction::Target, DontDelete|ReadOnly, 0, &DOMProcessingInstructionTableEntries[3] },
   { "data", DOMProcessingInstruction::Data, DontDelete, 0, 0 }
};

const struct HashTable DOMProcessingInstructionTable = { 2, 4, DOMProcessingInstructionTableEntries, 3 };

}; // namespace

namespace KJS {

const struct HashEntry DOMNotationTableEntries[] = {
   { "publicId", DOMNotation::PublicId, DontDelete|ReadOnly, 0, &DOMNotationTableEntries[2] },
   { 0, 0, 0, 0, 0 },
   { "systemId", DOMNotation::SystemId, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMNotationTable = { 2, 3, DOMNotationTableEntries, 2 };

}; // namespace

namespace KJS {

const struct HashEntry DOMEntityTableEntries[] = {
   { "publicId", DOMEntity::PublicId, DontDelete|ReadOnly, 0, &DOMEntityTableEntries[2] },
   { "notationName", DOMEntity::NotationName, DontDelete|ReadOnly, 0, 0 },
   { "systemId", DOMEntity::SystemId, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMEntityTable = { 2, 3, DOMEntityTableEntries, 2 };

}; // namespace

namespace KJS {

const struct HashEntry NodeConstructorTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "CDATA_SECTION_NODE", NodeConstructor::CDATA_SECTION_NODE, DontDelete|ReadOnly, 0, 0 },
   { "ATTRIBUTE_NODE", NodeConstructor::ATTRIBUTE_NODE, DontDelete|ReadOnly, 0, &NodeConstructorTableEntries[12] },
   { "DOCUMENT_FRAGMENT_NODE", NodeConstructor::DOCUMENT_FRAGMENT_NODE, DontDelete|ReadOnly, 0, 0 },
   { "DOCUMENT_TYPE_NODE", NodeConstructor::DOCUMENT_TYPE_NODE, DontDelete|ReadOnly, 0, 0 },
   { "DOCUMENT_NODE", NodeConstructor::DOCUMENT_NODE, DontDelete|ReadOnly, 0, 0 },
   { "COMMENT_NODE", NodeConstructor::COMMENT_NODE, DontDelete|ReadOnly, 0, 0 },
   { "ENTITY_NODE", NodeConstructor::ENTITY_NODE, DontDelete|ReadOnly, 0, &NodeConstructorTableEntries[13] },
   { "ELEMENT_NODE", NodeConstructor::ELEMENT_NODE, DontDelete|ReadOnly, 0, 0 },
   { "TEXT_NODE", NodeConstructor::TEXT_NODE, DontDelete|ReadOnly, 0, &NodeConstructorTableEntries[11] },
   { "ENTITY_REFERENCE_NODE", NodeConstructor::ENTITY_REFERENCE_NODE, DontDelete|ReadOnly, 0, 0 },
   { "PROCESSING_INSTRUCTION_NODE", NodeConstructor::PROCESSING_INSTRUCTION_NODE, DontDelete|ReadOnly, 0, 0 },
   { "NOTATION_NODE", NodeConstructor::NOTATION_NODE, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable NodeConstructorTable = { 2, 14, NodeConstructorTableEntries, 11 };

}; // namespace

namespace KJS {

const struct HashEntry DOMExceptionConstructorTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "WRONG_DOCUMENT_ERR", DOMExceptionConstructor::WRONG_DOCUMENT_ERR, DontDelete|ReadOnly, 0, 0 },
   { "INUSE_ATTRIBUTE_ERR", DOMExceptionConstructor::INUSE_ATTRIBUTE_ERR, DontDelete|ReadOnly, 0, 0 },
   { "INDEX_SIZE_ERR", DOMExceptionConstructor::INDEX_SIZE_ERR, DontDelete|ReadOnly, 0, 0 },
   { "INVALID_CHARACTER_ERR", DOMExceptionConstructor::INVALID_CHARACTER_ERR, DontDelete|ReadOnly, 0, &DOMExceptionConstructorTableEntries[17] },
   { "NAMESPACE_ERR", DOMExceptionConstructor::NAMESPACE_ERR, DontDelete|ReadOnly, 0, 0 },
   { "NO_DATA_ALLOWED_ERR", DOMExceptionConstructor::NO_DATA_ALLOWED_ERR, DontDelete|ReadOnly, 0, &DOMExceptionConstructorTableEntries[16] },
   { "DOMSTRING_SIZE_ERR", DOMExceptionConstructor::DOMSTRING_SIZE_ERR, DontDelete|ReadOnly, 0, 0 },
   { "NOT_FOUND_ERR", DOMExceptionConstructor::NOT_FOUND_ERR, DontDelete|ReadOnly, 0, &DOMExceptionConstructorTableEntries[15] },
   { 0, 0, 0, 0, 0 },
   { "NO_MODIFICATION_ALLOWED_ERR", DOMExceptionConstructor::NO_MODIFICATION_ALLOWED_ERR, DontDelete|ReadOnly, 0, &DOMExceptionConstructorTableEntries[18] },
   { "HIERARCHY_REQUEST_ERR", DOMExceptionConstructor::HIERARCHY_REQUEST_ERR, DontDelete|ReadOnly, 0, 0 },
   { "INVALID_MODIFICATION_ERR", DOMExceptionConstructor::INVALID_MODIFICATION_ERR, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "NOT_SUPPORTED_ERR", DOMExceptionConstructor::NOT_SUPPORTED_ERR, DontDelete|ReadOnly, 0, 0 },
   { "INVALID_STATE_ERR", DOMExceptionConstructor::INVALID_STATE_ERR, DontDelete|ReadOnly, 0, 0 },
   { "SYNTAX_ERR", DOMExceptionConstructor::SYNTAX_ERR, DontDelete|ReadOnly, 0, 0 },
   { "INVALID_ACCESS_ERR", DOMExceptionConstructor::INVALID_ACCESS_ERR, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMExceptionConstructorTable = { 2, 19, DOMExceptionConstructorTableEntries, 15 };

}; // namespace
