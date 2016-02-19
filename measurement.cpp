#include "measurement.h"
#include <stdint.h>
#include <QTime>
#include <QDate>
#include "widget.h"
#include <cmath>
#include <QSettings>

Measurement::Measurement(QSerialPort *port, QObject *parent):
    QObject(parent)
{
	this->port=port;
	this->activeCurrent=false;
	this->activeTemp=false;

    QString filesPrefix=QString(QDir::homePath() + "/scalelogs/%1_%2_").arg(QDate::currentDate().toString("yyyyMMdd")).arg(QTime::currentTime().toString("HHmmss"));
    qDebug() << filesPrefix.toLatin1();


	// Datei für Rohdaten öffnen
    adcSamplesFile.setFileName(filesPrefix+"adcSamples.txt");
    if(!adcSamplesFile.open(QIODevice::WriteOnly))	{
        qDebug() << "Fehler beim Öffnen der Datei: " << (filesPrefix+"adcSamples.txt").toLatin1();
		return;
	}
    adcSamplesFile.write("% Rohdaten von 31-Bit-ADC, ungefiltert (in LSB)\n");

	// Datei für gefilterte Daten öffnen
    filteredValuesFile.setFileName(filesPrefix+"filteredValues.txt");
    if(!filteredValuesFile.open(QIODevice::WriteOnly))	{
        qDebug() << "Fehler beim Öffnen der Datei: " << (filesPrefix+"filteredValues.txt").toLatin1();
		return;
	}
    filteredValuesFile.write("% gefilterte und justierte Daten in Gramm\n");
    filteredValuesFile.write(QString("% Datum: %1\n").arg(QDate::currentDate().toString()).toLatin1());
    filteredValuesFile.write(QString("% Zeit: %1\n").arg(QTime::currentTime().toString()).toLatin1());
    filteredValuesFile.write(QString("%1 % Abtastrate in Hz\n").arg(250.0/(1<<DOWN_SAMPLING_FAKTOR)).toLatin1());


    // Datei für Temperatur Messwerte öffnen
    temperatureFile.setFileName(filesPrefix+"temperatures.txt");
    if(!temperatureFile.open(QIODevice::WriteOnly))	{
        qDebug() << "Fehler beim Öffnen der Datei: " << (filesPrefix+"temperatures.txt").toLatin1();
        return;
    }
    temperatureFile.write("% Temperaturen in °C\n");
    temperatureFile.write(QString("% Datum: %1\n").arg(QDate::currentDate().toString()).toLatin1());
    temperatureFile.write(QString("% Zeit: %1\n").arg(QTime::currentTime().toString()).toLatin1());
    temperatureFile.write("% Uhrzeit Temperatur\n");


    rawSerialFile.setFileName(filesPrefix+"serialInput.raw");
    if(!rawSerialFile.open(QIODevice::WriteOnly))	{
		qDebug("Fehler beim Öffnen der Datei inputstream.raw.");
		return;
	}

	//Skalierungsfaktoren initialisieren
	//currentFactor=100.0/INT_MAX;

    QSettings set;

    currentFactor= set.value("factor", 1).toDouble();
    currentOffset= set.value("offset", -2<<23).toDouble();

	tempFactor=1;
	tempOffset=0;

	// Filterkette aufbauen
	for(int i=0; i<DOWN_SAMPLING_FAKTOR; i++)
	{
		Filter* filt=new Filter(i+1, this);

		// Filter aneinander hängen
		if(i==0)
		{
			// erster Filter
			connect(this, SIGNAL(valueReceived(double)), filt, SLOT(feedValue(double)));
		}
		else
		{
			// alle anderen Filter
			connect(filterChain.last(), SIGNAL(valueReady(double)), filt, SLOT(feedValue(double)));
		}

		filterChain.append(filt);
	}

	// letzter Filter
	connect(filterChain.last(), SIGNAL(valueReady(double)), this, SLOT(rescaleValue(double)));
}

//Destruktor
Measurement::~Measurement()	{
	//Wenn Messung läuft Messung anhalten
	if(activeCurrent)	{
		stopCurrentMeasurement(); //Messung anhalten
	}
	if(activeTemp)	{
		stopTempMeasurement();
	}

	// Dateien schließen
    adcSamplesFile.close();
    filteredValuesFile.close();
    rawSerialFile.close();
    temperatureFile.close();

	//Buffer löschen
	//delete receivedData;

	// Filter löschen
	for(int i=0; i<DOWN_SAMPLING_FAKTOR; i++)
	{
		delete filterChain.takeFirst();
	}

	//delete filterChain;
}

void Measurement::startCurrentMeasurement()	{
	if(!activeCurrent)
	{
		port->putChar(ADS1281_START_SIGN);
		//Eventuell Antwort
		activeCurrent=true;
	}
}

void Measurement::stopCurrentMeasurement()	{
	if(activeCurrent)
	{
		port->putChar(ADS1281_STOP_SIGN);
		//Eventuell Bestätigung
		activeCurrent=false;
	}
}

void Measurement::startTempMeasurement()	{
	if(!activeTemp)
	{
		port->putChar(MAX11202_START);
		activeTemp=true;
	}
}

void Measurement::stopTempMeasurement()	{
	if(activeTemp)
	{
		port->putChar(MAX11202_STOP);
		activeTemp=false;
	}
}

