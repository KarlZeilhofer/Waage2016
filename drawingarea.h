#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include <QList>
#include <QPolygon>


class DrawingArea : public QWidget
{
    Q_OBJECT
public:
    explicit DrawingArea(QWidget *parent = 0);

	void newValue(double value);

private:
	void paintEvent(QPaintEvent* event);

	QList<double>* values;
	double maxValue, minValue, delta; // delta = maxValue-minValue
	double stdDev, mean; // 

	qreal findBeautifulNumber(qreal a, qreal b);
	QString getNumberString(double v) ;


signals:
    
public slots:
    
};

#endif // DRAWINGAREA_H
