#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <qserialport.h>

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
    void handleError( QSerialPort::SerialPortError error );

  private:
    QByteArray bytes {};
    QSerialPort *serial;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
