/*
   Copyright 2022 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#ifndef RUN_FLOW_WINDOW_H
#define RUN_FLOW_WINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QProcess>
#include <QSplitter>
#include <QTabWidget>

#include <appl/smry_appl.hpp>
#include <appl/main_window.hpp>

class RunFlowWindow : public QMainWindow
{
    Q_OBJECT

public:
    RunFlowWindow(QWidget *parent = nullptr);

private slots:
    void onOpenDataFile();
    void onSaveDataFile();
    void onBrowseFlow();
    void onRunFlow();
    void onStopFlow();

    void onFlowReadyReadStdOut();
    void onFlowReadyReadStdErr();
    void onFlowFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void createWidgets();
    void createMenus();
    void loadSummaryResults();
    void updateButtonStates();

    QSplitter* m_mainSplitter;
    QTabWidget* m_tabWidget;

    QPlainTextEdit* m_editor;
    QPlainTextEdit* m_logOutput;

    QLineEdit* m_flowPathEdit;
    QLineEdit* m_dataFileEdit;

    QPushButton* m_browseFlowBtn;
    QPushButton* m_openDataBtn;
    QPushButton* m_saveDataBtn;
    QPushButton* m_runBtn;
    QPushButton* m_stopBtn;

    QLabel* m_statusLabel;

    QProcess* m_flowProcess;

    QString m_dataFilePath;
    MainWindow* m_summaryWindow;
};

#endif // RUN_FLOW_WINDOW_H
