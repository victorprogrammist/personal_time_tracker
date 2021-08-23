#ifndef DIALOGMANUAL_H
#define DIALOGMANUAL_H

#include <QDialog>
#include "dbtool.h"

namespace Ui {
class DialogManual;
}

class DialogManual : public QDialog
{
    Q_OBJECT

public:
    explicit DialogManual(QWidget *parent = nullptr);
    ~DialogManual();

    DbObj curObj;

    void actionOpenNew(uint idProject);
    void actionOpen(uint idManual);
    void actionCancel();
    void actionStore();
    void actionStoreAndClose();

private:
    Ui::DialogManual *ui;

    void init();
};

#endif // DIALOGMANUAL_H
