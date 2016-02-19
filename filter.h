#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include <QDebug>
#include <QList>
#include <QFile>
#include <QTextStream>

/* Die Koeffizienten für den Filter werden aus einer Datei
  mit folgendem Format eingelesen:
 b01 b11 b21 1 a11 a21
 b02 b12 b22 1 a12 a22
 ...
 Die einzelnen Koeffizienten sollen als double einlesbar sein*/

class Filter : public QObject
{
	Q_OBJECT

signals:
	void valueReady(double value);

public:
    //Konstruktor
	explicit Filter(int filterID, QObject *parent = 0); //Keine Fehlerprüfung beim Einlesen der Datei
    //Destruktor
    ~Filter();

public slots:
	//Berechnet den aktuellen Ausgangswert des Filters
	void feedValue(double input);


private:
	int filterID;
	bool output;	// wird der aktuelle Wert ausgegeben?

	//Speicher für die Koeffizienten
    QList<double>* b0;
    QList<double>* b1;
    QList<double>* b2;
    QList<double>* a1;
    QList<double>* a2;

    int count;  //Anzahl der Biquads

	QList<double>* w; //Speicher für die Zwischenergebnisse.

	bool firstRun;

	void readCoeffs(QString filename); //Liest die Koeffizienten für den TP-Filter mit Biquad-Struktur ein
	
};

#endif // FILTER_H
