#include "tietokanta.h"


void Tietokanta::buildSQLite()
{
    dropTables();

    QSqlDatabase::database().transaction();

    QStringList tables;
    tables
            <<
               "CREATE TABLE tulospalvelu ("
               "   id INTEGER PRIMARY KEY,"
               "   versio TEXT NOT NULL"
               ")"
            <<
               "CREATE TABLE tapahtuma ("
               "   id INTEGER PRIMARY KEY,"
               "   nimi TEXT NOT NULL,"
               "   tyyppi INTEGER"
               ")"
            <<
               "CREATE TABLE kilpailija ("
               "   id INTEGER PRIMARY KEY,"
               "   nimi TEXT NOT NULL"
               ")"
            <<
               "CREATE TABLE emit ("
               "    id VARCHAR(8) NOT NULL,"
               "    vuosi INTEGER(2) NOT NULL,"
               "    kuukausi INTEGER(2) NOT NULL,"
               "    laina BOOLEAN NOT NULL DEFAULT 0,"
               "    kilpailija INTEGER,"
               "    FOREIGN KEY (kilpailija) REFERENCES kilpailija(id)"
               "        ON UPDATE CASCADE"
               "        ON DELETE SET NULL,"
               "    PRIMARY KEY (id)"
               ")"
            <<
               "CREATE TABLE rata ("
               "   id INTEGER PRIMARY KEY,"
               "   nimi TEXT NOT NULL,"
               "   tapahtuma INTEGER NOT NULL,"
               "   FOREIGN KEY (tapahtuma) REFERENCES tapahtuma(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE"
               ")"
            <<
               "CREATE TABLE sarja ("
               "   id INTEGER PRIMARY KEY,"
               "   nimi TEXT NOT NULL,"
               "   tapahtuma INTEGER NOT NULL,"
               "   rata INTEGER,"
               "   sakko INTEGER NOT NULL DEFAULT 0," // Sekunnit (CLASSIC) tai pisteet (ROGAINING)
               "   yhteislahto DATETIME,"
               "   aikaraja TIME,"
               "   sakkoyksikko INTEGER NOT NULL DEFAULT 60," // 1 pisteen sakon yksikkö sekunteina (ROGAINING)
               "   FOREIGN KEY (tapahtuma) REFERENCES tapahtuma(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (rata) REFERENCES rata(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE SET NULL"
               ")"
            <<
               "CREATE TABLE tulos_tila ("
               "   id INTEGER PRIMARY KEY,"
               "   nimi TEXT NOT NULL"
               ")"
            <<
               "CREATE TABLE tulos ("
               "   id INTEGER PRIMARY KEY,"
               "   tapahtuma INTEGER NOT NULL,"
               "   emit VARCHAR(8) NOT NULL,"
               "   kilpailija INTEGER NOT NULL,"
               "   sarja INTEGER NOT NULL,"
               "   tila INTEGER NOT NULL,"
               "   aika TIME NOT NULL,"        // Lopullinen (sakot + korjaukset)
               "   maaliaika DATETIME NOT NULL,"
               "   pisteet INTEGER,"           // Lopulliset (sakot + korjaukset)
               "   sakko INTEGER NOT NULL DEFAULT 0," // Sekunteina (CLASSIC) tai pisteinä (ROGAINING)
               "   korj_aika INTEGER,"         // Yhteismäärä "manuaalisesti" lisättyä/vähennettyä aikaa, sekunteina
               "   korj_pisteet INTEGER,"      // Yhteismäärä "manuaalisesti" lisättyjä/vähennettyjä pisteitä
               "   poistettu INTEGER NOT NULL DEFAULT 0,"
               "   FOREIGN KEY (tapahtuma) REFERENCES tapahtuma(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (emit) REFERENCES emit(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (kilpailija) REFERENCES kilpailija(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (sarja) REFERENCES sarja(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (tila) REFERENCES tulos_tila(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE RESTRICT"
               ")"
            <<
               "CREATE TABLE valiaika ("
               "   id INTEGER PRIMARY KEY,"
               "   tulos INTEGER NOT NULL,"
               "   jarj INTEGER NOT NULL,"     // Järjestysnumero
               "   rasti INTEGER NOT NULL,"    // Rastikoodi eikä id
               "   aika TIME NOT NULL,"        // Juokseva aika, sakot mukana
               "   pisteet INTEGER,"           // Juoksevat pisteet (ellei NULL), sakot mukana
               "   sakko INTEGER,"             // Juokseva, sekunteina (CLASSIC) tai pisteinä (ROGAINING)
               "   FOREIGN KEY (tulos) REFERENCES tulos(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE"
               ")"
            <<
               "CREATE UNIQUE INDEX valiaika_index ON valiaika (tulos, jarj, rasti, aika)"
            <<
               "CREATE TABLE rasti ("
               "   id INTEGER PRIMARY KEY,"
               "   koodi INTEGER NOT NULL,"
               "   tapahtuma INTEGER NOT NULL,"
               "   pisteet INTEGER,"
               "   FOREIGN KEY (tapahtuma) REFERENCES tapahtuma(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE"
               ")"
            <<
               "CREATE UNIQUE INDEX rasti_index ON rasti (koodi, tapahtuma)"
            <<
               "CREATE TABLE leimasin ("
               "   koodi INTEGER NOT NULL,"
               "   tapahtuma INTEGER NOT NULL,"
               "   rasti INTEGER,"
               "   FOREIGN KEY (rasti) REFERENCES rasti(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE SET NULL,"    // Leimasin "vapautuu"
               "   FOREIGN KEY (tapahtuma) REFERENCES tapahtuma(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   PRIMARY KEY (koodi, tapahtuma)"
               ")"
            <<
               "CREATE TABLE rastisarja ("
               "   rata INTEGER NOT NULL,"
               "   rasti INTEGER NOT NULL,"
               "   jarj INTEGER NOT NULL DEFAULT 0,"
               "   FOREIGN KEY (rata) REFERENCES rata(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (rasti) REFERENCES rasti(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE"
               ")"
            <<
               "CREATE TABLE luettu_emit ("
               "   id INTEGER PRIMARY KEY,"
               "   tapahtuma INTEGER NOT NULL,"
               "   emit VARCHAR(8) NOT NULL,"
               "   luettu DATETIME NOT NULL,"
               "   tulos INTEGER,"
               "   FOREIGN KEY (tapahtuma) REFERENCES tapahtuma(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (emit) REFERENCES emit(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE,"
               "   FOREIGN KEY (tulos) REFERENCES tulos(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE"
               ")"
            <<
               "CREATE TABLE luettu_emit_rasti ("
               "   id INTEGER PRIMARY KEY,"
               "   luettu_emit INTEGER NOT NULL,"
               "   numero INTEGER NOT NULL,"
               "   koodi INTEGER NOT NULL,"
               "   aika INTEGER NOT NULL,"
               "   FOREIGN KEY (luettu_emit) REFERENCES luettu_emit(id)"
               "       ON UPDATE CASCADE"
               "       ON DELETE CASCADE"
               ")"
               ;

    QSqlQuery query;
    foreach (QString table, tables) {
        query.prepare(table);

        SQL_EXEC(query,);
    }

    QSqlDatabase::database().commit();

    insertData();
}

