/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "modeleventlogger.h"
#include "modeldumper.h"
#include "indexfinder.h"

#include <QStringList>
#include <QFile>

#include <kdebug.h>

#ifdef Grantlee_FOUND
#include <grantlee_core.h>
#include "grantlee_paths.h"

/**
  Don't escape the code generation output.

  'const QString &' should not become 'const QString &amp;'
*/
class NoEscapeOutputStream : public Grantlee::OutputStream
{
public:
  NoEscapeOutputStream()
    : Grantlee::OutputStream()
  {

  }

  NoEscapeOutputStream(QTextStream* stream)
    : OutputStream( stream )
  {

  }

  virtual QSharedPointer< Grantlee::OutputStream > clone() const {
    return QSharedPointer<Grantlee::OutputStream>( new NoEscapeOutputStream );
  }

  virtual QString escape( const QString& input ) const {
    return input;
  }
};
#endif


ModelEvent::ModelEvent(QObject* parent)
  : QObject(parent)
{

}

static const char * const sTypes[] = {
  "Init",
  "RowsInserted",
  "RowsRemoved",
  "DataChanged",
  "LayoutChanged",
  "ModelReset"
};

QString ModelEvent::type() const
{
  return QLatin1String(*(sTypes + m_type));
}

// ModelEvent::Type ModelEvent::type() const
// {
//   return m_type;
// }

void ModelEvent::setType(ModelEvent::Type type)
{
  m_type = type;
}

int ModelEvent::start() const
{
  return m_start;
}

void ModelEvent::setStart(int start)
{
  m_start = start;
}

int ModelEvent::end() const
{
  return m_end;
}

void ModelEvent::setEnd(int end)
{
  m_end = end;
}

QString ModelEvent::rowAncestors() const
{
  QString result("QList<int>()");

  foreach( const int row, m_rowAncestors)
  {
    result.append(" << ");
    result.append(QString::number(row));
  }
  return result;
}

// QList< int > ModelEvent::rowAncestors() const
// {
//   return m_rowAncestors;
// }

void ModelEvent::setRowAncestors(QList< int > rowAncestors)
{
  m_rowAncestors = rowAncestors;
}

bool ModelEvent::hasInterpretString() const
{
  return !m_interpretString.isEmpty();
}

QString ModelEvent::interpretString() const
{
  return m_interpretString;
}

void ModelEvent::setInterpretString(const QString& interpretString)
{
  m_interpretString = interpretString;
}

ModelEventLogger::ModelEventLogger(QAbstractItemModel *model, QObject* parent)
  : QObject(parent), m_model(model), m_modelDumper(new ModelDumper)
{
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged(QModelIndex,QModelIndex)));
  connect(model, SIGNAL(layoutAboutToBeChanged()), SLOT(layoutAboutToBeChanged()));
  connect(model, SIGNAL(layoutChanged()), SLOT(layoutChanged()));
  connect(model, SIGNAL(modelReset()), SLOT(modelReset()));
  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)));
  connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(rowsRemoved(QModelIndex,int,int)));

  ModelEvent *modelEvent = new ModelEvent(this);
  modelEvent->setType(ModelEvent::Init);
  modelEvent->setInterpretString(m_modelDumper->dumpModel(model));

  m_initEvent = QVariant::fromValue(static_cast<QObject*>(modelEvent));

}

void ModelEventLogger::writeLog()
{
#ifdef Grantlee_FOUND
  static int numLogs;
  QString logFileName = QString("main.%1.%2.cpp").arg(reinterpret_cast<qint64>(this)).arg(numLogs++);
  kDebug() << "Writing to " << logFileName;
  QFile outputFile(logFileName);
  const bool logFileOpened = outputFile.open(QFile::WriteOnly | QFile::Text);
  Q_ASSERT(logFileOpened);

  Grantlee::Engine engine;
  Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader);
  loader->setTemplateDirs(QStringList() << ":/templates");
  engine.addTemplateLoader(loader);
  engine.setPluginPaths(QStringList() << GRANTLEE_PLUGIN_PATH);

  // Write out.
  Grantlee::Template t = engine.loadByName("main.cpp");
  if (!t->error())
  {
    Grantlee::Context c;
    c.insert("initEvent", m_initEvent);
    c.insert("events", m_events);

    QTextStream textStream(&outputFile);
    NoEscapeOutputStream outputStream(&textStream);
    t->render(&outputStream, &c);
  }
  outputFile.close();

  if (t->error())
    kDebug() << t->errorString();
