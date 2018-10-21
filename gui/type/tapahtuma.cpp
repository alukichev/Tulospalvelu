#include "tapahtuma.h"

static inline int raceType(int v)
{
    return (v < 0 || v > RACE_ROGAINING) ? RACE_CLASSIC : v;
}

Tapahtuma * Tapahtuma::ms_tapahtuma = new Tapahtuma(0, 0);

Tapahtuma::Tapahtuma(QObject *parent, int id) :
    QObject(parent),
    m_id(id),
    m_nimi(),
    m_tyyppi(RACE_CLASSIC)
{
}


int Tapahtuma::id() const
{
    return m_id;
}

QString Tapahtuma::nimi() const
{
    return m_nimi;
}

int Tapahtuma::tyyppi(void) const
{
    return m_tyyppi;
}

void Tapahtuma::luoUusiTapahtuma(const QString &nimi, int tyyppi)
{
    tyyppi = raceType(tyyppi);

    QSqlQuery query;

    query.prepare("INSERT INTO tapahtuma (nimi, tyyppi) VALUES (?, ?)");

    query.addBindValue(nimi);
    query.addBindValue(tyyppi);

    SQL_EXEC(query,);

    ms_tapahtuma->m_id = query.lastInsertId().toInt();
    ms_tapahtuma->m_nimi = nimi;
    ms_tapahtuma->m_tyyppi = tyyppi;
}

const Tapahtuma* Tapahtuma::tapahtuma()
{
    if (ms_tapahtuma->m_nimi.isNull()) {
        Tapahtuma::valitseTapahtuma(ms_tapahtuma->m_id);
    }

    return ms_tapahtuma;
}

bool Tapahtuma::valitseTapahtuma(int id)
{
    ms_tapahtuma->m_id = id;
    ms_tapahtuma->m_nimi = QString();
    ms_tapahtuma->m_tyyppi = RACE_CLASSIC;

    QSqlQuery query;

    query.prepare("SELECT nimi, tyyppi FROM tapahtuma WHERE id = ?");

    query.addBindValue(ms_tapahtuma->m_id);

    SQL_EXEC(query, false);

    if (!query.next()) {
        return false;
    }

    ms_tapahtuma->m_nimi = query.value(0).toString();
    ms_tapahtuma->m_tyyppi = raceType(query.value(1).toInt());

    return true;
}
