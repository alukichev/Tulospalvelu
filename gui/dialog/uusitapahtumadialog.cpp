#include <QString>
#include <QStringList>
#include <QStringListModel>

#include <makrot.h>

#include "uusitapahtumadialog.h"
#include "ui_uusitapahtumadialog.h"

UusiTapahtumaDialog::UusiTapahtumaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UusiTapahtumaDialog)
{
    ui->setupUi(this);

    QStringList types{
            _("suunnistus"),        // RACE_CLASSIC
            _("pistesuunnistus")    // RACE_ROGAINING
    };
    QStringListModel *model = new QStringListModel(types, this);
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

int UusiTapahtumaDialog::tyyppi(void) const
{
    const int index = ui->tyyppiCombo->currentIndex();

    return index < 0 ? RACE_CLASSIC : index;
}
