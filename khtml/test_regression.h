#ifndef TEST_REGRESSION_H
#define TEST_REGRESSION_H

#include <khtml_part.h>

/**
 * @internal
 */
class RegressionTest : public QObject
{
  Q_OBJECT
public:
  RegressionTest(KHTMLPart *part, QString _sourceFilesDir, QString _resultFilesDir);

  bool testFile(QString filename);
  bool checkFile(QString filename, QByteArray data);

  void runTests();

private:
  KHTMLPart *m_part;
  QString m_sourceFilesDir;
  QString m_resultFilesDir;
};

#endif
