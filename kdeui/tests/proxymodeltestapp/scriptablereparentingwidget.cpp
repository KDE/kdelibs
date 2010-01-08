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

#include "scriptablereparentingwidget.h"

#include <QPlainTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QScriptEngine>

#include "kdebug.h"
#include <QLabel>
#include <QSplitter>

ScriptableReparentingProxyModel::ScriptableReparentingProxyModel(QObject* parent)
  : KReparentingProxyModel(parent),
    m_scriptEngine(new QScriptEngine(this))
{

}


bool ScriptableReparentingProxyModel::isDescendantOf(const QModelIndex& ancestor, const QModelIndex& descendant) const
{
  if (!m_implementationFunction.isValid())
    return KReparentingProxyModel::isDescendantOf(ancestor, descendant);

  QScriptValueList arguments = QScriptValueList() << ancestor.data().toInt() << descendant.data().toInt();
  QScriptValue returnValue = m_implementationFunction.call(QScriptValue(), arguments );

  if (!returnValue.isBool())
    return KReparentingProxyModel::isDescendantOf(ancestor, descendant);

  return returnValue.toBool();
}

void ScriptableReparentingProxyModel::setImplementation(const QString& implementation)
{
  beginChangeRule();
  m_implementationFunction = m_scriptEngine->evaluate(implementation);
  m_implementationFunction = m_scriptEngine->globalObject().property("isDescendantOf");
  endChangeRule();
}

ScriptableReparentingWidget::ScriptableReparentingWidget(QAbstractItemModel *rootModel, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_reparentingProxyModel(new ScriptableReparentingProxyModel(this))
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QSplitter *splitter = new QSplitter(Qt::Vertical, this);
  mainLayout->addWidget(splitter);

  m_treeView = new QTreeView(splitter);
  QWidget *container = new QWidget(splitter);
  QVBoxLayout *layout = new QVBoxLayout(container);
  m_textEdit = new QPlainTextEdit(container);
  m_textEdit->setFont(QFont("monospace"));

  layout->addWidget(new QLabel("function isDescendantOf (ancestor, descendant) {", container));
  QHBoxLayout *indentedLayout = new QHBoxLayout(container);
  indentedLayout->addSpacing(30);
  indentedLayout->addWidget(m_textEdit);
  layout->addLayout(indentedLayout);
  layout->addWidget(new QLabel("}", container));

  m_reparentingProxyModel->setSourceModel(rootModel);
  m_treeView->setModel(m_reparentingProxyModel);

  splitter->setStretchFactor(0, 100);

  connect(m_textEdit, SIGNAL(textChanged()), SLOT(textChanged()));
  textChanged();
}


void ScriptableReparentingWidget::textChanged()
{
  m_reparentingProxyModel->setImplementation("function isDescendantOf (ancestor, descendant) { " + m_textEdit->toPlainText()  + " }");
  m_treeView->expandAll();
}


#include "scriptablereparentingwidget.moc"
