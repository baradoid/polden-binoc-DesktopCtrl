#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this, SIGNAL(showStatusBarMessage(QString,int)),
            ui->statusBar, SLOT(showMessage(QString,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_audioOn_clicked()
{

}

void MainWindow::on_audioOff_clicked()
{

}

void MainWindow::on_pushButton_refreshCom_clicked()
{
    ui->comComboBox->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    const QString blankString = QObject::tr("N/A");
      QString description;
      QString manufacturer;
      QString serialNumber;
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
           description = serialPortInfo.description();
           manufacturer = serialPortInfo.manufacturer();
           serialNumber = serialPortInfo.serialNumber();
           qDebug() << endl
               << QObject::tr("Port: ") << serialPortInfo.portName() << endl
               << QObject::tr("Location: ") << serialPortInfo.systemLocation() << endl
               << QObject::tr("Description: ") << (!description.isEmpty() ? description : blankString) << endl
               << QObject::tr("Manufacturer: ") << (!manufacturer.isEmpty() ? manufacturer : blankString) << endl
               << QObject::tr("Serial number: ") << (!serialNumber.isEmpty() ? serialNumber : blankString) << endl
               << QObject::tr("Vendor Identifier: ") << (serialPortInfo.hasVendorIdentifier() ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16) : blankString) << endl
               << QObject::tr("Product Identifier: ") << (serialPortInfo.hasProductIdentifier() ? QByteArray::number(serialPortInfo.productIdentifier(), 16) : blankString) << endl
               << QObject::tr("Busy: ") << (serialPortInfo.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) << endl;
           ui->comComboBox->addItem(serialPortInfo.portName());
    }

}

void MainWindow::on_pushButtonComOpen_clicked()
{
    serial.setBaudRate(115200);
     if(ui->pushButtonComOpen->text() == "open"){
         if(serial.isOpen() == false){
             QString comName = ui->comComboBox->currentText();
             if(comName.length() > 0){
                 //UartThread.requestToStart(comName);
                 serial.setPortName(comName);
                 if (!serial.open(QIODevice::ReadWrite)) {
                     qDebug("%s port open FAIL", qUtf8Printable(comName));
                     return;
                 }
                 qDebug("%s port opened", qUtf8Printable(comName));
                 connect(&serial, SIGNAL(readyRead()),
                         this, SLOT(handleReadyRead()));
//                 connect(&serial, SIGNAL(bytesWritten(qint64)),
//                         this, SLOT(handleSerialDataWritten(qint64)));
                 ui->pushButtonComOpen->setText("close");
                 //emit showStatusBarMessage("connected", 3000);
                 ui->statusBar->showMessage("connected", 3000);
             }
         }
     }
     else{
         serial.close();
         //udpSocket->close();
         qDebug("port closed");
         ui->pushButtonComOpen->setText("open");
         //contrStringQueue.clear();
     }
}

void MainWindow::handleReadyRead()
{
    QByteArray str = serial.readAll();
}
