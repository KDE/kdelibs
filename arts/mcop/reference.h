#ifndef MCOP_COMPONENT_REFERENCE_H
#define MCOP_COMPONENT_REFERENCE_H

#include "common.h"

// Acts as a string or ObjectReference, but enables a different constructor
class Reference {
private:
	ObjectReference myref;
	std::string mys;
	bool strMode;
public:
	
	inline Reference(const ObjectReference& ref) {
		myref=ref;
		strMode=false;
	}

	inline Reference(const std::string& s) {
		mys=s;
		strMode=true;
	}
	
	inline Reference(const char* c) {
		mys=c;
		strMode=true;
	}
	inline Reference& operator=(const std::string& s) {mys=s; return *this;}
	inline Reference& operator=(const char*c) {mys=c; return *this;}
	inline bool isString() const {return strMode;}
//	inline operator const std::string() const {return mys;}
	inline const std::string& string() const {return mys;}
//	inline operator const ObjectReference() const {return myref;}
	inline const ObjectReference& reference() const {return myref;}
};

// Acts as a string, but enables a different constructor
class SubClass {
private:
	std::string mys;
public:

	inline SubClass(const std::string& s) : mys(s) {}
	inline SubClass(const char* c) : mys(c) {}
	inline SubClass& operator=(const std::string& s) {mys=s; return *this;}
	inline SubClass& operator=(const char*c) {mys=c; return *this;}
//	inline operator const std::string() const {return mys;}
	inline const std::string& string() const {return mys;}
};

#endif
