#include "sarja.h"

Sarja::Sarja(const QVariant &id, const QString &nimi, int sakko, QVariant yl, const QTime &aikaraja, const QList<Rasti> &rastit, bool rw) :
    m_id(id),
    m_nimi(nimi),
    m_sakko(sakko),
    m_yhteislahto(yl),
    m_aikaraja(aikaraja),
    m_rastit(rastit),
    m_rw(rw)
{
}

QVariant Sarja::getRataId(bool replace) const
{
    QVariant r;

    QSqlQuery query;
    query.prepare("SELECT rata FROM sarja WHERE id = ?");
    query.addBindValue(m_id);
    SQL_EXEC(query, r);

    if (query.next())
        r = query.value(0);

    if (!replace)
        return r;

    if (!r.isNull()) {
        query.prepare("DELETE FROM rata WHERE id = ?");
        query.addBindValue(r);
        SQL_EXEC(query, QVariant{});
        // Tässä kohdassa rata ja sen "rastisarja" tietueet ovat poistettu, sarjassa "rata" asetettu nulliksi
    }

    query.prepare("INSERT INTO rata(nimi, tapahtuma) VALUES (?, ?)");
    query.addBindValue(m_nimi);
    query.addBindValue(Tapahtuma::Id());
    SQL_EXEC(query, QVariant{});

    return query.lastInsertId();
}

QList<SarjaP> Sarja::haeSarjat(const Tapahtuma *tapahtuma, bool rw)
{
    QSqlQuery query;
    query.prepare("SELECT id, nimi, sakko, yhteislahto, aikaraja FROM sarja WHERE tapahtuma = ?");
    query.addBindValue(tapahtuma->id());

    QList<SarjaP> sarjat;
    SQL_EXEC(query, sarjat);

    while (query.next()) {
        QSqlRecord r = query.record();
        const QVariant sarja_id{r.value("id")};
        QList<Rasti> rastit{Rasti::haeRastit(sarja_id, rw)};

        sarjat << SarjaP{new Sarja(sarja_id, r.value("nimi").toString(), r.value("sakko").toInt(),
                                     r.value("yhteislahto"), r.value("aikaraja").toTime(), rastit, rw)};
    }

    return sarjat;
}

SarjaP Sarja::haeSarja(const QVariant &id)
{
    QSqlQuery query;
    query.prepare("SELECT nimi, sakko, yhteislahto, aikaraja FROM sarja WHERE tapahtuma = ? AND id = ?");
    query.addBindValue(Tapahtuma::tapahtuma()->id());
    query.addBindValue(id);

    SarjaP s;
    SQL_EXEC(query, s);

    if (query.next()) {
        QSqlRecord r = query.record();

        s.reset(new Sarja(id, r.value("nimi").toString(), r.value("sakko").toInt(),
                         r.value("yhteislahto"), r.value("aikaraja").toTime(), Rasti::haeRastitRO(id), false));
    }

    return s;
}

void Sarja::setNimi(const QVariant &nimi)
{
    if (m_rw)
        m_nimi = nimi.toString();
}

void Sarja::setSakko(const QVariant &sakko)
{
    if (m_rw)
        m_sakko = sakko.toInt();
}

void Sarja::setYhteislahto(const QVariant &yhteislahto)
{
    if (m_rw) {
        if (!yhteislahto.toDateTime().isValid())
            m_yhteislahto = QVariant();
        else {
            QDateTime a = yhteislahto.toDateTime();

            a.setTime(QTime(a.time().hour(), a.time().minute(), a.time().second()));
            m_yhteislahto = a;
        }
    }
}

void Sarja::setAikaraja(const QVariant& aikaraja)
{
    if (m_rw) {
        const QString s = aikaraja.toString().replace(':', '.');

        m_aikaraja = QTime::fromString(s, "h.m.s");
        if (!m_aikaraja.isValid())
            m_aikaraja = QTime::fromString(s, "h.m");
        if (!m_aikaraja.isValid())
            m_aikaraja = QTime::fromString(s, "m");
    }
}

