#ifndef TAPAHTUMA_H
#define TAPAHTUMA_H

#include <QtCore>
#include <QtSql>

#include "makrot.h"

class Tapahtuma
{
public:
    enum Type {
        RACE_CLASSIC = 0,
        RACE_ROGAINING,
    };

    int id() const;
    QString nimi() const;
    int tyyppi(void) const;
    QString tyyppiNimi(void) const;
    QStringList tyypit(void) const;

    static const Tapahtuma* Get(void);
    static inline int Id(void) { return Get()->id(); }
    static inline QString Nimi(void) { return Get()->nimi(); }
    static inline int Tyyppi(void) { return Get()->tyyppi(); }
    static inline QString TyyppiNimi(void) { return Get()->tyyppiNimi(); }
    static inline QStringList Tyypit(void) { return Get()->tyypit(); }

    static inline bool IsClassic(void) { return Tyyppi() == RACE_CLASSIC; }
    static inline bool IsRogaining(void) { return Tyyppi() == RACE_ROGAINING; }

    static void Luo(const QString& nimi, int tyyppi = RACE_CLASSIC);
    static bool Valitse(int id);

private:
    class P;

    Tapahtuma(int id = 0, const QString &nimi = QString(), int tyyppi = RACE_CLASSIC);

    int m_id;
    QString m_nimi;
    int m_tyyppi;

    static P ms_tapahtuma;
};

#endif // TAPAHTUMA_H
