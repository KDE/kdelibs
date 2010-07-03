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


#ifndef REPARENTINGPM_WIDGET_H
#define REPARENTINGPM_WIDGET_H

#include <QWidget>

#include "kreparentingproxymodel.h"

class DynamicTreeModel;

class ReparentingProxyModelWidget : public QWidget
{
  Q_OBJECT
public:
  ReparentingProxyModelWidget(QWidget *parent = 0);

private:
  DynamicTreeModel *m_rootModel;
};

#endif
