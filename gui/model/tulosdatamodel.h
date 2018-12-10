#ifndef TULOSDATAMODEL_H
#define TULOSDATAMODEL_H

#include <QtCore>

#include "model/emitdatamodel.h"

#include "type/sarja.h"
#include "type/rasti.h"
#include "type/emitleima.h"

#include "makrot.h"

class TulosDataModel : public EmitDataModel
{
    Q_OBJECT

public:
    struct Data
    {
        inline Data(const QVariant& _a, const QVariant& _b, const QVariant& _c, const QVariant& _d = QVariant{}) : a(_a), b(_b), c(_c), d(_d) {}

        QVariant a;
        QVariant b;
        QVariant c;
        QVariant d; // Juoksevat pisteet (ROGAINING)
    };

    explicit TulosDataModel(QObject *parent, QString numero, int vuosi, int kuukausi, QList<EmitLeima> leimat, SarjaP sarja = SarjaP{});

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void setSarja(SarjaP sarja);

    QTime getAika(bool sakkoton = false) const;
    int getPisteet(bool sakkoton = false) const;
    inline int getVirheet(void) const { return m_virheet; }
    QList<Data> getValiajat() const;

private:
    QList<Data> m_data;
    int m_haettu;
    int m_haettuLaite;
    QTime m_aika;   // Sakkoton aika
    int m_pisteet;  // Sakottomat pisteet
    int m_virheet;  // Yhteismäärä sakkoa, sekunteina (CLASSIC) tai pisteinä (ROGAINING)
};

#endif // TULOSDATAMODEL_H
