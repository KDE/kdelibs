/* Automatically generated from kjs_events.cpp using ../../kjs/create_hash_table. DO NOT EDIT ! */

namespace KJS {

const struct HashEntry EventConstructorTableEntries[] = {
   { "CAPTURING_PHASE", DOM::Event::CAPTURING_PHASE, DontDelete|ReadOnly, 0, &EventConstructorTableEntries[3] },
   { "BUBBLING_PHASE", DOM::Event::BUBBLING_PHASE, DontDelete|ReadOnly, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "AT_TARGET", DOM::Event::AT_TARGET, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable EventConstructorTable = { 2, 4, EventConstructorTableEntries, 3 };

}; // namespace

namespace KJS {

const struct HashEntry DOMEventTableEntries[] = {
   { "currentTarget", DOMEvent::CurrentTarget, DontDelete|ReadOnly, 0, &DOMEventTableEntries[8] },
   { 0, 0, 0, 0, 0 },
   { "type", DOMEvent::Type, DontDelete|ReadOnly, 0, 0 },
   { "target", DOMEvent::Target, DontDelete|ReadOnly, 0, &DOMEventTableEntries[7] },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "toElement", DOMEvent::ToElement, DontDelete|ReadOnly, 0, &DOMEventTableEntries[10] },
   { "eventPhase", DOMEvent::EventPhase, DontDelete|ReadOnly, 0, &DOMEventTableEntries[9] },
   { "bubbles", DOMEvent::Bubbles, DontDelete|ReadOnly, 0, 0 },
   { "cancelable", DOMEvent::Cancelable, DontDelete|ReadOnly, 0, &DOMEventTableEntries[11] },
   { "timeStamp", DOMEvent::TimeStamp, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMEventTable = { 2, 12, DOMEventTableEntries, 7 };

}; // namespace

namespace KJS {

const struct HashEntry DOMEventProtoTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "stopPropagation", DOMEvent::StopPropagation, DontDelete|Function, 0, &DOMEventProtoTableEntries[3] },
   { "preventDefault", DOMEvent::PreventDefault, DontDelete|Function, 0, &DOMEventProtoTableEntries[4] },
   { "initEvent", DOMEvent::InitEvent, DontDelete|Function, 3, 0 }
};

const struct HashTable DOMEventProtoTable = { 2, 5, DOMEventProtoTableEntries, 3 };

}; // namespace
