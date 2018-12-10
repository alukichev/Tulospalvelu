#include <QSet>

#include "tulosdatamodel.h"

TulosDataModel::TulosDataModel(QObject *parent, QString numero, int vuosi, int kuukausi, QList<EmitLeima> leimat, SarjaP sarja) :
    EmitDataModel(parent, numero, vuosi, kuukausi, leimat, SarjaP{}),
    m_haettu(0),
    m_haettuLaite(0),
    m_aika(), m_pisteet(0), m_virheet(0)
{
    foreach (const EmitLeima& d, m_leimat) {
        if (d.m_koodi == 250 || d.m_koodi == 254 || d.m_koodi == 99) {
            m_haettuLaite++;
        } else if (d.m_koodi != 0) {
            m_haettu++;
        }
    }

    setSarja(sarja);
}

QVariant TulosDataModel::data(const QModelIndex &index, int role) const
{
    if (!m_sarja)
        return QVariant();

    if (role == Qt::ForegroundRole || static_cast<int>(index.internalId()) <= -1)
        return EmitDataModel::data(index, role);

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    Data d = m_data.at(index.row());

    switch (index.column()) {
    case 0:
        return d.a;
    case 1:
        return d.b;
    case 2:
        return QTime(0,0).addSecs(d.c.toInt()).toString("HH:mm:ss") + _(" (%1)").arg(d.c.toString());
    }

    return QVariant();
}

Qt::ItemFlags TulosDataModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

int TulosDataModel::rowCount(const QModelIndex &parent) const
{
    if (!m_sarja) {
        return 0;
    }

    if (parent.isValid()) {
        if (parent.parent().isValid()) {
            return 0;
        }

        return m_data.count();
    }

    return 1;
}

