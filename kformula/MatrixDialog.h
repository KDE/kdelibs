#ifndef MATRIXDIALOG_H_INCLUDED
#define MATRIXDIALOG_H_INCLUDED

#include <qwidget.h>
#include <qdialog.h>
#include <qvalidator.h>

//  Here's a dialog for entering matrix sizes:
class MatrixDialog : public QDialog
{
  Q_OBJECT
public:
  int w, h;
  bool zeroFill;

  MatrixDialog(QWidget *parent);

  ~MatrixDialog() {}

protected slots:
  void setWidth(int value);
  void setHeight(int value);
  void setFill(bool value);

};

#endif // MATRIXDIALOG_H_INCLUDED
