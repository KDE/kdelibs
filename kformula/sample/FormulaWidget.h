#ifndef _FORMULA_WIDGET_H_
#define _FORMULA_WIDGET_H_

#include <QWidget>
#include <QVBoxLayout>
#include <KTextEdit>
#include <KFormula.h>

class FormulaWidget : public QWidget
{
    Q_OBJECT

public:
    FormulaWidget( QWidget* parent = 0 );
    ~FormulaWidget();

private:
    QVBoxLayout* m_layout;
    KFormula* m_formula;
    KTextEdit* m_text;
    
private Q_SLOTS:
    void setMathML();

};

#endif // _FORMULA_WIDGET_H_
