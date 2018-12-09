#include <QVector>

#include "tuloksetform.h"
#include "ui_tuloksetform.h"

static inline QString TimeFormat(const QTime& time)
{
    return (time.toString((time.hour() ? _("H.") : _("")) + (time.hour() || time.minute() ? _("mm.") : _("")) + _("ss")));
}

static QString RastivalitClassic(SarjaP s, const QList<Tulos>& db_tulokset)
{
    QList<Tulos> tulokset;
    tulokset.reserve(db_tulokset.size());

    foreach (Tulos t, db_tulokset) {
        QTime edellinen;

        QList<Valiaika> valiajat;
        valiajat.reserve(t.m_valiajat.size());

        foreach (Valiaika v, t.m_valiajat) {
            QTime aika = v.aika;

            if (!edellinen.isNull())
                v.aika = QTime(0, 0).addSecs(edellinen.secsTo(v.aika));
            edellinen = aika;

            valiajat << v;
        }
        t.m_valiajat = valiajat;

        tulokset << t;
    }

    // Kerätään ajat rasteittain
    const QList<Rasti> rastit = s->getRastit();
    const int maali_rasti = s->getMaalirasti().getKoodi();
    QMap<int, QList<QTime> > rastiAjat;

    for (int i = 0; i < rastit.size(); ++i) {
        QList<QTime> ajat;
        ajat.reserve(rastit.size());

        foreach (const Tulos& t, tulokset) {
            if (t.m_tila != Tulos::Hyvaksytty)
                continue;

            foreach (const Valiaika& v, t.m_valiajat) {
                if (v.jarj == i + 1) {
                    ajat << v.aika;
                    break;
                }
            }
        }

        rastiAjat.insert(i + 1, ajat);
    }

    QString res = _("<H3>%1   Rastivälien ajat</H3>\n\n<PRE>\n").arg(s->getNimi());

    res += _("%1 %2").arg("Sija", -4).arg("Nimi", -30);
    for (int i = 0; i < rastit.size(); ++i) {
        const Rasti& r = rastit.at(i);

        if (r.getKoodi() == maali_rasti)
            res += _(" %1").arg("    " + QString::number(i) + "-M ", -13);
        else
            res += _(" %1").arg("    " + QString::number(i) + "-" + QString::number(i + 1) + " ", -13);
    }

    res += _(" %1\n\n").arg("Tulos", -13);

    foreach (const Tulos& t, tulokset) {
        QString aika = TimeFormat(t.m_aika);
        QString line = _("%1 %2")
                .arg((t.m_tila == Tulos::Hyvaksytty ? (QString::number(t.m_sija) + _(".")) : _("")), 4)
                .arg(t.m_kilpailija, -30);

        for (int i = 0; i < rastit.size(); ++i) {
            Valiaika v;
            bool found = false;

            foreach (v, t.m_valiajat) {
                if (i + 1 == v.jarj) {
                    found = true;
                    break;
                }
            }

            if (!found)
                line += "         -    ";
            else {
                if (t.m_tila == Tulos::Hyvaksytty) {
                    v.sija = 1;

                    foreach (const QTime& t, rastiAjat.value(v.jarj)) {
                        if (t < v.aika)
                            ++v.sija;
                    }
                }

                line += _(" %1%2 ").arg(v.sija == -1 ? " " : QString::number(v.sija) + "-", 4).arg(TimeFormat(v.aika), -8);
            }
        }

        line += _(" %1 %2\n").arg(aika, -13).arg(t.m_kilpailija);
        res += line;
    }

    res += "</PRE>\n\n";

    return res;
}

