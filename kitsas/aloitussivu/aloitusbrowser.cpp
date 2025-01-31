#include "aloitusbrowser.h"


#include "db/kirjanpito.h"
#include "pilvi/pilvimodel.h"
#include "maaritys/tilitieto/tilitietopalvelu.h"
#include "pilvi/paivitysinfo.h"
#include "maaritys/tilikarttapaivitys.h"
#include "alv/alvilmoitustenmodel.h"
#include "sqlite/sqlitemodel.h"
#include "kieli/kielet.h"
#include "versio.h"
#include <QRegularExpression>
#include <QSettings>
#include "alv/alvkaudet.h"

AloitusBrowser::AloitusBrowser(QWidget* parent) :
    QTextBrowser(parent)
{
    setOpenLinks(false);
    connect( kp()->pilvi()->paivitysInfo(), &PaivitysInfo::infoSaapunut, this, &AloitusBrowser::naytaPaivitetty );
}

void AloitusBrowser::paivita()
{
    if( saldoPvm_.isValid() && kp()->yhteysModel()) {
        paivitaAvattu();
    } else {
        naytaTervetuloa();
    }
}

void AloitusBrowser::haSaldot(const QDate &saldoPvm)
{
    saldoPvm_ = saldoPvm;
    paivitaSaldot();
}

void AloitusBrowser::asetaTilioimatta(int tilioimatta)
{
    tilioimatta_ = tilioimatta;
    paivitaAvattu();
}

void AloitusBrowser::naytaPaivitetty()
{
    if( kp()->yhteysModel()) {
        paivitaAvattu();
    } else {
        naytaTervetuloa();
    }
}

void AloitusBrowser::paivitaAvattu()
{

    QString txt("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"qrc:/aloitus/aloitus.css\"></head><body>");

    if( !kp()->yhteysModel()->oikeudet()) {
        txt.append( eiOikeuttaUrputus()  );
    } else {
        paivitaVinkit();
        txt.append( kp()->pilvi()->paivitysInfo()->toHtml() );

        const int saldoja = saldot_.count();
        if( saldoja == 0) {
            txt.append("<h1>" + tr("Avataan kirjanpitoa...") + "</h1>");
        } else if( saldoja < 3) {
            txt.append( vinkit_.toHtml() );
            txt.append("<p><img src=qrc:/pic/kitsas150.png></p>");
        } else {
            txt.append( vinkit_.toHtml() );
            txt.append( saldoTaulu() );
        }
    }

    setHtml( txt + "</body></html");

}

void AloitusBrowser::vinkkaa(const QString &luokka, const QString &otsikko, const QString &teksti, const QString &linkki, const QString kuva, const QString ohjelinkki)
{
    vinkit_.info(luokka, otsikko, teksti, linkki, kuva, ohjelinkki);
}

void AloitusBrowser::paivitaVinkit()
{
    vinkit_.clear();
    paivitaTestiVinkki();
    paivitaTiliointiVinkki();
    paivitaPankkiyhteysVinkki();
    paivitaVarmuuskopioVinkki();
    paivitaVerotonValitus();
    paivitaPaivitysVinkki();
    paivitaAloitusVinkit();
    paivitaAlvVinkki();
    paivitaTilikausiVinkki();
    paivitaTilinpaatosVinkki();
}

QString AloitusBrowser::eiOikeuttaUrputus() const
{
    return QString("<h1>%1</h1><p>%2</p><p>%3</p>")
            .arg(tr("Tämä kirjanpito ei ole käytettävissä"))
            .arg(tr("Kirjanpidon omistajalla ei ole voimassa olevaa tilausta, omistaja on pyytänyt tunnuksen sulkemista, tilausmaksu on maksamatta tai "
                    "palvelun käyttö on estetty käyttösääntöjen vastaisena."))
            .arg(tr("Kirjanpidon palauttamiseksi käyttöön on oltava yhteydessä Kitsas Oy:n myyntiin."));
}

void AloitusBrowser::paivitaTestiVinkki()
{
    if( kp()->pilvi()->pilviLoginOsoite() != KITSAS_API) {
        vinkkaa("testaus", tr("Testauspalvelin käytössä"), tr("Käytä vain kehitys- ja testauskäytössä!"), QString(), "dev.png");
    }
    if( QString(KITSAS_VERSIO).contains("-") ) {
        vinkkaa("testaus", tr("Kehitysversio käytössä"), tr("Käytössäsi on Kitsaan kehitysversio, jota ei välttämättä ole vielä testattu kattavasti."), QString(), "tietyo.png");
    }
}

