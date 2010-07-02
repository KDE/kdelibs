/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef DYNAMICTREEMODEL_H
#define DYNAMICTREEMODEL_H

#include <QAbstractItemModel>

#include <QHash>
#include <QList>

#include <QDebug>

#include <kdebug.h>
#include "indexfinder.h"

#include "proxymodeltestsuite_export.h"

template<typename T> class QList;

class ModelMoveCommand;

class PROXYMODELTESTSUITE_EXPORT DynamicTreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  enum Roles
  {
    DynamicTreeModelId = Qt::UserRole,

    LastRole
  };

  explicit DynamicTreeModel(QObject *parent = 0);

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &index = QModelIndex()) const;
  int columnCount(const QModelIndex &index = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
  Qt::ItemFlags flags(const QModelIndex& index) const;
  Qt::DropActions supportedDropActions() const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits = 1,
                        Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

  void clear();
  QList<int> indexToPath(const QModelIndex &idx) const;
  ModelMoveCommand* getMoveCommand(const QList<int> &srcPath, int startRow, int endRow);

protected slots:

  /**
  Finds the parent id of the string with id @p searchId.

  Returns -1 if not found.
  */
  qint64 findParentId(qint64 searchId) const;

private:
  QHash<qint64, QString> m_items;
  QHash<qint64, QList<QList<qint64> > > m_childItems;
  qint64 nextId;
  qint64 newId() { return nextId++; };

  QModelIndex m_nextParentIndex;
  int m_nextRow;

  int m_depth;
  int maxDepth;

  friend class ModelInsertCommand;
  friend class ModelInsertWithDescendantsCommand;
  friend class ModelRemoveCommand;
  friend class ModelDataChangeCommand;
  friend class ModelMoveCommand;
  friend class ModelMoveLayoutChangeCommand;
  friend class ModelResetCommand;
  friend class ModelLayoutChangeCommand;
//   friend class ModelSortIndexCommand;
  friend class ModelSortIndexLayoutChangeCommand;
  friend class ModelInsertAndRemoveQueuedCommand;

};


class PROXYMODELTESTSUITE_EXPORT ModelChangeCommand : public QObject
{
  Q_OBJECT
public:

  ModelChangeCommand( DynamicTreeModel *model, QObject *parent = 0 );

  virtual ~ModelChangeCommand() {}

  void setAncestorRowNumbers(const QList<int> &rowNumbers) { m_rowNumbers = rowNumbers; }
  QList<int> srcAncestors() const { return m_rowNumbers; }

  QModelIndex findIndex(const QList<int> &rows) const;

  void setStartRow(int row) { m_startRow = row; }

  void setEndRow(int row) { m_endRow = row; }

  void setNumCols(int cols) { m_numCols = cols; }

  virtual void doCommand() = 0;

  QModelIndex parentIndex() const { return findIndex(m_rowNumbers); }
  int startRow() const { return m_startRow; }
  int endRow() const { return m_endRow; }

protected:
  DynamicTreeModel* m_model;
  QList<int> m_rowNumbers;
  int m_startRow;
  int m_endRow;
  int m_numCols;

};

typedef QList<ModelChangeCommand*> ModelChangeCommandList;

/**
  @brief Inserts a sub tree into the dynamictreemodel.

  As an alternative to setStartRow and setEndRow, the interpret command may be used.

  The interpret command is used to set the structure of the subtree.

  For example,
  @code
  cmd = new ModelInsertCommand(m_model, this);
  cmd->interpret(
    "- A"
    "- B"
    "- - C"
    "- D"
  );
  @endcode

  Will emit an insert for 3 rows, the second of which will have a child row. The interpretation
  string may be complex as long as it is valid. The text at the end of each row does not need to be consistent.
  There is a define DUMPTREE to make this command print the tree it inserts for better readability.

  @code
  cmd->interpret(
    "- A"
    "- - B"
    "- - C"
    "- - - C"
    "- - C"
    "- - - C"
    "- - - C"
    "- - C"
    "- D"
    "- - E"
    "- - F"
  );
  @endcode

  The string is valid if (depth of row (N + 1)) <= ( (depth of row N) + 1). For example, the following is invalid
  because the depth of B is 2 and the depth of A is 0.

  @code
  cmd->interpret(
    "- A"
    "- - - B"
    "- - C"
  @endcode
*/
class PROXYMODELTESTSUITE_EXPORT ModelInsertCommand : public ModelChangeCommand
{
  Q_OBJECT

  struct Token
  {
    enum Type { Branch, Leaf };
    Type type;
    QString content;
  };

public:

