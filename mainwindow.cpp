#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_series(0),
    m_axis(new QValueAxis),
    m_step(0),
    m_x(0),
    m_y(1)
{
    ui->setupUi(this);
    connect(this, SIGNAL(showStatusBarMessage(QString,int)),
            ui->statusBar, SLOT(showMessage(QString,int)));

    chart = new QChart();
    m_series = new QLineSeries(this);
    QPen green(Qt::red);
    green.setWidth(1);
    m_series->setPen(green);
    m_series->append(m_x, m_y);

    chart->addSeries(m_series);
    chart->createDefaultAxes();
    chart->setAxisX(m_axis, m_series);
    m_axis->setTickCount(10);
    //chart->axisX()->setRange(-100, 5);
    chart->axisX()->setRange(0, 8191);
    chart->axisY()->setRange(-10, 10000);

    chart->setTitle("xPos yPos");
    //chart->setAnimationOptions(QChart::AllAnimations);
    //chart->legend()->hide();


    QChartView chartView(chart);
    chartView.setRenderHint(QPainter::Antialiasing);
    //ui->widgetChart->
    //ui->widgetChart->set
    ui->widgetChartView->setChart(chart);
    ui->widgetChartView->setRenderHint(QPainter::Antialiasing);

    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    m_timer.setInterval(10);
    m_timer.start();

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
                 ui->statusBar->showMessage("connected", 2000);
             }
         }
     }
     else{
         serial.close();
         //udpSocket->close();
         qDebug("port closed");
         ui->pushButtonComOpen->setText("open");
         //contrStringQueue.clear();
         ui->statusBar->showMessage("disconnected", 2000);
     }
}

void MainWindow::handleReadyRead()
{
    QByteArray str = serial.readAll();
}

void MainWindow::handleTimeout()
{
    qreal x = chart->plotArea().width() / m_axis->tickCount();
    m_x +=  1; //(m_axis->max() - m_axis->min()) / m_axis->tickCount();
    m_y +=10;//qrand() % 8191 - 2.5;
    if(m_y > 8191)
        m_y = 0;
    //if(m_x > 1000){

      //  chart->scroll(1, 0);
    //}
    //else{
    qreal lowXrange = ((m_x-500) < 0)? 0 : (m_x-500);
        chart->axisX()->setRange(lowXrange, m_x+100);
        qDebug("m_x %.1f, lowXrange %.1f", m_x, lowXrange);
    //}
    m_series->append(m_x, m_y);
    //chart->scroll(x/10, 0);
    //ui->widgetChartView->repaint();


    //if (m_x == 100)
    //    m_timer.stop();
}
