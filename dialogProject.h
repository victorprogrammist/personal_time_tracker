#ifndef DIALOGPROJECT_H
#define DIALOGPROJECT_H

#include <QDialog>

namespace Ui {
class DialogProject;
}

class DialogProject : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProject(QWidget *parent = nullptr);
    ~DialogProject();

private:
    Ui::DialogProject *ui;
};

#endif // DIALOGPROJECT_H
