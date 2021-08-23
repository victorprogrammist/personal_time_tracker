
#include "dbtool.h"

QVariant& DbObj::field(const QString& name) {

    auto itr = m_fields.find(name);

    if (itr == m_fields.end()) {
        qDebug() << name;
        assert(false);
    }

    return itr->second;
}

void DbObj::clear() {
    m_id = 0;
    m_tableName.clear();
    m_fields.clear();
}

void DbObj::store() {
    assert( !m_fields.empty() );
    m_id = DbTool::storeRecord(m_tableName, m_id, m_fields);
}

void DbObj::initEmpty(const QString& tbName) {
    assert( empty() );
    m_tableName = tbName;
    for (const QString& name: DbTool::listFields(tbName))
        m_fields.insert({name,QVariant()});
}

void DbObj::init(const QString& tbName, uint id) {
    assert( empty() && id );
    m_id = id;
    auto fields = DbTool::listFields(tbName);
    DbTool::selRecordValues(tbName, id, fields,
        [&](const QString& name, const QVariant& value) {
            m_fields.insert({name,value}); });
}