void Tietokanta::buildMySQL()
{
    // FIXME tee loppuun

    dropTables();


    insertData();
}

void Tietokanta::dropTables()
{
    QStringList tables{
        "luettu_emit_rasti",
        "luettu_emit",
        "rasti",
        "valiaika",
        "tulos",
        "tulos_tila",
        "sarja",
        "emit",
        "kilpailija",
        "tapahtuma",
        "leimasin",
        "rata",
        "rastisarja"};

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("DROP TABLE IF EXISTS ?");

    foreach (QString table, tables) {
        query.addBindValue(table);
        SQL_EXEC(query,);
    }

    QSqlDatabase::database().commit();
}

void Tietokanta::insertData()
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("INSERT INTO tulos_tila (nimi) VALUES (?)");

    QStringList tulos_tila{"Avoin", _("Hyväksytty"), "DNF"};
    foreach (QString tila, tulos_tila) {
        query.addBindValue(tila);
        SQL_EXEC(query,);
    }

    query.prepare("INSERT INTO tulospalvelu (versio) VALUES (?)");
    query.addBindValue(MAJOR_VERSION);
    SQL_EXEC(query,);

    QSqlDatabase::database().commit();
}

void Tietokanta::vieKilpailijatietokanta(const QString &fn)
{
#ifdef USE_MYSQL
    // FIXME tämä täytyy tehdä eri tavalla kuin SQLite:n kanssa.
    QSqlDatabase kilpdat = QSqlDatabase::addDatabase("QSQLITE", "kilpdat");

    kilpdat.setDatabaseName(fn);

    if (!kilpdat.open()) {
        // FIXME Error
        return;
    }

    kilpdat.close();

#else
    QSqlQuery query;
    query.prepare("ATTACH DATABASE ? AS kilpdat");
    query.addBindValue(fn);
    SQL_EXEC(query,);

    SQLiteVieKilpailijatietokanta();

    query.prepare("DETACH kilpdat");
    SQL_EXEC(query,);
#endif
}


