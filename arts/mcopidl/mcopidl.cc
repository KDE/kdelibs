    /*

    Copyright (C) 1999 Stefan Westerfeld, stefan@space.twc.de
                       Nicolas Brodu, nicolas.brodu@free.fr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <stack>
#include <ctype.h>
#include "core.h"
#include "namespace.h"
#include <iostream>

using namespace std;
using namespace Arts;

int idl_in_include;
int idl_line_no;
string idl_filename;

/*
 * if we start parsing an include file, we push the name of the file
 * and the line number where we left it on the stack for later usage
 */
stack<pair<int,string> > idl_include_stack;

list<EnumDef> enums;
list<TypeDef> structs;
list<InterfaceDef> interfaces;
list<string> packetTypes;		// just an evil hack to get experimental video
list<string> customIncludes;	// just an evil hack to get experimental video
list<string> includes;		// files to include
list<string> includePath;	// path for the includes

// names that occur in included files -> no code generation
list<string> includedNames;

ModuleDef module;

void addEnumTodo( const EnumDef& edef )
{
	enums.push_back(edef);

	if(idl_in_include)
	{
		includedNames.push_back(edef.name);
	}
	else
	{
		module.enums.push_back(edef);
	}
}

void addStructTodo( const TypeDef& type )
{
	structs.push_back(type);

	if(idl_in_include)
	{
		includedNames.push_back(type.name);
	}
	else
	{
		module.types.push_back(type);
	}
}

void addInterfaceTodo( const InterfaceDef& iface )
{
	interfaces.push_back(iface);

	if(idl_in_include)
	{
		includedNames.push_back(iface.name);
	}
	else
	{
		module.interfaces.push_back(iface);
	}
}

bool fromInclude(string name)
{
	list<string>::iterator i;

	for(i=includedNames.begin(); i != includedNames.end();i++)
		if(*i == name) return true;

	return false;
}

void startInclude( const char *line )
{
	const char *file = "*unknown*";
	char *l = strdup(line);
	char *a = strtok(l,"<\"");
	if(a)
	{
		char *b = strtok(0,">\"");
		if(b) file = b;
	}
	free(l);

	idl_in_include++;
	idl_include_stack.push(make_pair(idl_line_no, idl_filename));
	idl_line_no = 0;
	idl_filename =  file;
}

void endInclude()
{
	assert(!idl_include_stack.empty());
	idl_line_no = idl_include_stack.top().first;
	idl_filename = idl_include_stack.top().second;
	idl_include_stack.pop();

	idl_in_include--;
}

bool isPacketType( string type )
{
	list<string>::iterator i;

	for(i=packetTypes.begin();i != packetTypes.end(); i++)
		if((*i) == type) return true;

	return false;
}

bool isStruct( string type )
{
	list<TypeDef>::iterator i;

	for(i=structs.begin();i != structs.end(); i++)
		if(i->name == type) return true;

	return false;
}

bool isEnum( string type )
{
	list<EnumDef>::iterator i;

	for(i=enums.begin();i != enums.end(); i++)
		if(i->name == type) return true;

	return false;
}

bool isInterface( string type )
{
	list<InterfaceDef>::iterator i;

	for(i=interfaces.begin();i != interfaces.end(); i++)
		if(i->name == type) return true;

	return (type == "object");
}

string formatMultiLineString(string s, string indent)
{
	string result = indent+"\"";
	string::iterator si = s.begin();

	int lineLen = 80-indent.size()-6;	
	int i = 0;

	while(si != s.end())
	{
		if(i == lineLen)
		{
			result += "\"\n" + indent + "\"";
			i = 0;
		}

		result += *si++;
		i++;
	}
	return result+"\"";
}

#define MODEL_MEMBER    1
#define MODEL_ARG       2
#define MODEL_READ      3
#define MODEL_REQ_READ  4
#define MODEL_RES_READ  5
#define MODEL_WRITE     6
#define MODEL_REQ_WRITE 7
#define MODEL_RESULT    8
#define MODEL_INVOKE    9
#define MODEL_STREAM    10
#define MODEL_MSTREAM   11
#define MODEL_ASTREAM   12
#define MODEL_AMSTREAM  13
#define MODEL_ASTREAM_PACKETPTR  14
#define MODEL_SEQ       1024

#define MODEL_MEMBER_SEQ	(MODEL_MEMBER|MODEL_SEQ)
#define MODEL_ARG_SEQ		(MODEL_ARG|MODEL_SEQ)
#define MODEL_READ_SEQ		(MODEL_READ|MODEL_SEQ)
#define MODEL_WRITE_SEQ		(MODEL_WRITE|MODEL_SEQ)
#define MODEL_REQ_READ_SEQ	(MODEL_REQ_READ|MODEL_SEQ)
#define MODEL_RES_READ_SEQ	(MODEL_RES_READ|MODEL_SEQ)
#define MODEL_REQ_WRITE_SEQ	(MODEL_REQ_WRITE|MODEL_SEQ)
#define MODEL_RESULT_SEQ	(MODEL_RESULT|MODEL_SEQ)
#define MODEL_INVOKE_SEQ	(MODEL_INVOKE|MODEL_SEQ)

/**
 * generates a piece of code for the specified type/name
 *
 * model determines if the code is a parameter declaration, type member
 * declaration, write-to-stream code for, read-from-stream code or whatever
 * else
 */
