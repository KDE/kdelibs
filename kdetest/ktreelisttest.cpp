/* outline.cpp
-------------------------------------------------------------------
$Id$
  
outline class implementations
An example using the QktsTreeListWidget

Copyright (C) 1996 Keith Brown and KtSoft

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABLILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details. You should have received a
copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc, 675 Mass Ave, 
Cambridge, MA 02139, USA.
  
-------------------------------------------------------------------
*/

#include "ktreelisttest.h"
#include "ktreelisttest.h"

OutlineWidget::OutlineWidget(QWidget* parent)
	: QWidget(parent, 0) 
{
	outlineTree = new KTreeList( this );
	collapseButton = new QPushButton( this );
	expandButton = new QPushButton( this );
	expandLevelLabel = new QLabel( this );
	upButton = new QPushButton( this );
	outButton = new QPushButton( this );
	inButton = new QPushButton( this );
	downButton = new QPushButton( this );
	expandOption = new QCheckBox( this );
	treeOption = new QCheckBox( this );
	quitButton = new QPushButton( this );
	itemEdit = new QLineEdit( this );
	addButton = new QPushButton( this );
	subButton = new QPushButton( this );
	changeButton = new QPushButton( this );
	delButton = new QPushButton( this );
	insertButton = new QPushButton( this );
	
	QGroupBox* tmpQGroupBox;
	tmpQGroupBox = new QGroupBox( this );
	tmpQGroupBox->setGeometry( 210, 210, 180, 150 );
	tmpQGroupBox->setFrameStyle( 34 );
	tmpQGroupBox->setTitle( "Item Position" );
	tmpQGroupBox->setAlignment( 1 );

	tmpQGroupBox = new QGroupBox( this );
	tmpQGroupBox->setGeometry( 210, 10, 180, 90 );
	tmpQGroupBox->setFrameStyle( 34 );
	tmpQGroupBox->setTitle( "Expansion Level" );
	tmpQGroupBox->setAlignment( 1 );

	tmpQGroupBox = new QGroupBox( this );
	tmpQGroupBox->setGeometry( 210, 110, 180, 80 );
	tmpQGroupBox->setFrameStyle( 34 );
	tmpQGroupBox->setTitle( "Display Options" );
	tmpQGroupBox->setAlignment( 1 );

	outlineTree->raise();
	outlineTree->setGeometry( 10, 10, 190, 230 );
	outlineTree->setFrameStyle( 51 );
	outlineTree->setLineWidth( 2 );
	outlineTree->setTreeDrawing(FALSE);
	outlineTree->setExpandButtonDrawing(FALSE);

	collapseButton->raise();
	collapseButton->setGeometry( 220, 40, 50, 30 );
	collapseButton->setText( "<<" );

	expandButton->raise();
	expandButton->setGeometry( 330, 40, 50, 30 );
	expandButton->setText( ">>" );

	expandLevelLabel->raise();
	expandLevelLabel->setGeometry( 280, 40, 40, 30 );
	expandLevelLabel->setFrameStyle( 50 );
	expandLevelLabel->setText( "" );
	expandLevelLabel->setAlignment( 36 );
  expandLevelLabel->setText("0");

	upButton->raise();
	upButton->setGeometry( 280, 230, 40, 30 );
	upButton->setText( "Up" );

	outButton->raise();
	outButton->setGeometry( 230, 270, 40, 30 );
	outButton->setText( "Out" );

	inButton->raise();
	inButton->setGeometry( 330, 270, 40, 30 );
	inButton->setText( "In" );

	downButton->raise();
	downButton->setGeometry( 280, 310, 40, 30 );
	downButton->setText( "Down" );

	expandOption->raise();
	expandOption->setGeometry( 220, 160, 160, 20 );
	expandOption->setText( "Expand buttons" );

	treeOption->raise();
	treeOption->setGeometry( 220, 130, 160, 20 );
	treeOption->setText( "Tree branches" );

	quitButton->raise();
	quitButton->setGeometry( 150, 330, 50, 30 );
	quitButton->setText( "Quit" );

	itemEdit->raise();
	itemEdit->setGeometry( 10, 250, 190, 30 );
	itemEdit->setText( "" );

	addButton->raise();
	addButton->setGeometry( 10, 290, 50, 30 );
	addButton->setText( "Add" );

	subButton->raise();
	subButton->setGeometry( 150, 290, 50, 30 );
	subButton->setText( "Sub" );

	changeButton->raise();
	changeButton->setGeometry( 10, 330, 50, 30 );
	changeButton->setText( "Chg" );

	delButton->raise();
	delButton->setGeometry( 80, 330, 50, 30 );
	delButton->setText( "Del" );

	insertButton->raise();
	insertButton->setGeometry( 80, 290, 50, 30 );
	insertButton->setText( "Ins" );

	resize( 397, 370 );
	
	connect(quitButton, SIGNAL(clicked()),
		qApp, SLOT(quit()));
	connect(addButton, SIGNAL(clicked()),
		this, SLOT(addItem()));
  connect(changeButton, SIGNAL(clicked()),
    this, SLOT(changeItem()));
	connect(subButton, SIGNAL(clicked()),
	  this, SLOT(addSubItem()));
	connect(collapseButton, SIGNAL(clicked()),
	  this, SLOT(collapse()));
	connect(expandButton, SIGNAL(clicked()),
	  this, SLOT(expand()));
	connect(insertButton, SIGNAL(clicked()),
	  this, SLOT(insertItem()));
	connect(downButton, SIGNAL(clicked()),
	  this, SLOT(itemDown()));
	connect(inButton, SIGNAL(clicked()),
	  this, SLOT(itemIn()));
	connect(outButton, SIGNAL(clicked()),
	  this, SLOT(itemOut()));
	connect(upButton, SIGNAL(clicked()),
	  this, SLOT(itemUp()));
	connect(delButton, SIGNAL(clicked()),
	  this, SLOT(removeItem()));
	connect(expandOption, SIGNAL(toggled(bool)),
		this, SLOT(toggleExpandButtons(bool)));
	connect(treeOption, SIGNAL(toggled(bool)),
		this, SLOT(toggleTree(bool)));
}

