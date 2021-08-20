#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "numFormat.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QStandardItemModel>
#include <QTableView>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTimer>

#include <set>
#include <map>

class MainWindow;
extern MainWindow* g_mainWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

inline QString DateTimeToIso(QDateTime dt) {
    return dt.toString(Qt::ISODate);
}

class SelectProject;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void actionStart();
    void actionStop();
    void actionShow();
    void actionHide();
    void actionExit();
    static void globalRestoreTableSettings(QTableView* tv, const QString& name);

private slots:

    void on_bt_add_clicked();

    void on_bt_rename_clicked();

    void on_bt_del_clicked();

    void onProjectItemChanged(
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight,
            const QList<int> &roles);

private:
    Ui::MainWindow *ui;

    const int m_roundMinutes = 1;
    const int m_notSplitLittlePeriod = 10; // secs

    QTimer m_maintenanceTimer;
    void maintenanceEvent();
    void restoreTableSettings(QTableView* tb, const QString& name);

    QString m_pathAppData;
    QSqlDatabase m_database;
    static QString getSettings(const QString& name);
    static void setSettings(const QString& name, const QString& value);

    static QString getName(const QString& tableName, uint id);
    static QVariant getValue(const QString& tableName, uint id, const QString& field);
    static QVariantList getValues(const QString& tableName, uint id, const QStringList &fields);

    SelectProject* m_dialog_selectProject = nullptr;
    int m_currentReportProject_id = -1;
    QString m_currentReportProject_name;

    std::map<QString,QString> m_changes;

    QStandardItemModel m_model_projects;
    QSqlQueryModel m_model_plainJourn;
    QSqlQueryModel m_model_totals;

    std::unique_ptr<TimeFormatDelegate> m_timeDelegate_plainJourn;
    std::unique_ptr<TimeFormatDelegate> m_timeDelegate_totals;

    uint m_currentIdSession = 0;
    uint m_currentIdStart = 0;
    QDateTime m_lastFixedTime;
    QDateTime m_nextExpectedTime;
    std::set<uint> m_activeProjects;

    QTimer *m_timer = nullptr;

    QDateTime nextRoundTime(QDateTime dt);
    QDateTime nextTimerShot(QDateTime curDt);
    void setNextTimerShot(QDateTime curDt);
    void timerTick();
    void fixTimeTick(bool isActionStop, bool isActionChangeBinds);
    void createNewStart(bool isActionStart, bool isActionChangeBinds);

    QSystemTrayIcon *m_trayIcon = nullptr;
    std::unique_ptr<QMenu> m_trayMenu;
    std::unique_ptr<QAction> m_actionExit;

    QRect lastGeometry;

    void keyPressEvent(QKeyEvent* event) override;
    void updateCurrentProjectForReports();
    void selectProjectForReports();
    void clearProjectForReports();
    void setQueryPlainJourn();
    void initTablePlainJournal();
    void setQueryTotals();
    void initTableTotals();
    void initTableProjects();
    void updateReports();
    void initDatabase();
    void outError(const QString& s);
    void actionAddNewProject();
    void setColorProjectItems();
    void showProjectItem(uint id, bool checked, const QString& name, QDateTime createDateTime);
};

#endif // MAINWINDOW_H
