#include "kformulatoolbar.h"
//All the formula pics borrowed from the original koffice/kformula--thanks to whoever drew them.
#include "pics/mini-bra.xpm"
#include "pics/mini-frac.xpm"
#include "pics/mini-integral.xpm"
#include "pics/mini-root.xpm"
#include "pics/matrix.xpm"
#include "pics/index0.xpm"
#include "pics/index1.xpm"
#include "pics/index2.xpm"
#include "pics/index3.xpm"
#include "pics/brackets.xpm"
#include "pics/abs.xpm"
#include "../toolbar/editcut.xpm"
#include "../toolbar/editcopy.xpm"
#include "../toolbar/editpaste.xpm"
#include "../toolbar/undo.xpm"
#include "../toolbar/redo.xpm"

KFormulaToolBar::KFormulaToolBar(QWidget *parent, const char *name, int _item_size)
  : KToolBar(parent, name, _item_size)
{
  insertButton(QPixmap((const char **)editcut), CUT_CHAR, true, "Cut");
  insertButton(QPixmap((const char **)editcopy), COPY_CHAR, true, "Copy");
  insertButton(QPixmap((const char **)editpaste), PASTE_CHAR, true, "Paste");

  insertSeparator();

  insertButton(QPixmap((const char **)undo), UNDO_CHAR, true, "Undo");
  insertButton(QPixmap((const char **)redo), REDO_CHAR, true, "Redo");

  insertSeparator();

  insertButton(QPixmap((const char **)index2), POWER, true, "Power");
  insertButton(QPixmap((const char **)index3), SUB, true, "Subscript");
  insertButton(QPixmap((const char **)mini_bra_xpm), PAREN, true, "Parentheses");
  insertButton(QPixmap((const char **)abs_xpm), ABS, true, "Absolute value");
  insertButton(QPixmap((const char **)brackets_xpm), BRACKET, true, "Brackets");
  insertButton(QPixmap((const char **)mini_frac_xpm), DIVIDE, true, "Fraction");
  insertButton(QPixmap((const char **)mini_root_xpm), SQRT, true, "Root");
  insertButton(QPixmap((const char **)mini_integral_xpm), INTEGRAL, true, "Integral");
  insertButton(QPixmap((const char **)matrix), MATRIX, true, "Matrix");
  insertButton(QPixmap((const char **)index0), LSUP, true, "Left superscript");
  insertButton(QPixmap((const char **)index1), LSUB, true, "Left subscript");
}


void KFormulaToolBar::connectToFormula(KFormulaEdit *formula)
{
  QObject::connect(this, SIGNAL(clicked(int)), formula, SLOT(insertChar(int)));
}

#include "kformulatoolbar.moc"
