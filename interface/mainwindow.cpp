#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPixmap>
#include <qlogging.h>
#include <qstringview.h>

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ), ui( new Ui::MainWindow )
{
    ui->setupUi( this );
    serial = new QSerialPort( this );
    serial->setPortName( "COM3" );
    serial->setBaudRate( QSerialPort::Baud115200 );

    if ( serial->open( QIODevice::ReadOnly ) )
    {
        connect( serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData );
        connect( serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError );
        // connect( serial, &QSerialPort::aboutToClose, this, &MainWindow::handleError );
    }
}

void MainWindow::readSerialData()
{
    auto rD { serial->readAll() };
    if ( bytes.isEmpty() && rD.indexOf( QByteArray::fromHex( "FFD8" ) ) != 0 )
        return;

    bytes.append( rD );
    if ( bytes.indexOf( QByteArray::fromHex( "FFD9" ) ) == -1 )
        return;
    QPixmap pm;
    pm.loadFromData( bytes, "JPG" );
    ui->label->setPixmap( pm );
    bytes.clear();
}
void MainWindow::handleError( QSerialPort::SerialPortError error )
{
    qDebug() << "ERROR!!!\t" << error << '\n';
}

// void MainWindow::handleError( QSerialPort::SerialPortError error )
// {
//     qDebug() << "ERROR!!!\n";
// }

MainWindow::~MainWindow()
{
    delete ui;
}