void AloitusBrowser::paivitaTiliointiVinkki()
{
    if( tilioimatta_ ) {
        vinkkaa("tilioimatta",tr("Tiliöinti on kesken %1 tiliotteessa.").arg(tilioimatta_), tr("Kirjanpitosi ei täsmää ennen kuin nämä tositteet on tiliöity loppuun saakka."),
                            "ktp:/huomio", "oranssi.png");
    }

}

void AloitusBrowser::paivitaPankkiyhteysVinkki()
{
    if( qobject_cast<PilviModel*>( kp()->yhteysModel() ) && kp()->pilvi()->tilitietoPalvelu()) {
        QDateTime uusinta = kp()->pilvi()->tilitietoPalvelu()->seuraavaUusinta();
        if( uusinta.isValid() && uusinta < QDateTime::currentDateTime()) {
            vinkkaa("varoitus", tr("Pankkiyhteyden valtuutus vanhentunut"),
                                tr("Valtuutus on vanhentunut %1. Tilitapahtumia ei voi hakea ennen valtuutuksen uusimista.").arg(uusinta.toString("dd.MM.yyyy")),
                                "ktp:/maaritys/tilitiedot", "verkossa.png") ;
        } else if ( uusinta.isValid()) {
            const int jaljella = QDateTime::currentDateTime().daysTo(uusinta);
            if( jaljella < 7) {
                vinkkaa("info", tr("Pankkiyhteyden valtuutus vanhenemassa"),
                                    tr("Pankkiyhteyden valtuutus vanhenee %1. Uusi valtuutus jatkaaksesi tilitapahtumien hakemista.").arg(uusinta.toString("dd.MM.yyyy")),
                                    "ktp:/maaritys/tilitapahtumat", "verkossa.png");
            } else if ( jaljella < 21 ) {
                vinkkaa("vinkki", tr("Uusi pankkiyhteyden valtuutus"),
                             tr("Pankkiyhteyden valtuutus vanhenee %1.").arg(uusinta.toString("dd.MM.yyyy")),
                             "ktp:/maaritys/tilitapahtumat", "verkossa.png") ;
            }
        }
    }

}

void AloitusBrowser::paivitaVarmuuskopioVinkki()
{
    SQLiteModel *sqlite = qobject_cast<SQLiteModel*>(kp()->yhteysModel());
    QRegularExpression reku("(\\d{2})(\\d{2})(\\d{2}).kitsas");
    if( sqlite && sqlite->tiedostopolku().contains(reku) ) {
        QRegularExpressionMatch matsi = reku.match(sqlite->tiedostopolku());
        vinkkaa("varoitus", tr("Varmuuskopio käytössä?"),
                     tr("Tämä tiedosto on todennäköisesti kirjanpitosi varmuuskopio päivämäärällä %1<br>"
                     "Tähän tiedostoon tehdyt muutokset eivät tallennu varsinaiseen kirjanpitoosi.")
                      .arg( QString("<b>%1.%2.20%3</b>" ).arg(matsi.captured(3), matsi.captured(2), matsi.captured(1)) ),
                            "","tiedostoon.png","aloittaminen/tietokone/#varoitus-varmuuskopion-käsittelemisestä");
    } else if ( sqlite &&  kp()->settings()->value("PilveenSiirretyt").toString().contains( kp()->asetukset()->uid() ) ) {
        vinkkaa("info", tr("Paikallinen kirjanpito käytössä"),
                            tr("Käytössäsi on omalle tietokoneellesi tallennettu kirjanpito. Muutokset eivät tallennu Kitsaan pilveen.</p>"
                                "<p>Pilvessä olevan kirjanpitosi voit avata Pilvi-välilehdeltä."),
                            "", "computer-laptop.png") ;
    }

}

