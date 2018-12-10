#include "emitleima.h"

QList<EmitLeima> EmitLeima::haeLeimat(const QVariant &luettu_emit_id)
{
    QList<EmitLeima> rastit;

    QSqlQuery query;
    query.prepare("SELECT * FROM luettu_emit_rasti WHERE luettu_emit = ? ORDER BY numero ASC");
    query.addBindValue(luettu_emit_id);
    SQL_EXEC(query, rastit);

    while (query.next()) {
        QSqlRecord r = query.record();

        rastit.append(EmitLeima(r.value("koodi").toInt(), r.value("aika").toInt()));
    }

    return rastit;
}
