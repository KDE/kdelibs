/*
 * tokenizer.h - Copyright 2005 Maksim Orlovich <maksim@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QSet>
#include <QString>
#include <QHash>

#include "step.h"
#include "path.h"
#include "predicate.h"
#include "expression.h"
#include "util.h"
#include "parser.h"

namespace khtml {
namespace XPath {


struct Token
{
	int     type;
	QString value;
	int     intValue; //0 if not set
	bool    hasString;

	Token(int _type): type(_type), intValue(0), hasString(false) {}
	Token(QString _value): type(ERROR+1), value(_value), intValue(0), hasString(true) {}
	Token(int _type, QString _value): type(_type), value(_value), intValue(0), hasString(true) {}
	Token(int _type, int     _value): type(_type), intValue(_value), hasString(false) {}
};

class Tokenizer
{
private:
	int m_nextPos;
	QString m_data;
	int m_lastTokenType;

	static QHash<QString, Step::AxisType>* s_axisNamesDict;
	static QSet<QString>* s_nodeTypeNamesDict;

	enum XMLCat {
		NameStart,
		NameCont,
		NotPartOfName
	};

	XMLCat charCat(QChar aChar);

	bool isAxisName(QString name, Step::AxisType *type = 0);
	bool isNodeTypeName(QString name);
	bool isOperatorContext();

	void  skipWS();
	Token makeTokenAndAdvance(int code, int advance = 1);
	Token makeIntTokenAndAdvance(int code, int val, int advance = 1);
	char  peekAheadHelper();
	char  peekCurHelper();

	Token lexString();
	Token lexNumber();
	Token lexNCName();
	Token lexQName();

	Token nextTokenInternal();
	Tokenizer();
	Tokenizer(const Tokenizer &rhs);                  // disabled
	Tokenizer &operator=(const Tokenizer &rhs);       // disabled
	~Tokenizer();
public:
	static Tokenizer &self();

	void reset(QString);
	Token nextToken();
};

// Interface to the parser
int khtmlxpathyylex();
void khtmlxpathyyerror(const char *str);
void initTokenizer(const DOM::DOMString& string);

} // namespace XPath

} // namespace khtml


#endif
// kate: indent-width 4; replace-tabs off; tab-width 4; indent-spaces: off;
