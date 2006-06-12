#ifndef CONSOLEDOCK_H
#define CONSOLEDOCK_H

#include <QDockWidget>

class QTextEdit;
class QLineEdit;
class QPushButton;

class ConsoleDock : public QDockWidget
{
    Q_OBJECT
public:
    ConsoleDock(QWidget *parent = 0);
    ~ConsoleDock();

private:
    QTextEdit *consoleEdit;
    QLineEdit *consoleInput;
    QPushButton *consoleInputButton;
};

#endif
