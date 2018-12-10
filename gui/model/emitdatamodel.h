#ifndef EMITDATAMODEL_H
#define EMITDATAMODEL_H

#include <QtWidgets>

#include "type/emitleima.h"
#include "type/sarja.h"
#include "type/rasti.h"

#include "makrot.h"

class EmitDataModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit EmitDataModel(QObject *parent, QString numero = "54321", int vuosi = 12, int kuukausi = 7, QList<EmitLeima> leimat = QList<EmitLeima>(), SarjaP sarja = SarjaP{});

    Qt::ItemFlags flags(const QModelIndex &index) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QModelIndex parent(const QModelIndex &child = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QString getNumero() const;
    int getVuosi() const;
    int getKuukausi() const;
    QList<EmitLeima> getRastit() const;

    void setSarja(SarjaP sarja);
    SarjaP getSarja() const;

protected:
    QString m_numero;
    int m_vuosi;
    int m_kuukausi;
    QList<EmitLeima> m_leimat;
    QList<QColor> m_varit;

    SarjaP m_sarja;
};

#endif // EMITDATAMODEL_H
