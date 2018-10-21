#ifndef UUSITAPAHTUMADIALOG_H
#define UUSITAPAHTUMADIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class UusiTapahtumaDialog;
}

class QString;

class UusiTapahtumaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UusiTapahtumaDialog(QWidget *parent = 0);
    ~UusiTapahtumaDialog();

    QString nimi(void) const;
    int tyyppi(void) const;

private:
    Ui::UusiTapahtumaDialog *ui;
};

#endif // UUSITAPAHTUMADIALOG_H