  explicit ModelInsertCommand(DynamicTreeModel *model, QObject *parent = 0 );
  virtual ~ModelInsertCommand() {}

  void interpret(const QString &treeString);

  virtual void doCommand();
  void doInsertTree(const QModelIndex &parent);

protected:
  QList<Token> tokenize(const QString &treeString) const;

  QList<int> getDepths(const QString &treeString) const;

  QString m_treeString;
};

class PROXYMODELTESTSUITE_EXPORT ModelInsertAndRemoveQueuedCommand : public ModelChangeCommand
{
  Q_OBJECT

public:

  explicit ModelInsertAndRemoveQueuedCommand(DynamicTreeModel *model, QObject *parent = 0 );
  virtual ~ModelInsertAndRemoveQueuedCommand() {}

  virtual void doCommand();

signals:
  void beginInsertRows(const QModelIndex &parent, int start, int end);
  void endInsertRows();
  void beginRemoveRows(const QModelIndex &parent, int start, int end);
  void endRemoveRows();

protected slots:
  void queuedBeginInsertRows(const QModelIndex &parent, int start, int end);
  void queuedEndInsertRows();
  void queuedBeginRemoveRows(const QModelIndex &parent, int start, int end);
  void queuedEndRemoveRows();

protected:
  void purgeItem(qint64 parent);
};

class PROXYMODELTESTSUITE_EXPORT ModelRemoveCommand : public ModelChangeCommand
{
  Q_OBJECT
public:
  explicit ModelRemoveCommand(DynamicTreeModel *model, QObject *parent = 0 );
  virtual ~ModelRemoveCommand() {}

  virtual void doCommand();

  void purgeItem(qint64 parent);
};

class PROXYMODELTESTSUITE_EXPORT ModelDataChangeCommand : public ModelChangeCommand
{
  Q_OBJECT
public:
  explicit ModelDataChangeCommand(DynamicTreeModel *model, QObject *parent = 0);

  virtual ~ModelDataChangeCommand() {}

  virtual void doCommand();

  void setStartColumn(int column) { m_startColumn = column; }

protected:
  int m_startColumn;
};

class PROXYMODELTESTSUITE_EXPORT ModelMoveCommand : public ModelChangeCommand
{
  Q_OBJECT
public:
  explicit ModelMoveCommand(DynamicTreeModel *model, QObject *parent);

  virtual ~ModelMoveCommand() {}

  virtual bool emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow);

  virtual void doCommand();

  virtual void emitPostSignal();

  void setDestAncestors( const QList<int> &rows ) { m_destRowNumbers = rows; }
  QList<int> destAncestors() const { return m_destRowNumbers; }

  void setDestRow(int row) { m_destRow = row; }

protected:
  QList<int> m_destRowNumbers;
  int m_destRow;
};

class PROXYMODELTESTSUITE_EXPORT ModelMoveLayoutChangeCommand : public ModelMoveCommand
{
  Q_OBJECT
public:
  explicit ModelMoveLayoutChangeCommand(DynamicTreeModel* model, QObject* parent);
  virtual ~ModelMoveLayoutChangeCommand();

  virtual bool emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow);

  virtual void emitPostSignal();

  void setEndOfMoveSourceAncestors(const QList<int> &rows ) { m_endOfMoveSourceAncestors = rows; }
  void setEndOfMoveDestAncestors(const QList<int> &rows ) { m_endOfMoveDestAncestors = rows; }

private:
  QModelIndexList m_beforeMoveList;
  QList<int> m_endOfMoveSourceAncestors;
  QList<int> m_endOfMoveDestAncestors;

};

class PROXYMODELTESTSUITE_EXPORT ModelResetCommand : public ModelChangeCommand
{
  Q_OBJECT
public:
  ModelResetCommand(DynamicTreeModel* model, QObject* parent = 0);
  virtual ~ModelResetCommand();

  void setInitialTree(const QString &treeString);

  /* reimp */ void doCommand();
private:
  QString m_treeString;
};


class PROXYMODELTESTSUITE_EXPORT ModelLayoutChangeCommand : public ModelChangeCommand
{
  Q_OBJECT
public:
  ModelLayoutChangeCommand(DynamicTreeModel* model, QObject* parent = 0);
  virtual ~ModelLayoutChangeCommand();

  struct PersistentChange
  {
    QList<int> oldPath;
    QList<int> newPath;
  };

  void setPersistentChanges(const QList<PersistentChange> &changes);

  void setInitialTree(const QString &treeString);

  /* reimp */ void doCommand();
private:
  QString m_treeString;
  QList<PersistentChange> m_changes;
};


#endif
