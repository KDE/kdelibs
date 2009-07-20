
#include "modelcommander.h"
#include "dynamictreemodel.h"


ModelCommander::ModelCommander(DynamicTreeModel* model, QObject *parent)
: QObject(parent), m_counter(0), m_model(model)
{
  setDefaultCommands();
}

void ModelCommander::setCommands(QList<QPair<QString, ModelChangeCommandList> > commands)
{
  m_commands = commands;
}

void ModelCommander::executeUntil(const QString& stopBefore)
{
  while(hasNextCommand())
  {
    QPair<QString, ModelChangeCommandList> nextChangeCommand = nextCommand();
    if (nextChangeCommand.first == stopBefore)
    {
      return;
    }
    executeNextCommand();
  }
}

void ModelCommander::executeNextCommand()
{
  QPair<QString, ModelChangeCommandList> nextChangeCommand = nextCommand();

  ++m_counter;

  foreach(ModelChangeCommand *command, nextChangeCommand.second)
  {
    command->doCommand();
  }
}

bool ModelCommander::hasNextCommand()
{
  return m_commands.size() > m_counter;
}

QPair<QString, ModelChangeCommandList> ModelCommander::nextCommand()
{
  return m_commands.at(m_counter);
}

void ModelCommander::setDefaultCommands()
{
  m_counter = 0;

  // Insert a single item at the top.
  ModelInsertCommand *ins;
  ins = new ModelInsertCommand(m_model, this);
  ins->setStartRow(0);
  ins->setEndRow(0);

  ModelChangeCommandList commandList;

  commandList << ins;

  setCommand("insert01", commandList);
  commandList.clear();

  // Give the top level item 10 children.
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 0 );
  ins->setStartRow(0);
  ins->setEndRow(9);

  commandList << ins;
  setCommand("insert02", commandList);
  commandList.clear();

  // Give the top level item 10 'older' siblings.
  ins = new ModelInsertCommand(m_model, this);
  ins->setStartRow(0);
  ins->setEndRow(9);

  commandList << ins;
  setCommand("insert03", commandList);
  commandList.clear();

  // Give the top level item 10 'younger' siblings.
  ins = new ModelInsertCommand(m_model, this);
  ins->setStartRow(11);
  ins->setEndRow(20);

  commandList << ins;
  setCommand("insert04", commandList);
  commandList.clear();

  // Add more children to the top level item.
  // First 'older'
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 );
  ins->setStartRow(0);
  ins->setEndRow(9);

  commandList << ins;

  setCommand("insert05", commandList);
  commandList.clear();

  // Then younger
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 );
  ins->setStartRow(20);
  ins->setEndRow(29);

  commandList << ins;

  setCommand("insert06", commandList);
  commandList.clear();

  // Then somewhere in the middle.
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 );
  ins->setStartRow(10);
  ins->setEndRow(19);

  commandList << ins;

  setCommand("insert07", commandList);
  commandList.clear();

  // Add some more items for removing later.
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 << 5 );
  ins->setStartRow(0);
  ins->setEndRow(9);
  commandList << ins;
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 << 5 << 5 );
  ins->setStartRow(0);
  ins->setEndRow(9);
  commandList << ins;
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 << 5 << 5 << 5 );
  ins->setStartRow(0);
  ins->setEndRow(9);
  commandList << ins;
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 << 6 );
  ins->setStartRow(0);
  ins->setEndRow(9);
  commandList << ins;
  ins = new ModelInsertCommand(m_model, this);
  ins->setAncestorRowNumbers(QList<int>() << 10 << 7 );
  ins->setStartRow(0);
  ins->setEndRow(9);
  commandList << ins;

  setCommand("insert08", commandList);
  commandList.clear();

  // Insert a tree of items in one go.
  ModelInsertWithDescendantsCommand *insWithDescs = new ModelInsertWithDescendantsCommand(m_model, this);
  insWithDescs->setStartRow(2);
  insWithDescs->setAncestorRowNumbers(QList<int>() << 10 );
  QList<ModelInsertWithDescendantsCommand::InsertFragment> fragments;
  ModelInsertWithDescendantsCommand::InsertFragment fragment;

  ModelInsertWithDescendantsCommand::InsertFragment subFragment;
  subFragment.numRows = 10;

  ModelInsertWithDescendantsCommand::InsertFragment subSubFragment;
  subSubFragment.numRows = 10;
  subFragment.subfragments.insert(4, subSubFragment);

  fragment.numRows = 10;
  fragment.subfragments.insert(5, subFragment);
  fragment.subfragments.insert(2, subFragment);
  fragments << fragment;
  insWithDescs->setFragments(fragments );
  commandList << insWithDescs;

  setCommand("insert09", commandList);
  commandList.clear();

  ModelDataChangeCommand *dataChange = new ModelDataChangeCommand(m_model, this);

  dataChange->setAncestorRowNumbers(QList<int>() << 10 );
  dataChange->setStartRow(0);
  dataChange->setEndRow(0);

  commandList << dataChange;

  setCommand("change01", commandList);
  commandList.clear();

  dataChange = new ModelDataChangeCommand(m_model, this);
  dataChange->setAncestorRowNumbers(QList<int>() << 10);
  dataChange->setStartRow(4);
  dataChange->setEndRow(7);

  commandList << dataChange;

  setCommand("change02", commandList);
  commandList.clear();

  ModelRemoveCommand *rem;

  // Remove a single item without children.
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 );
  rem->setStartRow(0);
  rem->setEndRow(0);

  commandList << rem;

  setCommand("remove01", commandList);
  commandList.clear();

  // Remove a single item with 10 children.
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 );
  rem->setStartRow(6);
  rem->setEndRow(6);

  commandList << rem;

  setCommand("remove02", commandList);
  commandList.clear();

  // Remove a single item with no children from the top.
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 << 5 );
  rem->setStartRow(0);
  rem->setEndRow(0);

  commandList << rem;

  setCommand("remove03", commandList);
  commandList.clear();

  // Remove a single second level item with no children from the bottom.
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 << 5 );
  rem->setStartRow(8);
  rem->setEndRow(8);

  commandList << rem;

  setCommand("remove04", commandList);
  commandList.clear();

  // Remove a single second level item with no children from the middle.
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 << 5 );
  rem->setStartRow(3);
  rem->setEndRow(3);

  commandList << rem;

  setCommand("remove05", commandList);
  commandList.clear();

  // clear the children of a second level item.
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 << 5 );
  rem->setStartRow(0);
  rem->setEndRow(6);

  commandList << rem;

  setCommand("remove06", commandList);
  commandList.clear();

  // Clear a sub-tree;
  rem = new ModelRemoveCommand(m_model, this);
  rem->setAncestorRowNumbers(QList<int>() << 10 );
  rem->setStartRow(4);
  rem->setEndRow(4);

  commandList << rem;

  setCommand("remove07", commandList);
  commandList.clear();
}


void ModelCommander::setCommand(const QString &name, ModelChangeCommandList list)
{
  QPair<QString, ModelChangeCommandList> pair = qMakePair(name, list);

  m_commands.append(pair);
}
