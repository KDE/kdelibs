/* Automatically generated from string_object.cpp using ./create_hash_table. DO NOT EDIT ! */

#include "lookup.h"

namespace KJS {

const struct HashEntry stringTableEntries[] = {
   { "toString", StringProtoFuncImp::ToString, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { "bold", StringProtoFuncImp::Bold, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "lastIndexOf", StringProtoFuncImp::LastIndexOf, DontEnum|DontDelete|ReadOnly|Function, 2, 0 },
   { "replace", StringProtoFuncImp::Replace, DontEnum|DontDelete|ReadOnly|Function, 2, 0 },
   { "match", StringProtoFuncImp::Match, DontEnum|DontDelete|ReadOnly|Function, 1, &stringTableEntries[26] },
   { "search", StringProtoFuncImp::Search, DontEnum|DontDelete|ReadOnly|Function, 1, &stringTableEntries[33] },
   { 0, 0, 0, 0, 0 },
   { "slice", StringProtoFuncImp::Slice, DontEnum|DontDelete|ReadOnly|Function, 0, &stringTableEntries[29] },
   { 0, 0, 0, 0, 0 },
   { "split", StringProtoFuncImp::Split, DontEnum|DontDelete|ReadOnly|Function, 1, &stringTableEntries[27] },
   { "anchor", StringProtoFuncImp::Anchor, DontEnum|DontDelete|ReadOnly|Function, 1, 0 },
   { "charCodeAt", StringProtoFuncImp::CharCodeAt, DontEnum|DontDelete|ReadOnly|Function, 1, 0 },
   { "toUpperCase", StringProtoFuncImp::ToUpperCase, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { "link", StringProtoFuncImp::Link, DontEnum|DontDelete|ReadOnly|Function, 1, 0 },
   { "indexOf", StringProtoFuncImp::IndexOf, DontEnum|DontDelete|ReadOnly|Function, 2, 0 },
   { 0, 0, 0, 0, 0 },
   { "small", StringProtoFuncImp::Small, DontEnum|DontDelete|ReadOnly|Function, 0, &stringTableEntries[31] },
   { "sub", StringProtoFuncImp::Sub, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "valueOf", StringProtoFuncImp::ValueOf, DontEnum|DontDelete|ReadOnly|Function, 0, &stringTableEntries[28] },
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "charAt", StringProtoFuncImp::CharAt, DontEnum|DontDelete|ReadOnly|Function, 1, 0 },
   { "fontsize", StringProtoFuncImp::Fontsize, DontEnum|DontDelete|ReadOnly|Function, 1, 0 },
   { "substr", StringProtoFuncImp::Substr, DontEnum|DontDelete|ReadOnly|Function, 2, 0 },
   { "substring", StringProtoFuncImp::Substring, DontEnum|DontDelete|ReadOnly|Function, 2, 0 },
   { "toLowerCase", StringProtoFuncImp::ToLowerCase, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { "big", StringProtoFuncImp::Big, DontEnum|DontDelete|ReadOnly|Function, 0, &stringTableEntries[34] },
   { "blink", StringProtoFuncImp::Blink, DontEnum|DontDelete|ReadOnly|Function, 0, &stringTableEntries[30] },
   { "fixed", StringProtoFuncImp::Fixed, DontEnum|DontDelete|ReadOnly|Function, 0, &stringTableEntries[32] },
   { "italics", StringProtoFuncImp::Italics, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { "strike", StringProtoFuncImp::Strike, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { "sup", StringProtoFuncImp::Sup, DontEnum|DontDelete|ReadOnly|Function, 0, 0 },
   { "fontcolor", StringProtoFuncImp::Fontcolor, DontEnum|DontDelete|ReadOnly|Function, 1, 0 }
};

const struct HashTable stringTable = { 2, 35, stringTableEntries, 26 };

}; // namespace
