
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "numFormat.h"

#include <QSqlDatabase>
#include <QSqlQuery>

void MainWindow::setQueryPlainJourn() {

    QString txt_prjName =
        " case when ts.countBindedProjects = 1 "
        " then "
        "    tp.name "
        " else "
        "    printf('<%i>', ts.countBindedProjects) "
        " end as Project ";

    QString txt_tbPrj =
        "   inner join STARTS as ts "
        "   on tt.idStart = ts.id "
        "   left join PROJECTS as tp "
        "   on ts.single_idProject = tp.id ";

    QSqlQuery query;

    if (!m_currentReportProject_id) {

        query.prepare(
        "select "
        "   datetime(tt.startDateTime, 'localtime') as dateTime, "
        "   tt.secsPastTime, "
        + txt_prjName +
        " from TIMETICKS as tt "
        + txt_tbPrj +
        " where "
        "   tt.secsPastTime > 0 "
        " order by "
        "   tt.startDateTime");

    } else {

        query.prepare(
        "select "
        "   datetime(tt.startDateTime, 'localtime') as dateTime, "
        "   tt.secsPastTime, "
        + txt_prjName +
        " from TIMETICKS as tt "
        "   inner join BINDS as tb "
        "   on tt.idStart = tb.idStart "
        + txt_tbPrj +
        " where tt.secsPastTime > 0 and tb.idProject = ? "
        " order by tt.startDateTime");

        query.addBindValue(m_currentReportProject_id);
    }

    query.exec();
    m_model_plainJourn.setQuery(query);
    m_model_plainJourn.setHeaderData(0, Qt::Horizontal, "When");
    m_model_plainJourn.setHeaderData(1, Qt::Horizontal, "Time");
    m_model_plainJourn.setHeaderData(2, Qt::Horizontal, "Prj/cnt");
}

void MainWindow::initTablePlainJournal() {

    auto *tb = ui->tb_plainJourn;
    tb->setModel(&m_model_plainJourn);
    setQueryPlainJourn();

    tb->setColumnWidth(0, 180);
    tb->setColumnWidth(1, 100);
    tb->horizontalHeader()->setStretchLastSection(true);

    restoreTableSettings(tb, "TablePlainJorn");

    m_timeDelegate_plainJourn = std::make_unique<TimeFormatDelegate>();
    tb->setItemDelegateForColumn(1, m_timeDelegate_plainJourn.get());
}

void MainWindow::setQueryTotals() {

    m_model_totals.setQuery(
    "select "
    "   tp.id, "
    "   tp.name as Project, "
    "   sum(tt.secsPastTime) as secsTime "
    " from PROJECTS as tp "
    "   inner join BINDS as tb "
    "   on tp.id = tb.idProject "
    "   inner join TIMETICKS as tt "
    "   on tb.idStart = tt.idStart "
    " group by tp.id, tp.name "
    " having sum(tt.secsPastTime) > 0 ");

    m_model_totals.setHeaderData(2, Qt::Horizontal, "Time");
}

void MainWindow::initTableTotals() {

    auto *tb = ui->tb_totals;
    tb->setModel(&m_model_totals);
    tb->horizontalHeader()->setStretchLastSection(true);
    setQueryTotals();
    restoreTableSettings(tb, "TableTotals");

    m_timeDelegate_totals = std::make_unique<TimeFormatDelegate>();
    tb->setItemDelegateForColumn(2, m_timeDelegate_totals.get());
}

void MainWindow::initTableProjects() {

    m_model_projects.setHorizontalHeaderLabels(
    {"id","Project","Created"});

    auto *tb = ui->tb_projects;

    tb->setModel(&m_model_projects);
    tb->setColumnWidth(0, 30);
    tb->setColumnWidth(1, 170);
    tb->horizontalHeader()->setStretchLastSection(true);
    tb->verticalHeader()->hide();
    restoreTableSettings(tb, "TableProjects");

    QSqlQuery query2(
    "select "
    "   id, "
    "   name, "
    "   createdDateTime, "
    "   checked "
    " from PROJECTS");

    while (query2.next())
        showProjectItem(
            query2.value("id").toUInt(),
            query2.value("checked").toBool(),
            query2.value("name").toString(),
            query2.value("createdDateTime").toDateTime());

    connect(
        &m_model_projects, &QStandardItemModel::dataChanged,
        this, &MainWindow::onProjectItemChanged);
}
