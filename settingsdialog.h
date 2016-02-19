#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    bool writeAdcSamples();
    bool writeFilteredSamples();
    bool writeFilteredValues();
    bool writeRawSerial();

private:
    void updateSampleRatePreview();
    
private slots:
    void on_pushButton_path_clicked();

    void on_comboBox_downsampleFactor_currentIndexChanged(const QString &arg1);

    void on_doubleSpinBox_inputSampleRate_valueChanged(double arg1);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
