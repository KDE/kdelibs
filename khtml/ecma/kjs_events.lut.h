/* Automatically generated from kjs_events.cpp using ../../kjs/create_hash_table. DO NOT EDIT ! */

using namespace KJS;

namespace KJS {

const struct HashEntry EventConstructorTableEntries[] = {
   { "CAPTURING_PHASE", DOM::Event::CAPTURING_PHASE, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[3] },
   { "BUBBLING_PHASE", DOM::Event::BUBBLING_PHASE, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[6] },
   { "MOUSEOUT", 8, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[7] },
   { "AT_TARGET", DOM::Event::AT_TARGET, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[4] },
   { "MOUSEDOWN", 1, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[5] },
   { "MOUSEUP", 2, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[13] },
   { "MOUSEOVER", 4, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[8] },
   { "MOUSEMOVE", 16, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[11] },
   { "MOUSEDRAG", 32, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[9] },
   { "CLICK", 64, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[10] },
   { "DBLCLICK", 128, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[14] },
   { "KEYDOWN", 256, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[12] },
   { "KEYUP", 512, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[18] },
   { "KEYPRESS", 1024, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[15] },
   { "DRAGDROP", 2048, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[17] },
   { "FOCUS", 4096, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[16] },
   { "BLUR", 8192, DontDelete|ReadOnly, 0, 0 },
   { "SELECT", 16384, DontDelete|ReadOnly, 0, 0 },
   { "CHANGE", 32768, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable EventConstructorTable = { 2, 19, EventConstructorTableEntries, 3 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMEventTableEntries[] = {
   { "currentTarget", DOMEvent::CurrentTarget, DontDelete|ReadOnly, 0, &DOMEventTableEntries[7] },
   { 0, 0, 0, 0, 0 },
   { "type", DOMEvent::Type, DontDelete|ReadOnly, 0, 0 },
   { "target", DOMEvent::Target, DontDelete|ReadOnly, 0, &DOMEventTableEntries[9] },
   { 0, 0, 0, 0, 0 },
   { "returnValue", DOMEvent::ReturnValue, DontDelete, 0, &DOMEventTableEntries[11] },
   { "srcElement", DOMEvent::SrcElement, DontDelete|ReadOnly, 0, 0 },
   { "eventPhase", DOMEvent::EventPhase, DontDelete|ReadOnly, 0, &DOMEventTableEntries[8] },
   { "bubbles", DOMEvent::Bubbles, DontDelete|ReadOnly, 0, 0 },
   { "cancelable", DOMEvent::Cancelable, DontDelete|ReadOnly, 0, &DOMEventTableEntries[10] },
   { "timeStamp", DOMEvent::TimeStamp, DontDelete|ReadOnly, 0, 0 },
   { "cancelBubble", DOMEvent::CancelBubble, DontDelete, 0, 0 }
};

const struct HashTable DOMEventTable = { 2, 12, DOMEventTableEntries, 7 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMEventProtoTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "stopPropagation", DOMEvent::StopPropagation, DontDelete|Function, 0, &DOMEventProtoTableEntries[3] },
   { "preventDefault", DOMEvent::PreventDefault, DontDelete|Function, 0, &DOMEventProtoTableEntries[4] },
   { "initEvent", DOMEvent::InitEvent, DontDelete|Function, 3, 0 }
};

const struct HashTable DOMEventProtoTable = { 2, 5, DOMEventProtoTableEntries, 3 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry EventExceptionConstructorTableEntries[] = {
   { "UNSPECIFIED_EVENT_TYPE_ERR", DOM::EventException::UNSPECIFIED_EVENT_TYPE_ERR, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable EventExceptionConstructorTable = { 2, 1, EventExceptionConstructorTableEntries, 1 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMUIEventTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { "view", DOMUIEvent::View, DontDelete|ReadOnly, 0, &DOMUIEventTableEntries[2] },
   { "detail", DOMUIEvent::Detail, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMUIEventTable = { 2, 3, DOMUIEventTableEntries, 2 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMUIEventProtoTableEntries[] = {
   { "initUIEvent", DOMUIEvent::InitUIEvent, DontDelete|Function, 5, 0 }
};

const struct HashTable DOMUIEventProtoTable = { 2, 1, DOMUIEventProtoTableEntries, 1 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMMouseEventTableEntries[] = {
   { "screenX", DOMMouseEvent::ScreenX, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[3] },
   { "screenY", DOMMouseEvent::ScreenY, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[2] },
   { "clientX", DOMMouseEvent::ClientX, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[5] },
   { "x", DOMMouseEvent::X, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[4] },
   { "clientY", DOMMouseEvent::ClientY, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[7] },
   { "y", DOMMouseEvent::Y, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[6] },
   { "offsetX", DOMMouseEvent::OffsetX, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[9] },
   { "offsetY", DOMMouseEvent::OffsetY, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[8] },
   { "ctrlKey", DOMMouseEvent::CtrlKey, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[10] },
   { "shiftKey", DOMMouseEvent::ShiftKey, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[15] },
   { "altKey", DOMMouseEvent::AltKey, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[11] },
   { "metaKey", DOMMouseEvent::MetaKey, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[12] },
   { "button", DOMMouseEvent::Button, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[13] },
   { "relatedTarget", DOMMouseEvent::RelatedTarget, DontDelete|ReadOnly, 0, &DOMMouseEventTableEntries[14] },
   { "fromElement", DOMMouseEvent::FromElement, DontDelete|ReadOnly, 0, 0 },
   { "toElement", DOMMouseEvent::ToElement, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMMouseEventTable = { 2, 16, DOMMouseEventTableEntries, 2 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMMouseEventProtoTableEntries[] = {
   { "initMouseEvent", DOMMouseEvent::InitMouseEvent, DontDelete|Function, 15, 0 }
};

const struct HashTable DOMMouseEventProtoTable = { 2, 1, DOMMouseEventProtoTableEntries, 1 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMTextEventTableEntries[] = {
   { "keyVal", DOMTextEvent::Key, DontDelete|ReadOnly, 0, &DOMTextEventTableEntries[2] },
   { "virtKeyVal", DOMTextEvent::VirtKey, DontDelete|ReadOnly, 0, &DOMTextEventTableEntries[4] },
   { "keyCode", DOMTextEvent::Key, DontDelete|ReadOnly, 0, &DOMTextEventTableEntries[3] },
   { "outputString", DOMTextEvent::OutputString, DontDelete|ReadOnly, 0, 0 },
   { "inputGenerated", DOMTextEvent::InputGenerated, DontDelete|ReadOnly, 0, &DOMTextEventTableEntries[5] },
   { "numPad", DOMTextEvent::NumPad, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMTextEventTable = { 2, 6, DOMTextEventTableEntries, 2 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMTextEventProtoTableEntries[] = {
   { "initTextEvent", DOMTextEvent::InitTextEvent, DontDelete|Function, 10, 0 }
};

const struct HashTable DOMTextEventProtoTable = { 2, 1, DOMTextEventProtoTableEntries, 1 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry MutationEventConstructorTableEntries[] = {
   { "ADDITION", DOM::MutationEvent::ADDITION, DontDelete|ReadOnly, 0, &MutationEventConstructorTableEntries[3] },
   { "MODIFICATION", DOM::MutationEvent::MODIFICATION, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "REMOVAL", DOM::MutationEvent::REMOVAL, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable MutationEventConstructorTable = { 2, 4, MutationEventConstructorTableEntries, 3 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMMutationEventTableEntries[] = {
   { "attrChange", DOMMutationEvent::AttrChange, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "relatedNode", DOMMutationEvent::RelatedNode, DontDelete|ReadOnly, 0, 0 },
   { "attrName", DOMMutationEvent::AttrName, DontDelete|ReadOnly, 0, 0 },
   { "prevValue", DOMMutationEvent::PrevValue, DontDelete|ReadOnly, 0, &DOMMutationEventTableEntries[5] },
   { "newValue", DOMMutationEvent::NewValue, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMMutationEventTable = { 2, 6, DOMMutationEventTableEntries, 5 };

} // namespace

using namespace KJS;

namespace KJS {

const struct HashEntry DOMMutationEventProtoTableEntries[] = {
   { "initMutationEvent", DOMMutationEvent::InitMutationEvent, DontDelete|Function, 8, 0 }
};

const struct HashTable DOMMutationEventProtoTable = { 2, 1, DOMMutationEventProtoTableEntries, 1 };

} // namespace