void AloitusBrowser::paivitaVerotonValitus()
{
    if( qobject_cast<PilviModel*>(kp()->yhteysModel()) && !kp()->pilvi()->pilvi().vat() && kp()->asetukset()->onko(AsetusModel::AlvVelvollinen)) {
        vinkkaa("varoitus", tr("Tilaus on tarkoitettu arvonlisäverottomaan toimintaan."),
                            tr("Pilvikirjanpidon omistajalla on tilaus, jota ei ole tarkoitettu arvonlisäverolliseen toimintaan. "
                               "Arvonlisäilmoitukseen liittyviä toimintoja ei siksi ole käytössä tälle kirjanpidolle."),
                            "", "euromerkki.png");

    }

}

void AloitusBrowser::paivitaPaivitysVinkki()
{
    if( TilikarttaPaivitys::onkoPaivitettavaa() && kp()->yhteysModel()->onkoOikeutta(YhteysModel::ASETUKSET) )
    {
        vinkkaa("info", tr("Päivitä tilikartta"),
                            tr("Tilikartasta saatavilla uudempi versio %1.")
                            .arg(TilikarttaPaivitys::paivitysPvm().toString("dd.MM.yyyy")),
                            "ktp:/maaritys/paivita", "paivita.png","asetukset/paivitys/");

    }

}

void AloitusBrowser::paivitaAloitusVinkit()
{
    // Ensin tietokannan alkutoimiin
    if( saldot_.count() && saldot_.count() < 3)
    {
        QString t("<ol><li> <a href=ktp:/maaritys/perus>" + tr("Tarkista perusvalinnat ja arvonlisäverovelvollisuus") + "</a> <a href='ohje:/asetukset/perusvalinnat'>(" + tr("Ohje") +")</a></li>");
        t.append("<li> <a href=ktp:/maaritys/yhteys>" + tr("Tarkista yhteystiedot ja logo") + "</a> <a href='ohje:/asetukset/yhteystiedot'>(" + tr("Ohje") +")</a></li>");
        t.append("<li> <a href=ktp:/maaritys/tilit>" + tr("Tutustu tilikarttaan ja tee tarpeelliset muutokset") +  "</a> <a href='ohje:/asetukset/tililuettelo/'>(" + tr("Ohje") + ")</a></li>");
        t.append("<li> <a href=ktp:/maaritys/kohdennukset>" + tr("Lisää tarvitsemasi kohdennukset") + "</a> <a href='ohje:/asetukset/kohdennukset/'>(" + tr("Ohje") + ")</a></li>");
        if( kp()->asetukset()->luku("Tilinavaus")==2)
            t.append("<li><a href=ktp:/maaritys/tilinavaus>" + tr("Tee tilinavaus") + "</a> <a href='ohje:/asetukset/tilinavaus/'>(Ohje)</a></li>");
        t.append("<li><a href=ktp:/kirjaa>" + tr("Voit aloittaa kirjausten tekemisen") +"</a> <a href='ohje:/kirjaus'>("+ tr("Ohje") + ")</a></li></ol>");

        vinkkaa("vinkki", tr("Kirjanpidon aloittaminen"),t,
                            "", "info64.png");

    }
    else if( kp()->asetukset()->luku("Tilinavaus")==2 && kp()->asetukset()->pvm("TilinavausPvm") <= kp()->tilitpaatetty() &&
             kp()->yhteysModel()->onkoOikeutta(YhteysModel::ASETUKSET))
        vinkkaa("vinkki", tr("Tee tilinavaus"),
                            tr("Syötä viimeisimmältä tilinpäätökseltä tilien "
                               "avaavat saldot %1 järjestelmään.").arg(kp()->asetukset()->pvm("TilinavausPvm").toString("dd.MM.yyyy")),
                            "ktp:/maaritys/tilinavaus", "rahaa.png", "asetukset/tilinavaus");

}

