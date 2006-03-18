/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef TEST_REGRESSION_WINDOW_H
#define TEST_REGRESSION_WINDOW_H

#include <kurl.h>
#include <kio/job.h>

#include <QQueue>
#include <QProcess>

#include "test_regression_gui.h"

class TestRegressionWindow : public QMainWindow
{
Q_OBJECT

public:
	TestRegressionWindow(QWidget *parent = 0);
	virtual ~TestRegressionWindow();

private Q_SLOTS:
	void toggleJSTests(bool checked);
	void toggleHTMLTests(bool checked);
	void toggleDebugOutput(bool checked);
	void toggleNoXvfbUse(bool checked);

	void setTestsDirectory();
	void setKHTMLDirectory();
	void setOutputDirectory();
		
	void directoryListingResult(KIO::Job *job, const KIO::UDSEntryList &list);
	void directoryListingFinished(KIO::Job *job);

	void pauseContinueButtonClicked();
	void saveLogButtonClicked();

	void treeWidgetContextMenuRequested(const QPoint &pos);

	void runTests();
	void runSingleTest();

	void processQueue();

	void testerExited(int exitCode, QProcess::ExitStatus exitStatus);
	void testerReceivedData();

	void addToIgnores();
	void removeFromIgnores();

private: // Helpers
	enum TestResult
	{
		Unknown			= 0,
		Crash			= 1,
		Pass			= 2,
		PassUnexpected	= 3,
		Fail			= 4,
		FailKnown		= 5
	};

	void initLegend();
	void initTestsDirectory();
	void initRegressionTesting(const QString &testFileName);
	void updateItemStatus(TestResult result, QTreeWidgetItem *item, const QString &testFileName);
	void updateLogOutput(const QString &data);
	void parseRegressionTestingOutput(QString data, TestResult result, const QString &cacheName);

	unsigned long countLogLines() const;
	unsigned long countMapItems(const QMap<QString, QStringList > &map) const;

	QString extractTestNameFromData(QString &data, TestResult &result) const;
	QStringList readListFile(const QString &fileName) const;
	void writeListFile(const QString &fileName, const QStringList &content) const;

	QString pathFromItem(const QTreeWidgetItem *item) const;

private:
	enum TestFlags
	{
		None		= 0,
		JSTests		= 1,
		HTMLTests	= 3,
		DebugOutput	= 4,
		NoXvfbUse	= 5
	};

	Ui::MainWindow m_ui;

	int m_flags;
	int m_runCounter;
	int m_testCounter;

	KUrl m_khtmlUrl;
	KUrl m_testsUrl;
	KUrl m_outputUrl;
	KUrl m_saveLogUrl;

	// Temporary variables
	TestResult m_lastResult;
	QString m_lastName;

	// Status pixmaps
	QPixmap m_failPixmap;
	QPixmap m_failKnownPixmap;
	QPixmap m_passPixmap;
	QPixmap m_passUnexpectedPixmap;
	QPixmap m_crashPixmap;
	QPixmap m_ignorePixmap;

	QProcess *m_activeProcess;
	QTreeWidgetItem *m_activeTreeItem;

	// Caches
	QMap<QString, QTreeWidgetItem *> m_itemMap;

	QMap<QString, QStringList> m_ignoreMap;
	QMap<QString, QStringList> m_failureMap;
	QMap<QString, QStringList> m_directoryMap;

	bool m_suspended;

	// Processing queue
	bool m_justProcessingQueue;
	QQueue<QString> m_processingQueue;
};

#endif

// vim:ts=4:tw=4:noet
