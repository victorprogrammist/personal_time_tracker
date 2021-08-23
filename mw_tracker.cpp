#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>

void MainWindow::fixDatabaseTimeTick(bool isActionStop, bool isActionChangeBinds) {
    if (!m_currentIdStart) return;

    auto curDt = QDateTime::currentDateTimeUtc();

    QSqlQuery query;
    query.prepare(
    "insert into TIMETICKS ("
    "   idStart, startDateTime, secsSinceEpoch, secsPastTime, actionStop) "
    " values(?,?,?,?,?)");

    query.addBindValue(m_currentIdStart);
    query.addBindValue(m_lastFixedTime);
    query.addBindValue(curDt.toSecsSinceEpoch());
    query.addBindValue(m_lastFixedTime.secsTo(curDt));
    query.addBindValue(isActionStop);
    query.exec();

    m_lastFixedTime = curDt;

    if (isActionStop || isActionChangeBinds)
        m_currentIdStart = 0;

    if (isActionStop) {
        m_currentIdSession = 0;
        m_timeTicks_db.stop();
    }

    updateReports();
}

void MainWindow::createNewStart(bool isActionStart, bool isActionChangeBinds) {

    // select list checked projects
    QSqlQuery query3("select id from PROJECTS where checked");

    m_activeProjects.clear();
    while (query3.next())
        m_activeProjects.insert(query3.value(0).toUInt());

    //**********************************************

    if (!m_countNestedTransactions++)
        m_database.transaction();

    fixDatabaseTimeTick(isActionStart, isActionChangeBinds);

    auto curDt = QDateTime::currentDateTimeUtc();

    uint countBindedProjects = m_activeProjects.size();
    uint single_idProject = 0;
    if (countBindedProjects == 1)
        single_idProject = *m_activeProjects.begin();

    QSqlQuery query;
    query.prepare(
    "insert into STARTS "
    " (secsSinceEpoch, dateTime, actionStart, single_idProject, countBindedProjects) "
    "values (?,?,?,?,?)");

    query.addBindValue(curDt.toSecsSinceEpoch());
    query.addBindValue(curDt);
    query.addBindValue(isActionStart);
    query.addBindValue(single_idProject);
    query.addBindValue(countBindedProjects);
    query.exec();

    m_currentIdStart = query.lastInsertId().toUInt();
    m_lastFixedTime = curDt;

    if (m_activeProjects.size()) {

        QStringList list;
        for (int idProject: m_activeProjects)
            list.append(QString("(%1,%2)").arg(m_currentIdStart).arg(idProject));

        QSqlQuery query2(
        "insert into BINDS (idStart,idProject) "
        " values " + list.join(",") );
    }

    if (!--m_countNestedTransactions)
        m_database.commit();

    //**********************************************
    if (isActionStart) {
        m_currentIdSession = m_currentIdStart;
        m_dateTimeStartSession = curDt;
        setNextTimerShot(curDt);
        setNextUiTimerTicks(curDt);
    }
}

