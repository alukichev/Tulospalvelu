#include "tulos.h"

// Palauttaa true, jos verta on parempi tai yhtä hyvä kuin aika ja/tai pisteet
static inline bool sijoita(const Tulos& verta, const QTime& aika, int pisteet)
{
    if (verta.m_tila != Tulos::Hyvaksytty)
        return false;

    switch (Tapahtuma::tapahtuma()->tyyppi()) {
    case RACE_ROGAINING:
        return pisteet < verta.m_pisteet || (pisteet == verta.m_pisteet && verta.m_aika < aika);
    case RACE_CLASSIC:
    default:
        break;
    }

    return verta.m_aika < aika;
}

Tulos::Tulos(int id, const QString &sarja, int sija, const QString &_emit,
             const QString &kilpailija, int tila, const QTime &aika, const QDateTime& maaliaika,
             const QList<Valiaika>& valiajat, int pisteet) :
    m_id(id),
    m_sarja(sarja),
    m_sija(sija),
    m_emit(_emit),
    m_kilpailija(kilpailija),
    m_tila(tila),
    m_aika(aika),
    m_maaliaika(maaliaika),
    m_pisteet(pisteet),
    m_valiajat(valiajat)
{
}

QList<Tulos> Tulos::haeTulokset(const Sarja* sarja)
{

    QSqlQuery query;
    QString sort = Tapahtuma::tapahtuma()->tyyppi() == RACE_ROGAINING
            ? "t.pisteet DESC, t.aika ASC\n"
            : "t.aika ASC\n";

    query.prepare(
                QString("SELECT\n"
                "  t.id,\n"
                "  t.emit,\n"
                "  k.nimi AS kilpailija,\n"
                "  t.tila,\n"
                "  t.aika,\n"
                "  t.maaliaika,\n"
                "  t.pisteet,\n"
                "  t.tila = 2 AS hyvaksytty\n"
                "FROM tulos AS t\n"
                "  JOIN kilpailija AS k ON k.id = t.kilpailija\n"
                "WHERE t.tapahtuma = ?\n"
                "  AND t.sarja = ?\n"
                "  AND NOT t.poistettu\n"
                "ORDER BY hyvaksytty DESC,\n") + sort
    );

    query.addBindValue(Tapahtuma::tapahtuma()->id());
    query.addBindValue(sarja->getId());

    QList<Tulos> tulokset;
    SQL_EXEC(query, tulokset);

    int sija = 1;
    while (query.next()) {
        const QSqlRecord r = query.record();
        const int tila = r.value("tila").toInt();
        const QTime aika = r.value("aika").toTime();
        const int pisteet = r.value("pisteet").toInt();

        if (tila == Tulos::Hyvaksytty && !tulokset.isEmpty() && sijoita(tulokset.constLast(), aika, pisteet))
            ++sija;

        tulokset << Tulos(r.value("id").toInt(), sarja->getNimi(), sija,
                          r.value("emit").toString(), r.value("kilpailija").toString(),
                          tila, aika, r.value("maaliaika").toDateTime(),
                          Valiaika::haeValiajat(r.value("id")), pisteet);
    }

    return tulokset;
}
