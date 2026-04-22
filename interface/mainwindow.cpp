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
    if ( bytes.isEmpty() && rD.indexOf( "bgn" ) != 0 )
        return;

    bytes.append( rD );
    if ( bytes.indexOf( "end" ) == -1 )
        return;
    bytes.erase( "bgn" );
    bytes.erase( "end" );
    QPixmap pm;
    // for ( uint32_t i = 0; i < 320 *; i++ )
    // {
    QImage img( reinterpret_cast<uint8_t *>( bytes.data() ), 320, 240, QImage::Format_RGB888 );
    ui->label->setPixmap( pm );
    bytes.clear();
    // }
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
