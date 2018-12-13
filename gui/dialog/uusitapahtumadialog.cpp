#include <QString>
#include <QStringList>
#include <QStringListModel>

#include <makrot.h>

#include "type/tapahtuma.h"
#include "uusitapahtumadialog.h"
#include "ui_uusitapahtumadialog.h"

UusiTapahtumaDialog::UusiTapahtumaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UusiTapahtumaDialog)
{
    ui->setupUi(this);

    QStringListModel *model = new QStringListModel(Tapahtuma::Tyypit(), this);
    ui->tyyppiCombo->setModel(model);
}

UusiTapahtumaDialog::~UusiTapahtumaDialog()
{
    delete ui;
}

QString UusiTapahtumaDialog::nimi(void) const
{
    return ui->nimiEdit->text();
}

Tapahtuma::Type UusiTapahtumaDialog::tyyppi(void) const
{
    const int index = ui->tyyppiCombo->currentIndex();

    return index < 0 ? Tapahtuma::RACE_CLASSIC : static_cast<Tapahtuma::Type>(index);
}
