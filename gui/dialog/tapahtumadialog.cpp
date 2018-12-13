#include <makrot.h>

#include "tapahtumadialog.h"
#include "ui_tapahtumadialog.h"
#include "uusitapahtumadialog.h"

TapahtumaDialog::TapahtumaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TapahtumaDialog),
    m_tapahtumaModel(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    ui->tapahtumaView->setModel(m_tapahtumaModel);

    ui->cancelButton->setEnabled(Tapahtuma::Id() != 0);

    sqlTapahtuma();
}

TapahtumaDialog::~TapahtumaDialog()
{
    delete ui;
}

void TapahtumaDialog::on_valitseButton_clicked()
{
    foreach (QModelIndex index, ui->tapahtumaView->selectionModel()->selectedRows(0)) {
        Tapahtuma::Valitse(index.data(Qt::EditRole).toInt());

        accept();

        return;
    }

    INFO(this, _("Valitse tapahtuma listasta."));
}

void TapahtumaDialog::sqlTapahtuma()
{
    QSqlQuery query;

    query.prepare("SELECT id, nimi FROM tapahtuma ORDER BY id DESC");

    SQL_EXEC(query,);

    m_tapahtumaModel->setQuery(query);
}


void TapahtumaDialog::on_uusiButton_clicked()
{
    UusiTapahtumaDialog d;
    if (d.exec() != QDialog::Accepted || d.nimi().isEmpty()) {
        return;
    }

    Tapahtuma::Luo(d.nimi(), d.tyyppi());

    accept();
}
