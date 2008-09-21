#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <KXmlGuiWindow>
#include "FormulaWidget.h"

class MainWindow : public KXmlGuiWindow
{
public:
    MainWindow( QWidget* parent = 0 );

private:
    FormulaWidget* m_formula;
};

#endif // _MAINWINDOW_
