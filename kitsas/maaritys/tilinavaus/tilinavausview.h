#ifndef TILINAVAUSVIEW_H
#define TILINAVAUSVIEW_H

#include <QTableView>

class QSortFilterProxyModel;
class TilinavausModel;


class TilinAvausView : public QTableView
{
    Q_OBJECT
public:
    TilinAvausView(QWidget* parent = nullptr);

    TilinavausModel *avausModel();

    void naytaPiilotetut(bool naytetaanko);
    void naytaVainKirjaukset(bool naytetaanko);
    void suodata(const QString& suodatusteksti);
    void nollaa();
    void tuoAvausTiedosto(const QString& polku);


protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);


    static QDate kkPaivaksi(const QString teksti);
protected:
    TilinavausModel* model_;
    QSortFilterProxyModel* proxy_;
    QSortFilterProxyModel* suodatus_;


    static QRegularExpression tiliRE__;    

};

#endif // TILINAVAUSVIEW_H
