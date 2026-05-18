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
    connect( serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData );
    connect( serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError );
    connect( serial, &QSerialPort::aboutToClose, this, &MainWindow::handleClose );

    handleClose();
}

void MainWindow::handleScan()
{
    QList<QSerialPortInfo> currentPorts { QSerialPortInfo::availablePorts()};
    for(const auto&p:currentPorts)
    {
        if(p.productIdentifier() == 1984)
        {
            if(timer)
            {
                delete timer;
                timer = 0;
            }
            serial->setPort(p);
            serial->setBaudRate(QSerialPort::Baud115200);
            serial->open( QIODevice::ReadOnly );
            break;
        }
    }
}

void MainWindow::readSerialData()
{
    auto rD { serial->readAll() };
    if ( bytes.isEmpty() && rD.indexOf( "bgnn" ) != 0 )
        return;

    bytes.append( rD );
    if ( bytes.indexOf( "endd" ) == -1 )
        return;
    bytes.remove( 0, 4 );
    bytes.remove( bytes.size() - 5, 4 );
    QImage image( reinterpret_cast<uint8_t *>( bytes.data() ), 320, 200, QImage::Format_RGB16 );
    ui->label->setPixmap( QPixmap::fromImage( image ).scaled( ui->label->width(), ui->label->height(), Qt::KeepAspectRatio ) );
    bytes.clear();
}

void MainWindow::handleError( QSerialPort::SerialPortError error )
{
    if(error == QSerialPort::SerialPortError::ResourceError && serial->isOpen())
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
