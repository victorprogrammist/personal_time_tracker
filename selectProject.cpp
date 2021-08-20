#include "selectProject.h"
#include "ui_selectProject.h"
#include "mainwindow.h"

#include <QKeyEvent>
#include <QSqlRecord>

void SelectProject::choose(const std::function<void(int)> &task) {
    m_currentTaskOnSelect = task;
    show();
}

void SelectProject::actionSelect(const QModelIndex& index) {

    if (!index.isValid()) {
        auto *tb = ui->tb_projects;

        if (tb->selectionModel()->hasSelection()) {
            QModelIndex idx = tb->currentIndex();
            if (idx.isValid())
                actionSelect(idx);
        }

        return;
    }

    m_lastSelectedId = m_model_projects.record(index.row()).value(0).toUInt();
    hide();

    if (m_currentTaskOnSelect)
        m_currentTaskOnSelect(m_lastSelectedId);
}

void SelectProject::showEvent(QShowEvent *ev) {
    QDialog::showEvent(ev);
    setQueryProjects();
    m_lastSelectedId = -1;
}

void SelectProject::setQueryProjects() {
    m_model_projects.setQuery(
    "select "
    "   id, "
    "   name as Project, "
    "   date(createDateTime, 'localtime') as Created "
    " from PROJECTS");
}

void SelectProject::keyPressEvent(QKeyEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        if (event->key() == Qt::Key_Escape)
             return hide();
        else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
            return actionSelect({});
    }

    QDialog::keyPressEvent(event);
}

SelectProject::SelectProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectProject)
{
    ui->setupUi(this);

    auto *tb = ui->tb_projects;

    tb->setModel(&m_model_projects);
    tb->horizontalHeader()->setStretchLastSection(true);
    setQueryProjects();
    MainWindow::globalRestoreTableSettings(tb, "TableSelectProject");

    connect(tb, &QTableView::doubleClicked,
    [this](const QModelIndex& index) { actionSelect(index); });

    connect(ui->bt_choose, &QPushButton::clicked, [this] { actionSelect({}); });
    connect(ui->bt_cancel, &QPushButton::clicked, [this] { hide(); });
}

SelectProject::~SelectProject()
{
    delete ui;
}
