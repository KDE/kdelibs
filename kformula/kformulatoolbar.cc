#include "kformulatoolbar.h"
#include <kiconloader.h>
#include <kglobal.h>
#include <kstddirs.h>

KFormulaToolBar::KFormulaToolBar(QWidget *parent, const char *name, int _item_size)
  : KToolBar(parent, name, _item_size)
{
  // doesn't matter if we do this twice - KStandardDirs ignores doubled additions
  KGlobal::dirs()->addResourceType("toolbar", KStandardDirs::kde_default("data") + "kformula/pics/");
  insertButton(ICON("editcut"), CUT_CHAR, true, "Cut");
  insertButton(ICON("editcopy"), COPY_CHAR, true, "Copy");
  insertButton(ICON("editpaste"), PASTE_CHAR, true, "Paste");

  insertSeparator();

  insertButton(ICON("undo"), UNDO_CHAR, true, "Undo");
  insertButton(ICON("redo"), REDO_CHAR, true, "Redo");

  insertSeparator();

  insertButton(ICON("index2"), POWER, true, "Power");
  insertButton(ICON("index3"), SUB, true, "Subscript");
  insertButton(ICON("bra"), PAREN, true, "Parentheses");
  insertButton(ICON("abs"), ABS, true, "Absolute value");
  insertButton(ICON("brackets"), BRACKET, true, "Brackets");
  insertButton(ICON("frac"), DIVIDE, true, "Fraction");
  insertButton(ICON("root"), SQRT, true, "Root");
  insertButton(ICON("integral"), INTEGRAL, true, "Integral");
  insertButton(ICON("matrix"), MATRIX, true, "Matrix");
  insertButton(ICON("index0"), LSUP, true, "Left superscript");
  insertButton(ICON("index1"), LSUB, true, "Left subscript");
}


void KFormulaToolBar::connectToFormula(KFormulaEdit *formula)
{
  QObject::connect(this, SIGNAL(clicked(int)), formula, SLOT(insertChar(int)));
}

#include "kformulatoolbar.moc"
