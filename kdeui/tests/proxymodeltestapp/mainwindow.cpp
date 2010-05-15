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

#include "mainwindow.h"

#include <QSplitter>
#include <QTabWidget>

#include "dynamictreemodel.h"

#include "breadcrumbswidget.h"
#include "breadcrumbnavigationwidget.h"
#include "breadcrumbdirectionwidget.h"
#include "descendantpmwidget.h"
#include "selectionpmwidget.h"
#include "statesaverwidget.h"
#include "proxymodeltestwidget.h"
#include "proxyitemselectionwidget.h"
// #include "reparentingpmwidget.h"
#include "recursivefilterpmwidget.h"
#include "lessthanwidget.h"
#include "matchcheckingwidget.h"

MainWindow::MainWindow() : KXmlGuiWindow()
{

  QTabWidget *tabWidget = new QTabWidget( this );
  DescendantProxyModelWidget *descPMWidget = new DescendantProxyModelWidget();
  SelectionProxyWidget *selProxyWidget = new SelectionProxyWidget();
  BreadcrumbsWidget *breadcrumbsWidget = new BreadcrumbsWidget();
  BreadcrumbDirectionWidget *breadcrumbDirectionWidget = new BreadcrumbDirectionWidget();
  BreadcrumbNavigationWidget *breadcrumbNavigationWidget = new BreadcrumbNavigationWidget();
  ProxyItemSelectionWidget *proxyItemSelectionWidget = new ProxyItemSelectionWidget();
  StateSaverWidget *stateSaverWidget = new StateSaverWidget();
//   ReparentingProxyModelWidget *reparentingProxyModelWidget = new ReparentingProxyModelWidget();
  RecursiveFilterProxyWidget *recursive = new RecursiveFilterProxyWidget();
  ProxyModelTestWidget *proxyModelTestWidget = new ProxyModelTestWidget();
  LessThanWidget *lessThanWidget = new LessThanWidget();
  MatchCheckingWidget *matchCheckingWidget = new MatchCheckingWidget();

  tabWidget->addTab(matchCheckingWidget, "Match Checking PM");
  tabWidget->addTab(descPMWidget, "descendant PM");
  tabWidget->addTab(selProxyWidget, "selection PM");
  tabWidget->addTab(breadcrumbsWidget, "Breadcrumbs");
  tabWidget->addTab(breadcrumbNavigationWidget, "Breadcrumb Navigation");
  tabWidget->addTab(breadcrumbDirectionWidget, "Breadcrumb Direction");
  tabWidget->addTab(proxyItemSelectionWidget, "Proxy Item selection");
//   tabWidget->addTab(reparentingProxyModelWidget, "reparenting PM");
  tabWidget->addTab(recursive, "Recursive Filter");
  tabWidget->addTab(lessThanWidget, "Less Than");
  tabWidget->addTab(proxyModelTestWidget, "Proxy Model Test");
  tabWidget->addTab(stateSaverWidget, "State Saver Test");

  setCentralWidget( tabWidget );
}


MainWindow::~MainWindow()
{
}


