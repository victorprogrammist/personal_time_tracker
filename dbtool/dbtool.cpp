
#include "dbtool.h"

#include <QSqlError>

QStringList DbTool::listFields(const QString& tableName) {
    if (tableName.isEmpty()) return {};

    QSqlQuery query(QString("pragma table_info(%1)").arg(tableName));

    QStringList r;
    while (query.next())
        r += query.value("name").toString();

    return r;
}

QString DbTool::getName(const QString& tableName, uint id) {
    if (!id) return "";
    return getValue(tableName, id, "Name").toString();
}

QVariant DbTool::getValue(const QString& tableName, uint id, const QString& field) {
    return getValues(tableName, id, {field}).at(0);
}

QVariantList DbTool::getValues(const QString& tableName, uint id, const QStringList& fields) {
    QVariantList r;
    r.reserve(fields.size());
    selRecordValues(tableName, id, fields,
        [&](const QString&, const QVariant& value) {
            r += value; });
    return r;
}

void DbTool::selRecordValues(
        const QString& tableName, uint id, const QStringList& fields,
        const std::function<void(const QString& name, const QVariant& value)>& task) {

    QSqlQuery query;
    query.prepare(
    "select " + fields.join(",") + " from " + tableName + " where id=?");
    query.addBindValue(id);
    query.exec();

    if (!query.next())
        return;

    uint sz = fields.size();
    for (uint ii = 0; ii < sz; ++ii)
        task(fields[ii], query.value(ii));
}

uint DbTool::storeRecord(
        const QString& tbName, uint id,
        const MapFields& values) {

    QStringList fields;
    QStringList questions;
    QVariantList vals;

    for (const auto& [nm,vl]: values) {
        if (nm.toLower() == "id") continue;
        fields += nm;
        questions += "?";
        vals += vl;
    }

    QString qtp;

    if (!id)
        qtp = "insert";
    else {
        fields += "id";
        questions += "?";
        vals += id;
        qtp = "replace";
    }

    auto qry =
        QString(
        "%1 into %2 (%3) values (%4)")
        .arg(qtp)
        .arg(tbName)
        .arg(fields.join(","))
        .arg(questions.join(","));

    QSqlQuery query;
    query.prepare(qry);
    for (const auto& vl: vals)
        query.addBindValue(vl);

    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return 0;
    }

    if (id)
        return id;

    return query.lastInsertId().toUInt();
}

static void initTableSettings() {
    static bool already = false;
    if (already) return;

    already = true;

    QSqlQuery query0(
        "create table if not exists SETTINGS ("
        "   name text primary key, "
        "   value text)");
}

QString DbTool::getSettings(const QString& name) {

    initTableSettings();

    QSqlQuery query;
    query.prepare("select value from SETTINGS where name=?");
    query.addBindValue(name);
    query.exec();

    if (query.next())
        return query.value(0).toString();

    return {};
}

void DbTool::setSettings(const QString& name, const QString& value) {

    initTableSettings();

    QSqlQuery query;
    query.prepare(
    "replace into SETTINGS (name,value) values(?,?)");
    query.addBindValue(name);
    query.addBindValue(value);
    query.exec();
}

