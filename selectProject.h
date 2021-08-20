#ifndef SelectProject_H
#define SelectProject_H

#include <QDialog>
#include <QSqlQueryModel>
#include <functional>

namespace Ui {
class SelectProject;
}

class SelectProject : public QDialog
{
    Q_OBJECT

public:
    explicit SelectProject(QWidget *parent = nullptr);
    ~SelectProject();

    int m_lastSelectedId = -1;

    void choose(const std::function<void(int)> &task);

private:
    Ui::SelectProject *ui;

    std::function<void(int)> m_currentTaskOnSelect;

    QSqlQueryModel m_model_projects;
    void setQueryProjects();
    void showEvent(QShowEvent *ev) override;
    void keyPressEvent(QKeyEvent* event) override;

    void actionSelect(const QModelIndex& index);
};

#endif // SelectProject_H
