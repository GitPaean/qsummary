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

#include <appl/run_flow_window.hpp>

#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QTextStream>
#include <QScrollBar>
#include <QStatusBar>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <appl/qsum_func_lib.hpp>
#include <appl/derived_smry.hpp>

#include <iostream>
#include <filesystem>


RunFlowWindow::RunFlowWindow(QWidget *parent)
    : QMainWindow(parent), m_flowProcess(nullptr), m_summaryWindow(nullptr)
{
    createWidgets();
    createMenus();
    updateButtonStates();

    setWindowTitle("run-flow");
    resize(1200, 800);
}

void RunFlowWindow::createWidgets()
{
    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(m_mainSplitter);

    // top: editor area with toolbar
    QWidget* topWidget = new QWidget(this);
    QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(2, 2, 2, 2);

    // toolbar row 1: DATA file
    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(new QLabel("DATA File:", this));
    m_dataFileEdit = new QLineEdit(this);
    m_dataFileEdit->setReadOnly(true);
    fileRow->addWidget(m_dataFileEdit, 1);
    m_openDataBtn = new QPushButton("Open...", this);
    fileRow->addWidget(m_openDataBtn);
    m_saveDataBtn = new QPushButton("Save", this);
    fileRow->addWidget(m_saveDataBtn);
    topLayout->addLayout(fileRow);

    // toolbar row 2: flow executable and run controls
    QHBoxLayout* flowRow = new QHBoxLayout();
    flowRow->addWidget(new QLabel("Flow Executable:", this));
    m_flowPathEdit = new QLineEdit(this);
    m_flowPathEdit->setPlaceholderText("Path to flow executable (e.g. /usr/bin/flow)");
    flowRow->addWidget(m_flowPathEdit, 1);
    m_browseFlowBtn = new QPushButton("Browse...", this);
    flowRow->addWidget(m_browseFlowBtn);
    m_runBtn = new QPushButton("Run", this);
    flowRow->addWidget(m_runBtn);
    m_stopBtn = new QPushButton("Stop", this);
    flowRow->addWidget(m_stopBtn);
    topLayout->addLayout(flowRow);

    // text editor
    m_editor = new QPlainTextEdit(this);
    QFont editorFont("Monospace");
    editorFont.setStyleHint(QFont::Monospace);
    editorFont.setPointSize(10);
    m_editor->setFont(editorFont);
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    topLayout->addWidget(m_editor, 1);

    m_mainSplitter->addWidget(topWidget);

    // bottom: tabbed area for log output
    m_tabWidget = new QTabWidget(this);

    m_logOutput = new QPlainTextEdit(this);
    m_logOutput->setReadOnly(true);
    QFont logFont("Monospace");
    logFont.setStyleHint(QFont::Monospace);
    logFont.setPointSize(9);
    m_logOutput->setFont(logFont);

    m_tabWidget->addTab(m_logOutput, "Flow Output");

    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);

    // status bar
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);

    // connect signals
    connect(m_openDataBtn, &QPushButton::clicked, this, &RunFlowWindow::onOpenDataFile);
    connect(m_saveDataBtn, &QPushButton::clicked, this, &RunFlowWindow::onSaveDataFile);
    connect(m_browseFlowBtn, &QPushButton::clicked, this, &RunFlowWindow::onBrowseFlow);
    connect(m_runBtn, &QPushButton::clicked, this, &RunFlowWindow::onRunFlow);
    connect(m_stopBtn, &QPushButton::clicked, this, &RunFlowWindow::onStopFlow);
}

void RunFlowWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* openAction = new QAction("&Open DATA File...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &RunFlowWindow::onOpenDataFile);
    fileMenu->addAction(openAction);

    auto* saveAction = new QAction("&Save DATA File", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &RunFlowWindow::onSaveDataFile);
    fileMenu->addAction(saveAction);

    auto* runMenu = menuBar()->addMenu("&Run");

    auto* runAction = new QAction("&Run Flow", this);
    runAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(runAction, &QAction::triggered, this, &RunFlowWindow::onRunFlow);
    runMenu->addAction(runAction);

    auto* stopAction = new QAction("&Stop Flow", this);
    connect(stopAction, &QAction::triggered, this, &RunFlowWindow::onStopFlow);
    runMenu->addAction(stopAction);
}

