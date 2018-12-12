#ifndef TAPAHTUMA_H
#define TAPAHTUMA_H

#include <QtCore>
#include <QtSql>

#include "makrot.h"

class Tapahtuma
{
public:
    int id() const;
    QString nimi() const;
    int tyyppi(void) const;


    static bool Valitse(int id);
    static const Tapahtuma* Get(void);
    static inline int Id(void) { return Get()->id(); }
    static inline int Tyyppi(void) { return Get()->tyyppi(); }

    static void Luo(const QString& nimi, int tyyppi = RACE_CLASSIC);

private:
    class P;

    Tapahtuma(int id = 0, const QString &nimi = QString(), int tyyppi = RACE_CLASSIC);

    int m_id;
    QString m_nimi;
    int m_tyyppi;

    static P ms_tapahtuma;
};

#endif // TAPAHTUMA_H
