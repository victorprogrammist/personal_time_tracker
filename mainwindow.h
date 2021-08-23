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
    void actionManualTime();
    static void globalRestoreTableSettings(QTableView* tv, const QString& name);

    void actionAddNewProject();
    void actionRenameProject();

private slots:

    void onProjectItemChanged(
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight,
            const QList<int> &roles);

private:
    Ui::MainWindow *ui;

    const int m_roundMinutes = 10;
    const int m_notSplitLittlePeriod = 10; // secs

    QTimer m_maintenanceTimer;
    void maintenanceEvent();
    void restoreTableSettings(QTableView* tb, const QString& name);
    void restoreGeometry();

    QString m_pathAppData;
    QSqlDatabase m_database;
    uint m_countNestedTransactions = 0;

    int m_currentReportProject_id = -1;

    std::map<QString,QString> m_changes;

    QStandardItemModel m_model_projects;
    QSqlQueryModel m_model_plainJourn;
    QSqlQueryModel m_model_totals;

    std::unique_ptr<TimeFormatDelegate> m_timeDelegate_plainJourn;
    std::unique_ptr<TimeFormatDelegate> m_timeDelegate_totals;

    int m_currentIdSession = -1;
    int m_currentIdStart = -1;
    std::set<uint> m_activeProjects;

    QDateTime m_dateTimeStartSession;
    QDateTime m_lastFixedTime;
    QDateTime m_nextExpectedTime;
    QTimer m_timeTicks_db;

    QDateTime m_uiNextExpectedTime;
    QTimer m_timeTicks_ui;

    void setNextUiTimerTicks(QDateTime preDt);
    void uiTimerTicks();
    void updateLabelTimeHasPassed(QDateTime curDt);

    QDateTime nextRoundTime(QDateTime dt);
    QDateTime nextTimerShot(QDateTime curDt);
    void setNextTimerShot(QDateTime curDt);
    void databaseTimerTick();
    void fixDatabaseTimeTick(bool isActionStop, bool isActionChangeBinds);
    void createNewStart(bool isActionStart, bool isActionChangeBinds);

    QSystemTrayIcon *m_trayIcon = nullptr;
    std::unique_ptr<QMenu> m_trayMenu;
    std::unique_ptr<QAction> m_actionExit;

    QRect lastGeometry;

    uint currentIdProject();
    void keyPressEvent(QKeyEvent* event) override;
    void setQueryPlainJourn();
    void initTablePlainJournal();
    void setQueryTotals();
    void initTableTotals();
    void initTableProjects();
    void initSelectCurrentProjectOfReports();
    void updateReports();
    void initDatabase();
    void outError(const QString& s);
    void setColorProjectItems();
    void showProjectItem(uint id, bool checked, const QString& name, QDateTime createdDateTime);
};

#endif // MAINWINDOW_H
