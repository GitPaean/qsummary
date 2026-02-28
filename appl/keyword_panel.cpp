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

#include <appl/keyword_panel.hpp>

#include <QHeaderView>
#include <algorithm>
#include <set>

KeywordPanel::KeywordPanel(QWidget *parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    auto* filterLabel = new QLabel("Filter:", this);
    layout->addWidget(filterLabel);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Type to filter keywords...");
    layout->addWidget(m_filterEdit);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({"Keyword", "Case"});
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeWidget->header()->setStretchLastSection(true);
    m_treeWidget->setAlternatingRowColors(true);
    layout->addWidget(m_treeWidget);

    m_plotButton = new QPushButton("Plot Selected", this);
    layout->addWidget(m_plotButton);

    m_statusLabel = new QLabel("", this);
    layout->addWidget(m_statusLabel);

    connect(m_filterEdit, &QLineEdit::textChanged, this, &KeywordPanel::onFilterChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &KeywordPanel::onItemDoubleClicked);
    connect(m_plotButton, &QPushButton::clicked, this, &KeywordPanel::onPlotButtonClicked);
}

std::string KeywordPanel::categoryName(const std::string& keyword)
{
    if (keyword.empty())
        return "Other";

    char first = keyword[0];
    switch (first) {
        case 'F': return "Field";
        case 'W': return "Well";
        case 'G': return "Group";
        case 'A': return "Aquifer";
        case 'B': return "Block";
        case 'R': return "Region";
        case 'S': return "Segment";
        case 'C': return "Connection";
        case 'N': return "Network";
        default:  return "Other";
    }
}

void KeywordPanel::updateKeywords(const std::vector<std::vector<std::string>>& vect_list,
                                   const std::vector<std::string>& root_names)
{
    m_vect_list = vect_list;
    m_root_names = root_names;
    populateTree(m_filterEdit->text());
}

void KeywordPanel::populateTree(const QString& filter)
{
    m_treeWidget->clear();

    std::string filter_upper = filter.toUpper().toStdString();

    // Collect unique keywords across all files
    std::set<std::string> unique_keywords;
    for (size_t i = 0; i < m_vect_list.size(); i++) {
        for (const auto& kw : m_vect_list[i]) {
            if (kw == "TIME" || kw == "YEARS" || kw == "DAY" || kw == "MONTH" || kw == "YEAR")
                continue;

            if (filter_upper.empty() || kw.find(filter_upper) != std::string::npos) {
                unique_keywords.insert(kw);
            }
        }
    }

    // Group by category
    std::map<std::string, std::vector<std::string>> categories;
    for (const auto& kw : unique_keywords) {
        categories[categoryName(kw)].push_back(kw);
    }

    int total_count = 0;

    for (auto& [cat_name, keywords] : categories) {
        auto* catItem = new QTreeWidgetItem(m_treeWidget);
        catItem->setText(0, QString::fromStdString(cat_name) +
                        " (" + QString::number(keywords.size()) + ")");
        catItem->setFlags(catItem->flags() & ~Qt::ItemIsSelectable);

        for (const auto& kw : keywords) {
            // Determine which cases have this keyword
            std::vector<int> case_indices;
            for (size_t i = 0; i < m_vect_list.size(); i++) {
                auto& vl = m_vect_list[i];
                if (std::find(vl.begin(), vl.end(), kw) != vl.end()) {
                    case_indices.push_back(i);
                }
            }

            auto* kwItem = new QTreeWidgetItem(catItem);
            kwItem->setText(0, QString::fromStdString(kw));

            // Show which cases have it
            QString case_str;
            for (size_t j = 0; j < case_indices.size(); j++) {
                if (j > 0) case_str += ", ";
                if (static_cast<size_t>(case_indices[j]) < m_root_names.size())
                    case_str += QString::fromStdString(m_root_names[case_indices[j]]);
            }
            kwItem->setText(1, case_str);

            // Store the first case index as data (-1 if not available)
            kwItem->setData(0, Qt::UserRole, case_indices.empty() ? -1 : case_indices[0]);

            total_count++;
        }
    }

    m_statusLabel->setText(QString::number(total_count) + " keywords");

    if (!filter_upper.empty())
        m_treeWidget->expandAll();
}

void KeywordPanel::onFilterChanged(const QString& text)
{
    populateTree(text);
}

void KeywordPanel::onItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
    if (!item || !item->parent())
        return;  // category item, ignore

    std::string keyword = item->text(0).toStdString();
    int smry_ind = item->data(0, Qt::UserRole).toInt();
    emit keywordSelected(keyword, smry_ind);
}

void KeywordPanel::onPlotButtonClicked()
{
    auto selectedItems = m_treeWidget->selectedItems();
    std::vector<std::pair<std::string, int>> selections;

    for (auto* item : selectedItems) {
        if (!item->parent())
            continue;  // skip category items

        std::string keyword = item->text(0).toStdString();
        int smry_ind = item->data(0, Qt::UserRole).toInt();
        selections.push_back({keyword, smry_ind});
    }

    if (!selections.empty())
        emit keywordsPlotRequested(selections);
}
