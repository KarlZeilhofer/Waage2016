#include "filter.h"
#include "global.h"


Filter::Filter(int filterID, QObject *parent) :
    QObject(parent) {

	//Listen für die Koeffizienten
	b0= new QList<double>();
	b1= new QList<double>();
	b2= new QList<double>();
	a1= new QList<double>();
	a2= new QList<double>();

	this->filterID=filterID;

    //Koeffizienten einlesen
	readCoeffs(COEFFS_FILE_NAME);

    //Anzahl der Biquads bestimmen
    count=b0->length();

	//Speicher für die Zwischenwerte mit 0 initialsisieren
	w=new QList<double>();
    for(int i=0;i<(count*2);i++)    {//Pro Biquad sind 2 Werte zu speichern
        w->append(0);
    }

	output=false;

}

//Dekonstruktor
Filter::~Filter()   {
	//Listen löschen
    delete b0;
    delete b1;
    delete b2;
    delete a1;
    delete a2;
    delete w;
}

//Berechnet aus dem aktuellen Eingangswert und den Zuständen den Ausgangswert
void Filter::feedValue(double input)    {
	//qDebug() << "filter " << filterID << " input: " << input;

	// by Birgit:
    double wact=0;
    for(int biquad=0; biquad<count; biquad++)   {
        //Neues w ausrechnen
		wact=input - w->at(biquad*2)*a1->at(biquad) - a2->at(biquad)*w->at(2*biquad+1);
		//Ausgang des einen Biquads berechnen ->Eingang des nächsten
		input=b0->at(biquad)*wact + b1->at(biquad)*w->at(2*biquad) + b2->at(biquad)*w->at(2*biquad+1);
		//Zustände ändern
        w->replace(biquad*2+1,w->at(biquad*2));
        //neuen Wert speichern
        w->replace(biquad*2,wact);
    }

	if(output)
	{
		emit valueReady(input);
		//qDebug() << "filter" << filterID << "output:" << input;
		output=false;
	}
	else
	{
		output=true;
	}
}


//Liest die benötigten Filterkoeffizienten aus der Datei mit filename ein
void Filter::readCoeffs(QString filename)    {
    QFile file(filename, this);
	//Datei zum Lesen öffnen
	if(!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Fehler beim Lesen der Filterkoeffizienten!";
		return;
	}

    //Textstream erstellen
    QTextStream tstream(&file);


	//So lange einlesen, bis am Ende, keine Fehlerprüfung
    while (!tstream.atEnd()) {
        double num;
        //b0 lesen
        tstream>> num;
        b0->append(num);
        //b1 lesen
		tstream>>num;
        b1->append(num);
        //b2 lesen
        tstream>>num;
        b2->append(num);
        //1 einlesen
        tstream>>num;
        //a1 lesen
        tstream>>num;
        a1->append(num);
        //a2 lesen
		tstream>>num;
        a2->append(num);
		//Leerzeichen und Zeilenumbruch überspringen
        tstream.skipWhiteSpace();
		//qDebug()<<"Koeffizienten\n"<<b0->last()<< " "<<b1->last()<<" "<<b2->last()<< " "<<a1->last()<<" "<<a2->last()<<"\n";
	}
    file.close();
}

