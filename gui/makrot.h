#ifndef MAKROT_H
#define MAKROT_H

#include <QtCore/QString>


// Tietokanta / muut isot muutokset
#define MAJOR_VERSION "1.5"

// Bugi korjaukset
#define MINOR_VERSION "0"

#define VERSION MAJOR_VERSION "." MINOR_VERSION

#define INFO(W, M) QMessageBox::information(W, _("Tulospalvelu"), M)
#define _(S) QString::fromUtf8(S)

#define SQL_EXEC(Q, R)  _SQL_EXEC(Q, R, exec)

#ifdef DEBUG
#define _SQL_EXEC(Q, R, how) ({ \
                                QTime __ti; \
                                __ti.start(); \
                                if (!Q.how()) { \
                                    qWarning() << "In" << __func__ << "():" << Q.lastQuery(); \
                                    qWarning() << Q.lastError(); \
                                    QSqlDatabase::database().rollback(); \
                                    return R; \
                                } \
                                qDebug() << __ti.elapsed() << Q.lastQuery(); \
                            })
#else
#define _SQL_EXEC(Q, R, how) ({ \
                                if (!Q.exec()) { \
                                    qWarning() << "In" << __func__ << "():" << Q.lastQuery(); \
                                    qWarning() << Q.lastError(); \
                                    QSqlDatabase::database().rollback(); \
                                    return R; \
                                }})
#endif

#define RACE_CLASSIC    0
#define RACE_ROGAINING  1

#endif // MAKROT_H
