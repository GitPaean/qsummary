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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <appl/smry_appl.hpp>
#include <appl/keyword_panel.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(std::vector<std::string> arg_vect,
               SmryAppl::loader_list_type& loaders,
               SmryAppl::input_list_type chart_input,
               std::unique_ptr<DerivedSmry>& derived_smry,
               QWidget *parent = nullptr);

    SmryAppl* getSmryAppl() { return m_smryAppl; }

private slots:
    void onOpenFiles();
    void onKeywordSelected(const std::string& keyword, int smry_ind);
    void onKeywordsPlotRequested(const std::vector<std::pair<std::string, int>>& selections);
    void onToggleKeywordPanel();

private:
    void createMenus();
    void updateKeywordPanel();

    SmryAppl* m_smryAppl;
    KeywordPanel* m_keywordPanel;
    QDockWidget* m_keywordDock;
};

#endif // MAIN_WINDOW_H
