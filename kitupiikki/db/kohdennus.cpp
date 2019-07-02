/*
   Copyright (C) 2017 Arto Hyvättinen

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include "kohdennus.h"


Kohdennus::Kohdennus(int tyyppi, const QString &nimi) :
    id_(0), tyyppi_(tyyppi), nimi_(nimi), muokattu_(true)
{

}

Kohdennus::Kohdennus(int id, int tyyppi, const QString& nimi, QDate alkaa, QDate paattyy)
    : id_(id), tyyppi_(tyyppi), nimi_(nimi), alkaa_(alkaa), paattyy_(paattyy),
      muokattu_(false)
{

}

Kohdennus::Kohdennus(QVariantMap &data) :
    KantaVariantti(data), muokattu_(false)
{
    qDebug() << "Kohdennus " << data;

    id_ = data_.take("id").toInt();

    QString tyyppiteksti = data_.take("tyyppi").toString();

    if( tyyppiteksti == "kustannuspaikka")
        tyyppi_ = KUSTANNUSPAIKKA;
    else if( tyyppiteksti == "projekti")
        tyyppi_ = PROJEKTI;
    else if( tyyppiteksti == "merkkaus")
        tyyppi_ = MERKKAUS;
    else if( tyyppiteksti == "oletus")
        tyyppi_ = EIKOHDENNETA;

    nimi_.aseta( data_.take("nimi"));
    alkaa_ = data_.take("alkaa").toDate();
    paattyy_ = data_.take("paattyy").toDate();
}

QIcon Kohdennus::tyyppiKuvake() const
{
    if( tyyppi() == KUSTANNUSPAIKKA)
        return QIcon(":/pic/kohdennus.png");
    else if( tyyppi() == PROJEKTI )
        return QIcon(":/pic/projekti.png");
    else if( tyyppi() == MERKKAUS )
        return QIcon(":/pic/tag.png");
    else
        return QIcon();
}

int Kohdennus::montakoVientia() const
{
    if( id() < 0)
        return 0;   // Uudella kohdennuksella ei vielä vientejä

    QSqlQuery kysely( QString("SELECT sum(id) FROM vienti WHERE kohdennus=%1").arg(id()) );
    if( kysely.next())
        return kysely.value(0).toInt();
    return 0;
}

void Kohdennus::asetaId(int id)
{
    id_ = id;
}

void Kohdennus::asetaNimi(const QString &nimi)
{
    nimi_.aseta( nimi );
    muokattu_ = true;
}

void Kohdennus::asetaAlkaa(const QDate &alkaa)
{
    alkaa_ = alkaa;
    muokattu_ = true;
}

void Kohdennus::asetaPaattyy(const QDate &paattyy)
{
    paattyy_ = paattyy;
    muokattu_ = true;
}

void Kohdennus::asetaTyyppi(Kohdennus::KohdennusTyyppi tyyppi)
{
    tyyppi_ = tyyppi;
    muokattu_ = true;
}

void Kohdennus::nollaaMuokattu()
{
    muokattu_ = false;
}