void Measurement::processData()	{
	//Empfangene Daten aus der Liste recievedData nehmen
	unsigned char nextByte;
    int len=0;
//	unsigned int num=0;
    bool waitingForNumbers=false;
	while(!receivedData.isEmpty()) {

		if(true){//!waitingForNumbers){
			nextByte=receivedData.first(); // ältestes Element aus dem Buffer betrachten
			switch(nextByte) {
				case ADS1281_MARKER:
					len=4; //Vier Bytes nötig
					waitingForNumbers=true;
					break;
				case TEMP_MARKER_1:
				case TEMP_MARKER_2:
				case TEMP_MARKER_3:
				case TEMP_MARKER_4:
					len = 3; //3 Bytes nötig4
					waitingForNumbers=true;
					break;
                case AD7190_MARKER:
                    len=3;
                    waitingForNumbers=true;
                    break;
				default:
					//Unerwartetes Zeichen
					//Zeichen entfernen und nächstes Einlesen
					receivedData.removeFirst();
					qDebug("unerwartetes Zeichen");
					waitingForNumbers=false;
					continue;
			}
		}

		//Prüfen ob bereits genügend Bytes übertragen
		if(receivedData.length()<len+1) {//Typ + Daten (len+1)
			//Zu wenig Daten übertragen
			return;
		}
		waitingForNumbers=false;

		//Noch genügend zeichen vorhanden -> Zahl einlesen

		unsigned char type=nextByte;
		receivedData.removeFirst();		// Marker-Byte verwerfen


		//qDebug() << "num: " << 5;

		if(type==ADS1281_MARKER)	{ // Strommessung

			// integer aus bytes zusammenbauen (MSB kommt als erstes)

			uint32_t uSigVal=0;
			for(int i=0; i<4; i++)
			{
				uSigVal<<=8;
				uSigVal|=receivedData.takeFirst();
			}
			int32_t signedVal=uSigVal;

			double value=(double)signedVal;

            if(Widget::app->settings.writeAdcSamples()){
                adcSamplesFile.write((QString::number(value, 'e', 10)+"\n").toLatin1());
            }

			if((uSigVal>>31) != (uSigVal&1)) // check, if sign-bit is equal to the redundant bit-0
			{
				qDebug() << "Sign-Bit Error: (value = " << value << ")";
			}
			else
			{
                emit valueReceived(value); // send value to filter
			}
        }else if(type == AD7190_MARKER)
        {
            // integer aus bytes zusammenbauen (MSB kommt als erstes)

            uint32_t uSigVal=0;
            for(int i=0; i<3; i++)
            {
                uSigVal<<=8;
                uSigVal|=receivedData.takeFirst();
            }
            int32_t signedVal=uSigVal;

            double value=(double)signedVal;

            if(Widget::app->settings.writeAdcSamples()){
                adcSamplesFile.write((QString::number(value, 'e', 10)+"\n").toLatin1());
            }

            emit valueReceived(value); // send value to filter
        }
        else{	// Temperaturmessung
            uint32_t uSigVal=0;
            for(int i=0; i<3; i++)
            {
                uSigVal<<=8;
                uSigVal|=receivedData.takeFirst();
            }
            uSigVal <<=8; // 24 bit hochschieben, damit das vorzeichenbit passt
            int32_t signedVal=uSigVal;

            double value=(double)signedVal;
            // convert to centigrade!
            double Uref = 3.000; // Volt, Referenzspannung
            double Uin = -Uref*(value/(1<<31)); // TODO: überprüfen, warum negative spannung gemessen wird.
            //qDebug() << "24Bit ADC input-Voltage:" << Uin << "V";
            double R0 = 10e3; // Referenzwiderstand
            double R_N = 10e3; // NTC Nennwiderstand
            double T_N = 25.0+273.15; // NTC Nenntemperatur
            double B = 3977; // Kelvin +/-0.75%
            double R_NTC = R0/(Uref/Uin -1.0);
            //qDebug() << "R_NTC =" << R_NTC << "Ohm";
            double T = 1.0/(log(R_NTC/R_N)/B + 1.0/T_N); // absolute NTC-Temperatur
            double temperature = T-273.15; // in grad celsius

            int sensorID=0;
            switch(type){
            case TEMP_MARKER_1: sensorID=1; break;
            case TEMP_MARKER_2: sensorID=2; break;
            case TEMP_MARKER_3: sensorID=3; break;
            case TEMP_MARKER_4: sensorID=4; break;
            }

            static bool visitedFirst = false; // gibt an, ob T1 besucht wurde
            if(!visitedFirst && sensorID == 1){
                visitedFirst = true;
            }
            if(visitedFirst){
                temperatureFile.write(QString("%1 %2 %3\n").
                                      arg(sensorID).
                                      arg(temperature).
                                      arg(QTime::currentTime().toString("hhmmss")).
                                      toLatin1());
                temperatureFile.flush();
            }

            emit temperatureReceived(sensorID, temperature);
		}
    }
}

// slot for serial connection
void Measurement::addNewData(unsigned char val)
{

    if(Widget::app->settings.writeRawSerial()){
        rawSerialFile.write((char*)(&val),1);
    }
	receivedData.append(val);
	processData();
}


// rescaling happens after filtering
void Measurement::rescaleValue(double value)
{
	rawValue=value;
	value=(value-currentOffset)*currentFactor;	// auf Gewicht umrechnen

    if(Widget::app->settings.writeFilteredValues()){
        filteredValuesFile.write((QString::number(value, 'e', 16)+"\n").toLatin1());
        filteredValuesFile.flush();
    }

    emit valueReady(value, rawValue); // send value to the Widget
}


void Measurement::tare()
{
	currentOffset=rawValue;
	qDebug() << "offset:" << currentOffset;

    QSettings set;
    set.setValue("offset", currentOffset);
}


void Measurement::adjust(double kalib_weight)
{
	if(kalib_weight!=0)
	{
		currentFactor=kalib_weight/(rawValue-currentOffset);
		qDebug() << "faktor:" << currentFactor;

        QSettings set;
        set.setValue("factor", currentFactor);
	}
}
