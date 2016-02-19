#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QList>
#include <QFile>
#include <QObject>

#include "global.h"
#include "filter.h"


class Measurement: public QObject{
	Q_OBJECT

signals:

	void valueReceived(double value);	// Wert von serieller Schnittstelle empfangen
    void valueReady(double value, double rawValue);		// Wert gefiltert und bereit zum Darstellen
    void temperatureReceived(int sensorID, double temperature); // ID = 1...4, T in °C

public:
	//Konstruktor
	explicit Measurement(QObject *parent=0);
    Measurement(QSerialPort* port, QObject *parent);
	//Destruktor
	~Measurement();

	void startTempMeasurement();// Startet MAX11202
	void stopTempMeasurement(); // Stoppt MAX11202

	void startCurrentMeasurement();// Startet ADS1281
	void stopCurrentMeasurement(); // Stoppt ADS1281

	//Gibt true zurück, wenn Spannungsmessung läuft
    inline bool tempIsRunning() {
        return activeTemp;
    }

	// Gibt true zurück, wenn Strommessung läuft
    inline bool currentIsRunning()	{
        return activeCurrent;
	}

	void addNewData(unsigned char val); //Fügt neue Daten zu recievedData hinzu

	void tare();						// Stellt Anzeige auf 0
	void adjust(double kalib_weight);	// Justiert die Anzeige auf Kalibrierwert


public slots:

	void rescaleValue(double value);


private:
	void processData();		//Verarbeitet die Daten im Buffer

	bool activeCurrent;
	bool activeTemp;

    QSerialPort *port;	//Serielle Schnittstelle, an die die Befehle geschickt werden
    QFile adcSamplesFile;			// Datei für ungefilterte Daten
    QFile filteredValuesFile;			// Datei für gefilterte und skalierte Anzeigewerte
    QFile rawSerialFile;     // Datei für die empfangenen Rohdaten der Seriellen Schnittstelle
    QFile temperatureFile;  // Datei mit den Temperaturmesswerten

	QList<unsigned char> receivedData;	//Speicher für empfangene Daten

	double rawValue;	// Rohwert nach Downsampling

	// Offset
	double currentOffset, tempOffset;

	// Skalierungsfaktoren
	double currentFactor, tempFactor;

	QList<Filter*> filterChain;

};

#endif // MEASUREMENT_H
