#ifndef SARJAVALINTADIALOG_H
#define SARJAVALINTADIALOG_H

#include <QtWidgets>

#include "type/sarja.h"

#include "makrot.h"

namespace Ui {
    class SarjaValintaDialog;
}

class SarjaValintaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SarjaValintaDialog(QWidget *parent, QList<SarjaP> sarjat);
    ~SarjaValintaDialog();

    SarjaP getSarja() const;

private:
    Ui::SarjaValintaDialog *ui;

    QList<SarjaP> m_sarjat;
};

#endif // SARJAVALINTADIALOG_H
