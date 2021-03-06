#include "emitdatamodel.h"

EmitDataModel::EmitDataModel(QObject *parent, QString numero, int vuosi, int kuukausi, QList<EmitLeima> leimat, SarjaP sarja) :
    QAbstractItemModel(parent),
    m_numero(numero),
    m_vuosi(vuosi),
    m_kuukausi(kuukausi),
    m_leimat(leimat),
    m_sarja(sarja)
{
    if (m_leimat.empty()) {
        m_leimat.reserve(50);

        for (int i = 0; i < 50; i++)
            m_leimat.append(EmitLeima{});
    }

    setSarja(sarja);
}

int EmitDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.parent().isValid()) {
            return 0;
        }

        return m_leimat.count();
    }

    return 1;
}

int EmitDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QModelIndex EmitDataModel::parent(const QModelIndex &child) const
{
    if (static_cast<int>(child.internalId()) == -1) {
        return QModelIndex();
    }

    return createIndex(0, 0, -1);
}

QModelIndex EmitDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return createIndex(row, column, row);
    }

    return createIndex(row, column, -1);
}


Qt::ItemFlags EmitDataModel::flags(const QModelIndex &index) const
{
    if (index.parent().isValid()) {
        if (index.column() == 0) {
            return QAbstractItemModel::flags(index);
        }
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}


QVariant EmitDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (section) {
        case 0:
            return _("Nro");
        case 1:
            return _("Koodi");
        case 2:
            return _("Aika");
        }
    }

    return QVariant();
}

QVariant EmitDataModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ForegroundRole) {
        return QVariant();
    }

    if (role == Qt::ForegroundRole) {
        if (!!m_sarja && static_cast<int>(index.internalId()) > -1) {
            return m_varit.at(index.row());
        }

        return QVariant();
    }

    if (static_cast<int>(index.internalId()) > -1) {
        switch (index.column()) {
            case 0:
                return index.row();
            case 1:
                return m_leimat.at(index.row()).m_koodi;
            case 2:
                return m_leimat.at(index.row()).m_aika;
        }
    }

    switch (index.column()) {
        case 0:
            return m_numero;

        case 1:
            return m_vuosi;

        case 2:
            return m_kuukausi;
    }

    return QVariant();
}

bool EmitDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) {
        return false;
    }

    if (static_cast<int>(index.internalId()) > -1) {
        EmitLeima rasti = m_leimat.at(index.row());

        switch (index.column()) {
            case 1:
                rasti.m_koodi = value.toInt();
                break;
            case 2:
                rasti.m_aika = value.toInt();
                break;
        }

        m_leimat.replace(index.row(), rasti);

        return true;
    }

    switch (index.column()) {
        case 0:
            m_numero = value.toString();
            break;

        case 1:
            m_vuosi = value.toInt();
            break;

        case 2:
            m_kuukausi = value.toInt();
            break;
    }

    return true;
}

QString EmitDataModel::getNumero() const
{
    return m_numero;
}

int EmitDataModel::getVuosi() const
{
    return m_vuosi;
}

int EmitDataModel::getKuukausi() const
{
    return m_kuukausi;
}

QList<EmitLeima> EmitDataModel::getRastit() const
{
    return m_leimat;
}

SarjaP EmitDataModel::getSarja() const
{
    return m_sarja;
}

void EmitDataModel::setSarja(SarjaP sarja)
{
    beginResetModel();

    m_sarja = sarja;
    m_varit.clear();

    if (!m_sarja) {
        for (int i = 0; i < m_leimat.count(); i++)
            m_varit.append(QColor(Qt::black));

        return;
    }

    const QList<Rasti> rastit = m_sarja->getRastit();

    int rasti_i = 0;
    int virheita = 0;
    const bool rogaining = Tapahtuma::IsRogaining();

    foreach (const EmitLeima& d, m_leimat) {
        if (d.m_koodi == 0) {
            m_varit.append(QColor(Qt::gray));
            continue;
        }

        if (d.m_koodi == 250 || d.m_koodi == 254) {
            m_varit.append(QColor(Qt::blue));
            continue;
        }

        bool ok = false;

        for (int i = 0; i <= virheita && rasti_i + i < rastit.count(); i++) {
            Rasti r = rastit.at(rasti_i + i);

            if (rogaining || r.sisaltaa(d.m_koodi)) {
                ok = true;
                break;
            }
        }

        if (ok) {
            rasti_i++;
            m_varit.append(QColor(Qt::darkGreen));
        } else {
            virheita++;
            m_varit.append(QColor(Qt::red));
        }
    }

    // Mikäli päästiin maaliin, voidaan merkitä väärin haetut harmaaksi
    if (rasti_i == rastit.count()) {
        for (int i = 0; i < m_varit.count(); i++)
            if (m_varit.at(i) == QColor(Qt::red))
                m_varit.replace(i, QColor(Qt::gray));
    }

    endResetModel();
}
