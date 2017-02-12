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

#ifndef TOSITELAJIT_H
#define TOSITELAJIT_H

#include <QWidget>

#include "ui_tositelajit.h"
// #include "tositelajitmodel.h"
#include "db/tositelajimodel.h"

#include "maarityswidget.h"

/**
 * @brief MääritysWidget tositelajien määrittämiseen
 *
 * Tositelajit erittelevät tositteita eri tarkenteilla omille numerosarjoilleen
 *
 */
class Tositelajit : public MaaritysWidget
{
    Q_OBJECT
public:
    explicit Tositelajit(QWidget *parent = 0);
    ~Tositelajit();

signals:

public slots:

public:
    bool tallenna();
    bool nollaa();

protected:
    Ui::Tositelajit *ui;
    TositelajiModel *model;
};

#endif // TOSITELAJIT_H
