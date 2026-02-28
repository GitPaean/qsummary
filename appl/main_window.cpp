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

#include <appl/main_window.hpp>

#include <QFileDialog>
#include <QDir>
#include <algorithm>
#include <iostream>

MainWindow::MainWindow(std::vector<std::string> arg_vect,
                       SmryAppl::loader_list_type& loaders,
                       SmryAppl::input_list_type chart_input,
                       std::unique_ptr<DerivedSmry>& derived_smry,
                       QWidget *parent)
    : QMainWindow(parent)
{
    m_smryAppl = new SmryAppl(arg_vect, loaders, chart_input, derived_smry, this);
    setCentralWidget(m_smryAppl);

    m_keywordPanel = new KeywordPanel(this);
    m_keywordDock = new QDockWidget("Keyword Browser", this);
    m_keywordDock->setWidget(m_keywordPanel);
    m_keywordDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_keywordDock);

    createMenus();

    updateKeywordPanel();

    connect(m_keywordPanel, &KeywordPanel::keywordSelected,
            this, &MainWindow::onKeywordSelected);
    connect(m_keywordPanel, &KeywordPanel::keywordsPlotRequested,
            this, &MainWindow::onKeywordsPlotRequested);
}

void MainWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* openAction = new QAction("&Open Summary Files...", this);
    openAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFiles);
    fileMenu->addAction(openAction);

    auto* viewMenu = menuBar()->addMenu("&View");

    auto* togglePanelAction = new QAction("Toggle &Keyword Panel", this);
    togglePanelAction->setShortcut(QKeySequence("Ctrl+K"));
    connect(togglePanelAction, &QAction::triggered, this, &MainWindow::onToggleKeywordPanel);
    viewMenu->addAction(togglePanelAction);
}

void MainWindow::onOpenFiles()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this, tr("Open Summary Files"),
        QDir::currentPath(),
        tr("Summary Files (*.SMSPEC *.ESMRY);;SMSPEC Files (*.SMSPEC);;ESMRY Files (*.ESMRY)"));

    if (fileNames.isEmpty())
        return;

    m_smryAppl->openSummaryFiles(fileNames);
    updateKeywordPanel();
}

void MainWindow::onKeywordSelected(const std::string& keyword, int /*smry_ind*/)
{
    if (!m_smryAppl->isSmryLoaded())
        return;

    // Add series for all loaded files that have this keyword (for comparison)
    auto vect_lists = m_smryAppl->getKeywordLists();
    for (size_t i = 0; i < vect_lists.size(); i++) {
        auto& vl = vect_lists[i];
        if (std::find(vl.begin(), vl.end(), keyword) != vl.end()) {
            m_smryAppl->addSeriesFromPanel(keyword, static_cast<int>(i));
        }
    }
}

void MainWindow::onKeywordsPlotRequested(const std::vector<std::pair<std::string, int>>& selections)
{
    if (!m_smryAppl->isSmryLoaded())
        return;

    auto vect_lists = m_smryAppl->getKeywordLists();

    for (const auto& sel : selections) {
        const auto& keyword = sel.first;
        // Add series for all loaded files that have this keyword (for comparison)
        for (size_t i = 0; i < vect_lists.size(); i++) {
            auto& vl = vect_lists[i];
            if (std::find(vl.begin(), vl.end(), keyword) != vl.end()) {
                m_smryAppl->addSeriesFromPanel(keyword, static_cast<int>(i));
            }
        }
    }
}

void MainWindow::onToggleKeywordPanel()
{
    m_keywordDock->setVisible(!m_keywordDock->isVisible());
}

void MainWindow::updateKeywordPanel()
{
    m_keywordPanel->updateKeywords(
        m_smryAppl->getKeywordLists(),
        m_smryAppl->getRootNameList());
}
