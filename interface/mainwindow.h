#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>

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

  private:
    QByteArray bytes {};
    QTimer *timer {};
    QSerialPort *serial;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