void Tietokanta::SQLiteVieKilpailijatietokanta()
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("CREATE TABLE kilpdat.tulospalvelu AS SELECT * FROM tulospalvelu");
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE kilpdat.kilpailija AS SELECT * FROM kilpailija");
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE kilpdat.emit AS SELECT * FROM emit");
    SQL_EXEC(query,);

    QSqlDatabase::database().commit();
}

bool Tietokanta::tuoKilpailijatietokanta(const QString &fn)
{
    bool res = false;

#ifdef USE_MYSQL
    // FIXME tämä täytyy tehdä eri tavalla kuin SQLite:n kanssa.
    QSqlDatabase kilpdat = QSqlDatabase::addDatabase("QSQLITE", "kilpdat");

    kilpdat.setDatabaseName(fn);

    if (!kilpdat.open()) {
        // FIXME Error
        return false;
    }

    kilpdat.close();

#else
    QSqlQuery query;
    query.prepare("ATTACH DATABASE ? AS kilpdat");
    query.addBindValue(fn);
    SQL_EXEC(query, false);

    res = SQLiteTuoKilpailijatietokanta();

    query.prepare("DETACH kilpdat");
    SQL_EXEC(query, false);
#endif

    return res;
}

bool Tietokanta::SQLiteTuoKilpailijatietokanta()
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;

    if (!Tietokanta::checkVersion(MAJOR_VERSION, "kilpdat.tulospalvelu") &&
            QMessageBox::question(0, _("Tulospalvelu - " VERSION),
                                  _("Kilpailijatietokanta (%1) ei ole yhteensopiva! Haluatko silti jatkaa?").arg(Tietokanta::getVersion("kilpdat.tulospalvelu")),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) != QMessageBox::Yes) {
        QSqlDatabase::database().rollback();
        return false;
    }

    // FIXME: Tämä hävittää tulokset ja luetut tiedot, "foreign key" suhteiden takia
    query.prepare("INSERT OR REPLACE INTO kilpailija SELECT * FROM kilpdat.kilpailija");
    SQL_EXEC(query, false);

    query.prepare("INSERT OR REPLACE INTO emit SELECT * FROM kilpdat.emit");
    SQL_EXEC(query, false);

    QSqlDatabase::database().commit();

    return true;
}

void Tietokanta::vieTulokset(const Tapahtuma &tapahtuma, const QString &fileName)
{
#ifdef USE_MYSQL
    // FIXME tee loppuun
#else
    QSqlQuery query;
    query.prepare("ATTACH DATABASE ? AS tulosdat");
    query.addBindValue(fileName);
    SQL_EXEC(query,);

    SQLiteVieTulokset(tapahtuma.id());

    query.prepare("DETACH tulosdat");
    SQL_EXEC(query,);
#endif

}

