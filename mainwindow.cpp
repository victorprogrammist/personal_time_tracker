#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "selectProject.h"

#include <QInputDialog>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlQuery>

/*
 * TODO: double open app
 * TODO: keyboard and mouse pressed
 * TODO: current window title
 * TODO: deletion/hiding project
 * TODO: reports
 */

void MainWindow::updateReports() {
    setQueryPlainJourn();
    setQueryTotals();
}

void MainWindow::updateCurrentProjectForReports() {
    int& id = m_currentReportProject_id;

    if (id < 0)
        id = 0;

    if (!id)
        m_currentReportProject_name = "<all projects>";
    else
        m_currentReportProject_name = getName("PROJECTS", id);

    ui->lb_curPrj_totals->setText(m_currentReportProject_name);
    ui->lb_curPrj_plainJourn->setText(m_currentReportProject_name);

    updateReports();
}

void MainWindow::selectProjectForReports() {

    if (!m_dialog_selectProject)
        m_dialog_selectProject = new SelectProject(this);

    m_dialog_selectProject->choose([this](int id) {
        if (id <= 0 || id == m_currentReportProject_id)
            return;

        m_currentReportProject_id = id;
        updateCurrentProjectForReports();
    });
}

void MainWindow::clearProjectForReports() {
    if (!m_currentReportProject_id) return;

    m_currentReportProject_id = 0;
    updateCurrentProjectForReports();
}

void MainWindow::outError(const QString& s) {

    QMessageBox msg(this);
    msg.setText(s);
    msg.exec();

    setEnabled(false);
}

QString MainWindow::getName(const QString& tableName, uint id) {
    if (!id) return "";
    return getValue(tableName, id, "Name").toString();
}

QVariant MainWindow::getValue(const QString& tableName, uint id, const QString& field) {
    return getValues(tableName, id, {field}).at(0);
}

QVariantList MainWindow::getValues(const QString& tableName, uint id, const QStringList& fields) {

    QSqlQuery query;
    query.prepare(
    "select " + fields.join(",") + " from " + tableName + " where id=?");

    query.addBindValue(id);
    query.exec();

    if (!query.next())
        return {};

    QVariantList r;
    uint sz = fields.size();
    r.reserve(sz);
    for (uint ii = 0; ii < sz; ++ii)
        r.append(query.value(ii));

    return r;
}

QString MainWindow::getSettings(const QString& name) {

    QSqlQuery query;
    query.prepare("select value from SETTINGS where name=?");
    query.addBindValue(name);
    query.exec();

    if (query.next())
        return query.value(0).toString();

    return {};
}

void MainWindow::setSettings(const QString& name, const QString& value) {

    QSqlQuery query;
    query.prepare(
    "replace into SETTINGS (name,value) values(?,?)");
    query.addBindValue(name);
    query.addBindValue(value);
    query.exec();
}

void MainWindow::actionExit() {
    if (!m_currentIdStart)
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

    if (!m_currentIdStart) {
        m_trayIcon->setIcon(QIcon(":icons/ico_on.ico"));

        createNewStart(true, false);

        ui->bt_stop->setEnabled(true);
        m_actionExit->setEnabled(false);
        ui->bt_exit->setEnabled(false);

        setColorProjectItems();
    }
}

void MainWindow::actionStop() {

    if (ui->bt_start->isChecked())
        ui->bt_start->setChecked(false);

    if (m_currentIdStart) {
        m_trayIcon->setIcon(QIcon(":icons/ico_off.ico"));

        fixTimeTick(true, false);

        ui->bt_stop->setEnabled(false);
        m_actionExit->setEnabled(true);
        ui->bt_exit->setEnabled(true);

        setColorProjectItems();
    }
}

void MainWindow::on_bt_add_clicked()
{
    actionAddNewProject();
}

void MainWindow::on_bt_rename_clicked()
{
    auto *list = ui->tb_projects;
    auto idx = list->currentIndex();
    list->edit(idx);
}

void MainWindow::on_bt_del_clicked()
{

}