static QString RastivalitRogaining(SarjaP s, const QList<Tulos>& tulokset)
{
    const int width = 10;

    QString r;
    r.reserve(80 * tulokset.size());

    r += _("<h3>%1  - rastivälit</h3>\n\n<pre>\n").arg(s->getNimi());

    foreach (const Tulos& t, tulokset) {
        const bool hyv = t.m_tila == Tulos::Hyvaksytty;
        const QList<Valiaika>& valiajat = t.m_valiajat;
        QString line1 = _("%1 %2").arg(hyv ? QString::number(t.m_sija) + "." : _(""), 4).arg(t.m_kilpailija, -30);
        QString line2 = _("%1").arg(' ', 35), line3 = line2, line4 = line2;

        for (int i = 0; i < valiajat.size(); ++i) {
            const Valiaika& v = valiajat.at(i);
            const int e_pisteet = !i ? 0 : valiajat.at(i - 1).pisteet;
            const QTime& e_aika = !i ? QTime(0, 0) : valiajat.at(i - 1).aika;
            const int aikaero = e_aika.secsTo(v.aika);

            line1 += _(" %1").arg(v.koodi, width);
            line2 += _(" %1").arg(TimeFormat(v.aika), width);
            line3 += _(" %1").arg(TimeFormat(QTime(0, 0).addSecs(aikaero)), width);
            line4 += _(" %1").arg(_("%1(+%2)").arg(v.pisteet).arg(v.pisteet - e_pisteet), width);
        }
        r += line1 + "\n" + line2 + "\n" + line3 + "\n" + line4 + "\n\n";
    }

    r += "</pre>\n\n";

    return r;
}

static QString TuloslistaClassic(const QList<Tulos>& tulokset)
{
    QString r;
    r.reserve(80 * tulokset.size());

    /*
    tulos += _("%1 %2 %3  %4\n")
           .arg("Sija", -5)
           .arg("Kilpailija", -30)
           .arg("Tulos", -8)
           .arg("", 9)
    ;
    */

    QTime ekaAika = QTime();
    foreach (const Tulos& t, tulokset) {
        QString erotus = "";
        QString aika;

        if (t.m_tila == Tulos::Hyvaksytty) {
            aika = TimeFormat(t.m_aika);

            if (ekaAika.isValid()) {
                erotus = "+" + TimeFormat(QTime(0, 0).addSecs(ekaAika.secsTo(t.m_aika)));
            } else {
                ekaAika = t.m_aika;
            }
        } else {
            aika = "Ei tulosta";
        }

        r += _("%1 %2 %3  %4\n")
               .arg((t.m_tila == Tulos::Hyvaksytty ? (QString::number(t.m_sija) + _(".")) : _("")), 5)
               .arg(t.m_kilpailija, -30)
               .arg(aika, 8)
               .arg(erotus, 9);
    }

    return r;
}

static QString TuloslistaRogaining(const QList<Tulos>& tulokset)
{
    QString r;
    r.reserve(80 * tulokset.size());

    // HUOM.: tulokset ovat järjestetty pisteiden mukaan, kärjellä eniten pisteitä

    r += _("%1 %2 %3 %4 %5 %6\n").arg(_("Sija"), 5).arg(_("Nimi"), -30).arg("Pisteet", 7).arg(_("Aika"), 8).arg("Sakko", 5).arg(_("Tulos"), 5);
    foreach (const Tulos& t, tulokset) {
        QString sija, tulos_pisteet, pisteet, sakko, aika;

        if (t.m_tila != Tulos::Hyvaksytty) {
            sija = aika = pisteet = sakko = "";
            tulos_pisteet = "Ei tulosta";
        }
        else {
            sija = QString::number(t.m_sija) + '.';
            tulos_pisteet = QString::number(t.m_pisteet);
            aika = TimeFormat(t.m_aika);
            pisteet = QString::number(t.m_pisteet + t.m_sakko);
            sakko = QString::number(-t.m_sakko);
        }

        r += _("%1 %2 %3 %4 %5 %6\n").arg(sija, 5).arg(t.m_kilpailija, -30).arg(pisteet, 7).arg(aika, 8).arg(sakko, 5).arg(tulos_pisteet, 5);
    }

    return r;
}

