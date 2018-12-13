#include <QSharedPointer>

#include "tapahtuma.h"

#define ARRAY_SIZE(a)   ((int)(sizeof(a) / sizeof(a[0])))

class Tapahtuma::P : public QSharedPointer<Tapahtuma>
{
public:
    inline Tapahtuma *data(void) {
        if (!B::data())
            this->reset(new Tapahtuma{});
        return B::data();
    }

    inline Tapahtuma *operator ->(void) { return this->data(); }

private:
    typedef QSharedPointer<Tapahtuma> B;
};

static const QString _tyypit[] = {
    _("suunnistus"),        // RACE_CLASSIC
    _("pistesuunnistus"),   // RACE_ROGAINING
    _("tuntematon laji")
};

static inline int raceType(int v)
{
    return (v < 0 || v > RACE_ROGAINING) ? RACE_CLASSIC : v;
}

Tapahtuma::P Tapahtuma::ms_tapahtuma;

Tapahtuma::Tapahtuma(int id, const QString& nimi, int tyyppi) :
    m_id(id),
    m_nimi(nimi),
    m_tyyppi(tyyppi)
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

QString Tapahtuma::tyyppiNimi(void) const
{
    const int i = m_tyyppi < ARRAY_SIZE(_tyypit) ? m_tyyppi : ARRAY_SIZE(_tyypit) - 1;
    return _tyypit[i];
}

void Tapahtuma::Luo(const QString &nimi, int tyyppi)
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

const Tapahtuma* Tapahtuma::Get(void)
{
    if (!ms_tapahtuma)
        Tapahtuma::Valitse(0);

    return ms_tapahtuma.data();
}

bool Tapahtuma::Valitse(int id)
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