void RunFlowWindow::onOpenDataFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open DATA File"),
        QDir::currentPath(),
        tr("DATA Files (*.DATA);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file: " + fileName);
        return;
    }

    QTextStream in(&file);
    m_editor->setPlainText(in.readAll());
    file.close();

    m_dataFilePath = fileName;
    m_dataFileEdit->setText(fileName);

    m_statusLabel->setText("Loaded: " + fileName);
    updateButtonStates();
}

void RunFlowWindow::onSaveDataFile()
{
    if (m_dataFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No DATA file loaded.");
        return;
    }

    QFile file(m_dataFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot save file: " + m_dataFilePath);
        return;
    }

    QTextStream out(&file);
    out << m_editor->toPlainText();
    file.close();

    m_statusLabel->setText("Saved: " + m_dataFilePath);
}

void RunFlowWindow::onBrowseFlow()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Select Flow Executable"),
        QDir::currentPath(),
        tr("All Files (*)"));

    if (!fileName.isEmpty())
        m_flowPathEdit->setText(fileName);
}

void RunFlowWindow::onRunFlow()
{
    if (m_dataFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please open a DATA file first.");
        return;
    }

    QString flowPath = m_flowPathEdit->text().trimmed();
    if (flowPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify the path to the flow executable.");
        return;
    }

    // save before running
    onSaveDataFile();

    m_logOutput->clear();
    m_logOutput->appendPlainText("Starting flow simulation...");
    m_logOutput->appendPlainText("Executable: " + flowPath);
    m_logOutput->appendPlainText("DATA file: " + m_dataFilePath);
    m_logOutput->appendPlainText("---");

    if (m_flowProcess) {
        m_flowProcess->deleteLater();
        m_flowProcess = nullptr;
    }

    m_flowProcess = new QProcess(this);

    connect(m_flowProcess, &QProcess::readyReadStandardOutput,
            this, &RunFlowWindow::onFlowReadyReadStdOut);
    connect(m_flowProcess, &QProcess::readyReadStandardError,
            this, &RunFlowWindow::onFlowReadyReadStdErr);
    connect(m_flowProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RunFlowWindow::onFlowFinished);

    QFileInfo dataInfo(m_dataFilePath);
    m_flowProcess->setWorkingDirectory(dataInfo.absolutePath());

    QStringList args;
    args << m_dataFilePath;

    m_statusLabel->setText("Running flow simulation...");
    m_flowProcess->start(flowPath, args);

    updateButtonStates();
}

void RunFlowWindow::onStopFlow()
{
    if (m_flowProcess && m_flowProcess->state() != QProcess::NotRunning) {
        m_flowProcess->terminate();
        if (!m_flowProcess->waitForFinished(5000))
            m_flowProcess->kill();
        m_logOutput->appendPlainText("--- Flow simulation stopped by user ---");
        m_statusLabel->setText("Simulation stopped");
    }
}

void RunFlowWindow::onFlowReadyReadStdOut()
{
    if (m_flowProcess) {
        QString output = m_flowProcess->readAllStandardOutput();
        m_logOutput->appendPlainText(output.trimmed());
        m_logOutput->verticalScrollBar()->setValue(m_logOutput->verticalScrollBar()->maximum());
    }
}

void RunFlowWindow::onFlowReadyReadStdErr()
{
    if (m_flowProcess) {
        QString output = m_flowProcess->readAllStandardError();
        m_logOutput->appendPlainText(output.trimmed());
        m_logOutput->verticalScrollBar()->setValue(m_logOutput->verticalScrollBar()->maximum());
    }
}

void RunFlowWindow::onFlowFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        m_logOutput->appendPlainText("--- Flow simulation completed successfully ---");
        m_statusLabel->setText("Simulation completed successfully");
        loadSummaryResults();
    } else {
        m_logOutput->appendPlainText("--- Flow simulation finished with exit code: " + QString::number(exitCode) + " ---");
        m_statusLabel->setText("Simulation finished with errors (exit code: " + QString::number(exitCode) + ")");
    }

    m_flowProcess->deleteLater();
    m_flowProcess = nullptr;
    updateButtonStates();
}

