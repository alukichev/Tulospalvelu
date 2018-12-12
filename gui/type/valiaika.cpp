#include "valiaika.h"

Valiaika::Valiaika(const QVariant& iid, int jjarj, int kkoodi, const QTime &aaika, int ppisteet, int ssija) :
    id(iid),
    jarj(jjarj),
    koodi(kkoodi),
    aika(aaika),
    pisteet(ppisteet),
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

        valiajat << Valiaika(r.value("id"), r.value("jarj").toInt(),
                             r.value("rasti").toInt(), r.value("aika").toTime(),
                             r.value("pisteet").toInt(), -1);
    }

    return valiajat;
}

QList<Valiaika> Valiaika::haeRastiValiajat(SarjaP sarja, int jarj)
{
    const int tapahtuma = Tapahtuma::Get()->id();
    QList<Valiaika> valiajat;

    if (tapahtuma == RACE_ROGAINING) // Pistesuunnistuksessa vapaa rastijärjestys kilpailijoille
        return valiajat;

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

    query.addBindValue(tapahtuma);
    query.addBindValue(sarja->getId());
    query.addBindValue(jarj);

    SQL_EXEC(query, valiajat);

    QList<QSqlRecord> res;
    while (query.next())
        res << query.record();

    foreach (const QSqlRecord& r, res) {
        int sija = 1;

        foreach (const QSqlRecord& rr, res) {
            if (rr.value("aika").toTime() < r.value("aika").toTime())
                ++sija;
        }

        valiajat << Valiaika(r.value("id"), r.value("jarj").toInt(),
                             r.value("rasti").toInt(), r.value("aika").toTime(),
                             0, sija);
    }

    return valiajat;
}
