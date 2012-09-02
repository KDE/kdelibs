/*
 * This file is part of the proxy model test suite.
 *
 * Copyright 2009  Stephen Kelly <steveire@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */


#ifndef PROXYMODELTESTWIDGET_H
#define PROXYMODELTESTWIDGET_H

#include <QWidget>

class DynamicTreeModel;
class ModelCommander;
class QPushButton;

class ProxyModelTestWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ProxyModelTestWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

private:
  DynamicTreeModel *m_rootModel;
};

#endif