void RunFlowWindow::loadSummaryResults()
{
    QFileInfo dataInfo(m_dataFilePath);
    QString baseName = dataInfo.absolutePath() + "/" + dataInfo.completeBaseName();

    // look for SMSPEC or ESMRY file
    QString smspecFile = baseName + ".SMSPEC";
    QString esmryFile = baseName + ".ESMRY";

    std::string summaryFile;
    FileType ftype;

    if (QFileInfo::exists(smspecFile)) {
        summaryFile = smspecFile.toStdString();
        ftype = FileType::SMSPEC;
    } else if (QFileInfo::exists(esmryFile)) {
        summaryFile = esmryFile.toStdString();
        ftype = FileType::ESMRY;
    } else {
        m_logOutput->appendPlainText("No summary file found to plot.");
        return;
    }

    m_logOutput->appendPlainText("Loading summary results from: " + QString::fromStdString(summaryFile));

    try {
        std::vector<std::filesystem::path> smry_files;
        std::vector<FileType> file_type;
        std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
        std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

        smry_files.push_back(std::filesystem::path(summaryFile));
        file_type.push_back(ftype);

        if (ftype == FileType::SMSPEC) {
            esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>(summaryFile);
        } else {
            lodsmry_loader[0] = std::make_unique<Opm::EclIO::ExtESmry>(summaryFile);
        }

        // build input charts for important vectors: FPR, FOPR, TCPU
        SmryAppl::input_list_type input_charts;

        std::vector<std::string> important_vectors = {"FPR", "FOPR", "TCPU"};

        std::vector<std::string> keywordList;
        if (ftype == FileType::SMSPEC)
            keywordList = esmry_loader[0]->keywordList();
        else
            keywordList = lodsmry_loader[0]->keywordList();

        for (const auto& vect_name : important_vectors) {
            bool found = false;
            for (const auto& key : keywordList) {
                if (key == vect_name) {
                    found = true;
                    break;
                }
            }

            if (found) {
                std::vector<SmryAppl::vect_input_type> vect_list;
                vect_list.push_back(std::make_tuple(0, vect_name, 0, false));
                input_charts.push_back(std::make_tuple(vect_list, ""));
            }
        }

        if (input_charts.empty()) {
            m_logOutput->appendPlainText("No important summary vectors (FPR, FOPR, TCPU) found.");
            return;
        }

        QSum::pre_load_smry(smry_files, input_charts, file_type, esmry_loader, lodsmry_loader, 1);

        std::vector<std::string> arg_vect = {summaryFile};

        SmryAppl::loader_list_type loaders = std::make_tuple(
            std::move(smry_files), std::move(file_type),
            std::move(esmry_loader), std::move(lodsmry_loader));

        std::unique_ptr<DerivedSmry> derived_smry;

        if (m_summaryWindow) {
            m_summaryWindow->close();
            m_summaryWindow->deleteLater();
        }

        m_summaryWindow = new MainWindow(arg_vect, loaders, input_charts, derived_smry);
        m_summaryWindow->resize(1600, 700);
        m_summaryWindow->setWindowTitle("run-flow: Summary Results");
        m_summaryWindow->move(200, 100);
        m_summaryWindow->show();

        m_logOutput->appendPlainText("Summary plots loaded successfully.");

    } catch (const std::exception& e) {
        m_logOutput->appendPlainText("Error loading summary: " + QString::fromStdString(e.what()));
    }
}

void RunFlowWindow::updateButtonStates()
{
    bool hasDataFile = !m_dataFilePath.isEmpty();
    bool isRunning = m_flowProcess && m_flowProcess->state() != QProcess::NotRunning;

    m_saveDataBtn->setEnabled(hasDataFile && !isRunning);
    m_runBtn->setEnabled(hasDataFile && !isRunning);
    m_stopBtn->setEnabled(isRunning);
    m_editor->setReadOnly(isRunning);
}
