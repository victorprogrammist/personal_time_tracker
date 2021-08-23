
#ifndef DBOBJ_H
#define DBOBJ_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <map>

using MapFields = std::map<QString,QVariant>;

struct DbTool {

    static QString getSettings(const QString& name);
    static void setSettings(const QString& name, const QString& value);

    static QString getName(const QString& tableName, uint id);
    static QVariant getValue(const QString& tableName, uint id, const QString& field);
    static QVariantList getValues(const QString& tableName, uint id, const QStringList &fields);

    static QStringList listFields(const QString& tableName);

    static void selRecordValues(
        const QString& tableName, uint id, const QStringList& fields,
        const std::function<void(const QString& name, const QVariant& value)>& task);

    static uint storeRecord(
        const QString& tbName, uint id,
        const MapFields& values);
};

struct DbObj {
    DbObj() {}
    DbObj(const QString& tbName, uint id) { init(tbName, id); }

    QVariant& field(const QString& name);
    bool isNew() const { return !m_id && !empty(); }
    bool empty() const { return !m_id && m_tableName.isEmpty() && m_fields.empty(); }

    void clear();
    void storeAndClear() { store(); clear(); }
    void store();
    void initEmpty(const QString& tbName);
    void init(const QString& tbName, uint id);

private:
    QString m_tableName;
    uint m_id = 0;
    MapFields m_fields;
};

#endif

