#include "widget.h"
#include "ui_widget.h"
#include <QClipboard>
#include <string.h>

#include <QSerialPort>
#include <QSerialPortInfo>


/*
 * Waage 2016
 *
 * 19.2.2016:
 *    ported Waage2 from Qt4 to Qt5, using now the QSerialPort
 *      Added Application name and organisation.
 *      adjustment-settings are stored in QSettings now
 *      added settings for downsampling-factor.
 * TODO: set up filter-chain for max. downsampling-factor,
 *      and dynamically reconnect the signals and slots.
*/




Widget* Widget::app = 0;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    app = this;

    port= new QSerialPort(this);
	connect(port, SIGNAL(readyRead()), this, SLOT(slotSerial()));

	mes=new Measurement(port, this);

    connect(mes, SIGNAL(valueReady(double,double)), this, SLOT(displayValues(double,double)));
    connect(mes, SIGNAL(temperatureReceived(int,double)), this, SLOT(displayTemperature(int,double)));

    setWindowTitle("Scale Logger");


    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBox_serialPort->addItem(QString("%1 (%2)").arg(info.description()).arg(info.portName()), info.portName());
        qDebug() << "Name : " << info.portName();
    }

}

Widget::~Widget()
{
	if(port->isOpen())
	{
		port->close();
	}
    delete ui;
}

void Widget::on_pushButton_Connect_clicked()
{
	if(!port->isOpen()) {

		// Serielle Schnittstelle verbinden
        QString portName = ui->comboBox_serialPort->itemData(ui->comboBox_serialPort->currentIndex()).toString();
        QSerialPortInfo info(portName);
        qDebug() << "Name : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        port->setPort(info);

        if(!port->open(QIODevice::ReadWrite))	{
        //if(!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered))	{
            qDebug() << "could not open serial port";
			return;
		}

        settings.setEnabled(false);

		//Einstellungen

		// Baudrate setzen
        if(!port->setBaudRate(BAUD_RATE))	{
			qDebug() << "Fehler beim Setzen der baudrate";
			return;
		}
		// 8 bit Daten
        if(!port->setDataBits(QSerialPort::Data8))	{
			qDebug() << "Fehler beim Festlegen der Bitanzahl";
			return;
		}
		//Kein Paritybit
        if (!port->setParity(QSerialPort::NoParity))	{
			qDebug() << "Fehler beim Konfigurieren des parity bits";
			return;
		}
		//Ein Stop-Bit
        if (!port->setStopBits(QSerialPort::OneStop))	{
			qDebug() << "Fehler beim Konfigurieren des stop bits";
			return;
		}
		//????		//Asynchroner Modus?
        if (!port->setFlowControl(QSerialPort::NoFlowControl))	{
			qDebug() << "Fehler beim Konfigurieren des asynchronen Modus";
			return;
		}

//		//??????? //Timeout
//		#if defined (Q_OS_UNIX)
//		// Method setTotalReadConstantTimeout() not supported in *.nix.
//        if (port->openMode() & QIODevice::Unbuffered)
//			port->setCharIntervalTimeout(0);//5 msec
//		#elif defined (Q_OS_WIN)
//		if (port->openMode() & AbstractSerial::Unbuffered)
//			port->setTotalReadConstantTimeout(100);
//		#endif

		//Aufschrift des Buttons ändern
		ui->pushButton_Connect->setText("Trennen");

		//????		//Messung starten Button aktivieren
		ui->pushButton_Start->setEnabled(true);

		// Statuszeile ändern
		ui->label_status->setText("verbunden");

	}
	else	{

        settings.setEnabled(true);

		//Trennen

		mes->stopCurrentMeasurement();
		mes->stopTempMeasurement();

		//Aufschrift ändern und deaktivieren
		ui->pushButton_Start->setText("Messung starten");
		ui->pushButton_Start->setEnabled(false);

		//Serielle Verbindung trennen
		port->close();

		//Aufschrift ändern
		ui->pushButton_Connect->setText("Verbinden");

		// Statuszeile ändern
		ui->label_status->setText("nicht verbunden");
	}
}

void Widget::slotSerial()	{

	QByteArray input = port->readAll(); //Einlesen, was im Buffer ist
	//Zu Array mit den Messdaten hinzufügen
	for(int i=0; i<input.length(); i++) {
		mes->addNewData((unsigned char)input[i]);
		//qDebug() << QString("%1 (0x%2 = %3)").arg(input[i]).arg(QString::number(input[1],16)).arg(QString::number(input[1],10)).toAscii();
    }
}


void Widget::on_pushButton_Start_clicked()	{//Messung starten/stoppen
	//Überprüfung ob Messung läuft
	if(!(mes->currentIsRunning()))	{

		// Messung starten

		/*
		//Überprüfung ob empfangene Daten gespeichert werden sollen
		QString name="";
		if(ui->checkBox_File->isChecked())	{
			//Dateinamen erfragen
			name=QFileDialog::getSaveFileName(this,tr("Empfangene Werte abspeichern"),QDir::currentPath ());
		}
		*/

		//Messungen starten
		if(ui->checkBox_Temp->isChecked())    {
			// Temperatur messen
			mes->startTempMeasurement();
		}
		mes->startCurrentMeasurement();

		//Aufschrift des Buttons ändern
		ui->pushButton_Start->setText("Messung anhalten");

		// CheckBox für Datei deaktivieren
		//ui->checkBox_File->setCheckable(false);

	}
    else	{
		//Messung anhalten
		mes->stopTempMeasurement();
		mes->stopCurrentMeasurement();

		//Aufschrift ändern
		ui->pushButton_Start->setText("Messung starten");

		// Checkbox für Datei aktivieren
		//ui->checkBox_File->setCheckable(true);
	}
}


void Widget::displayValues(double adjustedValue, double rawValue)
{
	//qDebug() << "display new value";
    ui->lineEdit_ActualValue->setText(QString::number(adjustedValue, 'f', 6));
    char str[50];
    sprintf(str,"%1.9lf", rawValue/(1<<30)); // todo: localise this string!
    ui->lineEdit_AbsoluteValue->setText(str);
    ui->drawingarea->newValue(adjustedValue);
	ui->drawingarea->update();
}

void Widget::displayTemperature(int sensorID, double temperature)
{
    QLineEdit* le=0;
    switch(sensorID){
    case 1: le=ui->lineEdit_T1; break;
    case 2: le=ui->lineEdit_T2; break;
    case 3: le=ui->lineEdit_T3; break;
    case 4: le=ui->lineEdit_T4; break;
    }
    if(le){
        le->setText(QString::number(temperature, 'g', 5));
    }
}

void Widget::on_checkBox_Temp_stateChanged(int arg1)
{
	if(ui->checkBox_Temp->isChecked() && mes->currentIsRunning())
	{
		mes->startTempMeasurement();
	}

	if(!ui->checkBox_Temp->isChecked())
	{
		mes->stopTempMeasurement();
	}
}


void Widget::on_pushButton_tare_clicked()
{
	mes->tare();
}


void Widget::on_pushButton_adjust_clicked()
{
	mes->adjust(ui->lineEdit_kalib_weight->text().toDouble());
}


void Widget::on_pushButton_copyAdjusted_clicked()
{
    QApplication::clipboard()->setText(ui->lineEdit_ActualValue->text());
}

void Widget::on_pushButton_copyAbs_clicked()
{
    QApplication::clipboard()->setText(ui->lineEdit_AbsoluteValue->text());
}

void Widget::on_pushButton_settings_clicked()
{
    settings.show();
}
