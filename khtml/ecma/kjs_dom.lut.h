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

const struct HashEntry DOMDocumentTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "implementation", DOMDocument::Implementation, DontDelete|ReadOnly, 0, 0 },
   { "getElementsByTagName", DOMDocument::GetElementsByTagName, DontDelete|ReadOnly|Function, 1, &DOMDocumentTableEntries[30] },
   { "createTreeWalker", DOMDocument::CreateTreeWalker, DontDelete|ReadOnly|Function, 4, 0 },
   { 0, 0, 0, 0, 0 },
   { "doctype", DOMDocument::DocType, DontDelete|ReadOnly, 0, 0 },
   { "createProcessingInstruction", DOMDocument::CreateProcessingInstruction, DontDelete|ReadOnly|Function, 1, 0 },
   { "getElementsByTagNameNS", DOMDocument::GetElementsByTagNameNS, DontDelete|ReadOnly|Function, 2, 0 },
   { "createTextNode", DOMDocument::CreateTextNode, DontDelete|ReadOnly|Function, 1, 0 },
   { 0, 0, 0, 0, 0 },
   { "styleSheets", DOMDocument::StyleSheets, DontDelete|ReadOnly|Function, 0, 0 },
   { "createCDATASection", DOMDocument::CreateCDATASection, DontDelete|ReadOnly|Function, 1, 0 },
   { 0, 0, 0, 0, 0 },
   { "createNodeIterator", DOMDocument::CreateNodeIterator, DontDelete|ReadOnly|Function, 3, 0 },
   { 0, 0, 0, 0, 0 },
   { "createElement", DOMDocument::CreateElement, DontDelete|ReadOnly|Function, 1, &DOMDocumentTableEntries[27] },
   { "documentElement", DOMDocument::DocumentElement, DontDelete|ReadOnly, 0, &DOMDocumentTableEntries[26] },
   { "createEntityReference", DOMDocument::CreateEntityReference, DontDelete|ReadOnly|Function, 1, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "createElementNS", DOMDocument::CreateElementNS, DontDelete|ReadOnly|Function, 2, &DOMDocumentTableEntries[28] },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "createEvent", DOMDocument::CreateEvent, DontDelete|ReadOnly|Function, 1, 0 },
   { "createComment", DOMDocument::CreateComment, DontDelete|ReadOnly|Function, 1, 0 },
   { "createDocumentFragment", DOMDocument::CreateDocumentFragment, DontDelete|ReadOnly|Function, 1, &DOMDocumentTableEntries[31] },
   { "createAttribute", DOMDocument::CreateAttribute, DontDelete|ReadOnly|Function, 1, &DOMDocumentTableEntries[29] },
   { "createAttributeNS", DOMDocument::CreateAttributeNS, DontDelete|ReadOnly|Function, 2, 0 },
   { "getElementById", DOMDocument::GetElementById, DontDelete|ReadOnly|Function, 1, 0 },
   { "createRange", DOMDocument::CreateRange, DontDelete|ReadOnly|Function, 0, 0 },
   { "getOverrideStyle", DOMDocument::GetOverrideStyle, DontDelete|ReadOnly|Function, 2, 0 }
};

const struct HashTable DOMDocumentTable = { 2, 32, DOMDocumentTableEntries, 26 };

}; // namespace
