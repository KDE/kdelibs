#ifndef LOCALVARIABLEDOCK_H
#define LOCALVARIABLEDOCK_H

#include <QDockWidget>
#include <QTreeWidgetItem>

namespace KJS { class Interpreter; }
class QTreeWidget;

class VariableItem : public QTreeWidgetItem
{
public:
    VariableItem(const QString &name, KJS::JSObject *instance, QTreeWidget *parent = 0)
        : QTreeWidgetItem(parent),
          m_name(name),
          m_instance(instance)
    {}

    QString name() { return m_name; }
    KJS::JSObject *instance() { return m_instance; }

private:
    QString        m_name;
    KJS::JSObject *m_instance;

};


class LocalVariablesDock : public QDockWidget
{
    Q_OBJECT
public:
    LocalVariablesDock(QWidget *parent = 0);
    ~LocalVariablesDock();

    void display(KJS::Interpreter *interpreter);

private:
    QTreeWidget *m_widget;

};

#endif
