#ifndef SARJA_H
#define SARJA_H

#include <QSharedPointer>
#include <QtCore>
#include <QtSql>

#include "type/rasti.h"
#include "type/tapahtuma.h"

#include "makrot.h"

class Sarja;
typedef QSharedPointer<Sarja> SarjaP;

class Sarja
{
public:
    static SarjaP dbInsert(void);
    static SarjaP haeSarja(const QVariant &id);
    static QList<SarjaP> haeSarjat(bool rw = true);
    static inline QList<SarjaP> haeSarjatRO(void) { return haeSarjat(false); }

    inline QVariant getId() const { return m_id; }
    inline QString getNimi() const { return m_nimi; }
    inline bool isSakko() const { return 0 < m_sakko; }
    inline int getSakko() const { return isSakko() ? m_sakko : 0; }
    inline QList<Rasti> getRastit() const { return m_rastit; }
    inline Rasti getMaalirasti() const { return m_rastit.isEmpty() ? Rasti{} : m_rastit.last(); }
    inline bool isYhteislahto() const { return !m_yhteislahto.isNull(); }
    inline QVariant getYhteislahto() const { return m_yhteislahto; }
    inline bool isAikaraja(void) const { return m_aikaraja.isValid(); }
    inline QTime getAikaraja(void) const { return m_aikaraja; }

    void setNimi(const QVariant& nimi);
    void setSakko(const QVariant& sakko);
    void setYhteislahto(const QVariant& yhteislahto);
    void setAikaraja(const QVariant& aikaraja);

    bool moveRasti(int& newindex, int from, int to);
    void replaceRasti(int index, const Rasti& rasti);
    void insertRasti(int index, const Rasti& rasti);
    void removeRasti(int index);

    bool dbUpdate();
    bool dbDelete() const;

private:
    Sarja(const QVariant& id, const QString& nimi, int sakko, QVariant yl, const QTime& aikaraja, const QList<Rasti>& rastit, bool rw = false);
    Sarja(const Sarja &s) = delete;
    Sarja& operator =(const Sarja &s) = delete;

    QVariant getRataId(bool replace = false) const;

    QVariant m_id;      // Tietokanta tunniste

    QString m_nimi;     // Nimi
    int m_sakko;    // Sakkomäärä sekuntteina (RACE_CLASSIC) tai pisteinä, -1 => Ei sakkoa
    QVariant m_yhteislahto; // Yhteislähtöaika
    QTime m_aikaraja;

    QList<Rasti> m_rastit;  // Rastit

    bool m_rw;
};

#endif // SARJA_H
