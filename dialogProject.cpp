#include "dialogProject.h"
#include "ui_dialogProject.h"

DialogProject::DialogProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProject)
{
    ui->setupUi(this);
}

DialogProject::~DialogProject()
{
    delete ui;
}