// simply appends a new item to the tree at the root level
void OutlineWidget::addItem()
{
  if(itemEdit->text())
    outlineTree->insertItem(itemEdit->text(), 0, -1);
  itemEdit->setText("");
}

// adds a new item as a child of the currently highlighted item
void OutlineWidget::addSubItem()
{
  if(itemEdit->text())
    outlineTree->addChildItem(itemEdit->text(), 0, outlineTree->currentItem());
  itemEdit->setText("");
}

// changes the text of the currently highlighted item to the text in 
// the edit box
void OutlineWidget::changeItem()
{
  if(itemEdit->text())
    outlineTree->changeItem(itemEdit->text(), 0, outlineTree->currentItem());
  itemEdit->setText("");
}

// collapses the tree globally to the next level
void OutlineWidget::collapse()
{
  int expansion = outlineTree->expandLevel();
  if(expansion > 0) {
    outlineTree->setExpandLevel(--expansion);
    char num[8];
    sprintf(num, "%d", expansion);
    expandLevelLabel->setText(num);
  }
}

// expands the tree globally to the next level
void OutlineWidget::expand()
{
  int expansion = outlineTree->expandLevel();
  outlineTree->setExpandLevel(++expansion);
  char num[8];
  sprintf(num, "%d", expansion);
  expandLevelLabel->setText(num);
}
 
// inserts a new item ahead of the current item, at the same level in the tree
void OutlineWidget::insertItem()
{
  if(itemEdit->text())
    outlineTree->insertItem(itemEdit->text(), 0, outlineTree->currentItem());
  itemEdit->setText("");
}

// moves the current item down one slot in its branch
void OutlineWidget::itemDown()
{
  int index = outlineTree->currentItem();
  if(index >= 0)
    outlineTree->lowerItem(index);
}

// indents the current item one level, splitting it into a new branch
void OutlineWidget::itemIn()
{
  outlineTree->split(outlineTree->currentItem());
}

// outdents the current item one level, making it a sibling of its parent
void OutlineWidget::itemOut()
{
  outlineTree->join(outlineTree->currentItem());
}

// moves the current item up one slot in its branch
void OutlineWidget::itemUp()
{
  int index = outlineTree->currentItem();
  if(index >= 0)
    outlineTree->raiseItem(index);
}

// toggles expand button drawing on/off
void OutlineWidget::toggleExpandButtons(bool on)
{
 outlineTree->setExpandButtonDrawing(on);
}

// toggles tree branch drawing on/off
void OutlineWidget::toggleTree(bool on)
{
 outlineTree->setTreeDrawing(on);
}

// removes the current item from the tree
void OutlineWidget::removeItem()
{
  outlineTree->removeItem(outlineTree->currentItem());
}

int main(int argc, char **argv)
{
  KApplication app(argc, argv);
  OutlineWidget outline;
  app.setMainWidget(&outline);
  outline.show();
  return app.exec();
}
#include "ktreelisttest.moc"

