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
#include <vector>
#include <list>
#include <stack>
#include <ctype.h>
#include "core.h"
#include <iostream>

using namespace std;

int idl_in_include;
int idl_line_no;
string idl_filename;

/*
 * if we start parsing an include file, we push the name of the file
 * and the line number where we left it on the stack for later usage
 */
stack<pair<int,string> > idl_include_stack;

list<EnumDef *> enums;
list<TypeDef *> structs;
list<InterfaceDef *> interfaces;
list<string> includes;		// files to include
list<string> includePath;	// path for the includes

// names that occur in included files -> no code generation
list<string> includedNames;

ModuleDef module;

void addEnumTodo( EnumDef *edef )
{
	enums.push_back(edef);

	if(idl_in_include)
	{
		includedNames.push_back(edef->name);
	}
	else
	{
		// TODO: memory management? Will be freed twice now?
		module.enums.push_back(edef);
	}
}

void addStructTodo( TypeDef *type )
{
	structs.push_back(type);

	if(idl_in_include)
	{
		includedNames.push_back(type->name);
	}
	else
	{
		// TODO: memory management? Will be freed twice now?
		module.types.push_back(type);
	}
}

void addInterfaceTodo( InterfaceDef *iface )
{
	interfaces.push_back(iface);
	
	if(idl_in_include)
	{
		includedNames.push_back(iface->name);
	}
	else
	{
		// TODO: memory management? Will be freed twice now?
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

bool isStruct( string type )
{
	list<TypeDef *>::iterator i;

	for(i=structs.begin();i != structs.end(); i++)
		if((*i)->name == type) return true;

	return false;
}

bool isEnum( string type )
{
	list<EnumDef *>::iterator i;

	for(i=enums.begin();i != enums.end(); i++)
		if((*i)->name == type) return true;

	return false;
}

bool isInterface( string type )
{
	list<InterfaceDef *>::iterator i;

	for(i=interfaces.begin();i != interfaces.end(); i++)
		if((*i)->name == type) return true;

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
		string indent = "", int wrapperMode = 0)
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
		if(model==MODEL_ASTREAM)	result = "FloatAsyncStream "+name;
		if(model==MODEL_AMSTREAM)	assert(false);
		if(model==MODEL_ASTREAM_PACKETPTR) result = "DataPacket<float> *";
		/*result = "float **"+name;*/

		if(model==MODEL_READ)
			result = name+" = stream.readFloat()";
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
		if(model==MODEL_WRITE)
			result = "stream.writeFloat("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeFloat("+name+")";
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
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return false; // error occured\n";
			result += indent + "bool returnCode = result->readBool();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "bool "+name+" = request->readBool();\n";
		if(model==MODEL_WRITE)
			result = "stream.writeBool("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeBool("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeBool("+name+");\n";
	}
	else if(type == "byte")
	{
		if(model==MODEL_MEMBER)		result = "mcopbyte";
		if(model==MODEL_MEMBER_SEQ) result = "std::vector<mcopbyte>";
		if(model==MODEL_ARG)		result = "mcopbyte";
		if(model==MODEL_ARG_SEQ)	result = "const std::vector<mcopbyte>&";
		if(model==MODEL_RESULT)		result = "mcopbyte";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<mcopbyte> *";
		if(model==MODEL_READ)
			result = name+" = stream.readByte()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readByteSeq("+name+")";
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return 0; // error occured\n";
			result += indent + "mcopbyte returnCode = result->readByte();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "mcopbyte "+name+" = request->readByte();\n";
		if(model==MODEL_WRITE)
			result = "stream.writeByte("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeByteSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeByte("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeByte("+name+");\n";
		if(model==MODEL_ASTREAM)
			result = "ByteAsyncStream "+name;
		if(model==MODEL_ASTREAM_PACKETPTR) result = "DataPacket<mcopbyte> *";
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
		if(model==MODEL_REQ_READ)
			result = indent + "long "+name+" = request->readLong();\n";
		if(model==MODEL_WRITE)
			result = "stream.writeLong("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeLongSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeLong("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeLong("+name+");\n";
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
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return\"\"; // error occured\n";
			result += indent + "std::string returnCode;\n";
			result += indent + "result->readString(returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_WRITE)
			result = "stream.writeString("+name+")";
		if(model==MODEL_WRITE_SEQ)
			result = "stream.writeStringSeq("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeString("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeString("+name+");\n";
	} else if(isStruct(type)) {
		if(model==MODEL_MEMBER)
			result = type;
		if(model==MODEL_MEMBER_SEQ)
			result = "std::vector<"+type+" *>";

		if(model==MODEL_ARG)	
			result = "const "+type+"&";
		if(model==MODEL_ARG_SEQ)
			result = "const std::vector<"+type+" *>&";

		if(model==MODEL_READ)
			result = name+".readType(stream)";
		if(model==MODEL_READ_SEQ)
			result = "readTypeSeq(stream,"+name+")";
		if(model==MODEL_REQ_READ)
			result = indent + type+" "+name+"(*request);\n";

		if(model==MODEL_WRITE)
			result = name+".writeType(stream)";
		if(model==MODEL_REQ_WRITE)
			result = name+".writeType(*request)";
		if(model==MODEL_WRITE_SEQ)
			result = "writeTypeSeq(stream,"+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "writeTypeSeq(*request,"+name+")";

		if(model==MODEL_INVOKE)
			result = indent + type + " *_returnCode = "+name+";\n"
			       + indent + "_returnCode->writeType(*result);\n"
				   + indent + "delete _returnCode;\n";

		if(model==MODEL_RES_READ)
		{
			result = indent +
					"if(!result) return new "+type+"(); // error occured\n";
			result += indent+ type + " *_returnCode = new "+type+"(*result);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}

		if(model==MODEL_RESULT)		result = type+"*";
		if(model==MODEL_RESULT_SEQ)	result = "std::vector<"+type+" *> *";
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
		if(type == "object") {
			type = "Object";
			wrapperMode = 0;
		}

		if(model==MODEL_MEMBER)		result = type+"_var";
		//if(model==MODEL_MEMBER_SEQ) result = "std::vector<"+type+">";
		if(model==MODEL_ARG) switch (wrapperMode) {
			case 1: result = type + ""; break;
			case 2: result = type + "_var"; break;
			default: result = type;
		}
		//if(model==MODEL_ARG_SEQ)	result = "const std::vector<"+type+">&";
		if(model==MODEL_RESULT)		result = type+(wrapperMode?"":"");
		//if(model==MODEL_RESULT_SEQ)	result = "std::vector<"+type+"> *";
		if(model==MODEL_READ)
			result = "readObject(stream,"+name+")";
		//if(model==MODEL_READ_SEQ)
		//	result = "stream.readLongSeq("+name+")";		// TODO
		if(model==MODEL_RES_READ)
		{
			result = indent + "if(!result) return 0; // error occured\n";
			result += indent + type+"_base* returnCode;\n";
			result += indent + "readObject(*result,returnCode);\n";
			result += indent + "delete result;\n";
			result += indent + "return "+type+"(returnCode);\n";
		}
		if(model==MODEL_REQ_READ)
		{
			result = indent + type +"_base* _temp_"+name+";\n";
			result += indent + "readObject(*request,_temp_"+name+");\n";
			result += indent + type+" "+name+" = _temp_"+name+";\n";
		}
		if(model==MODEL_WRITE)
			result = "writeObject(stream,"+name+"._base())";
		if(model==MODEL_REQ_WRITE)
			result = "writeObject(*request,"+name+"._base())";
		if(model==MODEL_INVOKE)
		{
			result = indent + type+" returnCode = "+name+";\n"
			       + indent + "writeObject(*result,returnCode._base());\n";
		}
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
	string header_name = prefix+".h";
	FILE *header = fopen(header_name.c_str(),"w");

	fprintf(header,generated_disclaimer);
	fprintf(header,"#ifndef %s\n",mkdef(prefix).c_str());
	fprintf(header,"#define %s\n\n",mkdef(prefix).c_str());
	fprintf(header,"#include \"common.h\"\n\n");

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
	string source_name = prefix+".cc";

	FILE *source = fopen(source_name.c_str(),"w");
	fprintf(source,generated_disclaimer);
	fprintf(source,"#include \"%s\"\n\n",header_name.c_str());

	return source;
}

void endSource(FILE *source)
{
	fclose(source);
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
	list<EnumDef *>::iterator edi;
	vector<EnumComponent *>::iterator i;

	for(edi = enums.begin();edi != enums.end(); edi++)
	{
		EnumDef *ed = *edi;

		if(fromInclude(ed->name)) continue; // should come from the include

		fprintf(header,"enum %s {",ed->name.c_str());
		int first = 0;
		for(i=ed->contents.begin();i != ed->contents.end();i++)
		{
			if(first != 0) fprintf(header,", ");
			first++;
			fprintf(header,"%s = %ld",(*i)->name.c_str(),(*i)->value);
		}
		fprintf(header,"};\n");
	}
}

void doStructHeader(FILE *header)
{
	list<TypeDef *>::iterator csi;
	vector<TypeComponent *>::iterator i;

	for(csi = structs.begin();csi != structs.end(); csi++)
	{
		TypeDef *d = *csi;

		if(fromInclude(d->name)) continue; // should come from the include

		fprintf(header,"class %s : public Type {\n",d->name.c_str());
		fprintf(header,"public:\n");

		/** constructor without arguments **/
		fprintf(header,"\t%s();\n",d->name.c_str());

		/** constructor with arguments **/
		fprintf(header,"\t%s(",d->name.c_str());
		int first = 0;
		for(i=d->contents.begin();i != d->contents.end();i++)
		{
			string name = createTypeCode((*i)->type,(*i)->name,MODEL_ARG);
			if(first != 0) fprintf(header,", ");
			first++;
			fprintf(header,"%s",name.c_str());
		}
		fprintf(header,");\n");

		/** constructor from stream **/
		fprintf(header,"\t%s(Buffer& stream);\n",d->name.c_str());

		/** copy constructor (from same type) **/
		fprintf(header,"\t%s(const %s& copyType);\n",
			d->name.c_str(),d->name.c_str());

		/** assignment operator **/
		fprintf(header,"\t%s& operator=(const %s& assignType);\n",
			d->name.c_str(),d->name.c_str());

		/** virtual destuctor (removes many annoying compiler warnings) **/
		fprintf(header,"\tvirtual ~%s();\n\n",d->name.c_str());

		/** data members **/
		for(i=d->contents.begin();i != d->contents.end();i++)
		{
			string name = createTypeCode((*i)->type,(*i)->name,MODEL_MEMBER);
			fprintf(header,"\t%s;\n",name.c_str());
		}

		fprintf(header,"\n// marshalling functions\n");

		/** marshalling function for reading from stream **/
		fprintf(header,"\tvoid readType(Buffer& stream);\n");

		/** marshalling function for writing to stream **/
		fprintf(header,"\tvoid writeType(Buffer& stream) const;\n");
		fprintf(header,"};\n\n");
	}
}

void doStructSource(FILE *source)
{
	list<TypeDef *>::iterator csi;
	vector<TypeComponent *>::iterator i;

	fprintf(source,"// Implementation\n");
	for(csi = structs.begin();csi != structs.end(); csi++)
	{
		TypeDef *d = *csi;

		if(fromInclude(d->name)) continue; // should come from the include

		fprintf(source,"%s::%s()\n{\n}\n\n",d->name.c_str(),d->name.c_str());

		fprintf(source,"%s::%s(",d->name.c_str(),d->name.c_str());
		int first = 0;
		for(i=d->contents.begin();i != d->contents.end();i++)
		{
			string name = createTypeCode((*i)->type,(*i)->name,MODEL_ARG);
			if(first != 0) fprintf(source,", ");
			first++;
			fprintf(source,"%s",name.c_str());
		}
		fprintf(source,")\n{\n");
		for(i=d->contents.begin();i != d->contents.end();i++)
			fprintf(source,"\tthis->%s = %s;\n",(*i)->name.c_str(),(*i)->name.c_str());
		fprintf(source,"}\n\n");

		/** constructor from stream **/
		fprintf(source,"%s::%s(Buffer& stream)\n{\n",d->name.c_str(),d->name.c_str());
		fprintf(source,"\treadType(stream);\n");
		fprintf(source,"}\n\n");

		/** copy constructor **/

		fprintf(source,"%s::%s(const %s& copyType) : ::Type(copyType)\n{\n",
			d->name.c_str(),d->name.c_str(),d->name.c_str());
		fprintf(source,"\tBuffer buffer;\n");
		fprintf(source,"\tcopyType.writeType(buffer);\n");
		fprintf(source,"\treadType(buffer);\n");
		fprintf(source,"}\n\n");

		/** assignment operator **/
		fprintf(source,"%s& %s::operator=(const %s& assignType)\n{\n",
			d->name.c_str(),d->name.c_str(),d->name.c_str());
		fprintf(source,"\tBuffer buffer;\n");
		fprintf(source,"\tassignType.writeType(buffer);\n");
		fprintf(source,"\treadType(buffer);\n");
		fprintf(source,"\treturn *this;\n");
		fprintf(source,"}\n\n");
	
		/** virtual destuctor: free type contents **/
		fprintf(source,"%s::~%s()\n{\n",d->name.c_str(),d->name.c_str());
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
	
		/** marshalling function for reading from stream **/
		fprintf(source,"void %s::readType(Buffer& stream)\n{\n",d->name.c_str());
		for(i=d->contents.begin();i != d->contents.end();i++)
		{
			string code = createTypeCode((*i)->type,(*i)->name,MODEL_READ);
			fprintf(source,"\t%s;\n",code.c_str());
		}
		fprintf(source,"}\n\n");

		/** marshalling function for writing to stream **/
		fprintf(source,"void %s::writeType(Buffer& stream) const\n{\n",d->name.c_str());
		for(i=d->contents.begin();i != d->contents.end();i++)
		{
			string code = createTypeCode((*i)->type,(*i)->name,MODEL_WRITE);
			fprintf(source,"\t%s;\n",code.c_str());
		}
		fprintf(source,"}\n\n");
	}
}

string createReturnCode(MethodDef *md, int wrapperMode = 0)
{
	return createTypeCode(md->type,"",MODEL_RESULT,"",wrapperMode);
}

string createParamList(MethodDef *md, int wrapperMode = 0)
{
	string result;
	int first = 0;
	vector<ParamDef *>::iterator pi;

	for(pi = md->signature.begin(); pi != md->signature.end(); pi++)
	{
		ParamDef *pd = *pi;
		string p = createTypeCode(pd->type,pd->name,MODEL_ARG,"",wrapperMode);

		if(first != 0) result += ", ";
		first++;
		result += p;
	}
	return result;
}

string createCallParamList(MethodDef *md)
{
	string result;
	bool first = true;
	vector<ParamDef *>::iterator pi;

	for(pi = md->signature.begin(); pi != md->signature.end(); pi++)
	{
		if (!first) result += ", ";
		first = false;
		result += (*pi)->name;
	}
	return result;
}

void createStubCode(FILE *source, string iface, string method, MethodDef *md)
{
	string rc = createReturnCode(md);
	string params = createParamList(md);
	vector<ParamDef *>::iterator pi;

	Buffer b;
	md->writeType(b);

	fprintf(source,"%s %s_stub::%s(%s)\n",rc.c_str(),iface.c_str(),
				method.c_str(), params.c_str());
	fprintf(source,"{\n");
	fprintf(source,"\tlong methodID = _lookupMethodFast(\"%s\");\n",
											b.toString("method").c_str());
	if(md->flags & methodTwoway)
	{
		fprintf(source,"\tlong requestID;\n");
		fprintf(source,"\tBuffer *request, *result;\n");
		fprintf(source,"\trequest = Dispatcher::the()->"
				"createRequest(requestID,_objectID,methodID);\n");
	}
	else
	{
		fprintf(source,"\tBuffer *request = Dispatcher::the()->"
				"createOnewayRequest(_objectID,methodID);\n");
	}

	for(pi = md->signature.begin(); pi != md->signature.end(); pi++)
	{
		ParamDef *pd = *pi;
		string p;
		p = createTypeCode(pd->type,pd->name,MODEL_REQ_WRITE);
		fprintf(source,"\t%s;\n",p.c_str());
	}
	fprintf(source,"\trequest->patchLength();\n");
	fprintf(source,"\t_connection->qSendBuffer(request);\n\n");

	if(md->flags & methodTwoway)
	{
		fprintf(source,"\tresult = "
			"Dispatcher::the()->waitForResult(requestID,_connection);\n");

		fprintf(source,"%s",
			createTypeCode(md->type,"",MODEL_RES_READ,"\t").c_str());
	}
	fprintf(source,"}\n\n");
}

bool haveStreams(InterfaceDef *d)
{
	vector<AttributeDef *>::iterator ai;

	for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		if((*ai)->flags & attributeStream) return true;

	return false;
}

bool haveAsyncStreams(InterfaceDef *d)
{
	vector<AttributeDef *>::iterator ai;

	for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		if(((*ai)->flags & attributeStream) && ((*ai)->flags & streamAsync))
			return true;

	return false;
}

void createDispatchFunction(FILE *source, long mcount,
								InterfaceDef *d, MethodDef *md,string name)
{
	/** calculate signature (prevents unused argument warnings) **/
	string signature = "void *object, ";

	if(md->signature.size() == 0)
		signature += "Buffer *";
	else
		signature += "Buffer *request";

	if(md->flags & methodTwoway)
	{
		if(md->type == "void")
			signature += ", Buffer *";
		else
			signature += ", Buffer *result";
	}
	else
	{
		if(md->type != "void")
		{
			cerr << "method " << md->name << " in interface " << d->name <<
			   " is declared oneway, but not void" << endl;
			exit(1);
		}
	}

	fprintf(source,"// %s\n",md->name.c_str());
	fprintf(source,"static void _dispatch_%s_%02ld(%s)\n",
			d->name.c_str(),mcount,signature.c_str());
	fprintf(source,"{\n");

	string call = "(("+d->name+"_skel *)object)->"+name + "(";
	int first = 1;
	vector<ParamDef *>::iterator pi;
	for(pi = md->signature.begin(); pi != md->signature.end(); pi++)
	{
		ParamDef *pd = *pi;
		string p;

		if(!first) call += ",";
		first = 0;
		call += pd->name;
		p = createTypeCode(pd->type,pd->name,MODEL_REQ_READ, "\t");
		fprintf(source,"%s",p.c_str());
	}
	call += ")";
	string invoke = createTypeCode(md->type,call,MODEL_INVOKE,"\t");
	fprintf(source,"%s",invoke.c_str());
	fprintf(source,"}\n\n");
}

// generate a list of all parents. There can be repetitions
vector<std::string> allParents(InterfaceDef& iface)
{
	vector<std::string> ret;
	list<InterfaceDef *>::iterator interIt;
	vector<std::string>::iterator si;
	// For all inherited interfaces
	for (si = iface.inheritedInterfaces.begin(); si != iface.inheritedInterfaces.end(); si++)
	{
		ret.push_back(*si);
		// Find the corresponding interface definition
		for (interIt=interfaces.begin(); interIt!=interfaces.end(); interIt++) {
			InterfaceDef *parent = *interIt;
			if (parent->name == (*si)) {
				// Now add this parent's parents
				vector<std::string> ppar = allParents(*parent);
				ret.insert(ret.end(), ppar.begin(), ppar.end());
				break;
			}
		}
	}
	return ret;
}

// generate a list of all parents - without repetitions
vector<string> allParentsUnique(InterfaceDef& iface)
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

InterfaceDef mergeAllParents(InterfaceDef& iface)
{
	InterfaceDef result = iface;

	vector<string> parents = allParentsUnique(iface);
	vector<string>::iterator pi;
	
	for(pi = parents.begin(); pi != parents.end(); pi++)
	{
		string parent = *pi;

		list<InterfaceDef *>::iterator i;
		for(i=interfaces.begin();i != interfaces.end(); i++)
		{
			InterfaceDef *d = *i;
			if(d->name == parent)
			{
				/* merge attributes */
				vector<AttributeDef *>::iterator ai;

				for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
					result.attributes.push_back(new AttributeDef(**ai));

				/* merge methods */
				vector<MethodDef *>::iterator mi;

				for(mi = d->methods.begin(); mi != d->methods.end(); mi++)
					result.methods.push_back(new MethodDef(**mi));
			}
		}
	}
	return result;
}

void doInterfacesHeader(FILE *header)
{
	list<InterfaceDef *>::iterator ii;
	vector<MethodDef *>::iterator mi;
	vector<AttributeDef *>::iterator ai;
	string inherits;

	/*
	 * this allows it to the various interfaces as parameters, returncodes
	 * and attributes even before their declaration
	 */
	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef *d = *ii;
		if(!fromInclude(d->name))
			fprintf(header,"class %s;\n",d->name.c_str());
	}
	fprintf(header,"\n");

	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef *d = *ii;

		if(fromInclude(d->name)) continue; // should come from the include

		// create abstract interface
		inherits = buildInheritanceList(*d,"_base");
		if(inherits == "") inherits = "virtual public Object_base";

		fprintf(header,"class %s_base : %s {\n",d->name.c_str(),inherits.c_str());
		fprintf(header,"public:\n");
		fprintf(header,"\tstatic unsigned long _IID; // interface ID\n\n");
		fprintf(header,"\tstatic %s_base *_create(const std::string& subClass"
						" = \"%s\");\n", d->name.c_str(),d->name.c_str());
		fprintf(header,"\tstatic %s_base *_fromString(std::string objectref);\n",
														d->name.c_str());
		fprintf(header,"\tstatic %s_base *_fromReference(ObjectReference ref,"
		                                " bool needcopy);\n\n",d->name.c_str());

		/* reference counting: _copy */
		fprintf(header,"\tinline %s_base *_copy() {\n"
					   "\t\tassert(_refCnt > 0);\n"
				       "\t\t_refCnt++;\n"
					   "\t\treturn this;\n"
					   "\t}\n\n",d->name.c_str());
				
		// Default I/O info
		fprintf(header,"\tvirtual vector<std::string> _defaultPortsIn() const;\n");
		fprintf(header,"\tvirtual vector<std::string> _defaultPortsOut() const;\n");
		fprintf(header,"\n");

		// Casting
		fprintf(header,"\tvoid *_cast(unsigned long iid);\n\n");

		/* attributes (not for streams) */
		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;
			string rc = createTypeCode(ad->type,"",MODEL_RESULT);
			string pc = createTypeCode(ad->type,"newValue",MODEL_ARG);

			if(ad->flags & attributeAttribute)
			{
				if(ad->flags & streamOut)  /* readable from outside */
				{
					fprintf(header,"\tvirtual %s %s() = 0;\n",rc.c_str(),
						ad->name.c_str());
				}
				if(ad->flags & streamIn)  /* writeable from outside */
				{
					fprintf(header,"\tvirtual void %s(%s) = 0;\n",
						ad->name.c_str(), pc.c_str());
				}
			}
		}
		
		/* methods */
		for(mi = d->methods.begin(); mi != d->methods.end(); mi++)
		{
			MethodDef *md = *mi;
			string rc = createReturnCode(md);
			string params = createParamList(md);

			fprintf(header,"\tvirtual %s %s(%s) = 0;\n",rc.c_str(),
				md->name.c_str(), params.c_str());
		}		
		fprintf(header,"};\n\n");

		// create stub

		inherits = buildInheritanceList(*d,"_stub");
		if(inherits == "") inherits = "virtual public Object_stub";

		fprintf(header,"class %s_stub : virtual public %s_base, %s {\n",
			d->name.c_str(), d->name.c_str(),inherits.c_str());
		fprintf(header,"protected:\n");
		fprintf(header,"\t%s_stub();\n\n",d->name.c_str());

		fprintf(header,"public:\n");
		fprintf(header,"\t%s_stub(Connection *connection, long objectID);\n\n",
			d->name.c_str());
			/* attributes (not for streams) */
		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;
			string rc = createTypeCode(ad->type,"",MODEL_RESULT);
			string pc = createTypeCode(ad->type,"newValue",MODEL_ARG);

			if(ad->flags & attributeAttribute)
			{
				if(ad->flags & streamOut)  /* readable from outside */
				{
					fprintf(header,"\t%s %s();\n",rc.c_str(),
						ad->name.c_str());
				}
				if(ad->flags & streamIn)  /* writeable from outside */
				{
					fprintf(header,"\tvoid %s(%s);\n",
						ad->name.c_str(), pc.c_str());
				}
			}
		}
			/* methods */
		for(mi = d->methods.begin(); mi != d->methods.end(); mi++)
		{
			MethodDef *md = *mi;
			string rc = createReturnCode(md);
			string params = createParamList(md);

			fprintf(header,"\t%s %s(%s);\n",rc.c_str(),
				md->name.c_str(), params.c_str());
		}
		fprintf(header,"};\n\n");

		// create skeleton

		inherits = buildInheritanceList(*d,"_skel");
		if(inherits == "") inherits = "virtual public Object_skel";

		fprintf(header,"class %s_skel : virtual public %s_base,"
			" %s {\n",d->name.c_str(),d->name.c_str(),inherits.c_str());

		bool firstStream = true;
		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;

			if(ad->flags & attributeStream)
			{
				if(firstStream)
				{
					fprintf(header,"protected:\n");
					fprintf(header,"\t// variables for streams\n");
					firstStream = false;
				}

				/** generate declaration of the variable: multi stream? **/
				string decl;

				if(ad->flags & streamMulti)
				{
					if(ad->flags & streamAsync)
						decl = createTypeCode(ad->type,ad->name,MODEL_AMSTREAM);
					else
						decl = createTypeCode(ad->type,ad->name,MODEL_MSTREAM);
				}
				else
				{
					if(ad->flags & streamAsync)
						decl = createTypeCode(ad->type,ad->name,MODEL_ASTREAM);
					else
						decl = createTypeCode(ad->type,ad->name,MODEL_STREAM);
				}

				decl += ";";

				/** write to source **/
				string comment;

				if(ad->flags & streamIn) comment = "incoming stream";
				if(ad->flags & streamOut) comment = "outgoing stream";

				fprintf(header,"\t%-40s  // %s\n",decl.c_str(),comment.c_str());
			}
		}
		if(!firstStream) fprintf(header,"\n");

		bool haveAsyncStreams = false;

		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;

			if((ad->flags & attributeStream) && (ad->flags & streamAsync))
			{
				if(!haveAsyncStreams)
				{
					fprintf(header,"\t// handler for asynchronous streams\n");
					haveAsyncStreams = true;
				}

				string ptype =
					createTypeCode(ad->type,"",MODEL_ASTREAM_PACKETPTR);

				if(ad->flags & streamIn)
				{
					fprintf(header,"\tvirtual void process_%s(%s) = 0;\n",
										ad->name.c_str(),ptype.c_str());
				}
				else
				{
					fprintf(header,"\tvirtual void request_%s(%s);\n",
										ad->name.c_str(),ptype.c_str());
				}
			}
		}
		if(haveAsyncStreams) fprintf(header,"\n");

		fprintf(header,"public:\n");
		fprintf(header,"\t%s_skel();\n\n",d->name.c_str());

		fprintf(header,"\tstatic std::string _interfaceNameSkel();\n");
		fprintf(header,"\tstd::string _interfaceName();\n");
		fprintf(header,"\tvoid _buildMethodTable();\n");
		fprintf(header,"\tvoid dispatch(Buffer *request, Buffer *result,"
						"long methodID);\n");

		if(haveAsyncStreams)
		  fprintf(header,"\tvoid notify(const Notification& notification);\n");

		fprintf(header,"};\n\n");
		
		
		// Create object wrapper for easy C++ syntax
		
		fprintf(header,"#include \"reference.h\"\n");
		
		// Allow connect facility only if there is something to connect to!
		if (haveStreams(d)) {
			fprintf(header,"#include \"flowsystem.h\"\n");
		}
		fprintf(header,"\n");
	
		/*
		inherits = buildInheritanceList(*d,"");
		bool hasParent = (inherits != "");

		if (hasParent) inherits = ": " + inherits;
		else inherits = ": virtual public SmartWrapper";
		*/
		inherits = ": public SmartWrapper";
		bool hasParent = false;

		fprintf(header,"class %s %s {\n",d->name.c_str(),inherits.c_str());
		fprintf(header,"private:\n");
		fprintf(header,"\tstatic Object_base* _Creator();\n");
		fprintf(header,"\t%s_base *_cache;\n",d->name.c_str());
		fprintf(header,"\tinline %s_base *_method_call() {\n",d->name.c_str());
		fprintf(header,"\t\t_pool->checkcreate();\n");
		fprintf(header,"\t\tif(_pool->base) {\n");
		fprintf(header,"\t\t\t_cache="
							"(%s_base *)_pool->base->_cast(%s_base::_IID);\n",
							d->name.c_str(),d->name.c_str());
		fprintf(header,"\t\t\tassert(_cache);\n");
		fprintf(header,"\t\t}\n");
		fprintf(header,"\t\treturn _cache;\n");
		fprintf(header,"\t}\n");

		fprintf(header,"\npublic:\n");

		// empty constructor: specify creator for create-on-demand
		fprintf(header,"\tinline %s() : SmartWrapper(_Creator), _cache(0) {}\n",d->name.c_str());
		
		// constructors from reference and for subclass
		fprintf(header,"\tinline %s(const SubClass& s) :\n"
			"\t\tSmartWrapper(%s_base::_create(s.string())), _cache(0) {}\n",
			d->name.c_str(),d->name.c_str());
		fprintf(header,"\tinline %s(const Reference &r) :\n"
			"\t\tSmartWrapper("
			"r.isString()?(%s_base::_fromString(r.string())):"
			"(%s_base::_fromReference(r.reference(),true))), _cache(0) {}\n",
			d->name.c_str(),d->name.c_str(), d->name.c_str());

		// copy constructors
		fprintf(header,"\tinline %s(%s_base* b) : SmartWrapper(b), _cache(0) {}\n",
			d->name.c_str(),d->name.c_str());
		fprintf(header,"\tinline %s(const %s& target) : SmartWrapper(target._pool), _cache(target._cache) {}\n",
			d->name.c_str(),d->name.c_str());
		fprintf(header,"\tinline %s(SmartWrapper::Pool& p) : SmartWrapper(p), _cache(0) {}\n",
			d->name.c_str());

		// copy operator. copy from _base* extraneous (uses implicit const object)
		fprintf(header,"\tinline %s& operator=(const %s& target) {\n",
			d->name.c_str(),d->name.c_str());
		// test for equality
		fprintf(header,"\t\tif (_pool == target._pool) return *this;\n");
		// Invalidate all parent caches
		vector<std::string> parents = allParentsUnique(*d); // can repeat values
		vector<std::string> done; // don't repeat values
		// Loop through all the values
		/*
		for (vector<std::string>::iterator si = parents.begin(); si != parents.end(); si++)
		{
				// repeated value? (virtual public like merging...)
				bool skipIt = false;
				for (vector<std::string>::iterator di = done.begin(); di != done.end(); di++) {
						if ((*di)==(*si)) {skipIt = true; break;}
				}
				if (skipIt) continue;
				fprintf(header,"\t\t%s::cacheOK=false;\n",(*si).c_str());
				done.push_back(*si);
		}
		*/
		fprintf(header,"\t\t_pool->Dec();\n");
		fprintf(header,"\t\t_pool = target._pool;\n");
		fprintf(header,"\t\t_cache = target._cache;\n");
		fprintf(header,"\t\t_pool->Inc();\n");
		fprintf(header,"\t\treturn *this;\n");
		fprintf(header,"\t}\n");

		for (vector<std::string>::iterator si = parents.begin();
												si != parents.end(); si++)
		{
			string &s = *si;
			fprintf(header,"\tinline operator %s() const { return %s(*_pool); }\n",
									s.c_str(), s.c_str());
		}
		fprintf(header,"\tinline operator Object() const { return Object(*_pool); }\n");
		//if(parents.empty()) /* no parents -> need to free pool self */
		//{
		//	fprintf(header,"\tinline ~%s() {\n",d->name.c_str());
		//	fprintf(header,"\t\t_pool->Dec();\n");
		//	fprintf(header,"\t}\n");
		//}
		// conversion to string
//		fprintf(header,"\tinline std::string toString() const {return _method_call()->_toString();}\n");
		// conversion to _base* object
		fprintf(header,"\tinline %s_base* _base() {return _cache?_cache:_method_call();}\n",d->name.c_str());
		fprintf(header,"\n");

		InterfaceDef allMerged = mergeAllParents(*d);
		d = &allMerged;

		/* attributes */
		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;
			string rc = createTypeCode(ad->type,"",MODEL_RESULT);
			string pc = createTypeCode(ad->type,"newValue",MODEL_ARG);

			if(ad->flags & attributeAttribute)
			{
				if(ad->flags & streamOut)  /* readable from outside */
				{
					fprintf(header,"\tinline %s %s() {return _cache?_cache->%s():_method_call()->%s();}\n",
						rc.c_str(), ad->name.c_str(), ad->name.c_str(), ad->name.c_str());
				}
				if(ad->flags & streamIn)  /* writeable from outside */
				{
					fprintf(header,"\tinline void %s(%s) {_cache?_cache->%s(newValue):_method_call()->%s(newValue);}\n",
						ad->name.c_str(), pc.c_str(), ad->name.c_str(), ad->name.c_str());
				}
			}
		}

		/* methods */
		for(mi = d->methods.begin(); mi != d->methods.end(); mi++)
		{
			MethodDef *md = *mi;
			string rc = createReturnCode(md, 1);
			string params = createParamList(md, 1);
			string callparams = createCallParamList(md);

			// map constructor methods to the real things
			if (md->name == "constructor") {
				fprintf(header,"\tinline %s(%s) : "
					"SmartWrapper(%s_base::_create()) {\n"
					"\t\t_cache=(%s_base *)_pool->base->_cast(%s_base::_IID);\n"
					"\t\tassert(_cache);\n"
					"\t\t_cache->constructor(%s);\n\t}\n",
					d->name.c_str(), params.c_str(), d->name.c_str(),
					d->name.c_str(), d->name.c_str(), callparams.c_str());
			} else {
				fprintf(header,"\tinline %s %s(%s) {return _cache?_cache->%s(%s):_method_call()->%s(%s);}\n",
					rc.c_str(),	md->name.c_str(), params.c_str(),
					md->name.c_str(), callparams.c_str(), md->name.c_str(), callparams.c_str());
			}
		}
		fprintf(header,"};\n\n");
	}
}

