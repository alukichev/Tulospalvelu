#ifndef RASTI_H
#define RASTI_H

#include <QtCore>
#include <QSharedPointer>
#include <QtSql>

#include "makrot.h"

class Sarja;
typedef QSharedPointer<Sarja> SarjaP;

class Rasti
{
public:
    // Palauttaa jokaista rastia ja koodi paria kohden yhden Rasti:n
    // Eli ei grouppaa rastinumeron perusteella. Tarvitaan RataModel:n käytössä
    static QList<Rasti> haeRastit(const QVariant &sarjaId, bool rw = true);
    static inline QList<Rasti> haeRastitRO(const QVariant &sarjaId) { return haeRastit(sarjaId, false); }
    static Rasti dbInsert(SarjaP sarja, int numero = 0, int koodi = 0, int pisteet = 0);
    static bool KoodiSallittu(int koodi);

    inline Rasti(bool rw = false) : m_id(), m_koodi(0), m_pisteet(0), m_koodit(), m_rw(rw) {}
    inline Rasti(int koodi, int pisteet) : m_id(), m_koodi(koodi), m_pisteet(pisteet), m_koodit(QList<int>{} << koodi), m_rw(true) {}

    inline QVariant getId() const { return m_id; }
    inline int getPisteet(void) const { return m_pisteet; }

    inline int getKoodi(void) const { return m_koodi; }
    inline int getKoodi(int i) const { return i < 0 || i >= m_koodit.count() ? -1 : m_koodit.at(i); }
    inline bool sisaltaa(int koodi) const { return m_koodit.contains(koodi); }

    Rasti& setPisteet(const QVariant& pisteet);
    Rasti& setKoodi(const Sarja& sarja, const QVariant& koodi);

    bool dbUpdate();
    bool dbDelete(const Sarja& sarja);

private:
    Rasti(const QVariant& id, int koodi, int pisteet, const QList<int>& koodit, bool rw);

    bool _dbDeref(const Sarja& s);
    Rasti _hae(int tapahtuma) const;

    QVariant m_id;          // Tietokanta tunniste
    int m_koodi;            // Rastin järjestysnumero
    int m_pisteet;          // Rastin pistemäärä
    QList<int> m_koodit;    // Rastikoodit
    bool m_rw;              // Päivitettävissä
};

#endif // RASTI_H
