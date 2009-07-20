
#ifndef MODEL_COMMANDER_H
#define MODEL_COMMANDER_H

#include "dynamictreemodel.h"

class QAbstractItemModel;

#include <QObject>

class ModelCommander : public QObject
{
  Q_OBJECT
public:
  explicit ModelCommander(DynamicTreeModel *model, QObject *parent);

  void setCommands(QList<QPair<QString, ModelChangeCommandList> > commands);
  void setDefaultCommands();

  void executeUntil(const QString &stopBefore = QString());
  bool hasNextCommand();
  void executeNextCommand();

protected:
  QPair<QString, ModelChangeCommandList> nextCommand();

  void setCommand(const QString &name, ModelChangeCommandList list);

private:
  DynamicTreeModel *m_model;
  QList<QPair<QString, ModelChangeCommandList> > m_commands;
  int m_counter;

};

#endif
