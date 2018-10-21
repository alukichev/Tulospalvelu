#ifndef SARJA_H
#define SARJA_H

#include <QtCore>
#include <QtSql>

#include "type/rasti.h"
#include "type/tapahtuma.h"

#include "makrot.h"

class Sarja : public QObject
{
    Q_OBJECT
public:
    explicit Sarja(QObject *parent, const QVariant& id, const QString& nimi, int sakko,
                   const QVariant& yhteislahto, const QTime& aikaraja, const QList<Rasti> rastit, bool data = false);

    static Sarja* haeSarja(QObject *parent, const QVariant &id);

    static QList<Sarja*> haeSarjat(QObject *parent, const Tapahtuma* tapahtuma = Tapahtuma::tapahtuma());
    static QList<Sarja*> haeSarjatData(QObject *parent, const Tapahtuma* tapahtuma = Tapahtuma::tapahtuma());

    QVariant getId() const;
    QString getNimi() const;
    bool isSakko() const;
    int getSakko() const;
    QList<Rasti> getRastit() const;
    Rasti getMaalirasti() const;
    bool isYhteislahto() const;
    QVariant getYhteislahto() const;
    bool isAikaraja(void) const;
    QTime getAikaraja(void) const;

    void setNimi(const QVariant& nimi);
    void setSakko(const QVariant& sakko);
    void setYhteislahto(const QVariant& yhteislahto);
    void setAikaraja(const QVariant& aikaraja);
    void replaceRasti(int index, const Rasti& rasti);
    void insertRasti(int index, const Rasti& rasti);
    void removeRasti(int index);

    static Sarja* dbInsert(QObject *parent, const Tapahtuma *tapahtuma);
    bool dbUpdate() const;
    bool dbDelete() const;

private:
    QVariant m_id;      // Tietokanta tunniste

    QString m_nimi;     // Nimi
    int m_sakko;    // Sakkomäärä sekuntteina (RACE_CLASSIC) tai pisteinä, -1 => Ei sakkoa
    QVariant m_yhteislahto; // Yhteislähtöaika
    QTime m_aikaraja;

    QList<Rasti> m_rastit;  // Rastit

    bool m_data;
    // FIXME näitä ei tarvita kun jatkossa ei käytetä kopiointia vaan osoittimia.
//    Sarja(const Sarja &s);
//    Sarja& operator =(const Sarja &s);
};

#endif // SARJA_H
