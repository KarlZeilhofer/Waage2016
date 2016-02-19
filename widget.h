#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>

#include <QSerialPort>
#include <QSerialPortInfo>

#include "global.h"
#include "measurement.h"
#include <QTimer>
#include <QFileDialog>
#include "filter.h"
#include "settingsdialog.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    void displayValues(double adjustedValue, double rawValue);
    void displayTemperature(int sensorID, double temperature);

private slots:
	void on_pushButton_Connect_clicked();
	void slotSerial();

	void on_pushButton_Start_clicked();


	void on_checkBox_Temp_stateChanged(int arg1);

	void on_pushButton_tare_clicked();

	void on_pushButton_adjust_clicked();

    void on_pushButton_copyAdjusted_clicked();

    void on_pushButton_copyAbs_clicked();

    void on_pushButton_settings_clicked();

private:

    Ui::Widget *ui;
    QSerialPort *port;	//Serielle Schnittstelle
	Measurement *mes;	//Measure-Ojekt, das die empfangenen Daten speichert und verarbeitet

public:
    static Widget* app;
    SettingsDialog settings;

};

#endif // WIDGET_H