#else
  kDebug() << "Grantlee not found. No log written.";
#endif
}

ModelEventLogger::~ModelEventLogger()
{
  writeLog();
  delete m_modelDumper;
}

void ModelEventLogger::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  ModelEvent *modelEvent = new ModelEvent(this);
  modelEvent->setType(ModelEvent::DataChanged);
  modelEvent->setRowAncestors(IndexFinder::indexToIndexFinder(topLeft.parent()).rows());
  modelEvent->setStart(topLeft.row());
  modelEvent->setEnd(bottomRight.row());

  m_events.append(QVariant::fromValue(static_cast<QObject*>(modelEvent)));
}

void ModelEventLogger::persistChildren(const QModelIndex &parent)
{
  for (int row = 0; row < m_model->rowCount(parent); ++row)
  {
    static const int column = 0;
    const QModelIndex idx = m_model->index(row, column, parent);
    m_persistentIndexes.append(idx);
    m_oldPaths.append(IndexFinder::indexToIndexFinder(idx).rows());
    if (m_model->hasChildren(idx))
      persistChildren(idx);
  }
}

void ModelEventLogger::layoutAboutToBeChanged()
{
  m_oldPaths.clear();
  persistChildren(QModelIndex());
}

void ModelEventLogger::layoutChanged()
{
  ModelEvent *modelEvent = new ModelEvent(this);
  modelEvent->setType(ModelEvent::LayoutChanged);
  modelEvent->setInterpretString(m_modelDumper->dumpModel(m_model));

  QList<ModelEvent::PersistentChange> changes;

  for( int i = 0; i < m_persistentIndexes.size(); ++i)
  {
    const QPersistentModelIndex pIdx = m_persistentIndexes.at(i);
    if (!pIdx.isValid())
    {
      ModelEvent::PersistentChange change;
      change.newPath = QList<int>();
      change.oldPath = m_oldPaths.at(i);
      changes.append(change);
      continue;
    }
    const QList<int> rows = IndexFinder::indexToIndexFinder(pIdx).rows();
    if (m_oldPaths.at(i) == rows)
      continue;

    ModelEvent::PersistentChange change;
    change.newPath = rows;
    change.oldPath = m_oldPaths.at(i);
    changes.append(change);
  }

  modelEvent->setChanges(changes);

  m_events.append(QVariant::fromValue(static_cast<QObject*>(modelEvent)));
}

void ModelEventLogger::modelReset()
{
  ModelEvent *modelEvent = new ModelEvent(this);
  modelEvent->setType(ModelEvent::LayoutChanged);
  modelEvent->setInterpretString(m_modelDumper->dumpModel(m_model));

  m_events.append(QVariant::fromValue(static_cast<QObject*>(modelEvent)));
}

void ModelEventLogger::rowsInserted(const QModelIndex& parent, int start, int end)
{
  ModelEvent *modelEvent = new ModelEvent(this);
  modelEvent->setType(ModelEvent::RowsInserted);
  modelEvent->setRowAncestors(IndexFinder::indexToIndexFinder(parent).rows());
  modelEvent->setStart(start);
  QString s = m_modelDumper->dumpTree(m_model, parent, start, end);
  modelEvent->setInterpretString(s);

  m_events.append(QVariant::fromValue(static_cast<QObject*>(modelEvent)));
}

void ModelEventLogger::rowsRemoved(const QModelIndex& parent, int start, int end)
{
  ModelEvent *modelEvent = new ModelEvent(this);
  modelEvent->setType(ModelEvent::RowsRemoved);
  modelEvent->setRowAncestors(IndexFinder::indexToIndexFinder(parent).rows());
  modelEvent->setStart(start);
  modelEvent->setEnd(end);

  m_events.append(QVariant::fromValue(static_cast<QObject*>(modelEvent)));
}
