#include "drawingarea.h"
#include <QPainter>
#include <QDebug>
#include <float.h>
#include <cmath>


#define VIS_POINTS 50

DrawingArea::DrawingArea(QWidget *parent) :
    QWidget(parent)
{
	values=new QList<double>;
	
}


void DrawingArea::newValue(double value)
{
	//qDebug() << "value:" << value;
	values->append(value);
	if(values->length()>width())
	{
		values->removeFirst();
	}

	// find limits for vertical axis

	maxValue=values->last();
	minValue=values->last();

	// make sure that the last VIS_POINTS values are visible

	mean =0;
	for(int i=0; i<values->length() && i<VIS_POINTS; i++)
	{
		double val=values->at(values->length()-1 - i);
		mean += val/VIS_POINTS;

		if(val>maxValue)
		{
			maxValue=val;
		}
		if(val<minValue)
		{
			minValue=val;
		}
	}
	
	// calculate the standard deviation
	stdDev=0;
	for(int i=0; i<values->length() && i<VIS_POINTS; i++)
	{
		double val=values->at(values->length()-1 - i);
		stdDev += (val-mean)*(val-mean);
	}
	stdDev /= (VIS_POINTS+1);
	stdDev = sqrt(stdDev);


	delta = maxValue-minValue; // nur vorläufig
	maxValue+=3.5*delta;
	minValue-=3.5*delta;
	delta = maxValue-minValue;

//	double diff = maxValue-mean;
//	maxValue+=diff;
//	minValue+=diff;
}


void DrawingArea::paintEvent(QPaintEvent* event)
{
	QPainter p(this);

	p.setBrush(Qt::white);

	p.drawRect(0, 0, width()-1, height()-1);

	QPolygon line;
	double scale = height()/delta;

//	qDebug() << "minValue" << minValue;
//	qDebug() << "maxValue" << maxValue;
//	qDebug() << "delta" << delta;

	for(int i=0; i<values->length(); i++)
	{
		// transform value to screen coordinate
		int y=height()-((values->at(i)-minValue)*scale);
		line.append(QPoint(i, y));
	}

    QPen pen(Qt::black);
    if(!line.isEmpty())
    {
        pen.setWidth(2);
        p.setPen(pen);

        p.drawPolyline(line);

        // highlight the last point
        pen = QPen(Qt::red);
        pen.setWidth(5);
        p.setPen(pen);
        p.drawPoint(line.last());
    }

	double scaleBar = findBeautifulNumber(delta/3, delta*0.8); // length in gramms

	// draw white background for text
	p.setPen(Qt::white);
	p.drawRect(30, height()/10+15, 50, 20);
	p.drawRect(30, height()/10+30, 80, 20);

	QPen pen2(Qt::blue);
	pen2.setWidth(5);
	p.setPen(pen2);
	p.drawText(30, height()/10+15, 50, 20, 0, getNumberString(scaleBar)+"g");
	p.drawLine(20, height()/10, 20, height()/10+scaleBar*scale);

	pen2.setWidth(5);
	p.setPen(pen2);
	p.drawText(30, height()/10+30, 80, 20, 0, getNumberString(stdDev)+"g");
}


/*
 * find most beautiful number in the interval [a;b]
 */
