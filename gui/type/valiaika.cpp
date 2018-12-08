#include "valiaika.h"

Valiaika::Valiaika(const QVariant& iid, int jjarj, int kkoodi, const QTime &aaika, int ssija) :
    id(iid),
    jarj(jjarj),
    koodi(kkoodi),
    aika(aaika),
    sija(ssija)
{
}

QList<Valiaika> Valiaika::haeValiajat(const QVariant &tulosId)
{
    QList<Valiaika> valiajat;

    QSqlQuery query;
    query.prepare("SELECT * FROM valiaika WHERE tulos = ? ORDER BY jarj ASC");
    query.addBindValue(tulosId);
    SQL_EXEC(query, valiajat);

    while (query.next()) {
        QSqlRecord r = query.record();

        valiajat << Valiaika(r.value("id"), r.value("numero").toInt(), r.value("koodi").toInt(), r.value("aika").toTime(), -1);
    }

    return valiajat;
}

QList<Valiaika> Valiaika::haeRastiValiajat(SarjaP sarja, int jarj)
{
    QList<Valiaika> valiajat;

    QSqlQuery query;
    query.prepare(
                "SELECT\n"
                "  v.*\n"
                "FROM valiaika AS v\n"
                "  JOIN tulos AS t ON t.id = v.tulos\n"
                "WHERE t.tapahtuma = ?\n"
                "  AND t.tila = 2\n"
                "  AND t.sarja = ?\n"
                "  AND v.jarj = ?\n"
                "ORDER BY v.aika ASC\n"
    );

    query.addBindValue(Tapahtuma::tapahtuma()->id());
    query.addBindValue(sarja->getId());
    query.addBindValue(jarj);

    SQL_EXEC(query, valiajat);

    QList<QSqlRecord> res;
    while (query.next())
        res << query.record();

    foreach (QSqlRecord r, res) {
        int sija = 1;

        foreach (QSqlRecord rr, res) {
            if (rr.value("aika").toTime() < r.value("aika").toTime()) {
                sija++;
            }
        }

        valiajat.append(Valiaika(r.value("id"), r.value("numero").toInt(), r.value("koodi").toInt(), r.value("aika").toTime(), sija));
    }

    return valiajat;
}
