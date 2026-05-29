#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPixmap>
#include <qlogging.h>
#include <qobject.h>
#include <qstringview.h>

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ), ui( new Ui::MainWindow )
{
    ui->setupUi( this );
    serial = new QSerialPort( this );
    connect( serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData );
    connect( serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError );
    connect( serial, &QSerialPort::aboutToClose, this, &MainWindow::handleClose );

    handleClose();
}

void MainWindow::handleScan()
{
    QList<QSerialPortInfo> currentPorts { QSerialPortInfo::availablePorts() };
    for ( const auto &p : currentPorts )
    {
        if ( p.productIdentifier() == 1984 )
        {
            if ( timer )
            {
                delete timer;
                timer = 0;
            }
            serial->setPort( p );
            serial->setBaudRate( QSerialPort::Baud115200 );
            serial->open( QIODevice::ReadWrite );
            serial->write( "s" );
            break;
        }
    }
}

void MainWindow::readSerialData()
{
    auto rD { serial->readAll() };
    if ( bytes.isEmpty() && rD.indexOf( "bgnn" ) != 0 )
        return;
    else
    {
        if ( bytes.isEmpty() )
        {
            offset.first = *reinterpret_cast<uint16_t *>( &rD[ 4 ] );
            auto H { *reinterpret_cast<uint16_t *>( &rD[ 6 ] ) };
            zoomed        = H & 0x8000;
            offset.second = H & 0x7FFF;
            updateOffsetLineEdits();
        }
        bytes.append( rD );
        if ( bytes.indexOf( "endd" ) == -1 )
            return;
        bytes.remove( 0, 8 );
        bytes.remove( bytes.size() - 5, 4 );
        QImage image( reinterpret_cast<uint8_t *>( bytes.data() ), 100, 48, QImage::Format_RGB16 );
        ui->label->setPixmap( QPixmap::fromImage( image ).scaled( ui->label->width(), ui->label->height(), Qt::KeepAspectRatio ) );
        bytes.clear();
    }
}

void MainWindow::handleError( QSerialPort::SerialPortError error )
{
    if ( error == QSerialPort::SerialPortError::ResourceError && serial->isOpen() )
        serial->close();
}

void MainWindow::handleClose()
{
    ui->label->clear();
    bytes.clear();
    timer = new QTimer( this );
    connect( timer, &QTimer::timeout, this, &MainWindow::handleScan );
    timer->start( 100 );
}

// void MainWindow::handleError( QSerialPort::SerialPortError error )
// {
//     qDebug() << "ERROR!!!\n";
// }

MainWindow::~MainWindow()
{
    delete timer;
    delete serial;
    delete ui;
}

void MainWindow::on_upPushButton_clicked()
{
    zoomed = 1;
    if ( offset.second > 10 )
    {
        offset.second -= 10;
        char data[ 3 ] { 'y', 0, 0 };
        ( *reinterpret_cast<uint16_t *>( &data[ 1 ] ) ) = offset.second;

        serial->write( data, 3 );
    }
}

void MainWindow::on_downPushButton_clicked()
{
    zoomed = 1;
    if ( offset.second < 1200 - 48 - 10 )
    {
        offset.second += 10;
        char data[ 3 ] { 'y', 0, 0 };
        ( *reinterpret_cast<uint16_t *>( &data[ 1 ] ) ) = offset.second;

        serial->write( data, 3 );
    }
}

void MainWindow::on_leftPushButton_clicked()
{
    zoomed = 1;
    if ( offset.first > 10 )
    {
        offset.first -= 10;
        char data[ 3 ] { 'x', 0, 0 };
        ( *reinterpret_cast<uint16_t *>( &data[ 1 ] ) ) = offset.first;

        serial->write( data, 3 );
    }
}

void MainWindow::on_rightPushButton_clicked()
{
    zoomed = 1;
    if ( offset.first < 1600 - 100 - 10 )
    {
        offset.first += 10;
        char data[ 3 ] { 'x', 0, 0 };
        ( *reinterpret_cast<uint16_t *>( &data[ 1 ] ) ) = offset.first;

        serial->write( data, 3 );
    }
}

void MainWindow::on_zoomedPushButton_clicked()
{
    zoomed = 0;
    serial->write( "f" );
    offset.first  = 0;
    offset.second = 0;
    updateOffsetLineEdits();
}

void MainWindow::on_xOffsetLineEdit_editingFinished()
{
    offset.first = ui->xOffsetLineEdit->text().toUInt();
    char data[ 3 ] { 'x', 0, 0 };
    ( *reinterpret_cast<uint16_t *>( &data[ 1 ] ) ) = offset.first;

    serial->write( data, 3 );
}

void MainWindow::on_yOffsetLineEdit_editingFinished()
{
    offset.second = ui->yOffsetLineEdit->text().toUInt();
    char data[ 3 ] { 'y', 0, 0 };
    ( *reinterpret_cast<uint16_t *>( &data[ 1 ] ) ) = offset.second;

    serial->write( data, 3 );
}

void MainWindow::updateOffsetLineEdits()
{
    if ( !ui->xOffsetLineEdit->hasFocus() )
        ui->xOffsetLineEdit->setText( QString::number( offset.first ) );
    if ( !ui->yOffsetLineEdit->hasFocus() )
        ui->yOffsetLineEdit->setText( QString::number( offset.second ) );
}

void MainWindow::on_shootButton_clicked()
{
    serial->write( "s" );
}
