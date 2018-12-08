#ifndef VALIAIKA_H
#define VALIAIKA_H

#include <QtCore>
#include <QtSql>

#include "type/tapahtuma.h"

#include "type/sarja.h"
#include "type/rasti.h"

#include "makrot.h"

struct Valiaika
{
    static QList<Valiaika> haeValiajat(const QVariant& tulosId);
    static QList<Valiaika> haeRastiValiajat(SarjaP sarja, int jarj);

    inline Valiaika(void) : id(), jarj(0), koodi(0), aika(), sija(0) {}

    QVariant id;  // Tietokanta tunniste
    int jarj;     // Rastin järjestysnumero
    int koodi;    // Rastikoodi
    QTime aika;   // Aika
    int sija;     // Sijoitus

private:
    Valiaika(const QVariant& iid, int jjarj, int kkoodi, const QTime& aaika, int ssija);
};

#endif // VALIAIKA_H
