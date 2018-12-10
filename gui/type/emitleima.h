#ifndef EMITLEIMA_H
#define EMITLEIMA_H

#include <QtCore>
#include <QtSql>

#include "makrot.h"

struct EmitLeima
{
    static QList<EmitLeima> haeLeimat(const QVariant& luettu_emit_id);

    inline EmitLeima(int koodi = 0, int aika = 0) : m_koodi(koodi), m_aika(aika) {}

    int m_koodi;    // Leimasinkoodi
    int m_aika;     // Juokseva aikaleima (nollauksesta) sekuntteina

};

#endif // EMITLEIMA_H
