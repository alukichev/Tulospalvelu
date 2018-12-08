#include "type/heap.h"
#include "ratamodel.h"

static inline Heap<int> ToHeap(const QList<Rasti>& rastit)
{
    Heap<int> h(Heap<int>::DefLess, rastit.size());

    foreach (const Rasti& r, rastit)
        h.insert(r.getKoodi());

    return h;
}

// Hae välistä 31..599 vapaa koodi
static inline int LaskeVapaaKoodi(SarjaP s)
{
    Heap<int> koodit = ToHeap(s->getRastit());

    if (koodit.isEmpty() || 31 < koodit.min())
        return 31;

    int koodi;
    for (koodi = koodit.extractMin(); !koodit.isEmpty(); koodi = koodit.extractMin())
        while (++koodi < koodit.min())
            if (Rasti::KoodiSallittu(koodi))
                return koodi;

    return koodi < 599 ? koodi + 1 : 0;
}

template<typename T>
static int FindPList(const QList<QSharedPointer<T> >& list, const T *p)
{
    int i;

    for (i = 0; i < list.size(); ++i)
        if (list.at(i).data() == p)
            return i;

    return -1;
}

static inline Rasti LuoRasti(bool *ok, SarjaP s, int tapahtumatyyppi)
{
    int koodi = LaskeVapaaKoodi(s);
    Rasti r(koodi, tapahtumatyyppi == RACE_ROGAINING ? koodi / 10 : 0);
    bool db = r.dbUpdate();

    if (ok)
        *ok = db;

    return r;
}

RataModel::RataModel(QObject *parent, const Tapahtuma *tapahtuma) :
    QAbstractItemModel(parent),
    m_tapahtuma(tapahtuma),
    m_sarjat(Sarja::haeSarjat(tapahtuma))
{
}

Qt::ItemFlags RataModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

int RataModel::columnCount(const QModelIndex &parent) const
{
    const bool is_rasti = parent.isValid();

    return is_rasti ? 4 : 5;
}

int RataModel::rowCount(const QModelIndex &parent) const
{
    const bool is_rasti = parent.isValid();

    if (!is_rasti)
        return m_sarjat.count();
    else
        return !parent.parent().isValid() ? m_sarjat.at(parent.row())->getRastit().count() : 0;
}

QVariant RataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return _("ID");
    case 1:
        return _("Nimi");
    case 2:
        return m_tapahtuma->tyyppi() == RACE_ROGAINING ? _("Sakkopisteet") : _("Sakkoaika");
    case 3:
        return _("Lähtöaika");
    case 4:
        return _("Aikaraja");
    }

    return QVariant();
}

QModelIndex RataModel::parent(const QModelIndex &child) const
{
    const bool valid = static_cast<int>(child.internalId()) != -1;

    if (!valid)
        return QModelIndex();
    else {
        const Sarja *s = static_cast<Sarja*>(child.internalPointer());
        const int row = FindPList(m_sarjat, s);

        return createIndex(row, 0, -1);
    }
}

QModelIndex RataModel::index(int row, int column, const QModelIndex &parent) const
{
    const bool is_rasti = parent.isValid();

    if (!is_rasti)
        return createIndex(row, column, -1);
    else
        return parent.parent().isValid() ? QModelIndex() : createIndex(row, column, m_sarjat.at(parent.row()).data());
}

QVariant RataModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::EditRole && role != Qt::DisplayRole)
        return QVariant();

    const bool is_rasti = static_cast<int>(index.internalId()) != -1;

    if (!is_rasti) {
        // Sarja
        SarjaP s = m_sarjat.at(index.row());

        switch (index.column()) {
        case 0:
            return s->getId();
        case 1:
            return s->getNimi();
        case 2:
            return s->isSakko() ? s->getSakko() : 0;
        case 3:
            return s->isYhteislahto() ? s->getYhteislahto().toDateTime() : QVariant();
        case 4:
            return s->isAikaraja() ? s->getAikaraja().toString("h.mm.ss") : QString();
        }
    }
    else {
        // Rata - rasti
        SarjaP s = m_sarjat.at(index.parent().row());
        const Rasti& r = s->getRastit().at(index.row());

        switch (index.column()) {
        case 0:
            return r.getId();
        case 1:
            return index.row() + 1;
        case 2:
            return r.getKoodi();
        case 3:
            return r.getPisteet();
        }
    }

    return QVariant();
}

bool RataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    const bool is_rasti = static_cast<int>(index.internalId()) != -1;

    bool res = false;
    bool valid = true;

    if (!is_rasti) {
        // Sarjan editointi
        SarjaP s = m_sarjat.at(index.row());

        switch (index.column()) {
        case 1:
            s->setNimi(value);
            break;
        case 2:
            s->setSakko(value);
            break;
        case 3:
            s->setYhteislahto(value);
            break;
        case 4:
            s->setAikaraja(value);
            break;
        case 0:
        default:
            valid = false;
            break;
        }

        res = valid ? s->dbUpdate() : false;
    }
    else {
        // Radan (rastin) editointi
        SarjaP s = m_sarjat.at(index.parent().row());
        Rasti r = s->getRastit().at(index.row());
        bool rchanged = false, schanged = false;

        switch (index.column()) {
        case 1: {
                bool ok;
                int to = value.toInt(&ok); // 1-based

                valid = ok ? s->moveRasti(to, index.row(), to - 1) : false;
                schanged = valid;
            }
            break;
        case 2:
            if (r.setKoodi(*s, value).getKoodi() == value.toInt()) {
                r.setPisteet(r.getKoodi() / 10);
                rchanged = true;
                schanged = true;
            }
            break;
        case 3:
            if (r.setPisteet(value).getPisteet() == value.toInt())
                rchanged = true;
            break;
        case 0:
        default:
            valid = false;
            break;
        }

        res = valid ? (rchanged ? r.dbUpdate() : true) : false;
        if (rchanged)
            s->replaceRasti(index.row(), r);
        if (schanged)
            s->dbUpdate();
    }

    if (res)
        emit dataChanged(index, index);

    return res;
}

bool RataModel::insertRow(int row, const QModelIndex &parent)
{
    const bool is_rasti = parent.isValid();
    bool ok = true;

    beginInsertRows(parent, row, row);

    if (!is_rasti)
        m_sarjat.insert(row, Sarja::dbInsert(m_tapahtuma));
    else {
        SarjaP s = m_sarjat.at(parent.row());

        s->insertRasti(row, LuoRasti(&ok, s, m_tapahtuma->tyyppi()));
        if (ok)
            s->dbUpdate();
    }

    endInsertRows();

    return ok;
}

bool RataModel::removeRow(int row, const QModelIndex &parent)
{
    const bool is_rasti = parent.isValid();

    beginRemoveRows(parent, row, row);

    const int s_index = is_rasti ? parent.row() : row;
    SarjaP s = m_sarjat.at(s_index);
    bool res;

    if (is_rasti) {
        Rasti r = s->getRastit().at(row);

        res = r.dbDelete(*s);
        if (res) {
            s->removeRasti(row);
            res = s->dbUpdate();
        }
    }
    else {
        res = s->dbDelete();

        if (res)
            m_sarjat.removeAt(row);
    }

    endRemoveRows();

    return res;
}
