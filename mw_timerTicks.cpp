
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>

//***************************************************************************
// ui ticks

static QDateTime uiNextTimerShot(QDateTime dt) {

    auto d1 = dt.date();
    auto t1 = dt.time();
    auto dt2 = QDateTime( d1, QTime(t1.hour(), t1.minute(), t1.second()), Qt::UTC);
    auto dt3 = dt2.addMSecs(1000);

    return dt3;
}

void MainWindow::setNextUiTimerTicks(QDateTime preDt) {
    QDateTime nextDt = uiNextTimerShot(preDt);
    qint64 nextMs = preDt.msecsTo(nextDt);
    m_timeTicks_ui.start(nextMs);
}

void MainWindow::uiTimerTicks() {

    if (m_currentIdSession <= 0)
        return;

    QDateTime curDt = QDateTime::currentDateTimeUtc();

    if (curDt < m_uiNextExpectedTime) {

        m_timeTicks_ui.start(
            std::max(1LL, curDt.msecsTo(m_uiNextExpectedTime)) );

        return;
    }

    updateLabelTimeHasPassed(curDt);
    setNextUiTimerTicks(curDt);
}

void MainWindow::updateLabelTimeHasPassed(QDateTime curDt) {

    int secs = m_dateTimeStartSession.secsTo(curDt);

    int s = secs % 60;
    secs /= 60;
    int m = secs % 60;
    secs /= 60;
    int h = secs;

    auto fmt = [](int v) -> QString {
        QString r = QString::number(v);
        while (r.size() < 2)
            r = "0" + r;
        return r;
    };

    QString txt = QString::number(h) + ":" + fmt(m) + ":" + fmt(s);

    ui->bt_start->setText(txt);
}

//***************************************************************************
// database ticks
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

void MainWindow::setNextTimerShot(QDateTime preDt) {

    QDateTime nextDt = nextTimerShot(preDt);

    m_nextExpectedTime = nextDt;

    QDateTime curDt = QDateTime::currentDateTimeUtc();
    qint64 nextMs = curDt.msecsTo(nextDt);

    m_timeTicks_db.start(nextMs);
}

void MainWindow::databaseTimerTick() {

    if (m_currentIdSession <= 0)
        return;

    QDateTime curDt = QDateTime::currentDateTimeUtc();
    if (curDt < m_nextExpectedTime) {
        m_timeTicks_db.start( std::max(1LL, curDt.msecsTo(m_nextExpectedTime)) );
        return;
    }

    fixDatabaseTimeTick(false, false);

    setNextTimerShot(m_lastFixedTime);
}

//***************************************************************************