qreal DrawingArea::findBeautifulNumber(qreal a, qreal b) 
{
	bool swapSign=false;

	if(a>b){
		qreal h=a;
		a=b;
		b=h;
	}

	// swap negative interval to positive
	if(a<0 && b<0){
		swapSign = true;
		qreal h = a;
		a=-b;
		b=-h;
	}

	// limit to positive numbers
	if(a<0){
		a=0;
	}

	// zero is the most beautiful number
	if(a==0 || b == 0){
		return 0;
	}

	if(a==b){
		return a;
	}

	// now is: 0<a<b

	// backup b
	qreal a_ = a;
	qreal b_ = b;

	int n=0;
	int ad=0, bd=0; // current digit of a and b
	int l=0; // floor(log(bigger number));
	while(ad == bd){
		// l = max(floor(log10(a)), floor(log10(b))); // is replaced by:
		l = floor(log10(b)); // because b is always larger than a

		ad = floor(a/pow(10, l));
		a = a - ad*pow(10,l);
		bd = floor(b/pow(10, l));
		b = b - bd*pow(10,l);

		n++;
	}
	// n is now the index of the first different digit
	// beginning with 1 at the most significand digit of the larger number
	// where ad is the digit at n for a
	// and bd is the digit at n for b
	// ad < bd

	// find the most beautiful digit in the interval [ad, bd]

	int mbd=0; // most beautiful digit
	bool cnf=false; // correct number found
	int k=0;

	while(!cnf && k<2){
		k++;
		// ranking of the beauty of digits :-)
		if      (ad<=0 && bd>=0) mbd=0;
		else if (ad<=2 && bd>=2) mbd=2;
		else if (ad<=5 && bd>=5) mbd=5;
		else if (ad<=1 && bd>=1) mbd=1;
		else if (ad<=4 && bd>=4) mbd=4;
		else if (ad<=6 && bd>=6) mbd=6;
		else if (ad<=8 && bd>=8) mbd=8;
		else if (ad<=3 && bd>=3) mbd=3;
		else if (ad<=7 && bd>=7) mbd=7;
		else mbd=9;

		b = b_; // restore b
		l = floor(log10(b));
		b = floor(b/pow(10,l-(n-1)+1));
		b = b * pow(10,l-(n-1)+1) + mbd*pow(10,l-(n-1)+1 -1);

		if(b>=a_ && b<=b_){
			cnf = true;
		}else{
			ad++; // increase ad once
		}
	};

	if(b<a_)
		qDebug()<<"Found number is smaller than the bottom limit";
	if(b>b_)
		qDebug()<<"Found number is bigger than the top limit";

	if(swapSign){
		return -b;
	}else{
		return b;
	}
}


// returns a String of the number v in enineering format with the postfixes for 10^-12 to 10^12
QString DrawingArea::getNumberString(double v) {
	double mantVal = v; // ATTENTION: abs() in <cmath> is for integers! has an overrun at 2^31
				 // use fabs() for doubles!
	double mantSign;

	if (v < 0) {
		mantVal = v * (-1);
		mantSign = -1;
	} else
		mantSign = 1;

	// first calculate scientific format
	// then, if necessary convert to other number-formats
	int expVal;
	int expSign;

	double lgMant = log10(mantVal);
	if (lgMant < 0) {
		expVal = -floor(lgMant); // corretion of value due to integer
		//rounding error of lgMant for negative values
		expSign = -1;
	} else {
		expVal = floor(lgMant);
		expSign = 1;
	}
	mantVal = mantVal * pow(10, (int) -(expVal * expSign));

	// TODO throw ERROR on exponent larger than 99

	if (expSign < 0) {
		if (expVal % 3 == 0) {
			// do nothing
		} else if (expVal % 3 == 1) {
			expVal = expVal + 2;
			mantVal = mantVal * 100;
		} else if (expVal % 3 == 2) {
			expVal = expVal + 1;
			mantVal = mantVal * 10;
		}
	} else {
		int expCorrection = expVal % 3;
		expVal = expVal - expCorrection;
		mantVal = mantVal * pow(10, expCorrection);
	}

#define LCD_MAX_DIGITS 4
	int numDigits = LCD_MAX_DIGITS;
	int digitsBD = (int) (log10(mantVal)) + 1; // number of significant digits before decimal point
	if (digitsBD < 1)
		digitsBD = 1; // zero is always displayed before decimal point.

	int dotIndex = 0;
	dotIndex = numDigits - digitsBD;
	// remove trailing zeros of mantissa
	if (dotIndex > 0) {
		QString str(QString::number(mantVal, 'f', dotIndex));
		int n = str.length() - 1; // -1 because startindex=0
		while (n>0 && str.at(n)=='0' && str.at(n)!='.')
			n--;
		n=str.length()-1-n; // -1 because of the '.'-character
		dotIndex=dotIndex-n;
		numDigits=numDigits-n;
	}

	QString ret;
	QString ending;
	switch((expVal*expSign)){
		case 12: ending = "T"; break;
		case 9: ending = "G"; break;
		case 6: ending = "M"; break;
		case 3: ending = "k"; break;
		case 0: ending = ""; break;
		case -3: ending = "m"; break;
		case -6: ending = "u"; break;
		case -9: ending = "n"; break;
		case -12: ending = "p"; break;
	}

	return  QString::number(mantVal*mantSign, 'g', 3)+ending;
}