bool Sarja::dbUpdate()
{
    if (!m_rw || m_id.isNull())
        return false;

    for (Rasti& r : m_rastit)
        if (!r.getId().isValid())
            r.dbUpdate();

    QSqlDatabase::database().transaction();

    // Luo rata uudestaan
    QVariant rata_id = getRataId(true);
    if (!rata_id.isValid() || rata_id.isNull())
        return false; // Transaktio on jo peruttu

    QSqlQuery query;

    if (!m_rastit.isEmpty()) {
        query.prepare("INSERT INTO rastisarja(rata, rasti, jarj) VALUES (?, ?, ?)");
        query.bindValue(0, rata_id);

        for (int i = 1; i <= m_rastit.size(); ++i) {
            const Rasti& rasti = m_rastit.at(i - 1);

            if (!rasti.getId().isValid()) {
                qInfo() << "Sarja::dbUpdate(): skipping" << rasti.getKoodi();
                continue;
            }

            query.bindValue(1, rasti.getId());
            query.bindValue(2, i);
            SQL_EXEC(query, false);
        }
    }

    // Päivitä sarjan tiedot
    query.prepare("UPDATE sarja SET nimi = ?, rata = ?, sakko = ?, yhteislahto = ?, aikaraja = ? WHERE id = ?");
    query.addBindValue(m_nimi);
    query.addBindValue(rata_id);
    query.addBindValue(m_sakko);
    query.addBindValue(m_yhteislahto);
    query.addBindValue(m_aikaraja);
    query.addBindValue(m_id);

    SQL_EXEC(query, false);

    QSqlDatabase::database().commit();

    return true;
}

void Sarja::replaceRasti(int index, const Rasti &rasti)
{
    m_rastit.replace(index, rasti);
}

bool Sarja::moveRasti(int &newindex, int from, int to)
{
    if (from < 0 || m_rastit.size() <= from)
        return false;

    if (to < 0)
        to = 0;

    if (m_rastit.size() <= to) {
        m_rastit.append(m_rastit.takeAt(from));
        newindex = m_rastit.size() - 1;
    }
    else {
        m_rastit.move(from, to);
        newindex = to;
    }

    return true;
}

SarjaP Sarja::dbInsert(const Tapahtuma *tapahtuma)
{
    if (!tapahtuma)
        tapahtuma = Tapahtuma::tapahtuma();

    QSqlDatabase::database().transaction();

    const bool rogaining = tapahtuma->tyyppi() == RACE_ROGAINING;
    const QString nimi = _("Uusi rata");
    const int sakko = rogaining ? 1 : -1;
    const QTime aikaraja = rogaining ? QTime(1, 0) : QTime();

    QSqlQuery query;
    query.prepare("INSERT INTO sarja (tapahtuma, nimi, sakko, aikaraja, sakkoyksikko) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(tapahtuma->id());
    query.addBindValue(nimi);
    query.addBindValue(sakko);
    query.addBindValue(aikaraja);
    query.addBindValue(int(60));

    SQL_EXEC(query, SarjaP{});

    QVariant id = query.lastInsertId();

    QSqlDatabase::database().commit();

    return SarjaP{new Sarja(id, nimi, sakko, QVariant(), aikaraja, QList<Rasti>{}, true)};
}

void Sarja::insertRasti(int index, const Rasti &rasti)
{
    m_rastit.insert(index, rasti);
}

bool Sarja::dbDelete() const
{
    if (!m_rw)
        return false;

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    QVariant rata_id = getRataId(false);

    if (!rata_id.isNull()) {
        query.prepare("DELETE FROM rata WHERE id = ?");
        query.addBindValue(rata_id);
        SQL_EXEC(query, false);
        // Tässä kohdassa rata ja sen "rastisarja" tietueet ovat poistettu, sarjassa "rata" asetettu nulliksi
    }

    query.prepare("DELETE FROM sarja WHERE id = ?");
    query.addBindValue(m_id);
    SQL_EXEC(query, false);

    QSqlDatabase::database().commit();

    return true;
}

void Sarja::removeRasti(int index)
{
    m_rastit.removeAt(index);
}
