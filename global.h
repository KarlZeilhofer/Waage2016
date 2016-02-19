#ifndef GLOBAL_H
#define GLOBAL_H

//Einstellungen für die serielle Schnittstelle
#define BAUD_RATE 115200UL //Baudrate

//Kommunikation über die serielle Schnittstelle mit dem Mikroprozessor

//ADS1291 (Strommessung)

//Kennzeichnung der Daten für die Übertragung
#define ADS1281_MARKER 'A'
#define AD7190_MARKER 'B'

//Befehle über die serielle Schnittstelle zum Steuern des ADCs
#define ADS1281_STOP_SIGN 'F'
#define ADS1281_START_SIGN 'S'


// MAX11202 (Temperatursensoren)

//Kennzeichnung der Daten bei der Übertragung
#define TEMP_MARKER_1 'T'
#define TEMP_MARKER_2 't'
#define TEMP_MARKER_3 'k'
#define TEMP_MARKER_4 'K'


#define MAX11202_START	'U'
#define MAX11202_STOP	'u'


//Datei mit den Filterkoeffizienten
#define COEFFS_FILE_NAME "coeffs.txt"


//Faktor für die Abtastratenreduktion (Wie oft soll sie halbiert werden?)
#define DOWN_SAMPLING_FAKTOR 1

#endif // GLOBAL_H
