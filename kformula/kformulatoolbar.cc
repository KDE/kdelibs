#include "kformulatoolbar.h"
#include <kiconloader.h>
#include <kglobal.h>
#include <kstddirs.h>

KFormulaToolBar::KFormulaToolBar(QWidget *parent, const char *name, int _item_size)
  : KToolBar(parent, name, _item_size)
{
  // doesn't matter if we do this twice - KStandardDirs ignores doubled additions
  KGlobal::dirs()->addResourceType("toolbar", KStandardDirs::kde_default("data") + "kformula/pics/");
  insertButton(BarIcon("editcut"), CUT_CHAR, true, "Cut");
  insertButton(BarIcon("editcopy"), COPY_CHAR, true, "Copy");
  insertButton(BarIcon("editpaste"), PASTE_CHAR, true, "Paste");

  insertSeparator();

  insertButton(BarIcon("undo"), UNDO_CHAR, true, "Undo");
  insertButton(BarIcon("redo"), REDO_CHAR, true, "Redo");

  insertSeparator();

  insertButton(BarIcon("index2"), POWER, true, "Power");
  insertButton(BarIcon("index3"), SUB, true, "Subscript");
  insertButton(BarIcon("bra"), PAREN, true, "Parentheses");
  insertButton(BarIcon("abs"), ABS, true, "Absolute value");
  insertButton(BarIcon("brackets"), BRACKET, true, "Brackets");
  insertButton(BarIcon("frac"), DIVIDE, true, "Fraction");
  insertButton(BarIcon("root"), SQRT, true, "Root");
  insertButton(BarIcon("integral"), INTEGRAL, true, "Integral");
  insertButton(BarIcon("matrix"), MATRIX, true, "Matrix");
  insertButton(BarIcon("index0"), LSUP, true, "Left superscript");
  insertButton(BarIcon("index1"), LSUB, true, "Left subscript");
}


void KFormulaToolBar::connectToFormula(KFormulaEdit *formula)
{
  QObject::connect(this, SIGNAL(clicked(int)), formula, SLOT(insertChar(int)));
}

#include "kformulatoolbar.moc"
