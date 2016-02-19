#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    updateSampleRatePreview();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::writeRawSerial()
{
    return ui->checkBox_writeRawSerial->isChecked();
}

void SettingsDialog::updateSampleRatePreview()
{
    int factor = ui->comboBox_downsampleFactor->currentText().toInt();
    double inputSR = ui->doubleSpinBox_inputSampleRate->value();

    ui->lineEdit_sampleRatePreview->setText(QString::number(inputSR/factor) + "S/s = " + QString::number(factor/inputSR) + "s/S");
}

bool SettingsDialog::writeAdcSamples()
{
    return ui->checkBox_writeAdcSamples->isChecked();
}

bool SettingsDialog::writeFilteredSamples()
{
    return ui->checkBox_writeFilteredSamples->isChecked();
}

bool SettingsDialog::writeFilteredValues()
{
    return ui->checkBox_writeFilteredValues->isChecked();
}

void SettingsDialog::on_pushButton_path_clicked()
{
    QDir currentDir;
    QString newDirName = QFileDialog::getExistingDirectory(this, tr("Ordner für Logfiles angeben..."), currentDir.absolutePath());
    if(!newDirName.isNull()){
        currentDir.setPath(newDirName);
    }
    ui->lineEdit_path->setText(currentDir.absolutePath());
}

void SettingsDialog::on_comboBox_downsampleFactor_currentIndexChanged(const QString &arg1)
{
    updateSampleRatePreview();
}

void SettingsDialog::on_doubleSpinBox_inputSampleRate_valueChanged(double arg1)
{
    updateSampleRatePreview();
}
