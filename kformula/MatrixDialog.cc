#include "MatrixDialog.h"
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

#define DEFAULT_SIZE 3
#define MAX_SIZE     20

//dialog constructor--make separate file!
MatrixDialog::MatrixDialog(QWidget *parent)
  : QDialog(parent, "Matrix Dialog", TRUE)
{
  w = h = DEFAULT_SIZE;

  zeroFill = FALSE;

  QPushButton *ok, *cancel;
  QLabel *rows, *columns;

  QGridLayout *grid = new QGridLayout(this, 4, 2, 10);

  rows = new QLabel("Rows:", this);
  columns = new QLabel("Columns:", this);

  grid->addWidget(rows, 0, 0);
  grid->addWidget(columns, 0, 1);

  QSpinBox *width, *height;

  width = new QSpinBox(1, MAX_SIZE, 1, this);
  grid->addWidget(width, 1, 0);
  width->setValue(DEFAULT_SIZE);
  connect(width, SIGNAL(valueChanged(int)), SLOT(setWidth(int)));

  height = new QSpinBox(1, MAX_SIZE, 1, this);
  grid->addWidget(height, 1, 1);
  height->setValue(DEFAULT_SIZE);
  connect(height, SIGNAL(valueChanged(int)), SLOT(setHeight(int)));

  QCheckBox *fill = new QCheckBox("Zero-Fill", this);
  grid->addMultiCellWidget(fill, 2, 2, 0, 1);
  connect(fill, SIGNAL(toggled(bool)), SLOT(setFill(bool)));

  ok = new QPushButton("OK", this);
  ok->setDefault(TRUE);
  grid->addWidget(ok, 3, 0);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  
  cancel = new QPushButton("Cancel", this);
  grid->addWidget(cancel, 3, 1);
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));

}

void MatrixDialog::setHeight(int value)
{
  h = value;
}

void MatrixDialog::setWidth(int value)
{
  w = value;
}

void MatrixDialog::setFill(bool value)
{
  zeroFill = value;
}

#include "MatrixDialog.moc"
