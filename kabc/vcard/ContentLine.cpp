/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <qcstring.h>
#include <qstrlist.h>
#include <qregexp.h>

#include <kdebug.h>

#include <VCardAdrParam.h>
#include <VCardAgentParam.h>
#include <VCardDateParam.h>
#include <VCardEmailParam.h>
#include <VCardImageParam.h>
#include <VCardSourceParam.h>
#include <VCardTelParam.h>
#include <VCardTextBinParam.h>
#include <VCardTextParam.h>

#include <VCardAdrValue.h>
#include <VCardAgentValue.h>
#include <VCardDateValue.h>
#include <VCardImageValue.h>
#include <VCardTextValue.h>
#include <VCardTextBinValue.h>
#include <VCardLangValue.h>
#include <VCardNValue.h>
#include <VCardURIValue.h>
#include <VCardSoundValue.h>
#include <VCardClassValue.h>
#include <VCardFloatValue.h>
#include <VCardOrgValue.h>
#include <VCardTelValue.h>
#include <VCardTextListValue.h>
#include <VCardUTCValue.h>
#include <VCardGeoValue.h>

#include <VCardRToken.h>
#include <VCardContentLine.h>

#include <VCardEntity.h>
#include <VCardEnum.h>
#include <VCardDefines.h>

using namespace VCARD;

ContentLine::ContentLine()
	:	Entity(),
		value_(0)
{
}

ContentLine::ContentLine(const ContentLine & x)
	:	Entity(x),
                group_ (x.group_),
                name_ (x.name_),
		paramList_(x.paramList_),
		value_(x.value_->clone())
{
}

ContentLine::ContentLine(const QCString & s)
	:	Entity(s),
		value_(0)
{
}

	ContentLine &
ContentLine::operator = (ContentLine & x)
{
	if (*this == x) return *this;
	
	paramList_ = x.paramList();
	value_ = x.value_->clone();

	Entity::operator = (x);
	return *this;
}

	ContentLine &
ContentLine::operator = (const QCString & s)
{
	Entity::operator = (s);
	delete value_;
	value_ = 0;
	return *this;
}

	bool
ContentLine::operator == (ContentLine & x)
{
	x.parse();
	
	QPtrListIterator<Param> it(x.paramList());
	
	if (!paramList_.find(it.current()))
		return false;

	return true;
}

ContentLine::~ContentLine()
{
	delete value_;
	value_ = 0;
}

	void
ContentLine::_parse()
{
	vDebug("parse");
	
	// Unqote newlines
	strRep_ = strRep_.replace( QRegExp( "\\\\n" ), "\n" );
	
	int split = strRep_.find(':');
	
	if (split == -1) { // invalid content line
		vDebug("No ':'");
		return;
	}
	
	QCString firstPart(strRep_.left(split));
	QCString valuePart(strRep_.mid(split + 1));
	
	split = firstPart.find('.');
	
	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.mid(split + 1);
	}
	
	vDebug("Group == " + group_);
	vDebug("firstPart == " + firstPart);
	vDebug("valuePart == " + valuePart);
	
	// Now we have the group, the name and param list together and the value.
	
	QStrList l;
	
	RTokenise(firstPart, ";", l);
	
	if (l.count() == 0) {// invalid - no name !
		vDebug("No name for this content line !");
		return;
	}
	
	name_ = l.at(0);

	// Now we have the name, so the rest of 'l' is the params.
	// Remove the name part.
	l.remove(0u);
	
	entityType_	= EntityNameToEntityType(name_);
	paramType_	= EntityTypeToParamType(entityType_);
	
	unsigned int i = 0;
	
	// For each parameter, create a new parameter of the correct type.

	QStrListIterator it(l);
	
	for (; it.current(); ++it, i++) {

		QCString str = *it;

		split = str.find("=");
		if (split < 0 ) {
			vDebug("No '=' in paramter.");
			continue;
		}
		
		QCString paraName = str.left(split);
		QCString paraValue = str.mid(split + 1);
		
		QStrList paraValues;
		RTokenise(paraValue, ",", paraValues);
		
		QStrListIterator it2( paraValues );
		
		for(; it2.current(); ++it2) {		
		
			Param *p = new Param;
			p->setName( paraName );
			p->setValue( *it2 );
	
			paramList_.append(p);
		}
	}

	// Create a new value of the correct type.

	valueType_ = EntityTypeToValueType(entityType_);
	
//	kdDebug(5710) << "valueType: " << valueType_ << endl;
	
	switch (valueType_) {
		
		case ValueSound:	value_ = new SoundValue;	break;
		case ValueAgent:	value_ = new AgentValue;	break;
		case ValueAddress:	value_ = new AdrValue;		break;
		case ValueTel:		value_ = new TelValue;		break;
		case ValueTextBin:	value_ = new TextBinValue;	break;
		case ValueOrg:		value_ = new OrgValue;		break;
		case ValueN:		value_ = new NValue;		break;
		case ValueUTC:		value_ = new UTCValue;		break;
		case ValueURI:		value_ = new URIValue;		break;
		case ValueClass:	value_ = new ClassValue;	break;
		case ValueFloat:	value_ = new FloatValue;	break;
		case ValueImage:	value_ = new ImageValue;	break;
		case ValueDate:		value_ = new DateValue;		break;
		case ValueTextList:	value_ = new TextListValue;	break;
		case ValueGeo:		value_ = new GeoValue;		break;
		case ValueText:
		case ValueUnknown:
		default:		value_ = new TextValue;		break;
	}
	
	*value_ = valuePart;
}

	void
ContentLine::_assemble()
{
	vDebug("Assemble (argl) - my name is \"" + name_ + "\"");
	strRep_.truncate(0);

	QCString line;
	
	if (!group_.isEmpty())
		line += group_ + '.';
	
	line += name_;

	vDebug("Adding parameters");
	ParamListIterator it(paramList_);
	
	for (; it.current(); ++it)
		line += ";" + it.current()->asString();
	
	vDebug("Adding value");
	if (value_ != 0)
		line += ":" + value_->asString();
	else
		vDebug("No value");

	// Quote newlines
	line = line.replace( QRegExp( "\n" ), "\\n" );
		
	// Fold lines longer than 72 chars
	const int maxLen = 72;
	uint cursor = 0;
	while( line.length() > ( cursor + 1 ) * maxLen ) {
		strRep_ += line.mid( cursor * maxLen, maxLen );
		strRep_ += "\r\n ";
		++cursor;
	}
	strRep_ += line.mid( cursor * maxLen );
}

	void
ContentLine::clear()
{
	group_.truncate(0);
	name_.truncate(0);
	paramList_.clear();
        delete value_;
	value_ = 0;
}
