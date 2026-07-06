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
                timer->stop();
                delete timer;
                timer = 0;
            }
            serial->setPort( p );
            serial->setBaudRate( QSerialPort::Baud115200 );
            serial->open( QIODevice::ReadWrite );
            timer                 = new QTimer( this );
            txData.additionalData = 1;
            connect( timer, &QTimer::timeout, this, [ & ]()
                     { serial->write( QByteArray( reinterpret_cast<uint8_t *>( &txData ) ) ); } );
            timer->start( 100 );
            break;
        }
    }
}

void MainWindow::readSerialData()
{
    if ( timer )
    {
        timer->stop();
        delete timer;
        timer = 0;
    }
    auto rD { serial->readAll() };
    if ( bytes.isEmpty() && rD.indexOf( "bgn" ) != 0 )
        return;
    else
    {
        // if ( bytes.isEmpty() )
        // {
        //     offset.first = *reinterpret_cast<uint16_t *>( &rD[ 4 ] );
        //     auto H { *reinterpret_cast<uint16_t *>( &rD[ 6 ] ) };
        //     ui->cameraCheckBox->setChecked( H & ( 1 << 14 ) );
        //     offset.second = H & 0xFFF;
        //     updateProperties();
        // }
        bytes.append( rD );
        auto endingPos { bytes.indexOf( "end" ) };
        if ( endingPos == -1 )
            return;
        const auto *rxData = reinterpret_cast<RxData_T *>( bytes.data() );
        auto p             = rxData->time;
        int seconds        = ( p >> 0 ) & 0x3F;
        int minutes        = ( p >> 6 ) & 0x3F;
        int hours          = ( p >> 12 ) & 0x1F;
        int day            = ( p >> 17 ) & 0x1F;
        int month          = ( p >> 22 ) & 0x0F;
        int year           = ( p >> 26 ) & 0x3F;

        QString timeStr = QString( "%1.%2.20%3 %4:%5:%6" )
                              .arg( day, 2, 10, QChar( '0' ) )
                              .arg( month, 2, 10, QChar( '0' ) )
                              .arg( year, 2, 10, QChar( '0' ) )
                              .arg( hours, 2, 10, QChar( '0' ) )
                              .arg( minutes, 2, 10, QChar( '0' ) )
                              .arg( seconds, 2, 10, QChar( '0' ) );
        ui->timeLabel->setText( timeStr );

        if ( !ui->xOffsetLineEdit->hasFocus() )
            ui->xOffsetLineEdit->setText( QString::number( rxData->x ) );

        if ( !ui->yOffsetLineEdit->hasFocus() )
            ui->yOffsetLineEdit->setText( QString::number( rxData->y ) );

        if ( !ui->aecLineEdit->hasFocus() )
            ui->aecLineEdit->setText( QString::number( rxData->aec ) );

        ui->avgLabel->setText( QString::number( rxData->avgLuminance ) );
        QImage image( reinterpret_cast<const uint8_t *>( rxData->frame ), WIDTH, HEIGHT, QImage::Format_RGB16 );
        ui->label->setPixmap( QPixmap::fromImage( image ).scaled( ui->label->width(), ui->label->height(), Qt::KeepAspectRatio ) );
        bytes.clear();
        serial->write( QByteArray( reinterpret_cast<uint8_t *>( &txData ) ) );
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
    uint8_t y { static_cast<uint8_t>( ui->yOffsetLineEdit->text().toUInt() ) };
    if ( y > 10 )
        y -= 10;
    else
    {
        if ( y == 0 )
            return;
        y = 0;
    }
    txData.command        = TxCommand::newYoffset;
    txData.additionalData = y;
}

void MainWindow::on_downPushButton_clicked()
{
    uint8_t y { static_cast<uint8_t>( ui->yOffsetLineEdit->text().toUInt() ) };
    if ( y < HEIGHT - 10 )
        y += 10;
    else
    {
        if ( y == HEIGHT )
            return;
        y = HEIGHT;
    }
    txData.command        = TxCommand::newYoffset;
    txData.additionalData = y;
}

void MainWindow::on_leftPushButton_clicked()
{
    uint8_t x { static_cast<uint8_t>( ui->xOffsetLineEdit->text().toUInt() ) };
    if ( x > 10 )
        x -= 10;
    else
    {
        if ( x == 0 )
            return;
        x = 0;
    }
    txData.command        = TxCommand::newXoffset;
    txData.additionalData = x;
}

void MainWindow::on_rightPushButton_clicked()
{
    uint8_t x { static_cast<uint8_t>( ui->xOffsetLineEdit->text().toUInt() ) };
    if ( x < WIDTH - 10 )
        x += 10;
    else
    {
        if ( x == WIDTH )
            return;
        x = WIDTH;
    }
    txData.command        = TxCommand::newXoffset;
    txData.additionalData = x;
}

void MainWindow::on_xOffsetLineEdit_editingFinished()
{
    uint8_t x { static_cast<uint8_t>( ui->xOffsetLineEdit->text().toUInt() ) };
    txData.command = TxCommand::newXoffset;
    if ( x > WIDTH )
        x = WIDTH;
    else if ( x < 0 )
        x = 0;
    txData.additionalData = x;
}

void MainWindow::on_yOffsetLineEdit_editingFinished()
{
    uint8_t y { static_cast<uint8_t>( ui->yOffsetLineEdit->text().toUInt() ) };
    txData.command = TxCommand::newYoffset;
    if ( y > HEIGHT )
        y = HEIGHT;
    else if ( y < 0 )
        y = 0;
    txData.additionalData = y;
}

void MainWindow::on_shootButton_clicked()
{
    txData.command        = TxCommand::takeShoot;
    txData.additionalData = 0;
}

void MainWindow::on_cameraCheckBox_clicked()
{
    txData.command        = TxCommand::aimingMode;
    txData.cameraEnabled  = ui->cameraCheckBox->isChecked();
    txData.additionalData = 0;
}

void MainWindow::on_aecLineEdit_editingFinished()
{
    uint16_t aec          = ui->aecLineEdit->text().toUInt();
    txData.command        = TxCommand::newAec;
    txData.additionalData = aec;
}
