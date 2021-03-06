#include "rastimodel.h"

RastiModel::RastiModel(QObject *parent) :
    RataModel(parent)
{
}

QVariant RastiModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignLeft;
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return _("ID");
    case 1:
        return _("Järj.nro.");
    case 2:
        return _("Koodi");
    case 3:
        return _("Pisteet");
    }

    return QVariant();
}

