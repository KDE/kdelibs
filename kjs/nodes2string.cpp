// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "nodes.h"

namespace KJS {
  /**
   * A simple text streaming class that helps with code indentation.
   */
  class SourceStream {
  public:
    enum Format {
      Endl, Indent, Unindent
    };

    UString toString() const { return str; }
    SourceStream& operator<<(const KJS::UString &);
    SourceStream& operator<<(Format f);
    SourceStream& operator<<(const Node *);
  private:
    UString str; /* TODO: buffer */
    UString ind;
  };
}

using namespace KJS;

SourceStream& SourceStream::operator<<(const KJS::UString &s)
{
  str += s;
  return *this;
}

SourceStream& SourceStream::operator<<(const Node *n)
{
  if (n)
    n->streamTo(*this);
  return *this;
}

SourceStream& SourceStream::operator<<(Format f)
{
  if (f == Endl)
    str += "\n" + ind;
  else if (f == Indent)
    ind += "  ";
  else
    ind = ind.substr(0, ind.size() - 2);

  return *this;
}

UString Node::toString() const
{
  SourceStream str;
  streamTo(str);

  return str.toString();
}

void NullNode::streamTo(SourceStream &s) const { s << "null"; }

void BooleanNode::streamTo(SourceStream &s) const
{
  s << (val ? "true" : "false");
}

void NumberNode::streamTo(SourceStream &s) const { s << UString::from(val); }

void StringNode::streamTo(SourceStream &s) const
{
  s << '"' << val << '"';
}

void RegExpNode::streamTo(SourceStream &s) const { s <<  pattern; }

void ThisNode::streamTo(SourceStream &s) const { s << "this"; }

void ResolveNode::streamTo(SourceStream &s) const { s << ident; }

void GroupNode::streamTo(SourceStream &s) const
{
  s << "(" << group << ")";
}

void ElisionNode::streamTo(SourceStream &s) const
{
  if (elision)
    s << elision << ",";
  else
    s << ",";
}

void ElementNode::streamTo(SourceStream &s) const
{
  if (list)
    s << list << ",";
  s << elision << node;
}

void ArrayNode::streamTo(SourceStream &s) const
{
  s << "[" << element << elision << "]";
}

void ObjectLiteralNode::streamTo(SourceStream &s) const
{
  if (list)
    s << "{ " << list << " }";
  else
    s << "{ }";
}

void PropertyValueNode::streamTo(SourceStream &s) const
{
  if (list)
    s << list << ", ";
  s << name << ": " << assign;
}

void PropertyNode::streamTo(SourceStream &s) const
{
  if (str.isNull())
    s << UString::from(numeric);
  else
    s << str;
}

void AccessorNode1::streamTo(SourceStream &s) const
{
  s << expr1 << "[" << expr2 << "]";
}

void AccessorNode2::streamTo(SourceStream &s) const
{
  s << expr << "." << ident;
}

void ArgumentListNode::streamTo(SourceStream &s) const
{
  if (list)
    s << list << ", ";
  s << expr;
}

void ArgumentsNode::streamTo(SourceStream &s) const
{
  s << "(" << list << ")";
}

void NewExprNode::streamTo(SourceStream &s) const
{
  s << "new " << expr << args;
}

void FunctionCallNode::streamTo(SourceStream &s) const
{
  s << expr << args;
}

void PostfixNode::streamTo(SourceStream &s) const
{
  s << expr;
  if (oper == OpPlusPlus)
    s << "++";
  else
    s << "--";
}

void DeleteNode::streamTo(SourceStream &s) const
{
  s << "delete " << expr;
}

void VoidNode::streamTo(SourceStream &s) const
{
  s << "void " << expr;
}

void TypeOfNode::streamTo(SourceStream &s) const
{
  s << "typeof " << expr;
}

void PrefixNode::streamTo(SourceStream &s) const
{
  s << expr << (oper == OpPlusPlus ? "++" : "--");
}

void UnaryPlusNode::streamTo(SourceStream &s) const
{
  s << "+" << expr;
}

void NegateNode::streamTo(SourceStream &s) const
{
  s << "-" << expr;
}

void BitwiseNotNode::streamTo(SourceStream &s) const
{
  s << "~" << expr;
}

void LogicalNotNode::streamTo(SourceStream &s) const
{
  s << "!" << expr;
}

void MultNode::streamTo(SourceStream &s) const
{
  s << term1 << oper << term2;
}

void AddNode::streamTo(SourceStream &s) const
{
  s << term1 << oper << term2;
}

void ShiftNode::streamTo(SourceStream &s) const
{
  s << term1;
  if (oper == OpLShift)
    s << "<<";
  else if (oper == OpRShift)
    s << ">>";
  else
    s << ">>>";
  s << term2;
}

void RelationalNode::streamTo(SourceStream &s) const
{
  s << expr1;
  switch (oper) {
  case OpLess:
    s << " < ";
    break;
  case OpGreater:
    s << " > ";
    break;
  case OpLessEq:
    s << " <= ";
    break;
  case OpGreaterEq:
    s << " >= ";
    break;
  case OpInstanceOf:
    s << " instanceof ";
    break;
  case OpIn:
    s << " in ";
    break;
  default:
    ;
  }
  s << expr2;
}

void EqualNode::streamTo(SourceStream &s) const
{
  s << expr1;
 switch (oper) {
 case OpEqEq:
   s << " == ";
   break;
 case OpNotEq:
   s << " != ";
   break;
 case OpStrEq:
   s << " === ";
   break;
 case OpStrNEq:
   s << " !== ";
   break;
 default:
   ;
 }
  s << expr2;
}

