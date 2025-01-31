/*
   Copyright (C) 2018 Arto Hyvättinen

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

#ifndef TILIMUUNTOMODEL_H
#define TILIMUUNTOMODEL_H

#include <QAbstractTableModel>
#include "model/euro.h"
#include "maaritys/tilinavaus/tilinavausmodel.h"

/**
 * @brief The TiliMuuntoModelin sisäinen tietorakenne
 */
class TilinMuunnos
{
public:
    TilinMuunnos(int numero = 0, QString nimi = QString(), int muunnettu = 0, QList<Euro> euroSaldot = QList<Euro>());
    QString tiliStr() const;

    int alkuperainen() const { return alkuperainenTilinumero_; }
    QString tiliNimi() const { return tilinNimi_; }
    int muunnettu() const { return muunnettuTilinumero_; }
    Euro saldo() const;
    QList<Euro> saldot() const { return saldo_; }

    void setMuunnettu(int tilinumero);
    void setSaldo(const Euro& saldo);
    int saldoja() const;

protected:
    int alkuperainenTilinumero_;
    QString tilinNimi_;
    int muunnettuTilinumero_;
    QList<Euro> saldo_;
};

/**
 * @brief The TiliMuuntoModel class
 */
class TiliMuuntoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Sarake
    {
        ALKUPERAINEN,
        NIMI,
        UUSI,
        SALDO
    };

    TiliMuuntoModel(QObject *parent = nullptr);
    TiliMuuntoModel(const QList<QPair<int, QString>> &tilit);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int tilinumeroIndeksilla(int indeksi) const;
    QList<AvausEra> eraIndeksilla(int indeksi);

    /**
     * @brief Tilien muunnostaulukko (alkuperäinen, muunnettu)
     * @return
     */
    QMap<QString,int> muunnettu();

    void asetaSaldoPaivat(QList<QDate> saldopaivat);
    QList<QDate> saldopaivat() const { return saldoPaivat_;}

    void lisaa(int numero, const QString& nimi, QList<Euro> euroSaldo = QList<Euro>());

    bool naytaMuuntoDialogi(QWidget* parent = nullptr);
    bool kaikkiMuunnettu() const;

protected:
    QList<TilinMuunnos> data_;
    QMap<QString,int> muunteluLista_;
    QList<QDate> saldoPaivat_;

    static QRegularExpression TyhjaPoisRE__;


};

#endif // TILIMUUNTOMODEL_H