static QString ValiajatClassic(SarjaP s, const QList<Tulos>& tulokset)
{
    QString res;
    const int maali_rasti = s->getMaalirasti().getKoodi();
    const QList<Rasti> rastit = s->getRastit();

    res += _("<H3>%1   Tilanne rasteilla</H3>\n\n<PRE>\n").arg(s->getNimi());
    res += _("%1 %2").arg("Sija", -4).arg("Nimi", -30);

    QVector<QList<Valiaika> > valiajat;
    valiajat.reserve(rastit.size());

    for (int i = 0; i < rastit.size(); ++i) {
        const Rasti& r = rastit.at(i);

        if (r.getKoodi() == maali_rasti)
            continue;

        valiajat.append(Valiaika::haeRastiValiajat(s, i + 1));
        res += _(" %1").arg("       " + QString::number(i + 1) + ".", -13);
    }

    res += _(" %1\n\n").arg("Tulos", -13);

    foreach (const Tulos& t, tulokset) {
        QString line = _("%1 %2")
                .arg((t.m_tila == Tulos::Hyvaksytty ? (QString::number(t.m_sija) + _(".")) : _("")), 4)
                .arg(t.m_kilpailija, -30);

        QString aika = TimeFormat(t.m_aika);
        for (int i = 0; i < rastit.size(); ++i) {
            const Rasti& r = rastit.at(i);

            if (r.getKoodi() == maali_rasti)
                continue;

            Valiaika v;
            bool found = false;

            foreach (v, t.m_valiajat) {
                if (i + 1 == v.jarj) {
                    found = true;
                    break;
                }
            }

            if (!found)
                line += _("         -    ");
            else {
                foreach (Valiaika tmp_v, valiajat.at(i)) {
                    if (tmp_v.id == v.id) {
                        v = tmp_v;
                        break;
                    }
                }

                line += _(" %1%2 ").arg(v.sija == -1 ? " " : QString::number(v.sija) + "-", 4).arg(TimeFormat(v.aika), -8);
            }
        }

        line += _(" %1 %2\n").arg(aika, -13).arg(t.m_kilpailija);
        res += line;
    }

    res += "</PRE>\n";

    return res;
}

TuloksetForm::TuloksetForm(QWidget *parent) :
    UtilForm(parent),
    ui(new Ui::TuloksetForm),
    m_filterModel(new QSortFilterProxyModel(this)),
    m_tulosModel(new QSqlQueryModel(this)),
    m_sarjat(),
    m_tulokset()
{
    ui->setupUi(this);
    ui->fileButton->setEnabled(false);

    m_filterModel->setSourceModel(m_tulosModel);
    ui->tulosView->setModel(m_filterModel);

    ui->tulosView->addAction(ui->actionPoistaTulos);

    // Pistesuunnistuksessa ei ole väliaikoja HTML- eikä XML- muodossa
    if (Tapahtuma::tapahtuma()->tyyppi() == RACE_ROGAINING) {
        ui->tab_2->hide();
        ui->tab_5->hide();
    }

    on_comboBox_currentIndexChanged(0);

    on_updateButton_clicked();
}

TuloksetForm::~TuloksetForm()
{
    delete ui;
}

void TuloksetForm::sqlTulokset()
{
    QSqlQuery query;

    query.prepare("SELECT COUNT(*) AS kpl, tila FROM tulos WHERE tapahtuma = ? AND NOT poistettu GROUP BY tila");

    query.addBindValue(Tapahtuma::tapahtuma()->id());

    SQL_EXEC(query,);

    int yht = 0;

    while (query.next()) {
        QSqlRecord r = query.record();

        switch (r.value("tila").toInt()) {
            case 1:
                ui->avoimiaLabel->setText(r.value("kpl").toString());
                break;
            case 2:
                ui->hyvaksyttyjaLabel->setText(r.value("kpl").toString());
                break;
            default:
                ui->DNFLabel->setText(r.value("kpl").toString());
                break;
        }

        yht += r.value("kpl").toInt();
    }

    ui->tuloksiaLabel->setText(QString::number(yht));
}

void TuloksetForm::on_closeButton_clicked()
{
    emit requestClose(this);
}

