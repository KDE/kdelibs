/* This file is part of the KDE libraries
    Copyright (C) 1999 Ilya Baran (ibaran@acs.bu.edu)

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
// $Id$

#ifndef KFORMULA_H_INCLUDED
#define KFORMULA_H_INCLUDED

#include <qarray.h>
#include <qpainter.h>
#include <qcstring.h>
#include <qrect.h>
#include <qstrlist.h>
#include "box.h"

struct charinfo { // used to determine where each character gets
                  // sent during parsing
  box * where;    // points to box where it ends up
  int posinbox;   // where in that box is it
  int posinstr;   // where is it in the parsed string after parenthesize
  bool left;      // whether it's to the left or right of its char
};

int operator==(struct charinfo a, struct charinfo b);

#define L_GROUP QChar('{')
#define R_GROUP QChar('}')
#define L_BRACE_UNSEEN QChar(129)
#define R_BRACE_UNSEEN QChar(130)

enum ErrorType {
  NO_ERROR = 0,
  DIVISION_BY_ZERO,
  ROOT_OF_NEGATIVE,
  UNDEFINED_VARIABLE,
  EMPTY_BOX,
  PARSE_ERROR
};

//this class is for parsing and storing the actual boxes that
//make up a formula
/**
 * I don't have time now to write real documentation.
 * The KFormula class is used for parsing and displaying formulas.
 *
 * If you want to draw a formula from a c-style expression,
 * call KFormula::fromUgly on your string and pass the result
 * to parse.  Decide where you want it to be drawn by calling setPos
 * and passing the center to it (for other alignments, you can always
 * get the size).  Then draw your formula by calling redraw (then you
 * pass the painter--for now its font is used for arranging and drawing
 * the formula).
 *
 * You can also evaluate a formula that you constructed with
 * the restricted flag set to true--pass the appropriate arguments
 * to evaluate.  Beware that the error handling is not great,
 * especially in fromUgly (whose output you need to pass to parse
 * before you can evaluate anything).
 *
 * The other use of KFormula is with KFormulaEdit--it's much cooler
 * that way.
 */
class KFormula {
private:

  static QString *SPECIAL; //all the special characters
  static QString *INTEXT;  //plus minus slash times relationals
  static QString *LOC;     //powers, subscripts, above, below
  static QString *DELIM;   //parentheses, absolute value, etc
  static QString *BIGOP;   //sums, products, integrals
  static QString *EVAL;    //things we can evaluate

  static void initStrings(void);

  bool restricted; // if will be evaluated

public:
  KFormula(bool restricted = FALSE);
  KFormula(int x, int y, bool restricted = FALSE);
  virtual ~KFormula();

  void redraw(QPainter &p);
  void setBoxes(QArray<box *> newBoxes);
  QArray<box *> getBoxes(void);
  void setPos(int x, int y); //sets the position for the center of the formula
  void parse(QString text, QArray<charinfo> *info = NULL);
  QRect getCursorPos(charinfo i);

  /**
   * The arguments to this will be replaced with a dict at some point...
   */
  double evaluate(QStrList &vars, const QArray<double> &vals,
		  int *error = NULL, box *b = NULL);
  QSize size();

private:
  QArray<box *> boxes;
  int posx;
  int posy;

private:
  void parenthesize(QString &temp, int &i, QArray<charinfo> *info);
  box * makeBoxes(QString str, int offset, int maxlen, QArray<charinfo> *info);

public:
  static QString special() { initStrings(); return *SPECIAL; }
  static QString intext() { initStrings();  return *INTEXT; }
  static QString loc() { initStrings(); return *LOC; }
  static QString delim() { initStrings(); return *DELIM; }
  static QString bigop() { initStrings(); return *BIGOP; }
  static QString eval() { initStrings(); return *EVAL; }
  static int findMatch(QString s, int pos); // find the matching brace
  static QString toUgly(QString);
  static QString fromUgly(QString);

};



#endif // KFORMULA_H_INCLUDED


