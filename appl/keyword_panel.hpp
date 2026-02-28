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

#ifndef KEYWORD_PANEL_H
#define KEYWORD_PANEL_H

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

#include <vector>
#include <string>
#include <map>

class KeywordPanel : public QWidget
{
    Q_OBJECT

public:
    explicit KeywordPanel(QWidget *parent = nullptr);

    void updateKeywords(const std::vector<std::vector<std::string>>& vect_list,
                        const std::vector<std::string>& root_names);

signals:
    void keywordSelected(const std::string& keyword, int smry_ind);
    void keywordsPlotRequested(const std::vector<std::pair<std::string, int>>& selections);

private slots:
    void onFilterChanged(const QString& text);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onPlotButtonClicked();

private:
    void populateTree(const QString& filter = "");

    std::string categoryName(const std::string& keyword);

    QLineEdit* m_filterEdit;
    QTreeWidget* m_treeWidget;
    QPushButton* m_plotButton;
    QLabel* m_statusLabel;

    std::vector<std::vector<std::string>> m_vect_list;
    std::vector<std::string> m_root_names;
};

#endif // KEYWORD_PANEL_H