void Tietokanta::SQLiteVieTulokset(int tapahtuma)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("CREATE TABLE tulosdat.tulospalvelu AS SELECT * FROM tulospalvelu");
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.kilpailija AS SELECT k.* FROM kilpailija AS k WHERE k.id IN (SELECT t.kilpailija FROM tulos AS t WHERE t.tapahtuma = ?)");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.emit AS SELECT e.* FROM emit AS e WHERE e.id IN (SELECT t.emit FROM tulos AS t WHERE t.tapahtuma = ?)");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.tapahtuma AS SELECT t.* FROM tapahtuma AS t WHERE t.id = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.rasti AS SELECT r.* FROM rasti AS r WHERE r.tapahtuma = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.sarja AS SELECT s.* FROM sarja AS s WHERE s.tapahtuma = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.rata AS SELECT r.* FROM rata AS r WHERE r.tapahtuma = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.rastisarja AS SELECT rs.* FROM rastisarja AS rs WHERE rs.rata IN (SELECT r.id FROM rata AS r WHERE r.tapahtuma = ?)");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.tulos_tila AS SELECT t.* FROM tulos_tila AS t");
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.tulos AS SELECT t.* FROM tulos AS t WHERE t.tapahtuma = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.valiaika AS SELECT v.* FROM valiaika AS v WHERE v.tulos IN (SELECT t.id FROM tulos AS t WHERE t.tapahtuma = ?)");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.luettu_emit AS SELECT l.* FROM luettu_emit AS l WHERE l.tapahtuma = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.luettu_emit_rasti AS SELECT lr.* FROM luettu_emit_rasti AS lr WHERE lr.luettu_emit IN (SELECT l.id FROM luettu_emit AS l WHERE l.tapahtuma = ?)");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    query.prepare("CREATE TABLE tulosdat.leimasin AS SELECT l.* FROM leimasin AS l WHERE l.tapahtuma = ?");
    query.addBindValue(tapahtuma);
    SQL_EXEC(query,);

    QSqlDatabase::database().commit();
}

bool Tietokanta::tuoTulokset(const Tapahtuma& tapahtuma, const QString &fileName)
{
    bool res = false;

#ifdef USE_MYSQL
    // FIXME tee
#else
    QSqlQuery query;
    query.prepare("ATTACH DATABASE ? AS tulosdat");
    query.addBindValue(fileName);
    SQL_EXEC(query, false);

    res = SQLiteTuoTulokset(tapahtuma);

    query.prepare("DETACH tulosdat");
    SQL_EXEC(query, false);
#endif

    return res;
}

