#include <QMap>
#include <QSet>

#include "rasti.h"

#include "type/sarja.h"

// Hae "Leimasin -> rasti id" taulukko
static QMap<int,int> GetLeimasimet(int tapahtuma = Tapahtuma::Id())
{
    QMap<int,int> r;

    QSqlQuery q;
    q.prepare("SELECT koodi, rasti FROM leimasin WHERE tapahtuma = ?");
    q.addBindValue(tapahtuma);
    SQL_EXEC(q, r);

    while (q.next())
        r.insert(q.value(0).toInt(), q.value(1).toInt());

    return r;
}

static inline QList<int> ParseLeimasimet(const QVariant& l)
{
    QList<int> list;

    foreach (const QString& s, l.toString().split(",")) {
        bool ok;
        const int k = s.toInt(&ok);

        if (ok)
            list << k;
    }

    return list;
}

bool Rasti::KoodiSallittu(int koodi)
{
    static QSet<int> _kielletyt;

    if (_kielletyt.isEmpty()) {
        _kielletyt.reserve(8);
        _kielletyt << 66 << 68 << 86 << 89 << 98 << 99 << 161 << 191;
    }

    return 31 <= koodi && koodi <= 599 && !_kielletyt.contains(koodi);
}

QList<Rasti> Rasti::haeRastit(const QVariant &sarjaId, bool rw)
{
    QList<Rasti> rastit;

    // Ensin selvittää rata id
    QSqlQuery query;
    query.prepare("SELECT rata FROM sarja WHERE id = ?");
    query.addBindValue(sarjaId);
    SQL_EXEC(query, rastit);

    if (query.next() && query.value(0).isNull())
        return rastit; // Rataa ei määritetty

    // Hae radan rastit ja leimasinten koodit
    const QVariant rata_id = query.value(0);

    query.prepare("SELECT r.id, r.koodi, r.pisteet, GROUP_CONCAT(l.koodi) AS leimasimet\n"
                  "FROM rastisarja AS rs\n"
                  " JOIN rasti AS r ON r.id = rs.rasti\n"
                  " JOIN leimasin AS l ON r.id = l.rasti\n"
                  "WHERE rs.rata = ?\n"
                  "GROUP BY r.id\n"
                  "ORDER BY rs.jarj ASC, r.koodi ASC, l.koodi ASC");
    query.addBindValue(rata_id);
    SQL_EXEC(query, rastit);

    // Luo Rasti
    while (query.next()) {
        QList<int> koodit = ParseLeimasimet(query.value("leimasimet"));

        if (koodit.isEmpty()) {
            qWarning() << "Rastilla" << query.value("koodi") << _("tyhjä koodilista, jätetään rasti pois radasta");
            continue;
        }

        rastit << Rasti(query.value("id"), query.value("koodi").toInt(), query.value("pisteet").toInt(), koodit, rw);
    }

    return rastit;
}

Rasti::Rasti(const QVariant& id, int koodi, int pisteet, const QList<int> &koodit, bool rw) :
    m_id(id),
    m_koodi(koodi),
    m_pisteet(pisteet),
    m_koodit(koodit), // Jos lista on tyhjä, niin se sopii
    m_rw(rw)
{
}

// Jos tietokannasta löytyy saman tapahtuman ja koodin rasti, palauta se. Muuten luo se,
// ilman leimasinlistaa.
Rasti Rasti::_hae(int tapahtuma) const
{
    QSqlQuery q;

    q.prepare("SELECT id, pisteet FROM rasti WHERE koodi = ? AND tapahtuma = ?");
    q.addBindValue(m_koodi);
    q.addBindValue(tapahtuma);
    SQL_EXEC(q, Rasti{});

    if (q.next()) {
        QVariant id = q.value(0), pisteet = q.value(1);

        q.prepare("SELECT GROUP_CONCAT(koodi) AS leimasimet FROM leimasin\n"
                  "WHERE rasti = ? AND tapahtuma = ?\n"
                  "GROUP BY rasti\n"
                  "ORDER BY koodi ASC");
        q.addBindValue(id);
        q.addBindValue(tapahtuma);
        SQL_EXEC(q, Rasti{});

        QList<int> koodit;
        if (q.next())
            koodit = ParseLeimasimet(q.value("leimasimet"));

        return Rasti{id, m_koodi, pisteet.toInt(), koodit, false};
    }
    else {
        q.prepare("INSERT INTO rasti(koodi, pisteet, tapahtuma) VALUES (?, ?, ?)");
        q.addBindValue(m_koodi);
        q.addBindValue(m_pisteet);
        q.addBindValue(tapahtuma);
        SQL_EXEC(q, Rasti{});

        return Rasti{q.lastInsertId(), m_koodi, m_pisteet, QList<int>{}, false};
    }
}

