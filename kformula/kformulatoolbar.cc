#include "kformulatoolbar.h"
//All the pics borrowed from the original koffice/kformula--thanks to whoever drew them.
#include "pics/mini-bra.xpm"
#include "pics/mini-frac.xpm"
#include "pics/mini-integral.xpm"
#include "pics/mini-root.xpm"
#include "pics/matrix.xpm"
#include "pics/index0.xpm"
#include "pics/index1.xpm"
#include "pics/index2.xpm"
#include "pics/index3.xpm"
#include "pics/delimiter.xpm"

KFormulaToolBar::KFormulaToolBar(QWidget *parent=0L, const char *name=0L, int _item_size = -1)
  : KToolBar(parent, name, _item_size)
{
  insertButton(QPixmap((const char **)index2), POWER, true, "Power");
  insertButton(QPixmap((const char **)index3), SUB, true, "Subscript");
  insertButton(QPixmap((const char **)mini_bra_xpm), PAREN, true, "Parentheses");
  insertButton(QPixmap((const char **)delimiter), BRACKET, true, "Brackets");
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
