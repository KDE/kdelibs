    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

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

    */

#include <stdio.h>
#include <vector>
#include <list>
#include <ctype.h>
#include "core.h"

int idl_line_no;
list<EnumDef *> enums;
list<TypeDef *> structs;
list<InterfaceDef *> interfaces;

ModuleDef module;

void addEnumTodo( EnumDef *edef )
{
	enums.push_back(edef);

	// TODO: memory management? Will be freed twice now?
	module.enums.push_back(edef);
}

void addStructTodo( TypeDef *type )
{
	structs.push_back(type);

	// TODO: memory management? Will be freed twice now?
	module.types.push_back(type);
}

void addInterfaceTodo( InterfaceDef *iface )
{
	interfaces.push_back(iface);

	// TODO: memory management? Will be freed twice now?
	module.interfaces.push_back(iface);
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
#define MODEL_SEQ       1024

#define MODEL_MEMBER_SEQ	(MODEL_MEMBER|MODEL_SEQ)
#define MODEL_ARG_SEQ		(MODEL_ARG|MODEL_SEQ)
#define MODEL_READ_SEQ		(MODEL_READ|MODEL_SEQ)
#define MODEL_WRITE_SEQ		(MODEL_WRITE|MODEL_SEQ)
#define MODEL_REQ_READ_SEQ	(MODEL_REQ_READ|MODEL_SEQ)
#define MODEL_RES_READ_SEQ	(MODEL_RES_READ|MODEL_SEQ)
#define MODEL_REQ_WRITE_SEQ	(MODEL_REQ_WRITE|MODEL_SEQ)
#define MODEL_RESULT_SEQ	(MODEL_RESULT|MODEL_SEQ)
#define MODEL_INVOKE_SEQ	(MODEL_RESULT|MODEL_SEQ)

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
			result = indent + "delete result;\n";
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
		if(model==MODEL_MEMBER_SEQ) result = "vector<float>";
		if(model==MODEL_ARG)		result = "float";
		if(model==MODEL_ARG_SEQ)	result = "const vector<float>&";
		if(model==MODEL_RESULT)		result = "float";
		if(model==MODEL_RESULT_SEQ)	result = "vector<float> *";
		if(model==MODEL_STREAM)		result = "float *"+name;
		if(model==MODEL_MSTREAM)	result = "float **"+name;

		if(model==MODEL_READ)
			result = name+" = stream.readFloat()";
		if(model==MODEL_RES_READ)
		{
			result = indent + "float returnCode = result->readFloat();\n";
			result += indent + "delete result;\n";
			result += indent + "return returnCode;\n";
		}
		if(model==MODEL_REQ_READ)
			result = indent + "float "+name+" = request->readFloat();\n";
		if(model==MODEL_WRITE)
			result = "stream.writeFloat("+name+")";
		if(model==MODEL_REQ_WRITE)
			result = "request->writeFloat("+name+")";
		if(model==MODEL_INVOKE)
			result = indent + "result->writeFloat("+name+");\n";
	}
	else if(type == "boolean")
	{
		if(model==MODEL_MEMBER)		result = "bool";
		if(model==MODEL_MEMBER_SEQ) result = "vector<bool>";
		if(model==MODEL_ARG)		result = "bool";
		if(model==MODEL_ARG_SEQ)	result = "const vector<bool>&";
		if(model==MODEL_RESULT)		result = "bool";
		if(model==MODEL_RESULT_SEQ)	result = "vector<bool> *";
		if(model==MODEL_READ)
			result = name+" = stream.readBool()";
		if(model==MODEL_RES_READ)
		{
			result = indent + "bool returnCode = result->readBool();\n";
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
	else if(type == "long")
	{
		if(model==MODEL_MEMBER)		result = "long";
		if(model==MODEL_MEMBER_SEQ) result = "vector<long>";
		if(model==MODEL_ARG)		result = "long";
		if(model==MODEL_ARG_SEQ)	result = "const vector<long>&";
		if(model==MODEL_RESULT)		result = "long";
		if(model==MODEL_RESULT_SEQ)	result = "vector<long> *";
		if(model==MODEL_READ)
			result = name+" = stream.readLong()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readLongSeq("+name+")";
		if(model==MODEL_RES_READ)
		{
			result = indent + "long returnCode = result->readLong();\n";
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
		if(model==MODEL_MEMBER)		result = "string";
		if(model==MODEL_MEMBER_SEQ)	result = "vector<string>";
		if(model==MODEL_ARG)		result = "const string&";
		if(model==MODEL_ARG_SEQ)	result = "const vector<string>&";
		if(model==MODEL_RESULT)		result = "string";
		if(model==MODEL_RESULT_SEQ)	result = "vector<string> *";
		if(model==MODEL_READ)
			result = "stream.readString("+name+")";
		if(model==MODEL_READ_SEQ)
			result = "stream.readStringSeq("+name+")";
		if(model==MODEL_REQ_READ)
		{
			result = indent + "string "+name+";\n"
				   + indent + "request->readString("+name+");\n";
		}
		if(model==MODEL_RES_READ)
		{
			result = indent + "string returnCode;\n";
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
			result = "vector<"+type+" *>";

		if(model==MODEL_ARG)	
			result = "const "+type+"&";
		if(model==MODEL_ARG_SEQ)
			result = "const vector<"+type+" *>&";

		if(model==MODEL_READ)
			result = name+".readType(stream)";
		if(model==MODEL_READ_SEQ)
			result = "readTypeSeq<"+type+">(stream,"+name+")";
		if(model==MODEL_REQ_READ)
			result = indent + type+" "+name+"(*request);\n";

		if(model==MODEL_WRITE)
			result = name+".writeType(stream)";
		if(model==MODEL_REQ_WRITE)
			result = name+".writeType(*request)";
		if(model==MODEL_WRITE_SEQ)
			result = "writeTypeSeq<"+type+">(stream,"+name+")";
		if(model==MODEL_REQ_WRITE_SEQ)
			result = "writeTypeSeq<"+type+">(*request,"+name+")";

		if(model==MODEL_INVOKE)
			result = indent + type + " *_returnCode = "+name+";\n"
			       + indent + "_returnCode->writeType(*result);\n"
				   + indent + "delete _returnCode;\n";

		if(model==MODEL_RES_READ)
		{
			result = indent + type + " *_returnCode = new "+type+"(*result);\n";
			result += indent + "delete result;\n";
			result += indent + "return _returnCode;\n";
		}

		if(model==MODEL_RESULT)		result = type+"*";
		if(model==MODEL_RESULT_SEQ)	result = "vector<"+type+" *> *";
	} else if(isEnum(type)) {
		if(model==MODEL_MEMBER)		result = type;
		if(model==MODEL_MEMBER_SEQ) result = "vector<"+type+">";
		if(model==MODEL_ARG)		result = type;
		if(model==MODEL_ARG_SEQ)	result = "const vector<"+type+">&";
		if(model==MODEL_RESULT)		result = type;
		if(model==MODEL_RESULT_SEQ)	result = "vector<"+type+"> *";
		if(model==MODEL_READ)
			result = name+" = ("+type+")stream.readLong()";
		if(model==MODEL_READ_SEQ)
			result = "stream.readLongSeq("+name+")";		// TODO
		if(model==MODEL_RES_READ)
		{
			result = indent + type+" returnCode = ("+
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

void doEnumHeader(FILE *header)
{
	list<EnumDef *>::iterator edi;
	vector<EnumComponent *>::iterator i;

	for(edi = enums.begin();edi != enums.end(); edi++)
	{
		EnumDef *ed = *edi;

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

		fprintf(source,"%s::%s(const %s& copyType) :Type()\n{\n",
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
				fprintf(source,"\tfreeTypeSeq<%s>(%s);\n",
					type.c_str(),(*i)->name.c_str());
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

string createReturnCode(MethodDef *md)
{
	return createTypeCode(md->type,"",MODEL_RESULT);
}

string createParamList(MethodDef *md)
{
	string result;
	int first = 0;
	vector<ParamDef *>::iterator pi;

	for(pi = md->signature.begin(); pi != md->signature.end(); pi++)
	{
		ParamDef *pd = *pi;
		string p = createTypeCode(pd->type,pd->name,MODEL_ARG);

		if(first != 0) result += ", ";
		first++;
		result += p;
	}
	return result;
}

void createStubCode(FILE *source, string iface, string method, MethodDef *md,
	long mcount)
{
	string rc = createReturnCode(md);
	string params = createParamList(md);
	vector<ParamDef *>::iterator pi;

	Buffer b;
	md->writeType(b);

	fprintf(source,"%s %s_stub::%s(%s)\n",rc.c_str(),iface.c_str(),
				method.c_str(), params.c_str());
	fprintf(source,"{\n");
	/*
	fprintf(source,"\tlong methodID;\n");
	fprintf(source,"\t{\n");
	fprintf(source,"\t\tBuffer _b;\n");
	fprintf(source,"\t\t_b.fromString(\"%s\",\"method\");\n",
											b.toString("method").c_str());
	fprintf(source,"\t\tmethodID = _lookupMethod(MethodDef(_b));\n");
	fprintf(source,"\t}\n");
	*/
	fprintf(source,"\tlong methodID = _lookupMethodFast(\"%s\");\n",
											b.toString("method").c_str());
	fprintf(source,"\tlong requestID;\n");
	fprintf(source,"\tBuffer *request, *result;\n");
	fprintf(source,"\trequest = Dispatcher::the()->"
				"createRequest(requestID,_objectID,methodID);\n");
	fprintf(source,"\t// methodID = %ld  =>  %s\n",mcount,md->name.c_str());

	for(pi = md->signature.begin(); pi != md->signature.end(); pi++)
	{
		ParamDef *pd = *pi;
		string p;
		p = createTypeCode(pd->type,pd->name,MODEL_REQ_WRITE);
		fprintf(source,"\t%s;\n",p.c_str());
	}
	fprintf(source,"\trequest->patchLength();\n");
	fprintf(source,"\t_connection->qSendBuffer(request);\n\n");

	fprintf(source,
		"\tresult = Dispatcher::the()->waitForResult(requestID);\n");

	fprintf(source,"%s",
		createTypeCode(md->type,"",MODEL_RES_READ,"\t").c_str());

	fprintf(source,"}\n\n");
}

bool haveStreams(InterfaceDef *d)
{
	vector<AttributeDef *>::iterator ai;

	for(ai = d->attributes.begin();ai != d->attributes.end();ai++)
		if((*ai)->flags & attributeStream) return true;

	return false;
}

void doInterfacesHeader(FILE *header)
{
	list<InterfaceDef *>::iterator ii;
	vector<MethodDef *>::iterator mi;
	vector<AttributeDef *>::iterator ai;
	string inherits;

	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef *d = *ii;

		// create abstract interface
		inherits = buildInheritanceList(*d,"");
		if(inherits == "") inherits = "virtual public Object";

		fprintf(header,"class %s : %s {\n",d->name.c_str(),inherits.c_str());
		fprintf(header,"public:\n");
		fprintf(header,"\tstatic %s *_fromString(string objectref);\n\n",
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

		fprintf(header,"class %s_stub : virtual public %s, %s {\n",
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

		fprintf(header,"class %s_skel : virtual public %s,"
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
					decl = createTypeCode(ad->type,ad->name,MODEL_MSTREAM);
				else
					decl = createTypeCode(ad->type,ad->name,MODEL_STREAM);

				decl += ";";

				/** write to source **/
				string comment;

				if(ad->flags & streamIn) comment = "incoming stream";
				if(ad->flags & streamOut) comment = "outgoing stream";

				fprintf(header,"\t%-40s  // %s\n",decl.c_str(),comment.c_str());
			}
		}
		if(!firstStream) fprintf(header,"\n");

		fprintf(header,"public:\n");
		fprintf(header,"\t%s_skel();\n\n",d->name.c_str());

		fprintf(header,"\tstatic string _interfaceNameSkel();\n");
		fprintf(header,"\tstring _interfaceName();\n");
		fprintf(header,"\tvoid _buildMethodTable();\n");
		fprintf(header,"\tvoid *_cast(string interface);\n");
		fprintf(header,"\tvoid dispatch(Buffer *request, Buffer *result,"
						"long methodID);\n");
		fprintf(header,"};\n\n");

	}
}

void doInterfacesSource(FILE *source)
{
	list<InterfaceDef *>::iterator ii;
	vector<MethodDef *>::iterator mi;
	vector<ParamDef *>::iterator pi;
	vector<AttributeDef *>::iterator ai;

	long mcount;

	for(ii = interfaces.begin();ii != interfaces.end(); ii++)
	{
		InterfaceDef *d = *ii;

		// create static functions

		fprintf(source,"%s *%s::_fromString(string objectref)\n",
											d->name.c_str(),d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\t%s *result = 0;\n",d->name.c_str());
		fprintf(source,"\tObjectReference r;\n\n");
		fprintf(source,"\tif(Dispatcher::the()->stringToObjectReference(r,objectref))\n");
		fprintf(source,"\t{\n");
		fprintf(source,
		"\t\tresult = (%s *)Dispatcher::the()->connectObjectLocal(r,\"%s\");\n",
										d->name.c_str(),d->name.c_str());
		fprintf(source,"\t\tif(!result)\n");
		fprintf(source,"\t\t{\n");
		fprintf(source,"\t\t\tConnection *conn = "
							"Dispatcher::the()->connectObjectRemote(r);\n");
		fprintf(source,"\t\t\tif(conn)\n");
		fprintf(source,"\t\t\t\tresult = new %s_stub(conn,r.objectID);\n",
										d->name.c_str());
		fprintf(source,"\t\t}\n");
		fprintf(source,"\t}\n");
		fprintf(source,"\treturn result;\n");
		fprintf(source,"}\n\n");

		// create stub

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

		// TODO:
		// first official method is 3, but this needs to be synchronized
		// with multiple inheritance and backward compatibility (what if
		// somebody adds a method to an interface)
		mcount = 3;

		for(mi = d->methods.begin(); mi != d->methods.end(); mi++)
		{
			MethodDef *md = *mi;
			createStubCode(source,d->name.c_str(),md->name.c_str(),md,mcount++);
		}

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
					md.flags = 0;
					/* no parameters (don't set md.signature) */

					createStubCode(source,d->name.c_str(),ad->name.c_str(),
						&md,mcount++);
				}
				if(ad->flags & streamIn)  /* writeable from outside */
				{
					md.name = "_set_"+ad->name;
					md.type = "void";
					md.flags = 0;
					md.signature.push_back(new ParamDef(ad->type,"newValue"));

					createStubCode(source,d->name.c_str(),ad->name.c_str(),
						&md,mcount++);
				}
			}
		}

		// create skeleton

		/** _interfaceName **/
		fprintf(source,"string %s_skel::_interfaceName()\n",	
													d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\treturn \"%s\";\n",d->name.c_str());
		fprintf(source,"}\n\n");

		fprintf(source,"string %s_skel::_interfaceNameSkel()\n",	
													d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\treturn \"%s\";\n",d->name.c_str());
		fprintf(source,"}\n\n");

		/** _cast operation **/
		fprintf(source,"void *%s_skel::_cast(string interface)\n",
			d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tif(interface == \"%s\") return (%s *)this;\n",
			d->name.c_str(),d->name.c_str());

		if(d->inheritedInterfaces.size())
		{
			vector<string>::iterator ii = d->inheritedInterfaces.begin();
			fprintf(source,"\n\tvoid *result;\n");
			while(ii != d->inheritedInterfaces.end())
			{
				fprintf(source,"\tresult = %s_skel::_cast(interface);\n",
					ii->c_str());
				fprintf(source,"\tif(result) return result;\n\n");

				ii++;
			}
		}

		fprintf(source,"\treturn 0;\n");
		fprintf(source,"}\n\n");

		/** dispatch operations **/
		Buffer methodTable;

		mcount = 0;
		for(mi = d->methods.begin(); mi != d->methods.end(); mi++, mcount++)
		{
			MethodDef *md = *mi;
			md->writeType(methodTable);

			/** calculate signature (prevents unused argument warnings) **/
			string signature = "void *object, ";

			if(md->signature.size() == 0)
				signature += "Buffer *, ";
			else
				signature += "Buffer *request, ";

			if(md->type == "void")
				signature += "Buffer *";
			else
				signature += "Buffer *result";


			fprintf(source,"// %s\n",md->name.c_str());
			fprintf(source,"static void _dispatch_%s_%02ld(%s)\n",
					d->name.c_str(),mcount,signature.c_str());
			fprintf(source,"{\n");

			string call = "(("+d->name+"_skel *)object)->"+md->name + "(";
			int first = 1;
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

		/** methodTable **/

		fprintf(source,"void %s_skel::_buildMethodTable()\n",d->name.c_str());
		fprintf(source,"{\n");
		fprintf(source,"\tBuffer m;\n");
		fprintf(source,"\tm.fromString(\"%s\",\"MethodTable\");\n",
			methodTable.toString("MethodTable").c_str());

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

#if 0
		/** streams **/
		if(haveStreams(d))
		{
			fprintf(source,
				"void *%s_skel::_lookupStream(string name)\n", d->name.c_str());
			fprintf(source,"{\n");

			for(ai = d->attributes.begin(); ai != d->attributes.end(); ai++)
			{
				AttributeDef *ad = *ai;
				if((ad->flags & attributeStream) == attributeStream)
				{
					fprintf(source,"\tif(name == \"%s\") return &%s;\n",
								ad->name.c_str(),ad->name.c_str());
				}
			}
			fprintf(source,"\treturn 0;\n");
			fprintf(source,"}\n\n");
		}
#endif
	}
}

void doInterfaceRepoSource(FILE *source, string prefix)
{
	Buffer b;
	module.moduleName = "";
	module.writeType(b);

	fprintf(source,"IDLFileReg IDLFileReg_%s(\"%s\",\"%s\");\n",
		prefix.c_str(),prefix.c_str(),b.toString("IDLFile").c_str());
}

void exit_usage(char *name)
{
	fprintf(stderr,"usage: %s <filename>\n",name);
	exit(1);
}
extern void mcopidlParse(const char *code);

int main(int argc, char **argv)
{
	FILE *f;
	idl_line_no = 1;

	vector<char> contents;
	if(argc != 2) exit_usage(argv[0]);

	char *prefix = strdup(argv[1]);

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

	f = fopen(argv[1],"r");
	if(!f) {
		fprintf(stderr,"file not found\n");
		exit(1);
	}

	char buffer[1024];
	long l;
	while((l = fread(buffer,1,1024,f)) > 0)
		contents.insert(contents.end(),buffer, buffer+l);
	
	// trailing zero byte (mcopidlParse wants a C-style string as argument)
	contents.push_back(0);

	fclose(f);
	mcopidlParse(&contents[0]);

	FILE *header = startHeader(prefix);
	doEnumHeader(header);
	doStructHeader(header);
	doInterfacesHeader(header);
	endHeader(header,prefix);

	FILE *source = startSource(prefix);
	doStructSource(source);
	doInterfacesSource(source);
	doInterfaceRepoSource(source,prefix);
	endSource(source);
}