void BitOperNode::streamTo(SourceStream &s) const
{
  s << expr1;
  if (oper == OpBitAnd)
    s << " & ";
  else if (oper == OpBitXOr)
    s << " ^ ";
  else
    s << " | ";
  s << expr2;
}

void BinaryLogicalNode::streamTo(SourceStream &s) const
{
  s << expr1 << (oper == OpAnd ? " && " : " || ") << expr2;
}

void ConditionalNode::streamTo(SourceStream &s) const
{
  s << logical << " ? " << expr1 << " : " << expr2;
}

void AssignNode::streamTo(SourceStream &s) const
{
  s << left;
  const char *opStr;
  switch (oper) {
  case OpEqual:
    opStr = " = ";
    break;
  case OpMultEq:
    opStr = " *= ";
    break;
  case OpDivEq:
    opStr = " /= ";
    break;
  case OpPlusEq:
    opStr = " += ";
    break;
  case OpMinusEq:
    opStr = " -= ";
    break;
  case OpLShift:
    opStr = " <<= ";
    break;
  case OpRShift:
    opStr = " >>= ";
    break;
  case OpURShift:
    opStr = " >>= ";
    break;
  case OpAndEq:
    opStr = " &= ";
    break;
  case OpXOrEq:
    opStr = " ^= ";
    break;
  case OpOrEq:
    opStr = " |= ";
    break;
  case OpModEq:
    opStr = " %= ";
    break;
  default:
    opStr = " ?= ";
  }
  s << opStr << expr;
}

void CommaNode::streamTo(SourceStream &s) const
{
  s << expr1 << ", " << expr2;
}

void StatListNode::streamTo(SourceStream &s) const
{
  s << list << statement;
}

void AssignExprNode::streamTo(SourceStream &s) const
{
  s << " = " << expr;
}

void VarDeclNode::streamTo(SourceStream &s) const
{
  s << ident << init;
}

void VarDeclListNode::streamTo(SourceStream &s) const
{
  if (list)
    s << list << ", ";
  s << var;
}

void VarStatementNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "var " << list << ";";
}

void BlockNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "{" << SourceStream::Indent
    << source << SourceStream::Unindent << SourceStream::Endl << "}";
}

void EmptyStatementNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << ";";
}

void ExprStatementNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << expr << ";";
}

void IfNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "if (" << expr << ")" << SourceStream::Indent
    << statement1 << SourceStream::Unindent;
  if (statement2)
    s << SourceStream::Endl << "else" << SourceStream::Indent
      << statement2 << SourceStream::Unindent;
}

void DoWhileNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "do " << SourceStream::Indent
    << statement << SourceStream::Unindent << SourceStream::Endl
    << "while (" << expr << ");";
}

void WhileNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "while (" << expr << ")" << SourceStream::Indent
    << statement << SourceStream::Unindent;
}

void ForNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "for ("
    << expr1  // TODO: doesn't properly do "var i = 0"
    << "; " << expr2
    << "; " << expr3
    << ")" << SourceStream::Indent << statement << SourceStream::Unindent;
}

void ForInNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "for (";
  if (varDecl)
    s << "var " << varDecl;
  if (init)
    s << " = " << init;
  s << " in " << expr << ")" << SourceStream::Indent
    << statement << SourceStream::Unindent;
}

void ContinueNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "continue";
  if (!ident.isNull())
    s << " " << ident;
  s << ";";
}

void BreakNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "break";
  if (!ident.isNull())
    s << " " << ident;
  s << ";";
}

void ReturnNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "return";
  if (value)
    s << " " << value;
  s << ";";
}

void WithNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "with (" << expr << ") "
    << statement;
}

void CaseClauseNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl;
  if (expr)
    s << "case " << expr;
  else
    s << "default";
  s << ":" << SourceStream::Indent;
  if (list)
    s << list;
  s << SourceStream::Unindent;
}

void ClauseListNode::streamTo(SourceStream &s) const
{
  const ClauseListNode *l = this;
  do {
    s << l;
    l = l->nx;
  } while (l);
}

void CaseBlockNode::streamTo(SourceStream &s) const
{
  const ClauseListNode *cl = list1;
  while (cl) {
    s << cl->clause();
    cl = cl->next();
  }
  if (def)
    s << def;
  cl = list2;
  while (cl) {
    s << cl->clause();
    cl = cl->next();
  }
}

void SwitchNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "switch (" << expr << ") {"
    << SourceStream::Indent << block << SourceStream::Unindent
    << SourceStream::Endl << "}";
}

void LabelNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << label << ":" << SourceStream::Indent
    << statement << SourceStream::Unindent;
}

void ThrowNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "throw " << expr << ";";
}

void CatchNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "catch (" << ident << ")" << block;
}

void FinallyNode::streamTo(SourceStream &s) const
{
  s << SourceStream::Endl << "finally " << block;
}

void TryNode::streamTo(SourceStream &s) const
{
  s << "try " << block
    << _catch
    << _final;
}

void ParameterNode::streamTo(SourceStream &s) const
{
  s << id;
  if (next)
    s << ", " << next;
}

void FunctionBodyNode::streamTo(SourceStream &s) const {
  s << SourceStream::Endl << "{" << SourceStream::Indent
    << source << SourceStream::Unindent << SourceStream::Endl << "}";
}

void FuncDeclNode::streamTo(SourceStream &s) const {
  s << "function " << ident << "(";
  if (param)
    s << param;
  s << ")" << body;
}

void FuncExprNode::streamTo(SourceStream &s) const
{
  s << "function " << "("
    << param
    << ")" << body;
}

void SourceElementNode::streamTo(SourceStream &s) const
{
  if (statement)
    s << statement;
  else
    s << function;
}

void SourceElementsNode::streamTo(SourceStream &s) const
{
  s << elements << element;
}

