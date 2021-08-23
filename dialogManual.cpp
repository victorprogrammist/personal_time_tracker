#include "dialogManual.h"
#include "ui_dialogManual.h"

#include "dbtool.h"
#include "selectProject.h"

DialogManual::DialogManual(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogManual)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->bt_cancel, &QPushButton::clicked, [this] { actionCancel(); });
    connect(ui->bt_ok, &QPushButton::clicked, [this] { actionStoreAndClose(); });
    connect(ui->bt_apply, &QPushButton::clicked, [this] { actionStore(); });

    auto *wg = ui->wg_selectProject;

    wg->setCaption("Project:");

    wg->setRequestPresent([this](uint id) -> QString {
        if (!id) return "<empty>";
        return DbTool::getName("PROJECTS", id); });

    wg->reqOpenSelect = [this] {
        auto* dlg = new SelectProject(this);
        dlg->choose([this](uint id) {
            ui->wg_selectProject->setId(id);
        });
    };
}

DialogManual::~DialogManual()
{
    delete ui;
}

void DialogManual::init() {

    ui->wg_selectProject->setId(curObj.field("idProject").toUInt());

    if (curObj.isNew())
        ui->ed_dateTime->setDateTime(QDateTime::currentDateTimeUtc());

    else {
        ui->ed_dateTime->setDateTime(
            curObj.field("dateTime").toDateTime());

        ui->ed_duration->setText(
            curObj.field("duration_manualInput").toString());

        ui->ed_memo->setPlainText(
            curObj.field("memo").toString());
    }
}

void DialogManual::actionCancel() {
    close();
}

void DialogManual::actionStore() {

    curObj.field("idProject") = ui->wg_selectProject->currentId();

    if (curObj.isNew())
        curObj.field("createdDateTime") = QDateTime::currentDateTimeUtc();

    auto *wg1 = ui->ed_dateTime;
    curObj.field("dateTime_manualInput") = wg1->text();
    curObj.field("dateTime") = wg1->dateTime().toUTC();

    auto *wg2 = ui->ed_duration;
    curObj.field("duration_manualInput") = wg2->text();
    curObj.field("secsDuration") = wg2->text().toUInt();
    curObj.field("memo") = ui->ed_memo->toPlainText();

    curObj.store();
}

void DialogManual::actionStoreAndClose() {
    actionStore();
    close();
}

void DialogManual::actionOpenNew(uint idProject) {
    curObj.initEmpty("MANUAL_TIME");
    curObj.field("idProject") = idProject;
    init();
    show();
}

void DialogManual::actionOpen(uint idManual) {
    curObj.init("MANUAL_TIME", idManual);
    init();
    show();
}


