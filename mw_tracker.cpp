#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>

void MainWindow::fixTimeTick(bool isActionStop, bool isActionChangeBinds) {
    if (!m_currentIdStart) return;

    auto curDt = QDateTime::currentDateTimeUtc();

    QSqlQuery query;
    query.prepare(
    "insert into TIMETICKS ("
    "   idStart, startDateTime, secsSinceEpoch, secsPastTime, actionStop) "
    " values(?,?,?,?,?)");

    query.addBindValue(m_currentIdStart);
    query.addBindValue(DateTimeToIso(m_lastFixedTime));
    query.addBindValue(curDt.toSecsSinceEpoch());
    query.addBindValue(m_lastFixedTime.secsTo(curDt));
    query.addBindValue(isActionStop);
    query.exec();

    m_lastFixedTime = curDt;

    if (isActionStop || isActionChangeBinds)
        m_currentIdStart = 0;

    if (isActionStop) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
        m_currentIdSession = 0;
    }

    updateReports();
}

QDateTime MainWindow::nextRoundTime(QDateTime dt) {

    const int r = m_roundMinutes;

    auto d1 = dt.date();
    auto t1 = dt.time();

    auto dt2 = QDateTime( d1, QTime(t1.hour(), (t1.minute() / r) * r), Qt::UTC);

    return dt2.addSecs(r * 60);
}

QDateTime MainWindow::nextTimerShot(QDateTime curDt) {

    auto dt = nextRoundTime(curDt);

    if (curDt.secsTo(dt) <= m_notSplitLittlePeriod)
        dt = nextRoundTime(dt);

    return dt;
}

void MainWindow::setNextTimerShot(QDateTime curDt) {

    QDateTime nextDt = nextTimerShot(curDt);

    m_nextExpectedTime = nextDt;

    qint64 nextMs = curDt.msecsTo(nextDt);

    m_timer->start(nextMs);
}

void MainWindow::timerTick() {

    if (!m_timer)
        return;

    QDateTime curDt = QDateTime::currentDateTimeUtc();
    if (curDt < m_nextExpectedTime) {
        m_timer->start( std::max(1LL, curDt.msecsTo(m_nextExpectedTime)) );
        return;
    }

    fixTimeTick(false, false);

    setNextTimerShot(m_lastFixedTime);
}

void MainWindow::createNewStart(bool isActionStart, bool isActionChangeBinds) {

    fixTimeTick(isActionStart, isActionChangeBinds);

    auto curDt = QDateTime::currentDateTimeUtc();

    QSqlQuery query;
    query.prepare(
    "insert into STARTS (secsSinceEpoch, dateTime, actionStart) "
    "values (?,?,?)");

    query.addBindValue(curDt.toSecsSinceEpoch());
    query.addBindValue(DateTimeToIso(curDt));
    query.addBindValue(isActionStart);
    query.exec();

    m_currentIdStart = query.lastInsertId().toUInt();
    m_lastFixedTime = curDt;

    QSqlQuery query2;
    query2.prepare(
    "insert into BINDS (idStart,idProject) "
    "select ?,id from PROJECTS where checked");
    query2.addBindValue(m_currentIdStart);
    query2.exec();

    QSqlQuery query3;
    query3.prepare("select idProject from BINDS where idStart=?");
    query3.addBindValue(m_currentIdStart);
    query3.exec();

    m_activeProjects.clear();
    while (query3.next())
        m_activeProjects.insert(query3.value("idProject").toUInt());

    if (isActionStart) {

        m_timer = new QTimer;
        connect(m_timer, &QTimer::timeout, [this] { timerTick(); });
        m_timer->setSingleShot(true);

        setNextTimerShot(curDt);

        m_currentIdSession = m_currentIdStart;
    }
}

