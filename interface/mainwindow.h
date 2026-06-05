#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPair>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow( QWidget *parent = nullptr );
    ~MainWindow();

  private slots:
    void readSerialData();
    void handleScan();
    void handleError( QSerialPort::SerialPortError error );
    void handleClose();

    void on_leftPushButton_clicked();
    void on_rightPushButton_clicked();
    void on_upPushButton_clicked();
    void on_downPushButton_clicked();

    // void on_zoomedPushButton_clicked();

    void on_xOffsetLineEdit_editingFinished();

    void on_yOffsetLineEdit_editingFinished();

    void on_shootButton_clicked();

    void on_cameraCheckBox_clicked();

    void on_aecLineEdit_editingFinished();

  private:
    void updateProperties();
    QPair<uint16_t, uint16_t> offset { 0, 0 };
    uint16_t aec { 0 };
    uint8_t luminance { 0 };
    bool zoomed { 0 };
    QByteArray bytes {};
    QTimer *timer {};
    QSerialPort *serial;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