void TuloksetForm::updateTulosEdit()
{
    const Tapahtuma *tapahtuma = Tapahtuma::tapahtuma();
    QTextEdit *edit = ui->tulosEdit;

    edit->clear();

    m_tulosString.clear();

    m_tulosString += _("<h2>%1</h2>\n").arg(tapahtuma->nimi());

    foreach (SarjaP s, m_sarjat) {

        m_tulosString += _("<p>");

        foreach (SarjaP s2, m_sarjat) {
            m_tulosString += _("<a href=\"#%2\">%1</a> ").arg(s2->getNimi(), (s2->getNimi()).toHtmlEscaped());
        }

        m_tulosString += _("</p>\n");
        m_tulosString += _("<h3><a name=\"%2\"></a>%1</h3>").arg(s->getNimi(), (s->getNimi()).toHtmlEscaped());

        const QList<Tulos> tulokset = m_tulokset.value(s->getNimi());
        int lahti = tulokset.count(), dnf = 0;

        foreach (const Tulos& t, tulokset) {
            if (t.m_tila != Tulos::Hyvaksytty) {
                dnf++;
            }
        }

        const QString status = _("(Lähti: %1, Ei tulosta: %2)\n\n").arg(QString::number(lahti)).arg(QString::number(dnf));
        const QString tuloslista = tapahtuma->tyyppi() == RACE_CLASSIC ? TuloslistaClassic(tulokset) : TuloslistaRogaining(tulokset);

        m_tulosString += _("<pre>%1%2</pre>").arg(status).arg(tuloslista);
    }

    edit->setText(m_tulosString);
}

void TuloksetForm::updateValiaikaEdit()
{
    QTextEdit *edit = ui->valiaikaEdit;
    const bool rogaining = Tapahtuma::tapahtuma()->tyyppi() == RACE_ROGAINING;

    edit->clear();

    m_valiaikaString.clear();
    m_valiaikaString += _("<H2>%1</H2>\n").arg(Tapahtuma::tapahtuma()->nimi());
/*    m_valiaikaString += _("<p>");

    foreach (const Sarja *s, m_sarjat) {
        m_valiaikaString += _("<a href=\"#%2\">%1</a> ").arg(s->getNimi(), Qt::escape(s->getNimi()));
    }

    m_valiaikaString += _("</p>\n");
*/
    foreach (SarjaP s, m_sarjat) {
        const QList<Tulos>& tulokset = m_tulokset.value(s->getNimi());

        if (rogaining)
            m_valiaikaString += RastivalitRogaining(s, tulokset);
        else {
            m_valiaikaString += ValiajatClassic(s, tulokset);
            m_valiaikaString += RastivalitClassic(s, tulokset);
        }
    }

    edit->append(m_valiaikaString);
}


void TuloksetForm::updateLehteenEdit()
{
    QPlainTextEdit *edit = ui->lehteenEdit;

    edit->clear();

    foreach (SarjaP s, m_sarjat) {
        QList<Tulos> tulokset = m_tulokset.value(s->getNimi());

        QString tulos;

        int lahti = tulokset.count();
        int dnf = 0;

        foreach (Tulos t, tulokset) {
            switch (t.m_tila) {
            case Tulos::DNF:
                dnf++;
                break;
            }
        }

        tulos += _("%1 (Lähti: %2, Ei tulosta: %3)\n")
                .arg(s->getNimi())
                .arg(QString::number(lahti))
                .arg(QString::number(dnf))
        ;

        foreach (Tulos t, tulokset) {
            QString sija = "";
            QString aika = "";

            switch (t.m_tila) {
            case Tulos::Avoin:
                aika = "Avoin";
                break;
            case Tulos::Hyvaksytty:
                aika = TimeFormat(t.m_aika);
                sija = _("%1)").arg(QString::number(t.m_sija));
                break;
            case Tulos::DNF:
                aika = "Ei tulosta";
                break;
            }

            tulos += _("%1 %2, , %3,  ")
                   .arg(sija)
                   .arg(t.m_kilpailija)
                   .arg(aika)
            ;
        }

        edit->appendPlainText(_("%1.\n").arg(tulos));
    }

    edit->appendPlainText("");
}

void TuloksetForm::updateXMLEdit()
{
    QString xml;
    TulosXMLWriter writer(&xml);

    writer.writeStartXML();

    foreach (SarjaP s, m_sarjat) {
        writer.writeEventClass(m_tulokset.value(s->getNimi()), s);
    }

    writer.writeEndXML();

    ui->xmlEdit->clear();
    ui->xmlEdit->setPlainText(xml);
}


void TuloksetForm::sqlTulos()
{
    QSqlQuery query;

    query.prepare(
                "SELECT\n"
                "  t.id,\n"
                "  t.emit,\n"
                "  tila.nimi AS tila,\n"
                "  t.aika,\n"
                "  s.nimi,\n"
                "  k.nimi\n"
                "FROM tulos AS t\n"
                "  JOIN tulos_tila AS tila ON tila.id = t.tila\n"
                "  JOIN kilpailija AS k ON k.id = t.kilpailija\n"
                "  JOIN sarja AS s ON s.id = t.sarja\n"
                "WHERE t.tapahtuma = ?\n"
                "  AND NOT t.poistettu\n"
                "ORDER BY t.id DESC\n"
    );

    query.addBindValue(Tapahtuma::tapahtuma()->id());

    SQL_EXEC(query,);

    m_tulosModel->setQuery(query);
}


