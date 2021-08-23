#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "selectProject.h"
#include "dialogManual.h"
#include "dbtool.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlQuery>

/*
 * TODO: double open app
 *
 * TODO: keyboard and mouse pressed
 *
 * TODO: current window title
 * TODO: deletion/hiding project
 * TODO: reports
 *
 * TODO: totals in common list of projects
 *
 * TODO: remove button Rename and make small button inplace list
 *
 * TODO: totals by days
 *
 * TODO: leave comments
 *
 * TODO: multilanguage
 *
 */

void MainWindow::updateReports() {
    setQueryPlainJourn();
    setQueryTotals();
}

void MainWindow::outError(const QString& s) {

    QMessageBox msg(this);
    msg.setText(s);
    msg.exec();

    setEnabled(false);
}

void MainWindow::actionManualTime() {

    auto dlg = new DialogManual(this);
    dlg->actionOpenNew(currentIdProject());
}

void MainWindow::actionExit() {
    if (m_currentIdSession) return;

    auto sz = frameGeometry().size();
    DbTool::setSettings("MainWindowSize", QString("%1,%2").arg(sz.width()).arg(sz.height()));

    qApp->quit();
}

void MainWindow::actionHide() {
    if (isHidden()) return;
    lastGeometry = geometry();
    hide();
}

void MainWindow::actionShow() {
    if (isVisible()) return;
    setGeometry(lastGeometry);
    show();
}

void MainWindow::actionStart() {

    if (!ui->bt_start->isChecked())
        ui->bt_start->setChecked(true);

    if (!m_currentIdSession) {
        createNewStart(true, false);

        if (!m_currentIdSession)
            return;

        m_trayIcon->setIcon(QIcon(":icons/ico_on.ico"));
        ui->bt_stop->setEnabled(true);
        m_actionExit->setEnabled(false);
        ui->bt_exit->setEnabled(false);
        setColorProjectItems();

        ui->bt_start->setText("0:00:00");
    }
}

void MainWindow::actionStop() {

    if (ui->bt_start->isChecked())
        ui->bt_start->setChecked(false);

    if (m_currentIdSession) {
        if (m_currentIdSession > 0)
            fixDatabaseTimeTick(true, false);

        m_currentIdSession = 0;
        m_currentIdStart = 0;

        m_trayIcon->setIcon(QIcon(":icons/ico_off.ico"));
        ui->bt_stop->setEnabled(false);
        m_actionExit->setEnabled(true);
        ui->bt_exit->setEnabled(true);
        setColorProjectItems();

        ui->bt_start->setText("START");
    }
}

