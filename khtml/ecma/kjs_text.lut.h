/* Automatically generated from kjs_text.cpp using ../../kjs/create_hash_table. DO NOT EDIT ! */

namespace KJS {

const struct HashEntry DOMCharacterDataTableEntries[] = {
   { "data", DOMCharacterData::Data, DontDelete, 0, &DOMCharacterDataTableEntries[2] },
   { 0, 0, 0, 0, 0 },
   { "length", DOMCharacterData::Length, DontDelete|ReadOnly, 0, 0 }
};

const struct HashTable DOMCharacterDataTable = { 2, 3, DOMCharacterDataTableEntries, 2 };

}; // namespace

namespace KJS {

const struct HashEntry DOMCharacterDataProtoTableEntries[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 },
   { "appendData", DOMCharacterData::AppendData, DontDelete|Function, 1, 0 },
   { "insertData", DOMCharacterData::InsertData, DontDelete|Function, 2, 0 },
   { "deleteData", DOMCharacterData::DeleteData, DontDelete|Function, 2, &DOMCharacterDataProtoTableEntries[7] },
   { 0, 0, 0, 0, 0 },
   { "substringData", DOMCharacterData::SubstringData, DontDelete|Function, 2, 0 },
   { "replaceData", DOMCharacterData::ReplaceData, DontDelete|Function, 2, 0 }
};

const struct HashTable DOMCharacterDataProtoTable = { 2, 8, DOMCharacterDataProtoTableEntries, 7 };

}; // namespace

namespace KJS {

const struct HashEntry DOMTextProtoTableEntries[] = {
   { "splitText", DOMText::SplitText, DontDelete|Function, 1, 0 }
};

const struct HashTable DOMTextProtoTable = { 2, 1, DOMTextProtoTableEntries, 1 };

}; // namespace
