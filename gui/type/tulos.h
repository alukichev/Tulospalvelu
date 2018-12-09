#ifndef TULOS_H
#define TULOS_H

#include <QtCore>

#include "type/tapahtuma.h"
#include "type/valiaika.h"
#include "type/sarja.h"

#include "makrot.h"

struct Tulos
{
    enum Tila {
        Avoin = 1,
        Hyvaksytty = 2,
        DNF = 3
    };

    static QList<Tulos> haeTulokset(SarjaP sarja);

    explicit Tulos(int id, const QString& sarja, int sija, const QString &_emit,
                   const QString& kilpailija, int tila, const QTime& aika, const QDateTime& maaliaika,
                   const QList<Valiaika>& valiajat, int pisteet = 0, int sakko = 0, int kpisteet = 0);

    int m_id;               // Tietokanta tunniste?, FIXME miksi ei ole QVariant?
    QString m_sarja;        // Sarjan nimi
    int m_sija;             // Sijoitus
    QString m_emit;         // Emitnumero
    QString m_kilpailija;   // Kilpailija nimi
    int m_tila;             // FIXME miksi ei käytetä Tila m_tila?
    QTime m_aika;           // Tulosaika, sakot mukaan lukien
    QDateTime m_maaliaika;  // Maaliin saapumisaika
    int m_pisteet;          // Kerättyjen rastien yhteispistemäärä (RACE_ROGAINING), sakot ja korjaus mukana
    int m_sakko;            // Yhteismäärä sakkoja sekunteina (RACE_CLASSIC) tai pisteinä (RACE_ROGAINING)
    int m_korjPisteet;      // "Manuaalinen" pistekorjaus

    QList<Valiaika> m_valiajat; // Väliajat
};

#endif // TULOS_H
