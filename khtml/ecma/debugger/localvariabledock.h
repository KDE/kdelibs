#ifndef LOCALVARIABLEDOCK_H
#define LOCALVARIABLEDOCK_H

#include <QDockWidget>

class QTreeView;
class QStandardItemModel;
class ObjectModel;
class ExecStateModel;

class LocalVariablesDock : public QDockWidget
{
    Q_OBJECT
public:
    LocalVariablesDock(QWidget *parent = 0);
    ~LocalVariablesDock();

    void display(KJS::ExecState *exec);
    void clear();

private:
    QTreeView   *m_view;
    ExecStateModel    *m_execModel;

};

#endif