string createTypeCode(string type, const string& name, long model,
		string indent = "")
{
	string result = "";

	if(type.length() >= 1 && type[0] == '*')
	{
		model |= MODEL_SEQ;
		type = type.substr(1,type.length()-1);
	}

	if(type == "void")
	{
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(result) delete result;\n";
		}
		else if(model==MODEL_INVOKE)
		{
			result = indent + name+";\n";
		}
		else
		{
			result = "void";
		}
	}
	else if(type == "float")
	{
		if(model==MODEL_MEMBER)		result = "float";
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<float>";
		if(model==MODEL_ARG)		result = "float";
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<float>&";
		if(model==MODEL_RESULT)		result = "float";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<float> *";
		if(model==MODEL_STREAM)		result = "float *"+name;
		if(model==MODEL_MSTREAM)	result = "float **"+name;
		if(model==MODEL_ASTREAM)	result = "Arts::FloatAsyncStream "+name;
		if(model==MODEL_AMSTREAM)	assert(false);
		if(model==MODEL_ASTREAM_PACKETPTR) result = "Arts::DataPacket<float> *";
		if(model==MODEL_READ)
			result = name+" = stream.readFloat()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readFloatSeq("+name+")";
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return 0.0; // error occured\n";
			result += indent + "float returnCode = result->readFloat();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<float> *_returnCode ="
												" new std::vector<float>;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "result->readFloatSeq(*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "float "+name+" = request->readFloat();\n";
		if(model==MODEL_REQ_READ_SEQ)
			result = indent + "std::vector<float> "+name+";\n"
				   + indent + "request->readFloatSeq("+name+");\n";
		if(model==MODEL_WRITE)
			result = "stream.writeFloat("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeFloatSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeFloat("+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "request->writeFloatSeq("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeFloat("+name+");\n";
		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<float> *_returnCode = "+name+";\n"
				   + indent + "result->writeFloatSeq(*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
	}
	else if(type == "boolean")
	{
		if(model==MODEL_MEMBER)		result = "bool";
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<bool>";
		if(model==MODEL_ARG)		result = "bool";
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<bool>&";
		if(model==MODEL_RESULT)		result = "bool";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<bool> *";
		if(model==MODEL_READ)
			result = name+" = stream.readBool()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readBoolSeq("+name+")";
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return false; // error occured\n";
			result += indent + "bool returnCode = result->readBool();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<bool> *_returnCode ="
												" new std::vector<bool>;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "result->readBoolSeq(*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "bool "+name+" = request->readBool();\n";
		if(model==MODEL_REQ_READ_SEQ)
			result = indent + "std::vector<bool> "+name+";\n"
				   + indent + "request->readBoolSeq("+name+");\n";
		if(model==MODEL_WRITE)
			result = "stream.writeBool("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeBoolSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeBool("+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "request->writeBoolSeq("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeBool("+name+");\n";
		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<bool> *_returnCode = "+name+";\n"
				   + indent + "result->writeBoolSeq(*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
	}
	else if(type == "byte")
	{
		if(model==MODEL_MEMBER)		result = "Arts::mcopbyte";
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<Arts::mcopbyte>";
		if(model==MODEL_ARG)		result = "Arts::mcopbyte";
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<Arts::mcopbyte>&";
		if(model==MODEL_RESULT)		result = "Arts::mcopbyte";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<Arts::mcopbyte> *";
		if(model==MODEL_READ)
			result = name+" = stream.readByte()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readByteSeq("+name+")";
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return 0; // error occured\n";
			result += indent + "Arts::mcopbyte returnCode = result->readByte();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<Arts::mcopbyte> *_returnCode ="
										" new std::vector<Arts::mcopbyte>;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "result->readByteSeq(*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "Arts::mcopbyte "+name+" = request->readByte();\n";
		if(model==MODEL_REQ_READ_SEQ)
			result = indent + "std::vector<Arts::mcopbyte> "+name+";\n"
				   + indent + "request->readByteSeq("+name+");\n";
		if(model==MODEL_WRITE)
			result = "stream.writeByte("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeByteSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeByte("+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "request->writeByteSeq("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeByte("+name+");\n";
		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<Arts::mcopbyte> *_returnCode = "+name+";\n"
				   + indent + "result->writeByteSeq(*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
		if(model==MODEL_ASTREAM)
			result = "Arts::ByteAsyncStream "+name;
		if(model==MODEL_ASTREAM_PACKETPTR) result = "Arts::DataPacket<Arts::mcopbyte> *";
	}
	else if(type == "long")
	{
		if(model==MODEL_MEMBER)		result = "long";
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<long>";
		if(model==MODEL_ARG)		result = "long";
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<long>&";
		if(model==MODEL_RESULT)		result = "long";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<long> *";
		if(model==MODEL_READ)
			result = name+" = stream.readLong()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readLongSeq("+name+")";
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return 0; // error occured\n";
			result += indent + "long returnCode = result->readLong();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<long> *_returnCode ="
												" new std::vector<long>;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "result->readLongSeq(*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "long "+name+" = request->readLong();\n";
		if(model==MODEL_REQ_READ_SEQ)
			result = indent + "std::vector<long> "+name+";\n"
				   + indent + "request->readLongSeq("+name+");\n";
		if(model==MODEL_WRITE)
			result = "stream.writeLong("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeLongSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeLong("+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "request->writeLongSeq("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeLong("+name+");\n";
		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<long> *_returnCode = "+name+";\n"
				   + indent + "result->writeLongSeq(*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
	} else if(type == "string") {
		if(model==MODEL_MEMBER)		result = "std::string";
		if(model==MODEL_MEMBER_SEQ)	result = "std::vector<std::string>";
		if(model==MODEL_ARG)		result = "const std::string&";
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<std::string>&";
		if(model==MODEL_RESULT)		result = "std::string";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<std::string> *";
		if(model==MODEL_READ)
			result = "stream.readString("+name+")";
		if(model==MODEL_READ_SEQ)
			result = "stream.readStringSeq("+name+")";
		if(model==MODEL_REQ_READ)
		{
			result = indent + "std::string "+name+";\n"
				   + indent + "request->readString("+name+");\n";
		}
		if(model==MODEL_REQ_READ_SEQ)
			result = indent + "std::vector<std::string> "+name+";\n"
				   + indent + "request->readStringSeq("+name+");\n";
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return\"\"; // error occured\n";
			result += indent + "std::string returnCode;\n";
			result += indent + "result->readString(returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<std::string> *_returnCode ="
											" new std::vector<std::string>;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "result->readStringSeq(*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_WRITE)
			result = "stream.writeString("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeStringSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeString("+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "request->writeStringSeq("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeString("+name+");\n";
		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<std::string> *_returnCode = "+name+";\n"
				   + indent + "result->writeStringSeq(*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
	} else if(isPacketType(type)) {
		if(model==MODEL_ASTREAM)
			result = type+"AsyncStream "+name;
		if(model==MODEL_ASTREAM_PACKETPTR) result = "Arts::DataPacket<"+type+"> *";
	} else if(isStruct(type)) {
		if(model==MODEL_MEMBER)
			result = type;
		if(model==MODEL_MEMBER_SEQ)
			result = "std::vector<"+type+">";

		if(model==MODEL_ARG)	
			result = "const "+type+"&";
		if(model==MODEL_ARG_SEQ)
			result = "const std::vector<"+type+">&";

		if(model==MODEL_READ)
			result = name+".readType(stream)";
		if(model==MODEL_READ_SEQ)
			result = "Arts::readTypeSeq(stream,"+name+")";
		if(model==MODEL_REQ_READ)
			result = indent + type+" "+name+"(*request);\n";
		//if(model==MODEL_REQ_READ_SEQ) TODO

		if(model==MODEL_WRITE)
			result = name+".writeType(stream)";
		if(model==MODEL_REQ_WRITE)
			result = name+".writeType(*request)";
		if(model==MODEL_WRITE_SEQ)
			result = "Arts::writeTypeSeq(stream,"+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "Arts::writeTypeSeq(*request,"+name+")";

		if(model==MODEL_INVOKE)
			result = indent + type + " _returnCode = "+name+";\n"
			       + indent + "_returnCode.writeType(*result);\n";

		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<"+type+"> *_returnCode = "+name+";\n"
				   + indent + "Arts::writeTypeSeq(*result,*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
		if(model==MODEL_RES_READ)
		{
			result = indent +
					"if(!result) return "+type+"(); // error occured\n";
			result += indent+ type + " _returnCode(*result);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<"+type+"> *_returnCode ="
											" new std::vector<"+type+">;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "Arts::readTypeSeq(*result,*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}

		if(model==MODEL_RESULT)		result = type;
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<"+type+"> *";
	} else if(isEnum(type)) {
		if(model==MODEL_MEMBER)		result = type;
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<"+type+">";
		if(model==MODEL_ARG)		result = type;
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<"+type+">&";
		if(model==MODEL_RESULT)		result = type;
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<"+type+"> *";
		if(model==MODEL_READ)
			result = name+" = ("+type+")stream.readLong()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readLongSeq("+name+")";		// TODO
		if(model==MODEL_RES_READ)
		{
			result = indent  +
						"if(!result) return ("+type+")0; // error occured\n";
			result += indent + type+" returnCode = ("+
												type+")result->readLong();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent +
				type+" "+name+" = ("+type+")request->readLong();\n";
		if(model==MODEL_WRITE)
			result = "stream.writeLong("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeLongSeq("+name+")";		// TODO
		if(model==MODEL_REQ_WRITE)
			result = "request->writeLong("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeLong("+name+");\n";
	} else if(isInterface(type)) {
		// the "object class" is called Object
		if(type == "object") type = "Arts::Object";

		if(model==MODEL_MEMBER)		result = type+"_var";
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<"+type+">";
		if(model==MODEL_ARG)		result = type;
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<"+type+">&";
		if(model==MODEL_RESULT)		result = type;
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<"+type+"> *";
		if(model==MODEL_READ)
			result = "Arts::readObject(stream,"+name+")";
		if(model==MODEL_READ_SEQ)
			result = "Arts::readObjectSeq(stream,"+name+")";
		if(model==MODEL_RES_READ)
		{
 			result = indent + "if (!result) return "+type+"::null();\n"; // error occured\n";
			result += indent + type+"_base* returnCode;\n";
			result += indent + "Arts::readObject(*result,returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return "+type+"::_from_base(returnCode);\n";
		}
		if(model==MODEL_RES_READ_SEQ)
		{
			result = indent + "std::vector<"+type+"> *_returnCode ="
												" new std::vector<"+type+">;\n";
			result += indent + "if(!result) return _returnCode; // error occured\n";
			result += indent + "Arts::readObjectSeq(*result,*_returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
		{
			result = indent + type +"_base* _temp_"+name+";\n";
			result += indent + "Arts::readObject(*request,_temp_"+name+");\n";
			result += indent + type+" "+name+" = "+type+"::_from_base(_temp_"+name+");\n";
		}
		if(model==MODEL_REQ_READ_SEQ)
			result = indent + "std::vector<"+type+"> "+name+";\n"
				   + indent + "Arts::readObjectSeq(*request,"+name+");\n";
		if(model==MODEL_WRITE)
			result = "Arts::writeObject(stream,"+name+"._base())";
		if(model==MODEL_WRITE_SEQ)
			result = "Arts::writeObjectSeq(stream,"+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "Arts::writeObject(*request,"+name+"._base())";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "Arts::writeObjectSeq(*request,"+name+")";
		if(model==MODEL_INVOKE)
		{
			result = indent + type+" returnCode = "+name+";\n"
			       + indent + "Arts::writeObject(*result,returnCode._base());\n";
		}
		if(model==MODEL_INVOKE_SEQ)
		{
			result = indent + "std::vector<"+type+"> *_returnCode = "+name+";\n"
				   + indent + "Arts::writeObjectSeq(*result,*_returnCode);\n"
				   + indent + "delete _returnCode;\n";
		}
	}
	else
	{
		fprintf(stderr,"error: undefined type %s occured\n",type.c_str());
		exit(1);
	}

	if((model & ~MODEL_SEQ) == MODEL_MEMBER
	|| (model & ~MODEL_SEQ) == MODEL_ARG)
	{
		result += " ";
		result += name;
	}
	return result;
}

string buildInheritanceList(const InterfaceDef& interface, const string& append)
{
	vector<string>::const_iterator ii;
	string result = "";
	bool first = true;

	for(ii=interface.inheritedInterfaces.begin();
		ii != interface.inheritedInterfaces.end();ii++)
	{
		if(!first) result += ",\n\t"; else first = false;
		result += "virtual public "+*ii+append;
	}

	return result;
}

string mkdef(string prefix)
{
	string result;

	for(unsigned int i=0;i<prefix.length();i++)
		result += toupper(prefix[i]);
	result += "_H";

	return result;
}

const char *generated_disclaimer =
	"/* this file was generated by the MCOP idl compiler - DO NOT EDIT */\n\n";

FILE *startHeader(string prefix)
{
	string header_name = prefix+".h.new";
	FILE *header = fopen(header_name.c_str(),"w");

	fprintf(header,generated_disclaimer);
	fprintf(header,"#ifndef %s\n",mkdef(prefix).c_str());
	fprintf(header,"#define %s\n\n",mkdef(prefix).c_str());
	fprintf(header,"#include \"common.h\"\n\n");
	
	list<string>::iterator cii;
	for(cii=customIncludes.begin(); cii != customIncludes.end(); cii++)
		fprintf(header,"#include \"%s\"\n",(*cii).c_str());
	if(!customIncludes.empty()) fprintf(header,"\n");

	return (header);
}

void endHeader(FILE *header, string prefix)
{
	fprintf(header,"#endif /* %s */\n",mkdef(prefix).c_str());
	fclose(header);
}

FILE *startSource(string prefix)
{
	string header_name = prefix+".h";
	string source_name = prefix+".cc.new";

	FILE *source = fopen(source_name.c_str(),"w");
	fprintf(source,generated_disclaimer);
	fprintf(source,"#include \"%s\"\n\n",header_name.c_str());

	return source;
}

void endSource(FILE *source)
{
	fclose(source);
}

/* moves file BASE.new to BASE, but only if there are any changes. Otherwise
   BASE.new is simply removed */
void moveIfChanged(string base)
{
	string newn = base+".new";
	FILE *oldf = fopen(base.c_str(), "r");
	if (!oldf) {
		rename(newn.c_str(), base.c_str());
		return;
	}
	FILE *newf = fopen(newn.c_str(), "r");
	if (!newf) {
		fclose(oldf);
		return;
	}
	bool different = false;
	unsigned char *oldb, *newb;
	size_t blen = 65536;
	oldb = new unsigned char[blen];
	newb = new unsigned char[blen];
	while (1) {
		size_t olen = fread(oldb, 1, blen, oldf);
		size_t nlen = fread(newb, 1, blen, newf);
		if (olen != nlen) {
			different = true;
			break;
		}
		if (!olen) break;
		if (memcmp(oldb, newb, olen)) {
			different = true;
			break;
		}
		if (olen < blen) break;
	}
	delete newb;
	delete oldb;
	fclose(newf);
	fclose(oldf);
	if (different) {
		rename(newn.c_str(), base.c_str());
	} else {
		unlink(newn.c_str());
	}
}

bool haveIncluded(string filename)
{
	list<string>::iterator i;

	for(i = ::includes.begin();i != ::includes.end();i++)
		if(*i == filename) return true;

	return false;
}

void doIncludeHeader(FILE *header)
{
	list<string>::iterator i;
	bool done_something = false;

	for(i = ::includes.begin();i != ::includes.end();i++)
	{
		char *include = strdup((*i).c_str());
		if(strlen(include) >= 4)
		{
			if(strcmp(&include[strlen(include)-4],".idl") == 0)
			{
				include[strlen(include)-4] = 0;
				if(!done_something)
				{
					fprintf(header,"// includes of other idl definitions\n");
					done_something = true;
				}
				fprintf(header,"#include \"%s.h\"\n",include);
			}
		}
		free(include);
	}
	if(done_something) fprintf(header,"\n");
}

void doEnumHeader(FILE *header)
{
	list<EnumDef>::iterator edi;
	vector<EnumComponent>::iterator i;
	NamespaceHelper nspace(header);

	for(edi = enums.begin();edi != enums.end(); edi++)
	{
		EnumDef& ed = *edi;

		if(fromInclude(ed.name)) continue; // should come from the include

		nspace.setFromSymbol(ed.name);
		string ename = nspace.printableForm(ed.name);
		if(ename == "_anonymous_") ename = "";	

		fprintf(header,"enum %s {",ename.c_str());
		int first = 0;
		for(i=ed.contents.begin();i != ed.contents.end();i++)
		{
			if(first != 0) fprintf(header,", ");
			first++;
			fprintf(header,"%s = %ld",i->name.c_str(),i->value);
		}	
		fprintf(header,"};\n");
	}
}

void doStructHeader(FILE *header)
{
	list<TypeDef>::iterator csi;
	vector<TypeComponent>::iterator i;
	NamespaceHelper nspace(header);

	for(csi = structs.begin();csi != structs.end(); csi++)
	{
		TypeDef& d = *csi;

		if(fromInclude(d.name)) continue; // should come from the include

		nspace.setFromSymbol(d.name.c_str());
		string tname = nspace.printableForm(d.name);

		fprintf(header,"class %s : public Arts::Type {\n",tname.c_str());
		fprintf(header,"public:\n");

		/** constructor without arguments **/
		fprintf(header,"\t%s();\n",tname.c_str());

		/** constructor with arguments **/
		fprintf(header,"\t%s(",tname.c_str());
		int first = 0;
		for(i=d.contents.begin();i != d.contents.end();i++)
		{
			string name = createTypeCode(i->type,"_a_" + i->name,MODEL_ARG);
			if(first != 0) fprintf(header,", ");
			first++;
			fprintf(header,"%s",name.c_str());
		}
		fprintf(header,");\n");

		/** constructor from stream **/
		fprintf(header,"\t%s(Arts::Buffer& stream);\n",tname.c_str());

		/** copy constructor (from same type) **/
		fprintf(header,"\t%s(const %s& copyType);\n",
			tname.c_str(),tname.c_str());

		/** assignment operator **/
		fprintf(header,"\t%s& operator=(const %s& assignType);\n",
			tname.c_str(),tname.c_str());

		/** data members **/
		for(i=d.contents.begin();i != d.contents.end();i++)
		{
			string name = createTypeCode(i->type,i->name,MODEL_MEMBER);
			fprintf(header,"\t%s;\n",name.c_str());
		}

		fprintf(header,"\n// marshalling functions\n");

		/** marshalling function for reading from stream **/
		fprintf(header,"\tvoid readType(Arts::Buffer& stream);\n");

		/** marshalling function for writing to stream **/
		fprintf(header,"\tvoid writeType(Arts::Buffer& stream) const;\n");
		fprintf(header,"};\n\n");
	}
}

void doStructSource(FILE *source)
{
	list<TypeDef>::iterator csi;
	vector<TypeComponent>::iterator i;

	fprintf(source,"// Implementation\n");
	for(csi = structs.begin();csi != structs.end(); csi++)
	{
		TypeDef& d = *csi;

		if(fromInclude(d.name)) continue; // should come from the include

		string tname = NamespaceHelper::nameOf(d.name);

		fprintf(source,"%s::%s()\n{\n}\n\n",d.name.c_str(),tname.c_str());

		fprintf(source,"%s::%s(",d.name.c_str(),tname.c_str());
		int first = 0;
		for(i=d.contents.begin();i != d.contents.end();i++)
		{
			string name = createTypeCode(i->type,"_a_" + i->name,MODEL_ARG);
			if(first != 0) fprintf(source,", ");
			first++;
			fprintf(source,"%s",name.c_str());
		}
		fprintf(source,")\n{\n");
		for(i=d.contents.begin();i != d.contents.end();i++)
		{
			string n = "_a_" + i->name;
			fprintf(source,"\tthis->%s = %s;\n",i->name.c_str(),n.c_str());
		}
		fprintf(source,"}\n\n");

		/** constructor from stream **/
		fprintf(source,"%s::%s(Arts::Buffer& stream)\n{\n",d.name.c_str(),tname.c_str());
		fprintf(source,"\treadType(stream);\n");
		fprintf(source,"}\n\n");

		/** copy constructor **/

		fprintf(source,"%s::%s(const %s& copyType) : Arts::Type(copyType)\n{\n",
			d.name.c_str(),tname.c_str(),d.name.c_str());
		fprintf(source,"\tArts::Buffer buffer;\n");
		fprintf(source,"\tcopyType.writeType(buffer);\n");
		fprintf(source,"\treadType(buffer);\n");
		fprintf(source,"}\n\n");

		/** assignment operator **/
		fprintf(source,"%s& %s::operator=(const %s& assignType)\n{\n",
			d.name.c_str(),d.name.c_str(),d.name.c_str());
		fprintf(source,"\tArts::Buffer buffer;\n");
		fprintf(source,"\tassignType.writeType(buffer);\n");
		fprintf(source,"\treadType(buffer);\n");
		fprintf(source,"\treturn *this;\n");
		fprintf(source,"}\n\n");

#if 0 /* not needed if types use vector<Type> instead of vector<Type *> */
		/** virtual destuctor: free type contents **/
		fprintf(source,"%s::~%s()\n{\n",d.name.c_str(),tname.c_str());
		for(i=d->contents.begin();i != d->contents.end();i++)
		{
			string stype = (*i)->type;
			string type = stype.substr(1,stype.length()-1);
			if(stype[0] == '*' && isStruct(type))
			{
				fprintf(source,"\tfreeTypeSeq(%s);\n",(*i)->name.c_str());
			}
		}
		fprintf(source,"}\n\n");
#endif	
		/** marshalling function for reading from stream **/
		fprintf(source,"void %s::readType(Arts::Buffer& stream)\n{\n",d.name.c_str());
		for(i=d.contents.begin();i != d.contents.end();i++)
		{
			string code = createTypeCode(i->type,i->name,MODEL_READ);
			fprintf(source,"\t%s;\n",code.c_str());
		}
		fprintf(source,"}\n\n");

		/** marshalling function for writing to stream **/
		fprintf(source,"void %s::writeType(Arts::Buffer& stream) const\n{\n",d.name.c_str());
		for(i=d.contents.begin();i != d.contents.end();i++)
		{
			string code = createTypeCode(i->type,i->name,MODEL_WRITE);
			fprintf(source,"\t%s;\n",code.c_str());
		}
		fprintf(source,"}\n\n");
	}
}

string createReturnCode(const MethodDef& md)
{
	return createTypeCode(md.type,"",MODEL_RESULT,"");
}

string createParamList(const MethodDef& md)
{
	string result;
	int first = 0;
	vector<ParamDef>::const_iterator pi;

	for(pi = md.signature.begin(); pi != md.signature.end(); pi++)
	{
		const ParamDef& pd = *pi;
		string p = createTypeCode(pd.type,pd.name,MODEL_ARG,"");

		if(first != 0) result += ", ";
		first++;
		result += p;
	}
	return result;
}

string createCallParamList(const MethodDef& md)
{
	string result;
	bool first = true;
	vector<ParamDef>::const_iterator pi;

	for(pi = md.signature.begin(); pi != md.signature.end(); pi++)
	{
		if (!first) result += ", ";
		first = false;
		result += pi->name;
	}
	return result;
}

void createStubCode(FILE *source, string iface, string method,
					const MethodDef& md)
{
	string rc = createReturnCode(md);
	string params = createParamList(md);
	vector<ParamDef>::const_iterator pi;

	Buffer b;
	md.writeType(b);

	fprintf(source,"%s %s_stub::%s(%s)\n",rc.c_str(),iface.c_str(),
				method.c_str(), params.c_str());
	fprintf(source,"{\n");
	fprintf(source,"\tlong methodID = _lookupMethodFast(\"%s\");\n",
											b.toString("method").c_str());
	if(md.flags & methodTwoway)
	{
		fprintf(source,"\tlong requestID;\n");
		fprintf(source,"\tArts::Buffer *request, *result;\n");
		fprintf(source,"\trequest = Arts::Dispatcher::the()->"
				"createRequest(requestID,_objectID,methodID);\n");
	}
	else
	{
		fprintf(source,"\tArts::Buffer *request = Arts::Dispatcher::the()->"
				"createOnewayRequest(_objectID,methodID);\n");
	}

	for(pi = md.signature.begin(); pi != md.signature.end(); pi++)
	{
		const ParamDef& pd = *pi;
		string p;
		p = createTypeCode(pd.type,pd.name,MODEL_REQ_WRITE);
		fprintf(source,"\t%s;\n",p.c_str());
	}
	fprintf(source,"\trequest->patchLength();\n");
	fprintf(source,"\t_connection->qSendBuffer(request);\n\n");

	if(md.flags & methodTwoway)
	{
		fprintf(source,"\tresult = "
			"Arts::Dispatcher::the()->waitForResult(requestID,_connection);\n");

		fprintf(source,"%s",
			createTypeCode(md.type,"",MODEL_RES_READ,"\t").c_str());
	}
	fprintf(source,"}\n\n");
}

bool haveStreams(const InterfaceDef& d)
{
	vector<AttributeDef>::const_iterator ai;

	for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		if(ai->flags & attributeStream) return true;

	return false;
}

bool haveAsyncStreams(const InterfaceDef& d)
{
	vector<AttributeDef>::const_iterator ai;

	for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		if((ai->flags & attributeStream) && (ai->flags & streamAsync))
			return true;

	return false;
}

string dispatchFunctionName(string interface, long mcount)
{
	char number[20];
	sprintf(number,"%02ld",mcount);

	string nspace = NamespaceHelper::namespaceOf(interface);
	for (string::iterator i = nspace.begin(); i != nspace.end(); i++)
		if(*i == ':') *i = '_';

	string iname = NamespaceHelper::nameOf(interface);

	return "_dispatch_" + nspace + "_" + iname + "_" + number;
}

void createDispatchFunction(FILE *source, long mcount,
					const InterfaceDef& d, const MethodDef& md,string name)
{
	/** calculate signature (prevents unused argument warnings) **/
	string signature = "void *object, ";

	if(md.signature.size() == 0)
		signature += "Arts::Buffer *";
	else
		signature += "Arts::Buffer *request";

	if(md.flags & methodTwoway)
	{
		if(md.type == "void")
			signature += ", Arts::Buffer *";
		else
			signature += ", Arts::Buffer *result";
	}
	else
	{
		if(md.type != "void")
		{
			cerr << "method " << md.name << " in interface " << d.name <<
			   " is declared oneway, but not void" << endl;
			exit(1);
		}
	}

	fprintf(source,"// %s\n",md.name.c_str());
	fprintf(source,"static void %s(%s)\n",
			dispatchFunctionName(d.name,mcount).c_str(), signature.c_str());
	fprintf(source,"{\n");

	string call = "(("+d.name+"_skel *)object)->"+name + "(";
	int first = 1;
	vector<ParamDef>::const_iterator pi;
	for(pi = md.signature.begin(); pi != md.signature.end(); pi++)
	{
		const ParamDef& pd = *pi;
		string p;

		if(!first) call += ",";
		first = 0;
		call += pd.name;
		p = createTypeCode(pd.type,pd.name,MODEL_REQ_READ, "\t");
		fprintf(source,"%s",p.c_str());
	}
	call += ")";
	string invoke = createTypeCode(md.type,call,MODEL_INVOKE,"\t");
	fprintf(source,"%s",invoke.c_str());
	fprintf(source,"}\n\n");
}

// generate a list of all parents. There can be repetitions
vector<std::string> allParents(const InterfaceDef& iface)
{
	vector<std::string> ret;
	list<InterfaceDef>::iterator interIt;
	vector<std::string>::const_iterator si;
	// For all inherited interfaces
	for (si = iface.inheritedInterfaces.begin(); si != iface.inheritedInterfaces.end(); si++)
	{
		ret.push_back(*si);
		// Find the corresponding interface definition
		for (interIt=interfaces.begin(); interIt!=interfaces.end(); interIt++) {
			InterfaceDef& parent = *interIt;
			if (parent.name == (*si)) {
				// Now add this parent's parents
				vector<std::string> ppar = allParents(parent);
				ret.insert(ret.end(), ppar.begin(), ppar.end());
				break;
			}
		}
	}
	return ret;
}

// generate a list of all parents - without repetitions
vector<string> allParentsUnique(const InterfaceDef& iface)
{
	map<string,bool> done;
	vector<string> parents = allParents(iface),result;
	vector<string>::iterator i;

	for(i=parents.begin();i!=parents.end();i++)
	{	
		string& name = *i;
		if(!done[name])
		{
			result.push_back(name);
			done[name] = true;
		}
	}

	return result;
}

InterfaceDef findInterface(const string& iface)
{
	list<InterfaceDef>::iterator i;
	for(i=interfaces.begin();i != interfaces.end(); i++)
	{
		const InterfaceDef& d = *i;
		if(d.name == iface) return d;
	}
	return InterfaceDef();
}

InterfaceDef mergeAllParents(const InterfaceDef& iface)
{
	InterfaceDef result = iface;

	vector<string> parents = allParentsUnique(iface);
	vector<string>::iterator pi;
	
	for(pi = parents.begin(); pi != parents.end(); pi++)
	{
		string parent = *pi;

		list<InterfaceDef>::iterator i;
		for(i=interfaces.begin();i != interfaces.end(); i++)
		{
			const InterfaceDef& d = *i;
			if(d.name == parent)
			{
				/* merge attributes */
				vector<AttributeDef>::const_iterator ai;

				for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
					result.attributes.push_back(*ai);

				/* merge methods */
				vector<MethodDef>::const_iterator mi;

				for(mi = d.methods.begin(); mi != d.methods.end(); mi++)
				{
					result.methods.push_back(*mi);
				}
			}
		}
	}
	return result;
}

struct ForwardCode {
	bool constructor;
	string fullifacename, result, mname, params, callparams;
	string baseclass;
};

void checkSymbolDefinition(const string& name, const string& type,
							const InterfaceDef& where, map<string,string>& defs)
{
	string xwhere = where.name + "::" + name + " ("+type+")";
	string& mapentry = defs[name];

	if(mapentry == "")
	{
		mapentry = xwhere;
	}
	else
	{
		cerr << idl_filename << ": warning: " << xwhere
				<< " collides with " << mapentry << endl;
	}
}

void doInterfacesHeader(FILE *header)
{
	list<InterfaceDef>::iterator ii;
	vector<MethodDef>::iterator mi;
	vector<AttributeDef>::iterator ai;
	string inherits;
	NamespaceHelper nspace(header);
	list<ForwardCode> forwardCode;

	/*
	 * this allows it to the various interfaces as parameters, returncodes
	 * and attributes even before their declaration
	 */
	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef& d = *ii;
		if(!fromInclude(d.name))
		{
			nspace.setFromSymbol(d.name);
			fprintf(header,"class %s;\n",nspace.printableForm(d.name).c_str());
		}
	}
	fprintf(header,"\n");

	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef& d = *ii;
		string iname;
		string fullifacename = d.name;

		if(fromInclude(d.name)) continue; // should come from the include

		// create abstract interface
		inherits = buildInheritanceList(d,"_base");
		if(inherits == "") inherits = "virtual public Arts::Object_base";

		nspace.setFromSymbol(d.name);
		iname = nspace.printableForm(d.name);

		fprintf(header,"class %s_base : %s {\n",iname.c_str(),inherits.c_str());
		fprintf(header,"public:\n");
		fprintf(header,"\tstatic unsigned long _IID; // interface ID\n\n");
		fprintf(header,"\tstatic %s_base *_create(const std::string& subClass"
						" = \"%s\");\n", iname.c_str(),d.name.c_str());
		fprintf(header,"\tstatic %s_base *_fromString(std::string objectref);\n",
														iname.c_str());
		fprintf(header,"\tstatic %s_base *_fromReference(Arts::ObjectReference ref,"
		                                " bool needcopy);\n\n",iname.c_str());

		fprintf(header,"\tstatic %s_base *_fromDynamicCast(const Arts::Object&"
										" object);\n", iname.c_str());

		/* reference counting: _copy */
		fprintf(header,"\tinline %s_base *_copy() {\n"
					   "\t\tassert(_refCnt > 0);\n"
				       "\t\t_refCnt++;\n"
					   "\t\treturn this;\n"
					   "\t}\n\n",iname.c_str());
				
		// Default I/O info
		fprintf(header,"\tvirtual std::vector<std::string> _defaultPortsIn() const;\n");
		fprintf(header,"\tvirtual std::vector<std::string> _defaultPortsOut() const;\n");
		fprintf(header,"\n");

		// Casting
		fprintf(header,"\tvoid *_cast(unsigned long iid);\n\n");

		/* attributes (not for streams) */
		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;
			string rc = createTypeCode(ad.type,"",MODEL_RESULT);
			string pc = createTypeCode(ad.type,"newValue",MODEL_ARG);

			if(ad.flags & attributeAttribute)
			{
				if(ad.flags & streamOut)  /* readable from outside */
				{
					fprintf(header,"\tvirtual %s %s() = 0;\n",rc.c_str(),
						ad.name.c_str());
				}
				if(ad.flags & streamIn)  /* writeable from outside */
				{
					fprintf(header,"\tvirtual void %s(%s) = 0;\n",
						ad.name.c_str(), pc.c_str());
				}
			}
		}
		
		/* methods */
		for(mi = d.methods.begin(); mi != d.methods.end(); mi++)
		{
			MethodDef& md = *mi;
			string rc = createReturnCode(md);
			string params = createParamList(md);

			fprintf(header,"\tvirtual %s %s(%s) = 0;\n",rc.c_str(),
				md.name.c_str(), params.c_str());
		}		
		fprintf(header,"};\n\n");

		// create stub

		inherits = buildInheritanceList(d,"_stub");
		if(inherits == "") inherits = "virtual public Arts::Object_stub";

		fprintf(header,"class %s_stub : virtual public %s_base, %s {\n",
			iname.c_str(), iname.c_str(),inherits.c_str());
		fprintf(header,"protected:\n");
		fprintf(header,"\t%s_stub();\n\n",iname.c_str());

		fprintf(header,"public:\n");
		fprintf(header,"\t%s_stub(Arts::Connection *connection, long objectID);\n\n",
			iname.c_str());
			/* attributes (not for streams) */
		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;
			string rc = createTypeCode(ad.type,"",MODEL_RESULT);
			string pc = createTypeCode(ad.type,"newValue",MODEL_ARG);

			if(ad.flags & attributeAttribute)
			{
				if(ad.flags & streamOut)  /* readable from outside */
				{
					fprintf(header,"\t%s %s();\n",rc.c_str(),
						ad.name.c_str());
				}
				if(ad.flags & streamIn)  /* writeable from outside */
				{
					fprintf(header,"\tvoid %s(%s);\n",
						ad.name.c_str(), pc.c_str());
				}
			}
		}
			/* methods */
		for(mi = d.methods.begin(); mi != d.methods.end(); mi++)
		{
			MethodDef& md = *mi;
			string rc = createReturnCode(md);
			string params = createParamList(md);

			fprintf(header,"\t%s %s(%s);\n",rc.c_str(),
				md.name.c_str(), params.c_str());
		}
		fprintf(header,"};\n\n");

		// create skeleton

		inherits = buildInheritanceList(d,"_skel");
		if(inherits == "") inherits = "virtual public Arts::Object_skel";

		fprintf(header,"class %s_skel : virtual public %s_base,"
			" %s {\n",iname.c_str(),iname.c_str(),inherits.c_str());

		bool firstStream = true;
		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;

			if(ad.flags & attributeStream)
			{
				if(firstStream)
				{
					fprintf(header,"protected:\n");
					fprintf(header,"\t// variables for streams\n");
					firstStream = false;
				}

				/** generate declaration of the variable: multi stream? **/
				string decl;

				if(ad.flags & streamMulti)
				{
					if(ad.flags & streamAsync)
						decl = createTypeCode(ad.type,ad.name,MODEL_AMSTREAM);
					else
						decl = createTypeCode(ad.type,ad.name,MODEL_MSTREAM);
				}
				else
				{
					if(ad.flags & streamAsync)
						decl = createTypeCode(ad.type,ad.name,MODEL_ASTREAM);
					else
						decl = createTypeCode(ad.type,ad.name,MODEL_STREAM);
				}

				decl += ";";

				/** write to source **/
				string comment;

				if(ad.flags & streamIn) comment = "incoming stream";
				if(ad.flags & streamOut) comment = "outgoing stream";

				fprintf(header,"\t%-40s  // %s\n",decl.c_str(),comment.c_str());
			}
		}
		if(!firstStream) fprintf(header,"\n");

		bool haveAsyncStreams = false;

		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;

			if((ad.flags & attributeStream) && (ad.flags & streamAsync))
			{
				if(!haveAsyncStreams)
				{
					fprintf(header,"\t// handler for asynchronous streams\n");
					haveAsyncStreams = true;
				}

				string ptype =
					createTypeCode(ad.type,"",MODEL_ASTREAM_PACKETPTR);

				if(ad.flags & streamIn)
				{
					fprintf(header,"\tvirtual void process_%s(%s) = 0;\n",
										ad.name.c_str(),ptype.c_str());
				}
				else
				{
					fprintf(header,"\tvirtual void request_%s(%s);\n",
										ad.name.c_str(),ptype.c_str());
				}
			}
		}
		if(haveAsyncStreams) fprintf(header,"\n");

		bool haveChangeNotifications = false;

		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;

			if((ad.flags & attributeAttribute) && (ad.flags & streamOut)
			&& (ad.type == "byte" || ad.type == "float" || ad.type == "long"
			||  ad.type == "string" || ad.type == "boolean"
			||  ad.type == "*byte" || ad.type == "*float" || ad.type == "*long"
			||  ad.type == "*string" || isEnum(ad.type)))
			{
				if(!haveChangeNotifications)
				{
					fprintf(header,"protected:\n");
					fprintf(header,"\t// emitters for change notifications\n");
					haveChangeNotifications = true;
				}

				string pc = createTypeCode(ad.type,"newValue",MODEL_ARG);

				fprintf(header,"\tinline void %s_changed(%s) {\n",
										ad.name.c_str(),pc.c_str());
				fprintf(header,"\t\t_emit_changed(\"%s_changed\",newValue);\n",
										ad.name.c_str());
				fprintf(header,"\t}\n");
			}
		}
		if(haveChangeNotifications) fprintf(header,"\n");

		fprintf(header,"public:\n");
		fprintf(header,"\t%s_skel();\n\n",iname.c_str());

		fprintf(header,"\tstatic std::string _interfaceNameSkel();\n");
		fprintf(header,"\tstd::string _interfaceName();\n");
		fprintf(header,"\tbool _isCompatibleWith(const std::string& interfacename);\n");
		fprintf(header,"\tvoid _buildMethodTable();\n");
		fprintf(header,"\tvoid dispatch(Arts::Buffer *request, Arts::Buffer *result,"
						"long methodID);\n");

		if(haveAsyncStreams)
		  fprintf(header,"\tvoid notify(const Arts::Notification& notification);\n");

		fprintf(header,"};\n\n");

		nspace.leaveAll();
		
		// Create object wrapper for easy C++ syntax
		
		fprintf(header,"#include \"reference.h\"\n");
		
		// Allow connect facility only if there is something to connect to!
/*		if (haveStreams(d)) {
			fprintf(header,"#include \"flowsystem.h\"\n");
		}
		fprintf(header,"\n");
*/
		nspace.setFromSymbol(d.name);

		inherits = ": public Arts::Object";

		fprintf(header,"class %s %s {\n",iname.c_str(),inherits.c_str());
		fprintf(header,"private:\n");
		fprintf(header,"\tstatic Arts::Object_base* _Creator();\n");
		fprintf(header,"\t%s_base *_cache;\n",iname.c_str());
		fprintf(header,"\tinline %s_base *_method_call() {\n",iname.c_str());
		fprintf(header,"\t\t_pool->checkcreate();\n");
		fprintf(header,"\t\tif(_pool->base) {\n");
		fprintf(header,"\t\t\t_cache="
							"(%s_base *)_pool->base->_cast(%s_base::_IID);\n",
							iname.c_str(),iname.c_str());
		fprintf(header,"\t\t\tassert(_cache);\n");
		fprintf(header,"\t\t}\n");
		fprintf(header,"\t\treturn _cache;\n");
		fprintf(header,"\t}\n");
		
		// This constructor is now protected. use ::null() and ::_from_base()
		// if necessary. It is protected, though there should be noinherited
		// class
		fprintf(header,"\nprotected:\n");
		fprintf(header,"\tinline %s(%s_base* b) : Arts::Object(b), _cache(0) {}\n\n",
			iname.c_str(),iname.c_str());
		
		fprintf(header,"\npublic:\n");
		fprintf(header,"\ttypedef %s_base _base_class;\n\n",iname.c_str());
		// empty constructor: specify creator for create-on-demand
		fprintf(header,"\tinline %s() : Arts::Object(_Creator), _cache(0) {}\n",iname.c_str());
		
		// constructors from reference and for subclass
		fprintf(header,"\tinline %s(const Arts::SubClass& s) :\n"
			"\t\tArts::Object(%s_base::_create(s.string())), _cache(0) {}\n",
			iname.c_str(),iname.c_str());
		fprintf(header,"\tinline %s(const Arts::Reference &r) :\n"
			"\t\tArts::Object("
			"r.isString()?(%s_base::_fromString(r.string())):"
			"(%s_base::_fromReference(r.reference(),true))), _cache(0) {}\n",
			iname.c_str(),iname.c_str(), iname.c_str());
		fprintf(header,"\tinline %s(const Arts::DynamicCast& c) : "
			"Arts::Object(%s_base::_fromDynamicCast(c.object())), "
			"_cache(0) {}\n", iname.c_str(),iname.c_str());

		// copy constructors
		fprintf(header,"\tinline %s(const %s& target) : Arts::Object(target._pool), _cache(target._cache) {}\n",
			iname.c_str(),iname.c_str());
		fprintf(header,"\tinline %s(Arts::Object::Pool& p) : Arts::Object(p), _cache(0) {}\n",
			iname.c_str());
			
		// null object
		// %s::null() returns a null object (and not just a reference to one)
		fprintf(header,"\tinline static %s null() {return %s((%s_base*)0);}\n",
			iname.c_str(),iname.c_str(),iname.c_str());
		fprintf(header,"\tinline static %s _from_base(%s_base* b) {return %s(b);}\n",
			iname.c_str(),iname.c_str(),iname.c_str());

		// copy operator.
		fprintf(header,"\tinline %s& operator=(const %s& target) {\n",
			iname.c_str(),iname.c_str());
		// test for equality
		fprintf(header,"\t\tif (_pool == target._pool) return *this;\n");
		fprintf(header,"\t\t_pool->Dec();\n");
		fprintf(header,"\t\t_pool = target._pool;\n");
		fprintf(header,"\t\t_cache = target._cache;\n");
		fprintf(header,"\t\t_pool->Inc();\n");
		fprintf(header,"\t\treturn *this;\n");
		fprintf(header,"\t}\n");

		// casts to parent interfaces
		vector<string> parents = allParentsUnique(d);
		for (vector<std::string>::iterator si = parents.begin();
												si != parents.end(); si++)
		{
			string &s = *si;
			fprintf(header,"\tinline operator %s() const { return %s(*_pool); }\n",
									s.c_str(), s.c_str());
		}

		// conversion to _base* object
		fprintf(header,"\tinline %s_base* _base() {return _cache?_cache:_method_call();}\n",iname.c_str());
		fprintf(header,"\n");

		vector<string> all = parents;
		vector<string>::iterator i;
		all.push_back(d.name);
		// InterfaceDef allMerged = mergeAllParents(d);

		map<string, string> definitionMap;

		for(i=all.begin();i != all.end();i++)
		{
			InterfaceDef id = findInterface(*i);
			string baseclass = id.name+"_base";

			/* attributes */
			for(ai = id.attributes.begin();ai != id.attributes.end();ai++)
			{
				AttributeDef& ad = *ai;
				ForwardCode fc;
				fc.fullifacename = fullifacename;
				fc.constructor = false;
				fc.mname = ad.name;
				fc.baseclass = baseclass;

				checkSymbolDefinition(ad.name, "attribute", id, definitionMap);

				if(ad.flags & attributeAttribute)
				{
					if(ad.flags & streamOut)  /* readable from outside */
					{
						fc.params = "";
						fc.callparams = "";
						fc.result = createTypeCode(ad.type,"",MODEL_RESULT);
						fprintf(header,"\tinline %s %s();\n",
							fc.result.c_str(), fc.mname.c_str());
						forwardCode.push_back(fc);
					}
					if(ad.flags & streamIn)  /* writeable from outside */
					{
						fc.params =
							createTypeCode(ad.type,"_newValue",MODEL_ARG);
						fc.callparams = "_newValue";
						fc.result="void";
						fprintf(header,"\tinline void %s(%s);\n",
							fc.mname.c_str(), fc.params.c_str());
						forwardCode.push_back(fc);
					}
				}
			}

			/* methods */
			for(mi = id.methods.begin(); mi != id.methods.end(); mi++)
			{
				MethodDef& md = *mi;
				ForwardCode fc;
				fc.fullifacename = fullifacename;
				fc.result = createReturnCode(md);
				fc.params = createParamList(md);
				fc.callparams = createCallParamList(md);
				fc.constructor = (md.name == "constructor");
				fc.baseclass = baseclass;

				checkSymbolDefinition(md.name, "method", id, definitionMap);

				// map constructor methods to the real things
				if (md.name == "constructor") {
					fc.mname = iname;
					fprintf(header,"\tinline %s(%s);\n",
											iname.c_str(),fc.params.c_str());
				} else {
					fc.mname = md.name;
					fprintf(header,"\tinline %s %s(%s);\n",fc.result.c_str(),
											md.name.c_str(),fc.params.c_str());
				}

				forwardCode.push_back(fc);
			}
		}
		fprintf(header,"};\n\n");
	}

	nspace.leaveAll();

	/*
	 * Forwarding code. We have to do this here, as the classes may depend on
	 * each other, e.g. an argument of one function are a SmartWrapper which is
	 * declared later in the text.
	 */
	if(!forwardCode.empty())
		fprintf(header,"// Forward wrapper calls to _base classes:\n\n");

	list<ForwardCode>::iterator fi;
	for(fi = forwardCode.begin(); fi != forwardCode.end(); fi++)
	{
		if(fi->constructor)
		{
			fprintf(header,"inline %s::%s(%s)\n", fi->fullifacename.c_str(),
							fi->mname.c_str(), fi->params.c_str());
			fprintf(header,"\t\t: Arts::Object(%s_base::_create())\n",
				fi->mname.c_str());
			fprintf(header,"{\n");
			fprintf(header,"\tstatic_cast<%s*>(_method_call())->constructor(%s);\n",
								fi->baseclass.c_str(),fi->callparams.c_str());
			fprintf(header,"}\n\n");
		}
		else
		{
			fprintf(header,"inline %s %s::%s(%s)\n",
							fi->result.c_str(), fi->fullifacename.c_str(),
							fi->mname.c_str(), fi->params.c_str());
			fprintf(header,"{\n");
			fprintf(header,"\t%s _cache?static_cast<%s*>(_cache)->%s(%s):"
								"static_cast<%s*>(_method_call())->%s(%s);\n",
							fi->result=="void"?"":"return",
							fi->baseclass.c_str(),
							fi->mname.c_str(),fi->callparams.c_str(),
							fi->baseclass.c_str(),
							fi->mname.c_str(),fi->callparams.c_str());

			fprintf(header,"}\n\n");
		}
	}
}

enum DefaultDirection {defaultIn, defaultOut};
bool addParentDefaults(InterfaceDef& iface, vector<std::string>& ports, DefaultDirection dir);
bool lookupParentPort(InterfaceDef& iface, string port, vector<std::string>& ports, DefaultDirection dir);

bool addDefaults(InterfaceDef& iface, vector<std::string>& ports, DefaultDirection dir)
{
	vector<AttributeDef>::iterator ai;
	vector<std::string>::iterator di;
	bool hasDefault = false;
	// Go through the default ports of this interface
	for (di = iface.defaultPorts.begin(); di != iface.defaultPorts.end(); di++) {
		bool foundIn = false, foundOut = false;
		// Find the corresponding attribute definition
		for (ai = iface.attributes.begin(); ai != iface.attributes.end(); ai++)
		{
			AttributeDef& ad = *ai;

			if ((ad.flags & attributeStream) && ((*di)==ad.name)) {
				// Add this port to the list
				if (ad.flags & streamIn) {
					foundIn=true;
					if (dir==defaultIn) ports.push_back(*di);
				}
				// Add this port to the list
				if (ad.flags & streamOut) {
					foundOut=true;
					if (dir==defaultOut) ports.push_back(*di);
				}
			}
		}
		bool found = false;
		// Not found, might come from a parent
		if (!(foundIn || foundOut)) {
			found = lookupParentPort(iface, *di, ports, dir);
		}
		if ((found) || (foundIn && (dir==defaultIn)) || (foundOut && (dir==defaultOut)))
			hasDefault = true;
	}
	// If no default was specified, then try to inherit some
	if (!hasDefault)
		hasDefault = addParentDefaults(iface, ports, dir);
	
	// Still have no default?
	// If we have only one stream in a given direction, make it default.
	if (!hasDefault) {
		vector<AttributeDef>::iterator foundPos;
		int found = 0;
		for (ai = iface.attributes.begin(); ai != iface.attributes.end(); ai++)
		{
			AttributeDef& ad = *ai;
			if (ad.flags & attributeStream) {
				if ((ad.flags & streamIn) && (dir == defaultIn)) {
					found++; foundPos=ai;
				}
				if ((ad.flags & streamOut) && (dir == defaultOut)) {
					found++; foundPos=ai;
				}
			}
		}
		if (found == 1) {hasDefault=true; ports.push_back(foundPos->name);}
	}
	return hasDefault;
}


bool addParentDefaults(InterfaceDef& iface, vector<std::string>& ports, DefaultDirection dir)
{
	list<InterfaceDef>::iterator interIt;
	vector<std::string>::iterator si;
	bool hasDefault = false;
	// For all inherited interfaces
	for (si = iface.inheritedInterfaces.begin(); si != iface.inheritedInterfaces.end(); si++)
	{
		// Find the corresponding interface definition
		for (interIt=interfaces.begin(); interIt!=interfaces.end(); interIt++) {
			InterfaceDef& parent = *interIt;
			if (parent.name == (*si)) {
				// Now add the default ports of this parent
				bool b = addDefaults(parent, ports, dir);
				if (b) hasDefault = true;
				break;
			}
		}
	}
	return hasDefault;
}

bool lookupParentPort(InterfaceDef& iface, string port, vector<std::string>& ports, DefaultDirection dir)
{
	list<InterfaceDef>::iterator interIt;
	vector<AttributeDef>::iterator ai;
	vector<std::string>::iterator si;
	// For all inherited interfaces
	for (si = iface.inheritedInterfaces.begin(); si != iface.inheritedInterfaces.end(); si++)
	{
		// Find the corresponding interface definition
		for (interIt=interfaces.begin(); interIt!=interfaces.end(); interIt++) {
			InterfaceDef& parent = *interIt;
			if (parent.name == (*si)) {
				// Now look at the ports of this parent
				vector<AttributeDef>::iterator foundPos;
				bool found = false;
				for (ai = parent.attributes.begin(); ai != parent.attributes.end(); ai++) {
					if ((ai->flags & attributeStream) && (ai->name==port)){
						if (((ai->flags & streamIn) && (dir == defaultIn))
						|| ((ai->flags & streamOut) && (dir == defaultOut))) {
							found = true; foundPos=ai; break;
						}
					}
				}
				if (found) {ports.push_back(port); return true;}
				// Not found, look recursively at the parent ancestors
				bool b = lookupParentPort(parent, port, ports, dir);
				if (b) return true; // done
				break;
			}
		}
	}
	return false;
}

void doInterfacesSource(FILE *source)
{
	list<InterfaceDef>::iterator ii;
	vector<MethodDef>::iterator mi;
	vector<AttributeDef>::iterator ai;

	long mcount;

	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef& d = *ii;

		if(fromInclude(d.name)) continue; // should come from the include

		string iname = NamespaceHelper::nameOf(d.name);

		// create static functions

		fprintf(source,"%s_base *%s_base::_create(const std::string& subClass)\n",
											d.name.c_str(),d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tArts::Object_skel *skel = "
							"Arts::ObjectManager::the()->create(subClass);\n");
		fprintf(source,"\tassert(skel);\n");
		fprintf(source,"\t%s_base *castedObject = "
							"(%s_base *)skel->_cast(%s_base::_IID);\n",
							d.name.c_str(),d.name.c_str(),d.name.c_str());
		fprintf(source,"\tassert(castedObject);\n");
		fprintf(source,"\treturn castedObject;\n");
		fprintf(source,"}\n\n");

	
		fprintf(source,"%s_base *%s_base::_fromString(std::string objectref)\n",
											d.name.c_str(),d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tArts::ObjectReference r;\n\n");
		fprintf(source,"\tif(Arts::Dispatcher::the()->stringToObjectReference(r,objectref))\n");
		fprintf(source,"\t\treturn %s_base::_fromReference(r,true);\n",
															d.name.c_str());
		fprintf(source,"\treturn 0;\n");
		fprintf(source,"}\n\n");

		fprintf(source,"%s_base *%s_base::_fromDynamicCast(const Arts::Object& object)\n",
											d.name.c_str(),d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tif(object.isNull()) return 0;\n\n");
		fprintf(source,"\t%s_base *castedObject = (%s_base *)object._base()->_cast(%s_base::_IID);\n",
											d.name.c_str(), d.name.c_str(), d.name.c_str());
		fprintf(source,"\tif(castedObject) return castedObject->_copy();\n\n");
		fprintf(source,"\treturn _fromString(object._toString());\n");
		fprintf(source,"}\n\n");

		fprintf(source,"%s_base *%s_base::_fromReference(Arts::ObjectReference r,"
		               " bool needcopy)\n",d.name.c_str(),d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\t%s_base *result;\n",d.name.c_str());
		fprintf(source,
		"\tresult = (%s_base *)Arts::Dispatcher::the()->connectObjectLocal(r,\"%s\");\n",
										d.name.c_str(),d.name.c_str());
		fprintf(source,"\tif(result)\n");
		fprintf(source,"\t{\n");
		fprintf(source,"\t\tif(!needcopy)\n");
		fprintf(source,"\t\t\tresult->_cancelCopyRemote();\n");
		fprintf(source,"\t}\n");
		fprintf(source,"\telse\n");
		fprintf(source,"\t{\n");
		fprintf(source,"\t\tArts::Connection *conn = "
							"Arts::Dispatcher::the()->connectObjectRemote(r);\n");
		fprintf(source,"\t\tif(conn)\n");
		fprintf(source,"\t\t{\n");
		fprintf(source,"\t\t\tresult = new %s_stub(conn,r.objectID);\n",
										d.name.c_str());
		fprintf(source,"\t\t\tif(needcopy) result->_copyRemote();\n");
		fprintf(source,"\t\t\tresult->_useRemote();\n");

		// Type checking
		/*
		 * One may wonder why we first claim that we want to use the object
		 * using _useRemote, then check if we *can* to use it (if the
		 * type is right), and finally, if we can't release it
		 * again. 
		 *
		 * However, if we don't, we can't release it either, as we may not
		 * release an object which we don't use. If we would not call release,
		 * we would on the other hand create a *local* memory leak, as the
		 * _stub wouldn't get removed.
		 */
		fprintf(source,"\t\t\tif (!result->_isCompatibleWith(\"%s\")) {\n",
			d.name.c_str());
		fprintf(source,"\t\t\t\tresult->_release();\n");
		fprintf(source,"\t\t\t\treturn 0;\n");
		fprintf(source,"\t\t\t}\n");

		fprintf(source,"\t\t}\n");
		fprintf(source,"\t}\n");
		fprintf(source,"\treturn result;\n");
		fprintf(source,"}\n\n");
		
		
		// Default I/O info
		vector<std::string> portsIn, portsOut;
		vector<std::string>::iterator si, di;
		addDefaults(d, portsIn, defaultIn);
		addDefaults(d, portsOut, defaultOut);
		
		vector<std::string> done; // don't repeat values
		fprintf(source,"std::vector<std::string> %s_base::_defaultPortsIn() const {\n",d.name.c_str());
		fprintf(source,"\tstd::vector<std::string> ret;\n");
		// Loop through all the values
		for (si = portsIn.begin(); si != portsIn.end(); si++)
		{
			// repeated value? (virtual public like merging...)
			bool skipIt = false;
			for (di = done.begin(); di != done.end(); di++) {
				if ((*di)==(*si)) {skipIt = true; break;}
			}
			if (skipIt) continue;
			fprintf(source,"\tret.push_back(\"%s\");\n",(*si).c_str());
			done.push_back(*si);
		}
		fprintf(source,"\treturn ret;\n}\n");
		done.clear();
		fprintf(source,"std::vector<std::string> %s_base::_defaultPortsOut() const {\n",d.name.c_str());
		fprintf(source,"\tstd::vector<std::string> ret;\n");
		// Loop through all the values
		for (si = portsOut.begin(); si != portsOut.end(); si++)
		{
			// repeated value? (virtual public like merging...)
			bool skipIt = false;
			for (di = done.begin(); di != done.end(); di++) {
				if ((*di)==(*si)) {skipIt = true; break;}
			}
			if (skipIt) continue;
			fprintf(source,"\tret.push_back(\"%s\");\n",(*si).c_str());
			done.push_back(*si);
		}
		fprintf(source,"\treturn ret;\n}\n\n");

		/** _cast operation **/
		vector<std::string> parentCast = allParentsUnique(d);

		fprintf(source,"void *%s_base::_cast(unsigned long iid)\n",
			d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tif(iid == %s_base::_IID) return (%s_base *)this;\n",
			d.name.c_str(),d.name.c_str());

		vector<std::string>::iterator pci;
		for(pci = parentCast.begin(); pci != parentCast.end();pci++)
		{
			string& pc = *pci;
			fprintf(source,"\tif(iid == %s_base::_IID) "
							"return (%s_base *)this;\n",pc.c_str(),pc.c_str());
		}
		fprintf(source,"\tif(iid == Arts::Object_base::_IID) return (Arts::Object_base *)this;\n");
		fprintf(source,"\treturn 0;\n");
		fprintf(source,"}\n\n");

		// create stub

		/** constructors **/
		fprintf(source,"%s_stub::%s_stub()\n" ,d.name.c_str(),iname.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\t// constructor for subclasses"
										" (don't use directly)\n");
		fprintf(source,"}\n\n");

		fprintf(source,"%s_stub::%s_stub(Arts::Connection *connection, "
						"long objectID)\n",d.name.c_str(),iname.c_str());
		fprintf(source,"	: Arts::Object_stub(connection, objectID)\n");
		fprintf(source,"{\n");
		fprintf(source,"\t// constructor to create a stub for an object\n");
		fprintf(source,"}\n\n");

		/** stub operations **/

			/** stub operations for object methods **/
		for(mi = d.methods.begin(); mi != d.methods.end(); mi++)
		{
			MethodDef& md = *mi;
			createStubCode(source,d.name.c_str(),md.name.c_str(),md);
		}

			/** stub operations for attributes **/
		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;

			if(ad.flags & attributeAttribute)
			{
				MethodDef md;
				if(ad.flags & streamOut)  /* readable from outside */
				{
					md.name = "_get_"+ad.name;
					md.type = ad.type;
					md.flags = methodTwoway;
					/* no parameters (don't set md.signature) */

					createStubCode(source,d.name.c_str(),ad.name.c_str(),md);
				}
				if(ad.flags & streamIn)  /* writeable from outside */
				{
					md.name = "_set_"+ad.name;
					md.type = "void";
					md.flags = methodTwoway;

					ParamDef pd;
					pd.type = ad.type;
					pd.name = "newValue";
					md.signature.push_back(pd);

					createStubCode(source,d.name.c_str(),ad.name.c_str(),md);
				}
			}
		}

		// create skeleton

		/** _interfaceName **/
		fprintf(source,"std::string %s_skel::_interfaceName()\n",	
													d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\treturn \"%s\";\n",d.name.c_str());
		fprintf(source,"}\n\n");

		// Run-time type compatibility check
		fprintf(source,"bool %s_skel::_isCompatibleWith(const std::string& interfacename)\n",
			d.name.c_str());
		fprintf(source,"{\n");
		// Interface is compatible with itself!
		fprintf(source,"\tif (interfacename == \"%s\") return true;\n",d.name.c_str());
		// It also provides the parent interfaces
		for(pci = parentCast.begin(); pci != parentCast.end();pci++)
		{
			fprintf(source,"\tif (interfacename == \"%s\") return true;\n", pci->c_str());
		}
		// and is ultimately an Object
		fprintf(source,"\tif (interfacename == \"Arts::Object\") return true;\n");
		fprintf(source,"\treturn false;\n"); // And nothing else
		fprintf(source,"}\n\n");
		
		fprintf(source,"std::string %s_skel::_interfaceNameSkel()\n",	
													d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\treturn \"%s\";\n",d.name.c_str());
		fprintf(source,"}\n\n");

		/** dispatch operations **/
		Buffer methodTable;

			/** dispatch operations for object methods **/
		mcount = 0;
		for(mi = d.methods.begin(); mi != d.methods.end(); mi++, mcount++)
		{
			MethodDef& md = *mi;
			md.writeType(methodTable);

			createDispatchFunction(source,mcount,d,md,md.name);
		}

			/** dispatch operations for attributes **/

		for(ai = d.attributes.begin();ai != d.attributes.end();ai++)
		{
			AttributeDef& ad = *ai;

			if(ad.flags & attributeAttribute)
			{
				MethodDef md;
				if(ad.flags & streamOut)  /* readable from outside */
				{
					md.name = "_get_"+ad.name;
					md.type = ad.type;
					md.flags = methodTwoway;
					/* no parameters (don't set md.signature) */

					md.writeType(methodTable);
					createDispatchFunction(source,mcount++,d,md,ad.name);
				}
				if(ad.flags & streamIn)  /* writeable from outside */
				{
					md.name = "_set_"+ad.name;
					md.type = "void";
					md.flags = methodTwoway;

					ParamDef pd;
					pd.type = ad.type;
					pd.name = "newValue";

					md.signature.push_back(pd);

					md.writeType(methodTable);
					createDispatchFunction(source,mcount++,d,md,ad.name);
				}
			}
		}

		/** methodTable **/

		string methodTableString = formatMultiLineString(
			methodTable.toString("MethodTable"),"        ");

		fprintf(source,"void %s_skel::_buildMethodTable()\n",d.name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tArts::Buffer m;\n");
		fprintf(source,"\tm.fromString(\n");
		fprintf(source,"%s,\n",methodTableString.c_str());
		fprintf(source,"\t\t\"MethodTable\"\n");
		fprintf(source,"\t);\n");

		long i;
		for(i=0;i<mcount;i++)
			fprintf(source,"\t_addMethod(%s,this,Arts::MethodDef(m));\n",
							dispatchFunctionName(d.name,i).c_str());

		vector<string>::iterator ii = d.inheritedInterfaces.begin();
		while(ii != d.inheritedInterfaces.end())
		{
			fprintf(source,"\t%s_skel::_buildMethodTable();\n",
				ii->c_str());
			ii++;
		}
		fprintf(source,"}\n\n");

		fprintf(source,"%s_skel::%s_skel()\n", d.name.c_str(),iname.c_str());
		fprintf(source,"{\n");
		for(ai = d.attributes.begin(); ai != d.attributes.end(); ai++)
		{
			AttributeDef& ad = *ai;
			if((ad.flags & attributeStream) == attributeStream)
			{
				fprintf(source,"\t_initStream(\"%s\",&%s,%d);\n",
							ad.name.c_str(),ad.name.c_str(),ad.flags);
			}
		}
		fprintf(source,"}\n\n");

		/** notification operation **/
		if(haveAsyncStreams(d))
		{
			fprintf(source,"void %s_skel::notify(const Arts::Notification "
			               "&notification)\n", d.name.c_str());
			fprintf(source,"{\n");
			for(ai = d.attributes.begin(); ai != d.attributes.end(); ai++)
			{
				AttributeDef& ad = *ai;
				if((ad.flags & (attributeStream|streamAsync))
								== (attributeStream|streamAsync))
				{
					const char *fname=(ad.flags&streamIn)?"process":"request";
					string packettype =
						createTypeCode(ad.type,"",MODEL_ASTREAM_PACKETPTR);

					fprintf(source,"\tif(%s.notifyID() == notification.ID)\n",
							ad.name.c_str());
					fprintf(source,
					   "\t\t%s_%s((%s)notification.data);\n",
						fname,ad.name.c_str(),packettype.c_str());
				}
			}
			fprintf(source,"}\n\n");

			/*
			 * create empty request_ methods for output streams
			 * (not everybody uses requesting)
			 */
			for(ai = d.attributes.begin(); ai != d.attributes.end(); ai++)
			{
				AttributeDef& ad = *ai;
				if((ad.flags & (attributeStream|streamAsync|streamOut))
								== (attributeStream|streamAsync|streamOut))
				{
					string packettype =
						createTypeCode(ad.type,"",MODEL_ASTREAM_PACKETPTR);
					fprintf(source,"void %s_skel::request_%s(%s)\n",
						d.name.c_str(),ad.name.c_str(),packettype.c_str());
					fprintf(source,"{\n");
					fprintf(source,"	assert(false); // this default is for "
					               "modules who don't want requesting\n");
					fprintf(source,"}\n\n");
				}
			}
		}
		
		// Smartwrapper statics
		// _Creator
		fprintf(source,"Arts::Object_base* %s::_Creator() {\n",d.name.c_str());
		fprintf(source,"\treturn %s_base::_create();\n",d.name.c_str());
		fprintf(source,"}\n\n");

		// IID
		fprintf(source,"unsigned long %s_base::_IID = "
			"Arts::MCOPUtils::makeIID(\"%s\");\n\n",d.name.c_str(),d.name.c_str());
	}
}

void doInterfaceRepoSource(FILE *source, string prefix)
{
	Buffer b;
	module.moduleName = "";
	module.writeType(b);

	string data = formatMultiLineString(b.toString("IDLFile"),"    ");

	fprintf(source,"static Arts::IDLFileReg IDLFileReg_%s(\"%s\",\n%s\n);\n",
		prefix.c_str(),prefix.c_str(),data.c_str());
}

void doTypeFile(string prefix)
{
	Buffer b;
	module.moduleName = prefix;
	module.writeType(b);
	
	FILE *typeFile = fopen((prefix+".mcoptype.new").c_str(),"w");
	unsigned long towrite = b.size();
	fwrite(b.read(towrite),1,towrite,typeFile);
	fclose(typeFile);
}

void doTypeIndex(string prefix)
{
	FILE *typeIndex = fopen((prefix+".mcopclass.new").c_str(),"w");

	vector<string> supportedTypes;

	vector<InterfaceDef>::iterator ii;
	for(ii = module.interfaces.begin(); ii != module.interfaces.end(); ii++)
		if(!fromInclude(ii->name)) supportedTypes.push_back(ii->name);

	vector<TypeDef>::iterator ti;
	for(ti = module.types.begin(); ti != module.types.end(); ti++)
		if(!fromInclude(ti->name)) supportedTypes.push_back(ti->name);

	string supportedTypesList;
	vector<string>::iterator si;
	bool first = true;
	for(si = supportedTypes.begin(); si != supportedTypes.end(); si++)
	{
		if(!first) supportedTypesList += ",";

		supportedTypesList += (*si);
		first = false;
	}
	fprintf(typeIndex, "# this file was generated by the MCOP idl compiler - DO NOT EDIT\n");
	fprintf(typeIndex,"Type=%s\n",supportedTypesList.c_str());
	fprintf(typeIndex,"TypeFile=%s.mcoptype\n",prefix.c_str());
	fclose(typeIndex);
}

void exit_usage(char *name)
{
	fprintf(stderr,"usage: %s [ <options> ] <filename>\n",name);
	fprintf(stderr,"\nOptions:\n");
	fprintf(stderr,"  -I <directory>     search in <directory> for includes\n");
	fprintf(stderr,"  -e <name>          exclude a struct/interface/enum from code generation\n");
	fprintf(stderr,"  -t                 create .mcoptype/.mcopclass files with type information\n");
	exit(1);
}
extern void mcopidlParse(const char *code);

bool match(vector<char>::iterator start, const char *string)
{
	while(*string && *start)
		if(*string++ != *start++) return false;

	return (*string == 0);
}

bool fileExists(const char *filename)
{
	FILE *test = fopen(filename,"r");
	if(test)
	{
		fclose(test);
		return true;
	}
	return false;
}

string searchFile(const char *filename,list<string>& path)
{
	if(fileExists(filename)) return filename;

	list<string>::iterator i;
	for(i = path.begin(); i != path.end(); i++)
	{
		string location = *i + "/" + filename;
		if(fileExists(location.c_str())) return location;
	}
	fprintf(stderr,"file '%s' not found\n",filename);
	exit(1);
}

void append_file_to_vector(const char *filename, vector<char>& v)
{
	FILE *f = fopen(filename,"r");
	if(!f) {
		fprintf(stderr,"file '%s' not found\n",filename);
		exit(1);
	}

	char buffer[1024];
	long l;
	while((l = fread(buffer,1,1024,f)) > 0)
		v.insert(v.end(),buffer, buffer+l);
	fclose(f);
}

void append_string_to_vector(const char *string, vector<char>& v)
{
	while(*string) v.push_back(*string++);
}

void preprocess(vector<char>& input, vector<char>& output)
{
	string filename;
	enum { lineStart, idlCode, commentC, filenameFind,
	            filenameIn1, filenameIn2 } state = lineStart;

	vector<char>::iterator i = input.begin();

	while(i != input.end())
	{
		if(state != commentC && match(i,"/*")) // check if here starts a comment
		{
			state = commentC;
			i += 2;
		}
		else if(state == commentC)
		{
			if(match(i,"*/")) // leave comment state?
			{
				state = idlCode;
				i += 2;
			}
			else // skip comments
			{
				if(*i == '\n') output.push_back(*i); // keep line numbering
				i++;
			}
		}
		else if(state == filenameFind)
		{
			switch(*i++)
			{
				case ' ':	// skip whitespaces
				case '\t':
					break;

				case '"':	state = filenameIn1;
					break;
				case '<':	state = filenameIn2;
					break;
				default:	cerr << "bad char after #include statement" << endl;
							assert(0); // error handling!
			}
		}
		else if((state == filenameIn1 && *i == '"')
			 || (state == filenameIn2 && *i == '>'))
		{
			append_string_to_vector("#startinclude <",output);
			append_string_to_vector(filename.c_str(),output);
			append_string_to_vector(">\n",output);

			if(!haveIncluded(filename))
			{
				::includes.push_back(filename);

				// load include, preprocess
				vector<char> file,filepp;

				string location = searchFile(filename.c_str(),includePath);
				append_file_to_vector(location.c_str(),file);
				preprocess(file,filepp);

				// append preprocessed file
				output.insert(output.end(),filepp.begin(),filepp.end());
			}

			append_string_to_vector("#endinclude",output);
			state = idlCode;
			i++;
		}
		else if(state == filenameIn1 || state == filenameIn2)
		{
			filename += *i++;
		}
		else if(state == lineStart) // check if we're on lineStart
		{
			if(match(i,"#include"))
			{
				i += 8;
				state = filenameFind;
				filename = "";
			}
			else
			{
				if(*i != ' ' && *i != '\t' && *i != '\n') state = idlCode;
				output.push_back(*i++);
			}
		}
		else
		{
		 	if(*i == '\n') state = lineStart;	// newline handling
			output.push_back(*i++);
		}
	}
}

int main(int argc, char **argv)
{
	/*
	 * parse command line options
	 */
	int c;
	bool makeTypeInfo = false;
	while((c = getopt(argc, argv, "I:P:C:te:")) != -1)
	{
		switch(c)
		{
			case 'I': includePath.push_back(optarg);
				break;
			case 'P': packetTypes.push_back(optarg);
				break;
			case 'C': customIncludes.push_back(optarg);
				break;
			case 't': makeTypeInfo = true;
				break;
			case 'e': includedNames.push_back(optarg);
				break;
			default: exit_usage(argv[0]);
				break;
		}
	}

	if((argc-optind) != 1) exit_usage(argv[0]);

	const char *inputfile = argv[optind];

	/*
	 * find out prefix (filename without .idl)
	 */

	char *prefix = strdup(inputfile);

	if(strlen(prefix) < 4 || strcmp(&prefix[strlen(prefix)-4],".idl")) {
		fprintf(stderr,"filename must end in .idl\n");
		exit(1);
	} else {
		prefix[strlen(prefix)-4] = 0;
	}

	/*
	 * strip path (mcopidl always outputs the result into the current directory)
	 */
	char *pathless = strrchr(prefix,'/');
	if(pathless)
		prefix = pathless+1;

	/*
	 * load file
	 */

	idl_line_no = 1;
	idl_in_include = 0;
	idl_filename = inputfile;

	vector<char> contents,contentspp;
	append_file_to_vector(inputfile,contents);

	// trailing zero byte (mcopidlParse wants a C-style string as argument)
	contents.push_back(0);

	// preprocess (throws includes into contents, removes C-style comments)
	preprocess(contents,contentspp);

	// call lex&yacc parser
	mcopidlParse(&contentspp[0]);

	// generate code for C++ header file
	FILE *header = startHeader(prefix);
	doIncludeHeader(header);
	doEnumHeader(header);
	doStructHeader(header);
	doInterfacesHeader(header);
	endHeader(header,prefix);
	moveIfChanged(string(prefix)+".h");

	// generate code for C++ source file
	FILE *source = startSource(prefix);
	doStructSource(source);
	doInterfacesSource(source);
	doInterfaceRepoSource(source,prefix);
	endSource(source);
	moveIfChanged(string(prefix)+".cc");

	// create type file
	if(makeTypeInfo)
	{
		doTypeFile(prefix);
		doTypeIndex(prefix);
		moveIfChanged(string(prefix)+".mcoptype");
		moveIfChanged(string(prefix)+".mcopclass");
	}

	return 0;
}
