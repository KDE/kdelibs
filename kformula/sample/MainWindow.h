#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <kxmlguiwindow.h>

class Ui_SampleFormulaWidget;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    MainWindow( QWidget* parent = 0 );
private Q_SLOTS:
    void textChanged(); 
private:
    Ui_SampleFormulaWidget *ui;
};

#endif // _MAINWINDOW_