void AloitusBrowser::paivitaAlvVinkki()
{
    // Muistutus arvonlisäverolaskelmasta
    if(  kp()->asetukset()->onko("AlvVelvollinen") && kp()->yhteysModel()->onkoOikeutta(YhteysModel::ALV_ILMOITUS) )
    {
        QDate kausialkaa = kp()->alvIlmoitukset()->viimeinenIlmoitus().addDays(1);
        int kausi = kp()->asetukset()->luku("AlvKausi");

        // Jos alv-ilmoitus on kuitenkin annettu jollain muulla ohjelmalla, ja se on jo käsitelty,
        // niin eipä sitten urputella...
        while( kp()->alvIlmoitukset()->kaudet()->kausi( kausialkaa ).tila() == AlvKausi::KASITELTY )
            kausialkaa = kausialkaa.addMonths(kausi);


        QDate laskennallinenalkupaiva = kausialkaa;        
        if( kausi == 1)
            laskennallinenalkupaiva = QDate( kausialkaa.year(), kausialkaa.month(), 1);
        else if( kausi == 3) {
            int kk = kausialkaa.month();
            if( kk < 4)
                kk = 1;
            else if( kk < 7)
                kk = 4;
            else if( kk < 10)
                kk = 7;
            else
                kk = 10;
            laskennallinenalkupaiva = QDate( kausialkaa.year(), kk, 1);
        } else if( kausi == 12)
            laskennallinenalkupaiva = QDate( kausialkaa.year(), 1, 1);

        QDate kausipaattyy = laskennallinenalkupaiva.addMonths( kausi ).addDays(-1);
        QDate erapaiva = kp()->alvIlmoitukset()->erapaiva(kausipaattyy);


        qlonglong paivaaIlmoitukseen = kp()->paivamaara().daysTo( erapaiva );
        if( paivaaIlmoitukseen < 0)
        {
            vinkkaa("varoitus", tr("Arvonlisäveroilmoitus myöhässä"),
                                tr("Arvonlisäveroilmoitus kaudelta %1 - %2 olisi pitänyt antaa %3 mennessä.")
                                .arg(kausialkaa.toString("dd.MM.yyyy"),kausipaattyy.toString("dd.MM.yyyy"),erapaiva.toString("dd.MM.yyyy")),
                                "ktp:/alvilmoitus", "vero64.png");
        }
        else if( paivaaIlmoitukseen < 12)
        {
            vinkkaa("vinkki", tr("Tee arvonlisäverotilitys"),
                                tr("Arvonlisäveroilmoitus kaudelta %1 - %2 on annettava %3 mennessä.")
                                .arg(kausialkaa.toString("dd.MM.yyyy"),kausipaattyy.toString("dd.MM.yyyy"),erapaiva.toString("dd.MM.yyyy")),
                                "ktp:/alvilmoitus", "vero64.png");
        }
    }

}

void AloitusBrowser::paivitaTilikausiVinkki()
{
    if( kp()->paivamaara().daysTo(kp()->tilikaudet()->kirjanpitoLoppuu()) < 30 && kp()->yhteysModel()->onkoOikeutta(YhteysModel::ASETUKSET))
    {
        vinkkaa("vinkki", tr("Aloita uusi tilikausi"),
                            tr("Tilikausi päättyy %1, jonka jälkeiselle ajalle ei voi tehdä kirjauksia ennen kuin uusi tilikausi aloitetaan.</p>"
                               "<p>Voit tehdä kirjauksia myös aiempaan tilikauteen, kunnes se on päätetty.").arg( kp()->tilikaudet()->kirjanpitoLoppuu().toString("dd.MM.yyyy") ),
                            "ktp:/uusitilikausi", "kansiot.png","tilikaudet/uusi/");

    }

}

void AloitusBrowser::paivitaTilinpaatosVinkki()
{
    // Tilinpäätöksen laatiminen
    if( kp()->yhteysModel()->onkoOikeutta(YhteysModel::TILINPAATOS)) {
        for(int i=0; i<kp()->tilikaudet()->rowCount(QModelIndex()); i++)
        {
            Tilikausi kausi = kp()->tilikaudet()->tilikausiIndeksilla(i);
            if( kausi.paattyy().daysTo(kp()->paivamaara()) > 1 &&
                                       kausi.paattyy().daysTo( kp()->paivamaara()) < 5 * 30
                    && ( kausi.tilinpaatoksenTila() == Tilikausi::ALOITTAMATTA || kausi.tilinpaatoksenTila() == Tilikausi::KESKEN) )
            {
                vinkkaa("vinkki", tr("Aika laatia tilinpäätös tilikaudelle %1").arg(kausi.kausivaliTekstina()),
                                    kausi.tilinpaatoksenTila() == Tilikausi::ALOITTAMATTA ?
                                         tr("Tee loppuun kaikki tilikaudelle kuuluvat kirjaukset ja laadi sen jälkeen tilinpäätös")
                                       : tr("Viimeistele ja vahvista tilinpäätös"),
                                    "ktp:/tilinpaatos", "kansiot.png","tilikaudet/tilinpaatos/");
            }
        }
    }

}