bool Tietokanta::SQLiteTuoTulokset(const Tapahtuma& tapahtuma)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    QSqlQuery queryValiaikaAppend;

    QSqlQuery queryAppend;

    QSqlQuery queryLuettuAppend;
    QSqlQuery queryLuettuRastiAppend;

    if (!Tietokanta::checkVersion(MAJOR_VERSION, "tulosdat.tulospalvelu") &&
            QMessageBox::question(0, _("Tulospalvelu - " VERSION),
                                  _("Tulostietokanta (%1) ei ole yhteensopiva! Haluatko silti jatkaa?").arg(Tietokanta::getVersion("tulosdat.tulospalvelu")),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) != QMessageBox::Yes) {
        QSqlDatabase::database().rollback();
        return false;
    }

    // Tarkistetaan että tulosdat sisältää vain yhden tapahtuman
    query.prepare("SELECT COUNT(*) FROM tulosdat.tapahtuma");
    SQL_EXEC(query, false);

    if (!query.next() || query.value(0).toInt() != 1) {
        INFO(0, _("Tulostietokanta ei sisällä vain yksiä tuloksia. Ei voida tuoda tuloksia."));
        return false;
    }

    // Tarkistetaan löytyykö tapahtumaa
    query.prepare("SELECT COUNT(*) FROM tulosdat.tapahtuma AS t WHERE t.nimi = ?");
    query.addBindValue(tapahtuma.nimi());
    SQL_EXEC(query, false);

    if (!query.next() || query.value(0).toInt() != 1) {
        // FIXME katso edellinen FIXME
        INFO(0, _("Tulostietokanta ei sisällä tätä tapahtumaa (%2).").arg(tapahtuma.nimi()));
        return false;
    }

    // Tarkistetaan että tapahtuman sarjat on olemassa
    query.prepare("SELECT COUNT(*) FROM tulosdat.sarja AS a WHERE a.nimi NOT IN (SELECT b.nimi FROM sarja AS b WHERE b.tapahtuma = ?)");
    query.addBindValue(tapahtuma.id());
    SQL_EXEC(query, false);

    if (!query.next() || query.value(0).toInt() != 0) {
        INFO(0, _("Tapahtumaa ei löydy tästä tulostietokannasta."));
        return false;
    }

    // Kilpailija taulun päivitys, lisätään kilpailijat joita ei ole.
    query.prepare("SELECT nimi FROM tulosdat.kilpailija WHERE nimi NOT IN (SELECT nimi FROM kilpailija)");
    SQL_EXEC(query, false);

    queryAppend.prepare("INSERT INTO kilpailija (nimi) VALUES (?)");
    while (query.next()) {
        queryAppend.addBindValue(query.value(0));
        SQL_EXEC(queryAppend, false);
    }

    // Emit
    // Nämä voidaan lisätä ja ohitetaan emit taulussa olevat emitit
    // Asetetaan kilpailija lopuksi
    query.prepare("INSERT OR IGNORE INTO emit (id, vuosi, kuukausi, laina, kilpailija) SELECT id, vuosi, kuukausi, laina, NULL FROM tulosdat.emit");
    SQL_EXEC(query, false);

    // TulosTila taulun päivitys, lisätään tilat joita ei ole.
    query.prepare("SELECT nimi FROM tulosdat.tulos_tila WHERE nimi NOT IN (SELECT nimi FROM tulos_tila)");
    SQL_EXEC(query, false);

    queryAppend.prepare("INSERT INTO tulos_tila (nimi) VALUES (?)");
    while (query.next()) {
        queryAppend.addBindValue(query.value(0));
        SQL_EXEC(queryAppend, false);
    }

    // Kasataan tulos "selkokieliseksi" ja yksitellen puretaan ja tarvittaessa lisätään kilpailija
    query.prepare(
                "SELECT\n"
                "  t.id AS id,\n"
                "  ta.nimi AS tapahtuma,\n"
                "  t.emit AS emit,\n"
                "  k.nimi AS kilpailija,\n"
                "  s.nimi AS sarja,\n"
                "  tt.nimi AS tila,\n"
                "  t.aika AS aika,\n"
                "  t.maaliaika AS maaliaika,\n"
                "  t.pisteet AS pisteet,\n"
                "  t.sakko AS sakko,\n"
                "  t.poistettu\n"
                "FROM tulosdat.tulos AS t\n"
                "  JOIN tulosdat.kilpailija AS k ON k.id = t.kilpailija\n"
                "  JOIN tulosdat.sarja AS s ON s.id = t.sarja\n"
                "  JOIN tulosdat.tulos_tila AS tt ON tt.id = t.tila\n"
                "  JOIN tulosdat.tapahtuma AS ta ON ta.id = t.tapahtuma\n"
    );

    queryValiaikaAppend.prepare(
                "INSERT INTO valiaika (\n"
                "  tulos,\n"
                "  jarj,\n"
                "  rasti,\n"
                "  aika,\n"
                "  pisteet,\n"
                "  sakko\n"
                ") SELECT\n"
                "  ?,\n"
                "  v.jarj,\n"
                "  v.rasti,\n"
                "  v.aika,\n"
                "  v.pisteet,\n"
                "  v.sakko\n"
                "FROM tulosdat.valiaika AS v\n"
                "WHERE v.tulos = ?\n"
    );

    queryLuettuAppend.prepare(
                "INSERT INTO luettu_emit (\n"
                "  tapahtuma,\n"
                "  emit,\n"
                "  luettu,\n"
                "  tulos\n"
                ") SELECT\n"
                "  ?,\n"
                "  l.emit,\n"
                "  l.luettu,\n"
                "  ?\n"
                "FROM tulosdat.luettu_emit AS l\n"
                "WHERE l.tulos = ?\n"
    );

    queryLuettuRastiAppend.prepare(
                "INSERT INTO luettu_emit_rasti (\n"
                "  luettu_emit,\n"
                "  numero,\n"
                "  koodi,\n"
                "  aika\n"
                ") SELECT\n"
                "  ?,\n"
                "  lr.numero,\n"
                "  lr.koodi,\n"
                "  lr.aika\n"
                "FROM tulosdat.luettu_emit_rasti AS lr\n"
                "  JOIN tulosdat.luettu_emit AS l ON l.id = lr.luettu_emit\n"
                "WHERE l.tulos = ?\n"
    );

    queryAppend.prepare(
                "INSERT INTO tulos (\n"
                "  tapahtuma,\n"
                "  emit,\n"
                "  kilpailija,\n"
                "  sarja,\n"
                "  tila,\n"
                "  aika,\n"
                "  maaliaika,\n"
                "  pisteet,\n"
                "  sakko,\n"
                "  poistettu\n"
                ") SELECT\n"
                "  ta.id,\n"
                "  t.emit,\n"
                "  k.id,\n"
                "  s.id,\n"
                "  tt.id,\n"
                "  t.aika,\n"
                "  t.maaliaika,\n"
                "  t.pisteet,\n"
                "  t.sakko,\n"
                "  t.poistettu\n"
                "FROM (\n"
                "  SELECT\n"
                "    t.id AS id,\n"
                "    ta.nimi AS tapahtuma,\n"
                "    t.emit AS emit,\n"
                "    k.nimi AS kilpailija,\n"
                "    s.nimi AS sarja,\n"
                "    tt.nimi AS tila,\n"
                "    t.aika AS aika,\n"
                "    t.maaliaika AS maaliaika,\n"
                "    t.pisteet AS pisteet,\n"
                "    t.sakko AS sakko,\n"
                "    t.poistettu AS poistettu\n"
                "  FROM tulosdat.tulos AS t\n"
                "    JOIN tulosdat.kilpailija AS k ON k.id = t.kilpailija\n"
                "    JOIN tulosdat.sarja AS s ON s.id = t.sarja\n"
                "    JOIN tulosdat.tulos_tila AS tt ON tt.id = t.tila\n"
                "    JOIN tulosdat.tapahtuma AS ta ON ta.id = t.tapahtuma\n"
                "  WHERE t.id = ?\n"
                ") AS t\n"
                "  JOIN tapahtuma AS ta ON ta.nimi = t.tapahtuma\n"
                "  JOIN kilpailija AS k ON k.nimi = t.kilpailija\n"
                "  JOIN tulos_tila AS tt ON tt.nimi = t.tila\n"
                "  JOIN sarja AS s ON s.nimi = t.sarja\n"
                "                 AND s.tapahtuma = ta.id\n"
    );

    SQL_EXEC(query, false);

    while (query.next()) {
        QSqlRecord r = query.record();
        QVariant newTulosId;
        QVariant newLuettuEmitId;

        queryAppend.addBindValue(r.value("id"));

        SQL_EXEC(queryAppend, false);

        newTulosId = queryAppend.lastInsertId();

        queryValiaikaAppend.addBindValue(newTulosId);
        queryValiaikaAppend.addBindValue(r.value("id"));

        SQL_EXEC(queryValiaikaAppend, false);

        queryLuettuAppend.addBindValue(tapahtuma.id());
        queryLuettuAppend.addBindValue(newTulosId);
        queryLuettuAppend.addBindValue(r.value("id"));

        SQL_EXEC(queryLuettuAppend, false);

        newLuettuEmitId = queryLuettuAppend.lastInsertId();

        queryLuettuRastiAppend.addBindValue(newLuettuEmitId);
        queryLuettuRastiAppend.addBindValue(r.value("id"));

        SQL_EXEC(queryLuettuRastiAppend, false);
    }

    // Päivitettään emit taulun kilpailija osoittamaan viimeisen tuloksen kilpailijaan.
    query.prepare("UPDATE emit SET kilpailija = (SELECT a.kilpailija FROM tulos AS a WHERE a.tapahtuma = ? AND a.emit = emit.id) WHERE NOT laina AND kilpailija IS NULL");

    query.addBindValue(tapahtuma.id());

    SQL_EXEC(query, false);

    QSqlDatabase::database().commit();

    return true;
}

bool Tietokanta::checkVersion(const QString &version, const QString &table)
{
    QSqlQuery query;

    if (table.isNull())
        query.prepare("SELECT COUNT(*) FROM tulospalvelu WHERE versio = ?");
    else
        query.prepare(_("SELECT COUNT(*) FROM %1 WHERE versio = ?").arg(table));

    query.addBindValue(version);
    SQL_EXEC(query, false);

    return query.next() && query.value(0).toInt() == 1;
}

QString Tietokanta::getVersion(const QString &table)
{
    QSqlQuery query;
    QString res = _("Tuntematon");

    if (table.isNull()) {
        query.prepare("SELECT versio FROM tulospalvelu LIMIT 1");
    } else {
        query.prepare(_("SELECT versio FROM %1 LIMIT 1").arg(table));
    }

    SQL_EXEC(query, res);

    if (!query.next()) {
        return res;
    }

    return query.value(0).toString();
}

void Tietokanta::backup(const QString &toTarget)
{
#ifdef USE_MYSQL
    Q_UNUSED(toTarget);
    // FIXME
#else
    QSqlDatabase::database().commit();

    QFile::copy("tulospalvelu.sqlite3", toTarget);
#endif
}
