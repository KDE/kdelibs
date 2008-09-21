#include "FormulaWidget.h"

FormulaWidget::FormulaWidget( QWidget* parent ) : QWidget( parent )
{
    m_layout = new QVBoxLayout();
    m_formula = new KFormula( this );
    m_text = new KTextEdit( this );

    m_layout->addWidget(m_formula);
    m_layout->addWidget(m_text);
    setLayout(m_layout);

    m_text->connect(m_text, SIGNAL(textChanged()),
                    this, SLOT(setMathML()));

    m_text->setPlainText("<math><mi>x</mi><mo>+</mo><mi>y</mi></math>");
}

FormulaWidget::~FormulaWidget()
{
    delete m_formula;
    delete m_text;
    delete m_layout;
}

void FormulaWidget::setMathML()
{
    m_formula->setMathML(m_text->toPlainText());
    m_formula->update();
}