void AloitusBrowser::paivitaSaldot()
{
    KpKysely *kysely = kpk("/saldot");
    if( kysely && saldoPvm_.isValid()) {
        kysely->lisaaAttribuutti("pvm", saldoPvm_);
        connect( kysely, &KpKysely::vastaus, this, &AloitusBrowser::saldotSaapuu);
        kysely->kysy();
    }

}

void AloitusBrowser::saldotSaapuu(QVariant *data)
{
    QVariantMap map = data->toMap();
    saldot_.clear();

    QMapIterator<QString,QVariant> iter(map);
    while( iter.hasNext()) {
        iter.next();
        saldot_.append(SaldoTieto(iter.key(), iter.value().toString()));
    }
    paivitaAvattu();
}

QString AloitusBrowser::saldoTaulu()
{
    QString txt;

    Tilikausi tilikausi = kp()->tilikaudet()->tilikausiPaivalle( saldoPvm_ );

    txt.append(tr("<p><h2 class=kausi>Tilikausi %1 - %2 </h1>").arg(tilikausi.alkaa().toString("dd.MM.yyyy"))
             .arg(tilikausi.paattyy().toString("dd.MM.yyyy")));


    txt.append("<table width=100% class=saldot>");

    int rivi=0;
    QString edellinen = "0";
    Euro tulossaldo;

    for(const auto& saldo : qAsConst(saldot_)) {
        const Tili* tili = saldo.tili();
        if( !tili) continue;
        if( tili->onko(TiliLaji::KAUDENTULOS) ) tulossaldo = saldo.saldo();
        const QString tiliteksti = QString::number( tili->numero() );

        if( tiliteksti.at(0) == '1' && edellinen.at(0) != '1')
            txt.append("<tr><td colspan=2 class=saldootsikko>" + tr("Vastaavaa") + "</td></tr>");
        if( tiliteksti.at(0) == '2' && edellinen.at(0) != '2')
            txt.append("<tr><td colspan=2 class=saldootsikko>" + tr("Vastattavaa") + "</td></tr>");
        if( tiliteksti.at(0) > '2' && edellinen.at(0) <= '2')
            txt.append("<tr><td colspan=2 class=saldootsikko>" + tr("Tuloslaskelma") + "</td></tr>");
        edellinen = tiliteksti;

        txt.append( QString("<tr class=%4><td><a href=\"selaa:%1\">%2</a></td><td class=euro>%3</td></tr>")
                    .arg(tili->numero())
                    .arg(tili->nimiNumero().toHtmlEscaped())
                    .arg(saldo.saldo().display(true))
                    .arg(rivi++ % 4 == 3 ? "tumma" : "vaalea"));
    }


    txt.append( QString("<tr class=tulosrivi><td>" + tr("Tilikauden tulos") + "</td><td class=euro>%L1 €</td></tr>")
                .arg(tulossaldo.display(true)) );
    txt.append("</table>");

    return txt;

}


void AloitusBrowser::naytaTervetuloa()
{
    const QString tiedostonnimi =
            kp()->pilvi()->kayttaja().moodi() == PilviKayttaja::PRO ?
            ( ":/aloitus/pro.html"  ) :
            ( Kielet::instanssi()->uiKieli() == "sv" ? ":/aloitus/svenska.html" : ":/aloitus/tervetuloa.html" );

    QFile tttiedosto(tiedostonnimi);
    tttiedosto.open(QIODevice::ReadOnly);
    QTextStream in(&tttiedosto);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("utf8");
#endif
    QString teksti = in.readAll();

    vinkit_.clear();
    paivitaTestiVinkki();

    teksti.replace("<INFO>", kp()->pilvi()->paivitysInfo()->toHtml() + vinkit_.toHtml() );

    setHtml( teksti );
}

AloitusBrowser::SaldoTieto::SaldoTieto()
{

}

AloitusBrowser::SaldoTieto::SaldoTieto(const QString &tilinumero, const QString &saldo) :
    tilinumero_{ tilinumero.toInt() }, saldo_{ saldo }
{

}

Tili *AloitusBrowser::SaldoTieto::tili() const
{
    return kp()->tilit()->tili(tilinumero_);
}

Euro AloitusBrowser::SaldoTieto::saldo() const
{
    return saldo_;
}
