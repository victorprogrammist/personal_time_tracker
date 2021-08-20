
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "numFormat.h"

#include <QKeyEvent>
#include <QDesktopServices>
#include <QMenu>
#include <QScreen>
#include <QStandardPaths>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>

void MainWindow::setQueryPlainJourn() {

    QSqlQuery query;

    if (!m_currentReportProject_id) {
        query.prepare(
        "select "
        "   datetime(startDateTime, 'localtime') as dateTime, "
        "   secsPastTime "
        " from TIMETICKS "
        " where secsPastTime > 0 "
        " order by startDateTime");
    } else {

        query.prepare(
        "select "
        "   datetime(tt.startDateTime, 'localtime') as dateTime, "
        "   tt.secsPastTime "
        " from TIMETICKS as tt "
        "   inner join BINDS as tb "
        "   on tt.idStart = tb.idStart "
        " where tt.secsPastTime > 0 and tb.idProject = ? "
        " order by tt.startDateTime");

        query.addBindValue(m_currentReportProject_id);
    }

    query.exec();
    m_model_plainJourn.setQuery(query);
    m_model_plainJourn.setHeaderData(0, Qt::Horizontal, "When");
    m_model_plainJourn.setHeaderData(1, Qt::Horizontal, "Time");
}

void MainWindow::initTablePlainJournal() {

    auto *tbr = ui->tb_plainJourn;
    tbr->setModel(&m_model_plainJourn);
    tbr->setColumnWidth(0, 160);
    tbr->horizontalHeader()->setStretchLastSection(true);
    setQueryPlainJourn();
    restoreTableSettings(tbr, "TablePlainJorn");

    m_timeDelegate_plainJourn = std::make_unique<TimeFormatDelegate>();
    ui->tb_plainJourn->setItemDelegateForColumn(1, m_timeDelegate_plainJourn.get());
}

void MainWindow::setQueryTotals() {

    m_model_totals.setQuery(
    "select "
    "   tp.id, "
    "   tp.name as Project, "
    "   sum(tt.secsPastTime) as secsTime "
    " from PROJECTS as tp "
    "   inner join BINDS as tb "
    "   on tp.id = tb.idProject "
    "   inner join TIMETICKS as tt "
    "   on tb.idStart = tt.idStart "
    " group by tp.id, tp.name "
    " having sum(tt.secsPastTime) > 0 ");

    m_model_totals.setHeaderData(2, Qt::Horizontal, "Time");
}

void MainWindow::initTableTotals() {

    auto *tbt = ui->tb_totals;
    tbt->setModel(&m_model_totals);
    tbt->horizontalHeader()->setStretchLastSection(true);
    setQueryTotals();
    restoreTableSettings(tbt, "TableTotals");

    m_timeDelegate_totals = std::make_unique<TimeFormatDelegate>();
    ui->tb_totals->setItemDelegateForColumn(2, m_timeDelegate_totals.get());
}

void MainWindow::initTableProjects() {

    m_model_projects.setHorizontalHeaderLabels(
    {"id","Project","Created"});

    auto *tb = ui->tb_projects;

    tb->setModel(&m_model_projects);
    tb->setColumnWidth(0, 30);
    tb->setColumnWidth(1, 170);
    tb->horizontalHeader()->setStretchLastSection(true);
    tb->verticalHeader()->hide();
    restoreTableSettings(tb, "TableProjects");

    QSqlQuery query2(
    "select "
    "   id, "
    "   name, "
    "   createDateTime, "
    "   checked "
    " from PROJECTS");

    while (query2.next())
        showProjectItem(
            query2.value("id").toUInt(),
            query2.value("checked").toBool(),
            query2.value("name").toString(),
            query2.value("createDateTime").toDateTime());

    connect(
        &m_model_projects, &QStandardItemModel::dataChanged,
        this, &MainWindow::onProjectItemChanged);
}

void MainWindow::maintenanceEvent() {

    for (const auto& [name,value]: m_changes)
        setSettings(name, value);

    m_changes.clear();
}

void MainWindow::globalRestoreTableSettings(QTableView* tv, const QString& name) {
    g_mainWindow->restoreTableSettings(tv, name);
}

void MainWindow::restoreTableSettings(QTableView* tv, const QString& name) {

    // initialize after all constructions and assignation of requests
    QTimer::singleShot(1, [this,tv,name] {

        auto str = getSettings(name).toStdString();
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QScreen* screen = QGuiApplication::primaryScreen();
    auto srect = screen->availableGeometry();
    auto p = srect.topRight();
    auto wrect = frameGeometry();

    setGeometry(
        p.x() - wrect.width() - 50,
        p.y() + 50,
        wrect.width(),
        wrect.height());

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

    connect(ui->bt_exit, &QPushButton::clicked, [this] { actionExit(); });
    connect(ui->bt_hide, &QPushButton::clicked, [this] { actionHide(); });
    connect(ui->bt_start, &QPushButton::clicked, [this]() { actionStart(); });
    connect(ui->bt_stop, &QPushButton::clicked, [this]() { actionStop(); });

    ui->bt_stop->setEnabled(false);

    connect(ui->bt_openDatabaseFolder, &QPushButton::clicked,
        [this] { QDesktopServices::openUrl(m_pathAppData); });

    //**************************************************
    initDatabase();
    initTableProjects();
    initTablePlainJournal();
    initTableTotals();
    updateCurrentProjectForReports();

    //**************************************************
    connect(ui->bt_selectPrj_totals, &QPushButton::clicked, [this]{ selectProjectForReports(); });
    connect(ui->bt_clearPrj_totals, &QPushButton::clicked, [this]{ clearProjectForReports(); });
    connect(ui->bt_selectPrj_plainJourn, &QPushButton::clicked, [this]{ selectProjectForReports(); });
    connect(ui->bt_clearPrj_plainJourn, &QPushButton::clicked, [this]{ clearProjectForReports(); });

    //**************************************************
    connect(&m_maintenanceTimer, &QTimer::timeout,
        [this] { maintenanceEvent(); });
    m_maintenanceTimer.start(3000);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->type() == QEvent::KeyPress && event->key() == Qt::Key_Escape)
         actionHide();
    else
        QMainWindow::keyPressEvent(event);
}

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

    QSqlQuery query0(
        "create table if not exists SETTINGS ("
        "   name text primary key, "
        "   value text)");

    QSqlQuery query1(
        "create table if not exists PROJECTS ("
        "   id integer primary key, "
        "   createDateTime text, "
        "   name text, "
        "   checked bool"
        ")");

    QSqlQuery query4(
        "create table if not exists STARTS ("
        "   id integer primary key,"
        "   secsSinceEpoch integer,"
        "   dateTime text, "
        "   actionStart bool, "
        "   single_idProject integer, "
        "   countBindsProjects integer "
        ")");

    QSqlQuery query3(
        "create table if not exists BINDS ("
        "   idStart integer,"
        "   idProject integer,"
        "   primary key( idStart, idProject )"
        ")");

    QSqlQuery query5(
        "create table if not exists TIMETICKS ("
        "   id integer primary key, "
        "   idStart integer, "
        "   startDateTime text, "
        "   secsSinceEpoch integer, "
        "   secsPastTime integer, "
        "   actionStop bool"
        ")");
}
