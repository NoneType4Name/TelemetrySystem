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
    serial->setPortName( "COM6" );
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
    bytes.remove( 0, 3 );
    bytes.remove( bytes.size() - 4, 3 );
    auto d = bytes.size();
    QImage image( reinterpret_cast<uint8_t *>( bytes.data() ), 300, 240, QImage::Format_RGB16 );
    ui->label->setPixmap( QPixmap::fromImage( image ) );
    //ui->label->setScaledContents(true);
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