void TuloksetForm::on_updateButton_clicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QSqlDatabase::database().transaction();

    m_tulokset.clear();
    m_sarjat = Sarja::haeSarjatRO();
    foreach (SarjaP s, m_sarjat)
        m_tulokset.insert(s->getNimi(), Tulos::haeTulokset(s));

    sqlTulokset();

    // Tabit
    sqlTulos();
    updateTulosEdit();
    updateValiaikaEdit();
    updateLehteenEdit();
    updateXMLEdit();

    QSqlDatabase::database().commit();

    QApplication::restoreOverrideCursor();
}

void TuloksetForm::on_fileButton_clicked()
{
    bool html = false;
    QString *tulos = 0;
    QString text;
    QString xml;

    switch (ui->tabWidget->currentIndex()) {
    case 1:
        html = true;
        tulos = &m_tulosString;
        break;
    case 2:
        html = true;
        tulos = &m_valiaikaString;
        break;
    case 3:
        html = false;
        text = ui->lehteenEdit->toPlainText();
        break;
    case 4:
        html = false;
        xml = ui->xmlEdit->toPlainText();
    }

    if (!tulos && text.isNull() && xml.isNull()) {
        return;
    }


    QString fn = QFileDialog::getSaveFileName(this, _("Valitse tiedosto"));

    if (fn.isNull()) {
        return;
    }

    QFile file(fn);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        INFO(this, _("Tiedostoa ei voida avata kirjoittamista varten."));
    }

    if (html) {
        file.write(_("<html><head><title>%1</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\"/></head><body>\n").arg(Tapahtuma::tapahtuma()->nimi()).toLatin1());
        file.write(tulos->toLatin1());
        file.write("</body></html>");
    } else if (!xml.isEmpty()) {
        file.write(xml.toUtf8());
    } else {
        file.write(text.replace('\n', "\r\n").toLatin1());
    }

    file.close();
}

void TuloksetForm::on_tulosAvaaButton_clicked()
{
    foreach (QModelIndex index, ui->tulosView->selectionModel()->selectedRows(0)) {
        emit requestTulosForm(index.data(Qt::EditRole));
    }
}

void TuloksetForm::on_lineEdit_textChanged(const QString &arg1)
{
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterModel->setFilterWildcard(arg1);
}

void TuloksetForm::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        m_filterModel->setFilterKeyColumn(5);
        break;
    case 1:
        m_filterModel->setFilterKeyColumn(1);
        break;
    case 2:
        m_filterModel->setFilterKeyColumn(4);
        break;
    case 3:
        m_filterModel->setFilterKeyColumn(2);
        break;
    }
}

void TuloksetForm::on_actionPoistaTulos_triggered()
{
    QModelIndex index;

    foreach (index, ui->tulosView->selectionModel()->selectedRows(0)) {
        break;
    }

    if (!index.isValid()) {
        return;
    }

    QSqlDatabase::database().transaction();

    QSqlQuery query;

    //query.prepare("DELETE FROM tulos WHERE id = ?");
    query.prepare("UPDATE tulos SET poistettu = 1 WHERE id = ?");

    query.addBindValue(index.data(Qt::EditRole));

    SQL_EXEC(query,);

/*  PRAGMA foreign_keys = ON Hoitaa poistot.
    query.prepare("DELETE FROM valiaika WHERE tulos = ?");

    query.addBindValue(index.data(Qt::EditRole));

    SQL_EXEC(query,);
*/
    QSqlDatabase::database().commit();

    on_updateButton_clicked();
}

void TuloksetForm::updateForm()
{
    on_updateButton_clicked();
}

void TuloksetForm::on_tabWidget_currentChanged(int index)
{
    ui->fileButton->setEnabled(index);
}

void TuloksetForm::focusToSearch()
{
    ui->lineEdit->setFocus();
}