bool Rasti::_dbDeref(const Sarja& s)
{
    if (!m_id.isValid())
        return true;

//    qInfo() << "Rasti::_dbDeref()" << m_id << m_koodi;

    QSqlDatabase::database().transaction();

    // Poista rasti sarjan radasta
    QSqlQuery q;
    q.prepare("SELECT rata FROM sarja WHERE id = ?");
    q.addBindValue(s.getId());
    SQL_EXEC(q, true); // Ei ole sarjaa

    QVariant rata_id;
    if (q.next())
         rata_id = q.value(0);
    if (!rata_id.isNull()) {
        q.prepare("DELETE FROM rastisarja WHERE rasti = ? AND rata = ?");
        q.addBindValue(m_id);
        q.addBindValue(rata_id);
        SQL_EXEC(q, false);
    }

    // Poista rasti ellei ole muuassa radassa käyttöä, poistaen myös leimasimet
    q.prepare("SELECT COUNT(*) FROM rastisarja WHERE rasti = ?");
    q.addBindValue(m_id);
    SQL_EXEC(q, false);

    if (!q.next() || q.value(0).toInt() == 0) {
        q.prepare("DELETE FROM leimasin WHERE rasti = ?");
        q.addBindValue(m_id);
        SQL_EXEC(q, false);

        q.prepare("DELETE FROM rasti WHERE id = ?");
        q.addBindValue(m_id);
        SQL_EXEC(q, false);

    }

    QSqlDatabase::database().commit();
    m_id.clear();

    return true;
}

Rasti &Rasti::setPisteet(const QVariant &pisteet)
{
    if (m_rw)
        m_pisteet = pisteet.toInt();

    return *this;
}

Rasti& Rasti::setKoodi(const Sarja& sarja, const QVariant &koodi)
{
    if (!m_rw)
        return *this;

//    qInfo() << "Rasti::setKoodi()" << m_id << m_koodi << koodi;

    bool ok;
    int k = koodi.toInt(&ok);
    if (!ok || !KoodiSallittu(k) || (k == m_koodi && m_koodit.size() == 1 && m_koodit.at(0) == k))
        return *this;

    if (!_dbDeref(sarja))
        return *this;

    m_koodi = k;

    if (!m_koodit.isEmpty())
        m_koodit.clear();
    m_koodit << k;

    return *this;
}

bool Rasti::dbUpdate()
{
    if (!m_rw)
        return false;

//    qInfo() << "Rasti::dbUpdate()" << m_id << m_koodi << m_pisteet << "leimasimet: " << m_koodit.size();

    const int tapahtuma = Tapahtuma::Id();
    const Rasti db_rasti = _hae(tapahtuma);

    if (!db_rasti.m_id.isValid())
        return false; // Virhe

    QSqlDatabase::database().transaction();

    m_id = db_rasti.m_id;

    // Jos rastia ei ollut, se on luotu nyt. Päivitä sen tiedot
    // Jos rasti on olemassa jo, sen tiedot ovat db_rasti:ssa. Päivitä vain jos eroaa.
    if (db_rasti.m_pisteet != m_pisteet) {
        QSqlQuery query;

        query.prepare("UPDATE rasti SET pisteet = ? WHERE id = ?");
        query.addBindValue(m_pisteet);
        query.addBindValue(m_id);
        SQL_EXEC(query, false);
    }

//    qInfo() << "db_rasti:" << db_rasti.m_id << db_rasti.m_koodi << db_rasti.m_pisteet << db_rasti.m_koodit.size() << "(" << (db_rasti.m_koodit.size() ? db_rasti.m_koodit.first() : 0) << ")";

    if (db_rasti.m_koodit != m_koodit) {
        QSqlQuery query;

//        qInfo() << "Päivitetään leimasimet";

        // Poista tähän rastiin viittaavat leimasimet
        query.prepare("DELETE FROM leimasin WHERE tapahtuma = ? AND rasti = ?");
        query.addBindValue(tapahtuma);
        query.addBindValue(m_id);
        SQL_EXEC(query, false);

        // Lisää/päivitä leimasimet
        const QMap<int,int> t_leimasimet = GetLeimasimet(tapahtuma);
        foreach (int lkoodi, m_koodit) {
            const QMap<int,int>::const_iterator leim_found = t_leimasimet.find(lkoodi);

            if (leim_found == t_leimasimet.constEnd()) {
                // Leimasinta ko. koodilla ei ole - luodaan
                query.prepare("INSERT INTO leimasin(koodi, tapahtuma, rasti) VALUES (?, ?, ?)");
                query.addBindValue(lkoodi);
                query.addBindValue(tapahtuma);
                query.addBindValue(m_id);
                SQL_EXEC(query, false);
            }
            else if (leim_found.value() != m_id.toInt()) {
                // Leimasin on siirrettävä muulta rastilta - siirretään
                query.prepare("UPDATE leimasin SET rasti = ? WHERE koodi = ? AND tapahtuma = ?");
                query.addBindValue(m_id);
                query.addBindValue(lkoodi);
                query.addBindValue(tapahtuma);
                SQL_EXEC(query, false);
            }
        }
    }

    QSqlDatabase::database().commit();

    return true;
}

bool Rasti::dbDelete(const Sarja &sarja)
{
    return m_rw && m_id.isValid() && _dbDeref(sarja);
}
