#include "sarja.h"

Sarja::Sarja(QObject *parent, const QVariant &id, const QString &nimi, int sakko,
             const QVariant& yhteislahto, const QTime &aikaraja, const QList<Rasti> rastit,
             bool data) :
    QObject(parent),
    m_id(id),
    m_nimi(nimi),
    m_sakko(sakko),
    m_yhteislahto(yhteislahto),
    m_aikaraja(aikaraja),
    m_rastit(rastit),
    m_data(data)
{
}

QList<Sarja*> Sarja::haeSarjat(QObject *parent, const Tapahtuma *tapahtuma)
{
    QList<Sarja*> sarjat;

    QSqlQuery query;

    query.prepare("SELECT * FROM sarja WHERE tapahtuma = ?");

    query.addBindValue(tapahtuma->id());

    SQL_EXEC(query, sarjat);

    while (query.next()) {
        QSqlRecord r = query.record();

        sarjat.append(new Sarja(parent, r.value("id"), r.value("nimi").toString(), r.value("sakko").toInt(),
                                r.value("yhteislahto"), r.value("aikaraja").toTime(), Rasti::haeRastit(r.value("id"))));
    }

    return sarjat;
}

QList<Sarja*> Sarja::haeSarjatData(QObject *parent, const Tapahtuma *tapahtuma)
{
    QList<Sarja*> sarjat;

    QSqlQuery query;

    query.prepare("SELECT * FROM sarja WHERE tapahtuma = ?");

    query.addBindValue(tapahtuma->id());

    SQL_EXEC(query, sarjat);

    while (query.next()) {
        QSqlRecord r = query.record();

        sarjat.append(new Sarja(parent, r.value("id"), r.value("nimi").toString(), r.value("sakko").toInt(),
                                r.value("yhteislahto"), r.value("aikaraja").toTime(), Rasti::haeRastitData(r.value("id")), true));
    }

    return sarjat;
}

Sarja * Sarja::haeSarja(QObject *parent, const QVariant &id)
{
    QSqlQuery query;

    query.prepare("SELECT * FROM sarja WHERE tapahtuma = ? AND id = ?");

    query.addBindValue(Tapahtuma::tapahtuma()->id());
    query.addBindValue(id);

    SQL_EXEC(query, 0);

    if (query.next()) {
        QSqlRecord r = query.record();

        return new Sarja(parent, id, r.value("nimi").toString(), r.value("sakko").toInt(),
                         r.value("yhteislahto"), r.value("aikaraja").toTime(), Rasti::haeRastit(id));
    }

    return 0;
}

QVariant Sarja::getId() const
{
    return m_id;
}

QString Sarja::getNimi() const
{
    return m_nimi;
}

QList<Rasti> Sarja::getRastit() const
{
    return m_rastit;
}

Rasti Sarja::getMaalirasti() const
{
    if (m_rastit.isEmpty()) {
        return Rasti(QVariant(), 0, QList<int>());
    }

    return m_rastit.last();
}

bool Sarja::isYhteislahto() const
{
    return !m_yhteislahto.isNull();
}

QVariant Sarja::getYhteislahto() const
{
    return m_yhteislahto;
}

bool Sarja::isAikaraja() const
{
    return m_aikaraja.isValid();
}

QTime Sarja::getAikaraja() const
{
    return m_aikaraja;
}

bool Sarja::isSakko() const
{
    return m_sakko != -1;
}

int Sarja::getSakko() const
{
    return isSakko() ? m_sakko : 0;
}

void Sarja::setNimi(const QVariant &nimi)
{
    if (!m_data) {
        return;
    }

    m_nimi = nimi.toString();
}

void Sarja::setSakko(const QVariant &sakko)
{
    if (!m_data) {
        return;
    }

    m_sakko = sakko.toInt();
    if (!m_sakko)
        m_sakko = -1;
}

void Sarja::setYhteislahto(const QVariant &yhteislahto)
{
    if (!m_data) {
        return;
    }

    if (yhteislahto.toDateTime().isValid()) {
        QDateTime a = yhteislahto.toDateTime();
        a.setTime(QTime(
            a.time().hour(),
            a.time().minute(),
            a.time().second()
        ));
        m_yhteislahto = a;
    } else {
        m_yhteislahto = QVariant();
    }
}

void Sarja::setAikaraja(const QVariant& aikaraja)
{
    if (!m_data) {
        return;
    }

    const QString s = aikaraja.toString().replace(':', '.');

    m_aikaraja = QTime::fromString(s, "h.m.s");
    if (!m_aikaraja.isValid())
        m_aikaraja = QTime::fromString(s, "h.m");
    if (!m_aikaraja.isValid())
        m_aikaraja = QTime::fromString(s, "m");

    qInfo() << "setAikaraja(" << s << ") -> " << m_aikaraja;
}

bool Sarja::dbUpdate() const
{
    if (!m_data) {
        return false;
    }

    QSqlQuery query;

    query.prepare("UPDATE sarja SET nimi = ?, sakko = ?, yhteislahto = ?, aikaraja = ? WHERE id = ?");

    query.addBindValue(m_nimi);
    query.addBindValue(m_sakko);
    query.addBindValue(m_yhteislahto);
    query.addBindValue(m_aikaraja);
    query.addBindValue(m_id);

    SQL_EXEC(query, false);

    return true;
}

void Sarja::replaceRasti(int index, const Rasti &rasti)
{
    m_rastit.replace(index, rasti);
}

Sarja * Sarja::dbInsert(QObject *parent, const Tapahtuma *tapahtuma)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;

    const bool rogaining = tapahtuma && tapahtuma->tyyppi() == RACE_ROGAINING;
    const QString nimi = _("Uusi rata");
    const int sakko = rogaining ? 1 : -1;
    const QTime aikaraja = rogaining ? QTime(1, 0) : QTime();

    query.prepare("INSERT INTO sarja (tapahtuma, nimi, sakko, aikaraja) VALUES (?, ?, ?, ?)");

    query.addBindValue(tapahtuma->id());
    query.addBindValue(nimi);
    query.addBindValue(sakko);
    query.addBindValue(aikaraja);

    SQL_EXEC(query, 0);

    QVariant id = query.lastInsertId();

    QSqlDatabase::database().commit();

    return new Sarja(parent, id, nimi, sakko, QVariant(), aikaraja, QList<Rasti>(), true);
}

void Sarja::insertRasti(int index, const Rasti &rasti)
{
    m_rastit.insert(index, rasti);
}

bool Sarja::dbDelete() const
{
    if (!m_data) {
        return false;
    }

    QSqlDatabase::database().transaction();

    QSqlQuery query;

    query.prepare("DELETE FROM rasti WHERE sarja = ?");

    query.addBindValue(m_id);

    SQL_EXEC(query, false);

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
