
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dbtool.h"
#include "selectProject.h"

#include <QKeyEvent>
#include <QDesktopServices>
#include <QMenu>
#include <QScreen>

#include <QSqlDatabase>
#include <QSqlQuery>

void MainWindow::maintenanceEvent() {

    for (const auto& [name,value]: m_changes)
        DbTool::setSettings(name, value);

    m_changes.clear();
}

void MainWindow::globalRestoreTableSettings(QTableView* tv, const QString& name) {
    g_mainWindow->restoreTableSettings(tv, name);
}

void MainWindow::restoreTableSettings(QTableView* tv, const QString& name) {
    // TODO: maybe THIS here not needed.

    // initialize after all constructions and assignation of requests
    QTimer::singleShot(1, [this,tv,name] {

        auto str = DbTool::getSettings(name).toStdString();
        if (!str.empty()) {
            auto ba = QByteArray::fromHex(QByteArray::fromStdString(str));
            tv->horizontalHeader()->restoreState(ba);
        }

        // connect after applying settings so no event of changing after applying settings
        QTimer::singleShot(1, [this,tv,name] {
            connect(
                tv->horizontalHeader(),
                &QHeaderView::sectionResized,
                [this,tv,name](int, int, int) {
                    auto str = tv->horizontalHeader()->saveState().toHex().toStdString();
                    m_changes[name] = QString::fromStdString(str);
                });
         });
    });
}

MainWindow::~MainWindow()
{
    maintenanceEvent();
    delete ui;
}

void MainWindow::restoreGeometry() {

    QScreen* screen = QGuiApplication::primaryScreen();
    auto srect = screen->availableGeometry();
    auto p = srect.topRight();

    QSize sz;
    QString strSz = DbTool::getSettings("MainWindowSize");

    if (strSz.isEmpty())
        sz = frameGeometry().size();
    else {
        auto ar = strSz.split(",");
        sz.setWidth(ar[0].toInt());
        sz.setHeight(ar[1].toInt());
    }

    const int screenBorder = 50;

    if (sz.width() > srect.width() - screenBorder*2)
        sz.setWidth(srect.width() - screenBorder*2);

    if (sz.height() > srect.height() - screenBorder*2)
        sz.setHeight(srect.height() - screenBorder*2);

    setGeometry(
        p.x() - sz.width() - screenBorder,
        p.y() + screenBorder,
        sz.width(),
        sz.height());
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    ui->tabWidget->setCurrentWidget(ui->tab_timeTracker);

    setStyleSheet("QPushButton:checked { background-color: limegreen; }");

    m_actionExit = std::unique_ptr<QAction>(new QAction("Exit"));
    connect(m_actionExit.get(), &QAction::triggered, [this] { actionExit(); });

    m_trayMenu = std::unique_ptr<QMenu>(new QMenu);
    m_trayMenu->addAction("Show", [this] { actionShow(); });
    m_trayMenu->addAction("Hide", [this] { actionHide(); });
    m_trayMenu->addAction(m_actionExit.get());

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu( m_trayMenu.get() );
    m_trayIcon->setIcon(QIcon(":icons/ico_off.ico"));
    m_trayIcon->show();

    connect(ui->bt_add, &QPushButton::clicked, [this] { actionAddNewProject(); });
    connect(ui->bt_rename, &QPushButton::clicked, [this] {actionRenameProject(); });
    connect(ui->bt_exit, &QPushButton::clicked, [this] { actionExit(); });
    connect(ui->bt_hide, &QPushButton::clicked, [this] { actionHide(); });
    connect(ui->bt_start, &QPushButton::clicked, [this] { actionStart(); });
    connect(ui->bt_stop, &QPushButton::clicked, [this] { actionStop(); });
    connect(ui->bt_manual, &QPushButton::clicked, [this] { actionManualTime(); });

    connect(ui->bt_openDatabaseFolder, &QPushButton::clicked,
        [this] { QDesktopServices::openUrl(m_pathAppData); });

    //**************************************************
    initDatabase();
    restoreGeometry();

    initTableProjects();
    initTablePlainJournal();
    initTableTotals();
    initSelectCurrentProjectOfReports();

    //**************************************************
    connect(&m_maintenanceTimer, &QTimer::timeout,
        [this] { maintenanceEvent(); });
    m_maintenanceTimer.start(3000);

    connect(&m_timeTicks_db, &QTimer::timeout,
        [this] { databaseTimerTick(); });
    m_timeTicks_db.setSingleShot(true);

    connect(&m_timeTicks_ui, &QTimer::timeout,
        [this] { uiTimerTicks(); });
    m_timeTicks_ui.setSingleShot(true);
    //**************************************************

    m_currentIdSession = -1;
    actionStop();
}

void MainWindow::initSelectCurrentProjectOfReports() {
    auto *wg1 = ui->wg_curPrjReports_1;
    auto *wg2 = ui->wg_curPrjReports_2;
    auto *wg3 = ui->wg_curPrjReports_3;

    wg1->setCaption("Filter:");
    wg2->setCaption("Filter:");
    wg3->setCaption("Filter:");

    auto fn_setId = [this,wg1,wg2,wg3](uint id) {
        wg1->setId(id);
        wg2->setId(id);
        wg3->setId(id);
        if (id == m_currentReportProject_id) return;
        m_currentReportProject_id = id;
        DbTool::setSettings("CurrentReportProject", QString::number(id));
        updateReports();
    };

    connect(wg1, &WidgetSelectObject::changeId, fn_setId);
    connect(wg2, &WidgetSelectObject::changeId, fn_setId);
    connect(wg3, &WidgetSelectObject::changeId, fn_setId);

    auto fn_Choose = [this,fn_setId] {
        auto dlg = new SelectProject(this);
        dlg->choose([this,fn_setId](int id) { fn_setId(id); });
    };

    wg1->reqOpenSelect = fn_Choose;
    wg2->reqOpenSelect = fn_Choose;
    wg3->reqOpenSelect = fn_Choose;

    auto fn_Present = [](uint id) -> QString {
        if (!id)
            return "<all projects>";
        else
            return DbTool::getName("PROJECTS", id);
    };

    wg1->setRequestPresent(fn_Present);
    wg2->setRequestPresent(fn_Present);
    wg3->setRequestPresent(fn_Present);

    uint id = DbTool::getSettings("CurrentReportProject").toUInt();
    fn_setId(id);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->type() == QEvent::KeyPress && event->key() == Qt::Key_Escape)
        actionHide();
    else
        QMainWindow::keyPressEvent(event);
}
