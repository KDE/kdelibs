    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef MCOP_ANYREF_H
#define MCOP_ANYREF_H

#include "buffer.h"
#include <string>

/*
 * BC - Status (2000-09-30): AnyRefBase, AnyRef, AnyConstRef
 *
 * These classes will be kept binary compatibile. To change it, adding a new
 * representation is necessary. No private d pointer for this reason.
 */

namespace Arts {

class Any;
class AnyRefBase {
protected:
	void *data;

	/*
	 * This is used for specifying which is the content of a sequence or
	 * an enum or similar. For normal types, it remains unset, as their
	 * name can be generated from rep.
	 */
	std::string _type;

	/*
	 * What representation data is pointing to?
	 *
	 * repInt (int *), repDouble (double *) and repConstChar (const char *)
	 * are no native MCOP types, but alternative ways or representing the
	 * native "int", "float" and "const char *" types
	 */

	// BC: adding new representations and types is possible, however, existing
	//     numbers may not be changed
	enum Representation {
		repVoid = 0,
		repByte = 10,
		repLong = 20, repInt = 21 /* treated as long */,
		repFloat = 30, repDouble = 31 /* treated as float */,
		repString = 40, repCString = 41 /* string */,
		repBool = 50,
		repByteSeq = 510,
		repLongSeq = 520,
		repFloatSeq = 530,
		repStringSeq = 540,
		repBoolSeq = 550,
		repAny = 1000			  /* may hold any type */
	} rep;

	void _write(Buffer *b) const;
	void _read(Buffer *b) const;

	AnyRefBase(const void *data, Representation rep)
		: data(const_cast<void *>(data)), rep(rep) { };
	AnyRefBase(const void *data, Representation rep, const char *type)
		: data(const_cast<void *>(data)), _type(type), rep(rep) { };
	AnyRefBase(const AnyRefBase &copy)
		: data(copy.data), _type(copy._type), rep(copy.rep) { }
public:
	std::string type() const;
};

class AnyConstRef : public AnyRefBase {
public:
	AnyConstRef()					  	: AnyRefBase(0,repVoid) { };
	AnyConstRef(const mcopbyte& value)	: AnyRefBase(&value,repByte) { };
	AnyConstRef(const int& 	value)		: AnyRefBase(&value,repInt) { };
	AnyConstRef(const long& 	value)	: AnyRefBase(&value,repLong) { };
	AnyConstRef(const float& 	value)	: AnyRefBase(&value,repFloat) { };
	AnyConstRef(const double& 	value)	: AnyRefBase(&value,repDouble) { };
	AnyConstRef(const std::string& value) : AnyRefBase(&value,repString) { };
	AnyConstRef(const char *value)		: AnyRefBase(value,repCString) { };
	AnyConstRef(const bool& 	value)	: AnyRefBase(&value,repBool) { };

	AnyConstRef(const std::vector<mcopbyte>& v)
										: AnyRefBase(&v,repByteSeq) { };
	AnyConstRef(const std::vector<long>& v)	
										: AnyRefBase(&v,repLongSeq) { };
	AnyConstRef(const std::vector<float>& v)
										: AnyRefBase(&v,repFloatSeq) { };
	AnyConstRef(const std::vector<std::string>& v)
										: AnyRefBase(&v,repStringSeq) { };
	AnyConstRef(const std::vector<bool>& v)
										: AnyRefBase(&v,repBoolSeq) { };

	AnyConstRef(const Any& value)		: AnyRefBase(&value,repAny) { };

	AnyConstRef(const AnyConstRef& ref) : AnyRefBase(ref) { }
	void write(Buffer *b) const			{ _write(b); }
};

class AnyRef : public AnyRefBase {
public:
	AnyRef()					  		: AnyRefBase(0,repVoid) { };

	// primitive types
	AnyRef(mcopbyte& value)				: AnyRefBase(&value,repByte) { };
	AnyRef(int& 	value)				: AnyRefBase(&value,repInt) { };
	AnyRef(long& 	value)				: AnyRefBase(&value,repLong) { };
	AnyRef(float& 	value)				: AnyRefBase(&value,repFloat) { };
	AnyRef(double& 	value)				: AnyRefBase(&value,repDouble) { };
	AnyRef(std::string&	value)			: AnyRefBase(&value,repString) { };
	AnyRef(bool& 	value)				: AnyRefBase(&value,repBool) { };

	// sequence of primitive types
	AnyRef(std::vector<mcopbyte>& value)	: AnyRefBase(&value,repByteSeq) { };
	AnyRef(std::vector<long>& value)		: AnyRefBase(&value,repLongSeq) { };
	AnyRef(std::vector<float>& value)		: AnyRefBase(&value,repFloatSeq) { };
	AnyRef(std::vector<std::string>& value)	: AnyRefBase(&value,repStringSeq){};
	AnyRef(std::vector<bool>& value)	: AnyRefBase(&value,repBoolSeq){};

	AnyRef(Any& value)						: AnyRefBase(&value,repAny) { };

	AnyRef(const AnyRef& ref) : AnyRefBase(ref) { }

	void read(Buffer *b) const			{ _read(b);  }
	void write(Buffer *b) const			{ _write(b); }
};

};
#endif /* MCOP_ANYREF_H */