void TulosDataModel::setSarja(SarjaP sarja)
{
    beginResetModel();

    m_sarja = sarja;
    m_aika = QTime();
    m_pisteet = 0;
    m_virheet = 0;
    m_varit.clear();
    m_data.clear();

    if (!m_sarja)
        return;

    const QList<Rasti> rastit = m_sarja->getRastit();

    int data_i = 0;
    int rasti_i = 0;
    const bool rogaining = Tapahtuma::tapahtuma()->tyyppi() == RACE_ROGAINING;

    QSet<int> haetut_rastit;
    haetut_rastit.reserve(rastit.size());

    while (data_i < m_leimat.count() || rasti_i < rastit.count()) {
        EmitLeima d(-1, -1);

        if (data_i < m_leimat.count())
           d = m_leimat.at(data_i);

        // ohitetaan 0 koodilla olevat rastit
        if (d.m_koodi == 0) {
            data_i++;
            continue;
        }

        Rasti r;
        if (rasti_i < rastit.count())
            r = rastit.at(rasti_i);

        // Luetut rastit loppuivat
        if (d.m_koodi == -1 && d.m_aika == -1) {
            if (!rogaining && rasti_i < rastit.count()) {
                m_data.append(Data(rasti_i + 1, _("-"), _("rasti puuttuu")));
                m_varit.append(QColor(Qt::red));
            }
            rasti_i++;
            continue;
        }

        // Lukulaitteiden koodit ja 99 koodi
        // Käsitellään seuraava RastiData ja Rasti ei liiku
        if (d.m_koodi == 250 || d.m_koodi == 254 || d.m_koodi == 99) {
            m_data.append(Data(_("*"), d.m_koodi, d.m_aika));
            m_varit.append(QColor(Qt::blue));
            data_i++;
            continue;
        }

        if (rogaining) {
            int piste_inc = 0;

            for (int num = 1; num <= rastit.size(); ++num) {
                const Rasti& rr = rastit.at(num - 1);

                if (rr.sisaltaa(d.m_koodi)) {
                    // Tunnetuista rasteista saadaan pisteet vain kerran
                    if (!haetut_rastit.contains(num)) {
                        piste_inc = rr.getPisteet();
                        haetut_rastit.insert(num);
                    }

                    break;
                }
            }

            m_pisteet += piste_inc;
            m_data.append(Data(data_i + 1, d.m_koodi, d.m_aika, m_pisteet));
            m_varit.append(QColor(piste_inc ? Qt::darkGreen : Qt::gray));

            ++data_i;
            ++rasti_i;
        }
        else {
            // Suunnistus - rastit haettava järjestyksessä
            // Oikein ja oikeassa paikassa haettu rasti
            if (r.sisaltaa(d.m_koodi)) {
                m_data.append(Data(rasti_i + 1, d.m_koodi, d.m_aika));
                m_varit.append(QColor(Qt::darkGreen));
                data_i++;
                rasti_i++;

                continue;
            }

            // Tarkistetaan onko haettu rasti radalla
            bool found = false;

            for (int i = rasti_i; i < rastit.count(); i++) {
                Rasti rr = rastit.at(i);
                if (rr.sisaltaa(d.m_koodi)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                m_data.append(Data(_("?"), d.m_koodi, d.m_aika));
                m_varit.append(QColor(Qt::gray));
                data_i++;

                continue;
            }

            // Tarkistetaan onko rastia haettu
            found = false;

            for (int i = data_i; i < m_leimat.count(); i++) {
                EmitLeima dd = m_leimat.at(i);

                if (r.sisaltaa(dd.m_koodi)) {
                    found = true;
                    break;
                }
            }

            bool last = true;

            // Tarkistetaan ettei rastia ole myöhempänä.
            // Tämä silmukoiden takia
            for (int i = rasti_i + 1; i < rastit.count(); i++) {
                Rasti rr = rastit.at(i);

                if (rr.sisaltaa(r.getKoodi(0))) {
                    last = false;
                    break;
                }
            }

            // merkitään harmaaksi rastit, jotka haettiin tässä välissä
            if (found && last) {
                for (; data_i < m_leimat.count(); data_i++) {
                    EmitLeima dd = m_leimat.at(data_i);

                    if (r.sisaltaa(dd.m_koodi)) {
                        m_data.append(Data(rasti_i + 1, dd.m_koodi, dd.m_aika));
                        m_varit.append(QColor(Qt::darkGreen));

                        break;
                    }
                    else {
                        m_data.append(Data(_("?"), dd.m_koodi, dd.m_aika));
                        m_varit.append(QColor(Qt::gray));
                    }
                }

                data_i++;
                rasti_i++;

                continue;
            }

            QString tila = _("rasti puuttuu");

            for (int i = data_i; i < m_leimat.count(); i++) {
                EmitLeima dd = m_leimat.at(i);

                if (r.sisaltaa(dd.m_koodi)) {
                    tila = _("rasti väärin");
                    break;
                }
            }

            m_data.append(Data(rasti_i + 1, _("-"), tila));
            m_varit.append(QColor(Qt::red));

            rasti_i++;
        }
    }

    // Laske sakkoton aika
    int aika = 0, aika_250 = 0;
    foreach (const EmitLeima& d, m_leimat) {
        if (d.m_koodi == 0)
            continue;

        if (d.m_koodi == 250) {
            aika_250 = d.m_aika;
            continue;
        }

        // Pistesuunnistuksessa aika otetaan viimeiseksi kerätystä rastileimasta tai maalileimasta
        // FIXME: normaalisuunnistuksessa, jos sarjassa sakot, puuttuva maalirasti aiheuttaa 0
        // FIXME: pistesuunnistuksessa viimeisen leiman pitää kuulla rataan
        if (rogaining || m_sarja->getMaalirasti().sisaltaa(d.m_koodi))
            aika = d.m_aika;
    }

    if (!aika)
        aika = aika_250;

    m_aika = QTime(0,0).addSecs(aika);

    // Laske virheet (HUOM.: ajan laskettua)
    if (!rogaining) {
        foreach (const EmitLeima& d, m_leimat) {
            if (d.m_aika <= 5 && d.m_koodi != 0)
                ++m_virheet;
        }

        m_virheet += m_varit.count(QColor(Qt::red));
    }
    else if (m_sarja->isAikaraja()) {
            const int diff_s = m_sarja->getAikaraja().secsTo(m_aika);

            m_virheet = diff_s <= 0 ? 0 : (diff_s + 59) / 60; // Joka alkavasta minuutista tulee 1 virhe
    }

    endResetModel();
}

QTime TulosDataModel::getAika(bool sakkoton) const
{
    const bool rogaining = Tapahtuma::tapahtuma()->tyyppi() == RACE_ROGAINING;
    return sakkoton || !m_sarja || rogaining ? m_aika : m_aika.addSecs(getVirheet() * m_sarja->getSakko());
}

int TulosDataModel::getPisteet(bool sakkoton) const
{
    const bool rogaining = Tapahtuma::tapahtuma()->tyyppi() == RACE_ROGAINING;
    int r = m_pisteet;

    if (!sakkoton && rogaining) {
        r -= getVirheet();
        if (r < 0)
            r = 0;
    }

    return r;
}

QList<TulosDataModel::Data> TulosDataModel::getValiajat() const
{
    QList<Data> valiajat;

    if (!m_sarja)
        return valiajat;

    for (int i = 0; i < m_data.count(); i++)
        if (m_varit.at(i) == Qt::darkGreen)
            valiajat << m_data.at(i);

    return valiajat;
}
