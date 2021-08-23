
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardPaths>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>

void MainWindow::initDatabase() {

    m_pathAppData =
        QStandardPaths::writableLocation(
            QStandardPaths::StandardLocation::AppDataLocation);

    QDir dir(m_pathAppData);
    if (!dir.exists())
        dir.mkpath(".");

    const QString driver = "QSQLITE";

    if (!QSqlDatabase::isDriverAvailable(driver))
        return outError("SQLite is not available");

    QString path = m_pathAppData + "/database.sqlite";

    ui->labelBasePath->setText(path);

    m_database = QSqlDatabase::addDatabase(driver);
    m_database.setDatabaseName(path);

    if (!m_database.open())
        return outError("Connection with database failed");

    QSqlQuery query1(
        "create table if not exists PROJECTS ("
        "   id integer primary key, "
        "   createdDateTime text, "
        "   name text, "
        "   checked bool"
        ")");

    QSqlQuery query2(
        "create table if not exists MANUAL_TIME ("
        "   id integer primary key, "
        "   idProject integer not null, "
        "   createdDateTime text, "
        "   dateTime_manualInput text, "
        "   dateTime text, "
        "   duration_manualInput text, "
        "   secsDuration integer, "
        "   memo text "
        ")");

    QSqlQuery query4(
        "create table if not exists STARTS ("
        "   id integer primary key,"
        "   secsSinceEpoch integer,"
        "   dateTime text, "
        "   actionStart bool, "
        "   single_idProject integer, "
        "   countBindedProjects integer "
        ")");

    QSqlQuery query3(
        "create table if not exists BINDS ("
        "   idStart integer not null,"
        "   idProject integer not null,"
        "   primary key( idStart, idProject )"
        ")");

    QSqlQuery query5(
        "create table if not exists TIMETICKS ("
        "   id integer primary key, "
        "   idStart integer not null, "
        "   startDateTime text, "
        "   secsSinceEpoch integer, "
        "   secsPastTime integer, "
        "   actionStop bool"
        ")");
}
