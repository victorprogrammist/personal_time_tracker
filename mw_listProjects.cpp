#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QInputDialog>

#include <QSqlDatabase>
#include <QSqlQuery>

const int role_Id = Qt::UserRole+1;

uint MainWindow::currentIdProject() {
    auto idx = ui->tb_projects->currentIndex();

    if (!idx.isValid())
        return 0;

    auto *it = m_model_projects.item(idx.row(), 0);
    uint id = it->data(role_Id).toUInt();

    return id;
}

void MainWindow::setColorProjectItems() {

    uint cR = m_model_projects.rowCount();
    uint cC = m_model_projects.columnCount();

    for (uint iR = 0; iR < cR; ++iR) {

        bool isActive = false;

        if (m_currentIdStart > 0) {
            QStandardItem *it = m_model_projects.item(iR, 0);
            uint id = it->data(role_Id).toUInt();
            isActive = m_activeProjects.contains(id);
        }

        for (uint iC = 0; iC < cC; ++iC) {
            QStandardItem *it = m_model_projects.item(iR, iC);

            if (!isActive)
                it->setBackground({});
            else
                it->setBackground(QColorConstants::Svg::limegreen);
        }
    }
}

void MainWindow::onProjectItemChanged(
        const QModelIndex &topLeft,
        const QModelIndex &bottomRight,
        const QList<int> &roles) {

    bool f_checkedChanged = roles.contains(Qt::CheckStateRole);
    bool f_nameChanged = roles.contains(Qt::DisplayRole);

    if (!f_checkedChanged && !f_nameChanged)
        return;

    uint id = topLeft.data(role_Id).toUInt();
    bool checked = topLeft.data(Qt::CheckStateRole).toInt() == Qt::Checked;
    QString name = topLeft.data(Qt::DisplayRole).toString();

    QSqlQuery query;
    query.prepare("update PROJECTS set name=?, checked=? where id=?");
    query.addBindValue(name);
    query.addBindValue(checked);
    query.addBindValue(id);
    query.exec();

    if (m_currentIdSession && f_checkedChanged) {
        createNewStart(false, true);
        setColorProjectItems();
    }
}

void MainWindow::showProjectItem(
        uint id,
        bool checked,
        const QString& name,
        QDateTime createdDateTime) {

    QStandardItem *it_id = new QStandardItem(QString::number(id));
    it_id->setData(id, role_Id);
    it_id->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    it_id->setEditable(false);

    QStandardItem* it_name = new QStandardItem(name);
    it_name->setData(id, role_Id);
    it_name->setCheckable(true);

    if (checked)
        it_name->setCheckState(Qt::Checked);

    QStandardItem* it_dt = new QStandardItem(
        createdDateTime
        .toLocalTime()
        .toString("dd.MM.yyyy"));
    it_dt->setEditable(false);

    m_model_projects.appendRow({it_id, it_name, it_dt});
}

void MainWindow::actionRenameProject() {
    auto *list = ui->tb_projects;
    auto idx = list->currentIndex();
    list->edit(idx);
}

void MainWindow::actionAddNewProject() {

    bool ok;
    QString name = QInputDialog::getText(
        this,
        tr("Add new project"),
        tr("New project name:"),
        QLineEdit::Normal,
        "", &ok);

    if (!ok || name.isEmpty())
        return;

    auto dt = QDateTime::currentDateTimeUtc();

    QSqlQuery query;
    query.prepare("insert into PROJECTS (name,createdDateTime) values (?,?)");
    query.addBindValue(name);
    query.addBindValue(dt);
    query.exec();

    showProjectItem(query.lastInsertId().toUInt(), false, name, dt);
}