enum DefaultDirection {defaultIn, defaultOut};
bool addParentDefaults(InterfaceDef& iface, vector<std::string>& ports, DefaultDirection dir);
bool lookupParentPort(InterfaceDef& iface, string port, vector<std::string>& ports, DefaultDirection dir);

bool addDefaults(InterfaceDef& iface, vector<std::string>& ports, DefaultDirection dir)
{
	vector<AttributeDef *>::iterator ai;
	vector<std::string>::iterator di;
	bool hasDefault = false;
	// Go through the default ports of this interface
	for (di = iface.defaultPorts.begin(); di != iface.defaultPorts.end(); di++) {
		bool foundIn = false, foundOut = false;
		// Find the corresponding attribute definition
		for (ai = iface.attributes.begin(); ai != iface.attributes.end(); ai++) {
			if (((*ai)->flags & attributeStream) && ((*di)==(*ai)->name)) {
				// Add this port to the list
				if ((*ai)->flags & streamIn) {
					foundIn=true;
					if (dir==defaultIn) ports.push_back(*di);
				}
				// Add this port to the list
				if ((*ai)->flags & streamOut) {
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
		vector<AttributeDef *>::iterator foundPos;
		int found = 0;
		for (ai = iface.attributes.begin(); ai != iface.attributes.end(); ai++) {
			if ((*ai)->flags & attributeStream) {
				if (((*ai)->flags & streamIn) && (dir == defaultIn)) {
					found++; foundPos=ai;
				}
				if (((*ai)->flags & streamOut) && (dir == defaultOut)) {
					found++; foundPos=ai;
				}
			}
		}
		if (found == 1) {hasDefault=true; ports.push_back((*foundPos)->name);}
	}
	return hasDefault;
}


bool addParentDefaults(InterfaceDef& iface, vector<std::string>& ports, DefaultDirection dir)
{
	list<InterfaceDef *>::iterator interIt;
	vector<std::string>::iterator si;
	bool hasDefault = false;
	// For all inherited interfaces
	for (si = iface.inheritedInterfaces.begin(); si != iface.inheritedInterfaces.end(); si++)
	{
		// Find the corresponding interface definition
		for (interIt=interfaces.begin(); interIt!=interfaces.end(); interIt++) {
			InterfaceDef *parent = *interIt;
			if (parent->name == (*si)) {
				// Now add the default ports of this parent
				bool b = addDefaults(*parent, ports, dir);
				if (b) hasDefault = true;
				break;
			}
		}
	}
	return hasDefault;
}

bool lookupParentPort(InterfaceDef& iface, string port, vector<std::string>& ports, DefaultDirection dir)
{
	list<InterfaceDef *>::iterator interIt;
	vector<AttributeDef *>::iterator ai;
	vector<std::string>::iterator si, di;
	// For all inherited interfaces
	for (si = iface.inheritedInterfaces.begin(); si != iface.inheritedInterfaces.end(); si++)
	{
		// Find the corresponding interface definition
		for (interIt=interfaces.begin(); interIt!=interfaces.end(); interIt++) {
			InterfaceDef *parent = *interIt;
			if (parent->name == (*si)) {
				// Now look at the ports of this parent
				vector<AttributeDef *>::iterator foundPos;
				bool found = false;
				for (ai = parent->attributes.begin(); ai != parent->attributes.end(); ai++) {
					if (((*ai)->flags & attributeStream) && ((*ai)->name==port)){
						if ((((*ai)->flags & streamIn) && (dir == defaultIn))
						|| (((*ai)->flags & streamOut) && (dir == defaultOut))) {
							found = true; foundPos=ai; break;
						}
					}
				}
				if (found) {ports.push_back(port); return true;}
				// Not found, look recursively at the parent ancestors
				bool b = lookupParentPort(*parent, port, ports, dir);
				if (b) return true; // done
				break;
			}
		}
	}
	return false;
}

void doInterfacesSource(FILE *source)
{
	list<InterfaceDef *>::iterator ii;
	vector<MethodDef *>::iterator mi;
	vector<AttributeDef *>::iterator ai;

	long mcount;

	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef *d = *ii;

		if(fromInclude(d->name)) continue; // should come from the include

		// create static functions

		fprintf(source,"%s_base *%s_base::_create(const std::string& subClass)\n",
											d->name.c_str(),d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tObject_skel *skel = "
							"ObjectManager::the()->create(subClass);\n");
		fprintf(source,"\tassert(skel);\n");
		fprintf(source,"\t%s_base *castedObject = "
							"(%s_base *)skel->_cast(%s_base::_IID);\n",
							d->name.c_str(),d->name.c_str(),d->name.c_str());
		fprintf(source,"\tassert(castedObject);\n");
		fprintf(source,"\treturn castedObject;\n");
		fprintf(source,"}\n\n");

	
		fprintf(source,"%s_base *%s_base::_fromString(std::string objectref)\n",
											d->name.c_str(),d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tObjectReference r;\n\n");
		fprintf(source,"\tif(Dispatcher::the()->stringToObjectReference(r,objectref))\n");
		fprintf(source,"\t\treturn %s_base::_fromReference(r,true);\n",
															d->name.c_str());
		fprintf(source,"\treturn 0;\n");
		fprintf(source,"}\n\n");

		fprintf(source,"%s_base *%s_base::_fromReference(ObjectReference r,"
		               " bool needcopy)\n",d->name.c_str(),d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\t%s_base *result;\n",d->name.c_str());
		fprintf(source,
		"\tresult = (%s_base *)Dispatcher::the()->connectObjectLocal(r,\"%s\");\n",
										d->name.c_str(),d->name.c_str());
		fprintf(source,"\tif(!result)\n");
		fprintf(source,"\t{\n");
		fprintf(source,"\t\tConnection *conn = "
							"Dispatcher::the()->connectObjectRemote(r);\n");
		fprintf(source,"\t\tif(conn)\n");
		fprintf(source,"\t\t{\n");
		fprintf(source,"\t\t\tresult = new %s_stub(conn,r.objectID);\n",
										d->name.c_str());
		fprintf(source,"\t\t\tif(needcopy) result->_copyRemote();\n");
		fprintf(source,"\t\t\tresult->_useRemote();\n");
		fprintf(source,"\t\t}\n");
		fprintf(source,"\t}\n");
		fprintf(source,"\treturn result;\n");
		fprintf(source,"}\n\n");
		
		
		// Default I/O info
		vector<std::string> portsIn, portsOut;
		vector<std::string>::iterator si, di;
		addDefaults(*d, portsIn, defaultIn);
		addDefaults(*d, portsOut, defaultOut);
		
		vector<std::string> done; // don't repeat values
		fprintf(source,"vector<std::string> %s_base::_defaultPortsIn() const {\n",d->name.c_str());
		fprintf(source,"\tvector<std::string> ret;\n");
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
		fprintf(source,"vector<std::string> %s_base::_defaultPortsOut() const {\n",d->name.c_str());
		fprintf(source,"\tvector<std::string> ret;\n");
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
		vector<std::string> parentCast = allParentsUnique(*d);

		fprintf(source,"void *%s_base::_cast(unsigned long iid)\n",
			d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tif(iid == %s_base::_IID) return (%s_base *)this;\n",
			d->name.c_str(),d->name.c_str());

		vector<std::string>::iterator pci;
		for(pci = parentCast.begin(); pci != parentCast.end();pci++)
		{
			string& pc = *pci;
			fprintf(source,"\tif(iid == %s_base::_IID) "
							"return (%s_base *)this;\n",pc.c_str(),pc.c_str());
		}
		fprintf(source,"\tif(iid == Object_base::_IID) return (Object_base *)this;\n");
		fprintf(source,"\treturn 0;\n");
		fprintf(source,"}\n\n");

		// create stub

		/** constructors **/
		fprintf(source,"%s_stub::%s_stub()\n" ,d->name.c_str(),d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\t// constructor for subclasses"
										" (don't use directly)\n");
		fprintf(source,"}\n\n");

		fprintf(source,"%s_stub::%s_stub(Connection *connection, "
						"long objectID)\n",d->name.c_str(),d->name.c_str());
		fprintf(source,"	: Object_stub(connection, objectID)\n");
		fprintf(source,"{\n");
		fprintf(source,"\t// constructor to create a stub for an object\n");
		fprintf(source,"}\n\n");

		/** stub operations **/

			/** stub operations for object methods **/
		for(mi = d->methods.begin(); mi != d->methods.end(); mi++)
		{
			MethodDef *md = *mi;
			createStubCode(source,d->name.c_str(),md->name.c_str(),md);
		}

			/** stub operations for attributes **/
		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;

			if(ad->flags & attributeAttribute)
			{
				MethodDef md;
				if(ad->flags & streamOut)  /* readable from outside */
				{
					md.name = "_get_"+ad->name;
					md.type = ad->type;
					md.flags = methodTwoway;
					/* no parameters (don't set md.signature) */

					createStubCode(source,d->name.c_str(),ad->name.c_str(),&md);
				}
				if(ad->flags & streamIn)  /* writeable from outside */
				{
					md.name = "_set_"+ad->name;
					md.type = "void";
					md.flags = methodTwoway;
					md.signature.push_back(new ParamDef(ad->type,"newValue"));

					createStubCode(source,d->name.c_str(),ad->name.c_str(),&md);
				}
			}
		}

		// create skeleton

		/** _interfaceName **/
		fprintf(source,"std::string %s_skel::_interfaceName()\n",	
													d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\treturn \"%s\";\n",d->name.c_str());
		fprintf(source,"}\n\n");

		fprintf(source,"std::string %s_skel::_interfaceNameSkel()\n",	
													d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\treturn \"%s\";\n",d->name.c_str());
		fprintf(source,"}\n\n");

		/** dispatch operations **/
		Buffer methodTable;

			/** dispatch operations for object methods **/
		mcount = 0;
		for(mi = d->methods.begin(); mi != d->methods.end(); mi++, mcount++)
		{
			MethodDef *md = *mi;
			md->writeType(methodTable);

			createDispatchFunction(source,mcount,d,md,md->name);
		}

			/** dispatch operations for attributes **/

		for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		{
			AttributeDef *ad = *ai;

			if(ad->flags & attributeAttribute)
			{
				MethodDef md;
				if(ad->flags & streamOut)  /* readable from outside */
				{
					md.name = "_get_"+ad->name;
					md.type = ad->type;
					md.flags = methodTwoway;
					/* no parameters (don't set md.signature) */

					md.writeType(methodTable);
					createDispatchFunction(source,mcount++,d,&md,ad->name);
				}
				if(ad->flags & streamIn)  /* writeable from outside */
				{
					md.name = "_set_"+ad->name;
					md.type = "void";
					md.flags = methodTwoway;
					md.signature.push_back(new ParamDef(ad->type,"newValue"));

					md.writeType(methodTable);
					createDispatchFunction(source,mcount++,d,&md,ad->name);
				}
			}
		}

		/** methodTable **/

		string methodTableString = formatMultiLineString(
			methodTable.toString("MethodTable"),"        ");

		fprintf(source,"void %s_skel::_buildMethodTable()\n",d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tBuffer m;\n");
		fprintf(source,"\tm.fromString(\n");
		fprintf(source,"%s,\n",methodTableString.c_str());
		fprintf(source,"\t\t\"MethodTable\"\n");
		fprintf(source,"\t);\n");

		long i;
		for(i=0;i<mcount;i++)
			fprintf(source,"\t_addMethod(_dispatch_%s_%02ld,this,"
			                    "MethodDef(m));\n", d->name.c_str(),i);

		vector<string>::iterator ii = d->inheritedInterfaces.begin();
		while(ii != d->inheritedInterfaces.end())
		{
			fprintf(source,"\t%s_skel::_buildMethodTable();\n",
				ii->c_str());
			ii++;
		}
		fprintf(source,"}\n\n");

		fprintf(source,"%s_skel::%s_skel()\n", d->name.c_str(),d->name.c_str());
		fprintf(source,"{\n");
		for(ai = d->attributes.begin(); ai != d->attributes.end(); ai++)
		{
			AttributeDef *ad = *ai;
			if((ad->flags & attributeStream) == attributeStream)
			{
				fprintf(source,"\t_initStream(\"%s\",&%s,%d);\n",
							ad->name.c_str(),ad->name.c_str(),ad->flags);
			}
		}
		fprintf(source,"}\n\n");

		/** notification operation **/
		if(haveAsyncStreams(d))
		{
			fprintf(source,"void %s_skel::notify(const Notification "
			               "&notification)\n", d->name.c_str());
			fprintf(source,"{\n");
			for(ai = d->attributes.begin(); ai != d->attributes.end(); ai++)
			{
				AttributeDef *ad = *ai;
				if((ad->flags & (attributeStream|streamAsync))
								== (attributeStream|streamAsync))
				{
					const char *fname=(ad->flags&streamIn)?"process":"request";
					string packettype =
						createTypeCode(ad->type,"",MODEL_ASTREAM_PACKETPTR);

					fprintf(source,"\tif(%s.notifyID() == notification.ID)\n",
							ad->name.c_str());
					fprintf(source,
					   "\t\t%s_%s((%s)notification.data);\n",
						fname,ad->name.c_str(),packettype.c_str());
				}
			}
			fprintf(source,"}\n\n");

			/*
			 * create empty request_ methods for output streams
			 * (not everybody uses requesting)
			 */
			for(ai = d->attributes.begin(); ai != d->attributes.end(); ai++)
			{
				AttributeDef *ad = *ai;
				if((ad->flags & (attributeStream|streamAsync|streamOut))
								== (attributeStream|streamAsync|streamOut))
				{
					string packettype =
						createTypeCode(ad->type,"",MODEL_ASTREAM_PACKETPTR);
					fprintf(source,"void %s_skel::request_%s(%s)\n",
						d->name.c_str(),ad->name.c_str(),packettype.c_str());
					fprintf(source,"{\n");
					fprintf(source,"	assert(false); // this default is for "
					               "modules who don't want requesting\n");
					fprintf(source,"}\n\n");
				}
			}
		}
		
		// Smartwrapper statics
		// _Creator
		fprintf(source,"Object_base* %s::_Creator() {\n",d->name.c_str());
		fprintf(source,"\treturn %s_base::_create();\n",d->name.c_str());
		fprintf(source,"}\n\n");

		// IID
		fprintf(source,"unsigned long %s_base::_IID = "
			"MCOPUtils::makeIID(\"%s\");\n\n",d->name.c_str(),d->name.c_str());
	}
}

void doInterfaceRepoSource(FILE *source, string prefix)
{
	Buffer b;
	module.moduleName = "";
	module.writeType(b);

	string data = formatMultiLineString(b.toString("IDLFile"),"    ");

	fprintf(source,"static IDLFileReg IDLFileReg_%s(\"%s\",\n%s\n);\n",
		prefix.c_str(),prefix.c_str(),data.c_str());
}

void exit_usage(char *name)
{
	fprintf(stderr,"usage: %s <filename>\n",name);
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
	bool performInclude = false;
	string filename;
	enum { lineStart, idlCode, commentC, filenameFind,
	            filenameIn1, filenameIn2 } state = lineStart;

	vector<char>::iterator i = input.begin();

	while(i != input.end())
	{
		int skip = 1;

		if(match(i,"/*")) // check if that is a comment for some reason
		{
			state = commentC;
		}
		else if(state == commentC) // leave comment state?
		{
			if(match(i,"*/")) state = idlCode;
		}
		else if(state == filenameFind)
		{
			if(*i != ' ' && *i != '\t')
			{
				if(*i == '"')
				{
					state = filenameIn1;
				}
				else if(*i == '<')
				{
					state = filenameIn2;
				}
				else
				{
					cout << *i << endl;
					assert(0); // error handling!
				}
			}
		}
		else if((state == filenameIn1 && *i == '"') || (state == filenameIn2 && *i == '>'))
		{
			performInclude = true;
			state = idlCode;
		}
		else if(state == filenameIn1 || state == filenameIn2)
		{
			filename += *i;
		}
		else if(state == lineStart) // check if we're on lineStart
		{
			if(match(i,"#include"))
			{
				skip = 8;
				state = filenameFind;
				filename = "";
			}
			else if(*i != ' ' && *i != '\t') state = idlCode;
		}
		if(*i == '\n' && state != commentC) state = lineStart; // newline handling
		while(skip--) {
			output.push_back(*i);
			i++;
		}
		if((state == lineStart) && performInclude)
		{
			if(!haveIncluded(filename))
			{
				::includes.push_back(filename);

				string location = searchFile(filename.c_str(),includePath);
				append_file_to_vector(location.c_str(),output);
			}
			append_string_to_vector("#endinclude\n",output);
			performInclude = false;
		}
	}
}

int main(int argc, char **argv)
{
	/*
	 * parse command line options
	 */
	int c;
	while((c = getopt(argc, argv, "I:")) != -1)
	{
		switch(c)
		{
			case 'I': includePath.push_back(optarg);
				break;
			default: fprintf(stderr,"unknown option\n");
					 exit_usage(argv[0]);
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
		fprintf(stderr,"filename must end in .idl");
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

	vector<char> contents;
	append_file_to_vector(inputfile,contents);

	// trailing zero byte (mcopidlParse wants a C-style string as argument)
	contents.push_back(0);

	// preprocess (throws includes into contents)
	vector<char> contentspp;
	preprocess(contents,contentspp);
	contents = contentspp;

	// call lex&yacc parser
	mcopidlParse(&contents[0]);

	// generate code for C++ header file
	FILE *header = startHeader(prefix);
	doIncludeHeader(header);
	doEnumHeader(header);
	doStructHeader(header);
	doInterfacesHeader(header);
	endHeader(header,prefix);

	// generate code for C++ source file
	FILE *source = startSource(prefix);
	doStructSource(source);
	doInterfacesSource(source);
	doInterfaceRepoSource(source,prefix);
	endSource(source);
}
