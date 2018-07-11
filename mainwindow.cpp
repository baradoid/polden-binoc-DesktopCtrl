#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
//#include <QtCharts/QChart>
//#include <QtCharts/QChartView>
//#include <QtCharts/QAbstractAxis>
//#include <QtCharts/QSplineSeries>
//#include <QtCharts/QValueAxis>
#include <QTime>
#include <QtEndian>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    //m_series(0),
    //m_axis(new QValueAxis),
    m_step(0),
    m_x(0),
    m_y(1),
    max(0),
    recvdComPacks(0),
    comPacksPerSec(0),
    bytesSec(0),
    lastByteSecFixTime(0),
    settings("murinets", "binoControl"),
    udpDgmCount(0)
{
    ui->setupUi(this);
    connect(this, SIGNAL(showStatusBarMessage(QString,int)),
            ui->statusBar, SLOT(showMessage(QString,int)));

    //chart = new QChart();
    //m_series = new QLineSeries(this);
//    QPen green(Qt::red);
//    green.setWidth(1);
//    m_series->setPen(green);
//    m_series->append(m_x, m_y);

//    chart->addSeries(m_series);
//    chart->createDefaultAxes();
//    chart->setAxisX(m_axis, m_series);
//    m_axis->setTickCount(10);
//    //chart->axisX()->setRange(-100, 5);
//    chart->axisX()->setRange(0, 8191);
//    chart->axisY()->setRange(-10, 10000);

//    chart->setTitle("xPos yPos");
    //chart->setAnimationOptions(QChart::AllAnimations);
    //chart->legend()->hide();


    //QChartView chartView(chart);
    //chartView.setRenderHint(QPainter::Antialiasing);
    //ui->widgetChart->
    //ui->widgetChart->set
    //ui->widgetChartView->setChart(chart);
    //ui->widgetChartView->setRenderHint(QPainter::NonCosmeticDefaultPen);

    //QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    //m_timer.setInterval(10);
    //m_timer.start();
    uiUpdateTimer.setInterval(50);
    QObject::connect(&uiUpdateTimer, SIGNAL(timeout()), this, SLOT(uiUpdate()));
    uiUpdateTimer.start();

    udpSocket = new QUdpSocket(this);
    udpSocket->bind();

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));

    on_pushButton_refreshCom_clicked();

//    if(ui->radioButtonConnCom->isChecked()){
//        ui->pushButtonRegistr->setEnabled(false);
//    }

    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    // You may want to use QRegularExpression for new code with Qt 5 (not mandatory).
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    ui->lineEditDstAddr->setValidator(ipValidator);

    ui->lineEditDstPort->setValidator(new QIntValidator(0, 65535, this));



    ui->lineEditDstAddr->setText(settings.value("dst_addr", "127.0.0.1").toString());
    ui->lineEditDstPort->setText(settings.value("dst_port", 8055).toString());
}

MainWindow::~MainWindow()
{
    settings.setValue("dst_addr", ui->lineEditDstAddr->text());
    settings.setValue("dst_port", ui->lineEditDstPort->text().toInt());

    delete ui;
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
                 recvdComPacks = 0;
                 startRecvTime = QTime::currentTime();
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

void MainWindow::processStr(QString str)
{
    recvdComPacks++;
    //qDebug() << str.length() <<":" <<str;
    if(str.length() != 41){
        str.remove("\r\n");
        qDebug() << "string length " << str.length() << "not equal 41" << qPrintable(str);
    }
    QStringList strList = str.split(" ");
    if(strList.size() > 2){
        int xPos1 = strList[0].toInt(Q_NULLPTR, 16);
        int xPos2 = strList[1].toInt(Q_NULLPTR, 16);
        float temp = strList[2].toInt(Q_NULLPTR, 10)/10.;
        int dist = strList[3].toInt(Q_NULLPTR, 10);
        ui->lineEditEnc1->setText(QString::number(xPos1));
        ui->lineEditEnc2->setText(QString::number(xPos2));
        ui->lineEditRange->setText(QString::number(dist));
        ui->lineEditTerm1->setText(QString::number(temp));
        //qDebug() << xPos1 << xPos2;
    }

    //appendPosToGraph(xPos);

}

void MainWindow::handleReadyRead()
{    
    QByteArray ba = serial.readAll();

    bytesRecvd += ba.length();

    for(int i=0; i<ba.length(); i++){
        recvStr.append((char)ba[i]);
        if(ba[i]=='\n'){
            processStr(recvStr);
            recvStr.clear();
        }
    }

}

//void MainWindow::appendPosToGraph(int pt)
//{
//    qreal x = chart->plotArea().width() / m_axis->tickCount();
//    int msecs = QTime::currentTime().msecsSinceStartOfDay()/1000;
//    m_x =  startRecvTime.elapsed()/10; //(m_axis->max() - m_axis->min()) / m_axis->tickCount();
//    m_y = pt;//qrand() % 8191 - 2.5;

//    //if(m_x > 1000){

//      //  chart->scroll(1, 0);
//    //}
//    //else{
//    qreal min=8192, max=0;
//    for(int i=0; i<m_series->count(); i++){
//        qreal pt = m_series->at(i).y();
//        if(pt > max)
//            max = pt;
//        if(pt < min)
//            min = pt;

//    }

//    qreal lowXrange = ((m_x-500) < 0)? 0 : (m_x-500);
//        chart->axisX()->setRange(lowXrange, m_x+100);
//        if(pt > max){
//            max = pt;

//        }

//       // chart->axisY()->setRange(min, max);
//        //qDebug("m_x %.1f, lowXrange %.1f", m_x, lowXrange);
//    //}


//    //m_series->append(m_x, m_y);
//    if(m_series->count() > 1000){
//        m_series->removePoints(0, 100);
//    }
//}


//void MainWindow::handleTimeout()
//{
//    qreal x = chart->plotArea().width() / m_axis->tickCount();
//    m_x +=  1; //(m_axis->max() - m_axis->min()) / m_axis->tickCount();
//    m_y +=10;//qrand() % 8191 - 2.5;
//    if(m_y > 8191)
//        m_y = 0;
//    //if(m_x > 1000){

//      //  chart->scroll(1, 0);
//    //}
//    //else{
//    qreal lowXrange = ((m_x-50) < 0)? 0 : (m_x-50);
//        chart->axisX()->setRange(lowXrange, m_x+100);
//        qDebug("m_x %.1f, lowXrange %.1f", m_x, lowXrange);
//    //}
//    m_series->append(m_x, m_y);
//    //chart->scroll(x/10, 0);
//    //ui->widgetChartView->repaint();


//    //if (m_x == 100)
//    //    m_timer.stop();
//}

void MainWindow::uiUpdate()
{
    QString msg = QString("packs/sec %1, bytes/sec %2").arg(comPacksPerSec).arg(bytesSec);
    ui->statusBar->showMessage(msg, 2000);
    //recvdComPacks = 0;
    int msecsSinceSod = QTime::currentTime().msecsSinceStartOfDay();
    if((msecsSinceSod - lastByteSecFixTime) >= 1000){
        lastByteSecFixTime = msecsSinceSod;
        bytesSec = bytesRecvd;
        bytesRecvd = 0;
        comPacksPerSec = recvdComPacks;
        recvdComPacks = 0;
    }
    ui->lineEditPackCount->setText(QString::number(udpDgmCount));
}

void MainWindow::on_audioOn_clicked()
{        
    sendCmd("audioOn\n");
}

void MainWindow::on_audioOff_clicked()
{
    sendCmd("audioOff\n");
}

void MainWindow::on_pushButtonPwrOn_clicked()
{
    sendCmd("pwrOn\n");
}

void MainWindow::on_pushButtonPwrOff_clicked()
{
    sendCmd("pwrOff\n");
}

void MainWindow::on_pushButtonFanOn_clicked()
{
    sendCmd("fanOn\n");
}

void MainWindow::on_pushButtonFanOff_clicked()
{
    sendCmd("fanOff\n");
}

void MainWindow::on_pushButtonUsbPwrOn_clicked()
{    
    sendCmd("usbOn\n");
}

void MainWindow::on_pushButtonUsbPwrOff_clicked()
{
    sendCmd("usbOff\n");
}

void MainWindow::on_pushButtonHeatOn_clicked()
{
    sendCmd("heatOn\n");
}

void MainWindow::on_pushButtonHeatOff_clicked()
{
    sendCmd("heatOff\n");
}

#pragma pack(push,1)
typedef struct{
    int16_t pos1;
    int16_t pos2;
    int16_t distance;
//    int8_t headTemp;
//    int8_t batteryTemp;
//    int32_t cashCount;
} CbDataUdp;
#pragma pack(pop)

void MainWindow::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        //qDebug() << datagram.data();
        //processStr(QString(datagram.data()));
        if(datagram.data().length() != sizeof(CbDataUdp)){
            if(datagram.data() == "hb"){
                udpSocket->writeDatagram("hba", datagram.senderAddress(),
                                        datagram.senderPort());

            }
            else{
                qWarning("udp datagram len %d less than expected data stuct %d: %s", datagram.data().length(), sizeof(CbDataUdp), (char*)datagram.data().constData());
            }
            continue;
        }
        else{
            QByteArray ba = datagram.data();
            CbDataUdp* cbData = (CbDataUdp*)datagram.data().constData();
            ui->lineEditEnc1->setText(QString::number(cbData->pos1));
            ui->lineEditEnc2->setText(QString::number(cbData->pos2));
            ui->lineEditRange->setText(QString::number(cbData->distance));
            //ui->lineEditTerm1->setText(QString::number(cbData->headTemp));
        }
        udpDgmCount++;
    }
}


void MainWindow::on_pushButtonRegistr_clicked()
{    
    sendUdpCmd("reg");
}


void MainWindow::sendCmd(const char* s)
{
    if(ui->radioButtonConnCom->isChecked()){
        if(serial.isOpen()){
            qInfo("sended %d", serial.write(s));
        }
    }
    else if(ui->radioButtonConnUdp->isChecked()){
        sendUdpCmd(s);
    } //"192.168.43.1"
}

void MainWindow::on_radioButtonConnCom_clicked()
{
    //ui->pushButtonRegistr->setEnabled(false);
}

void MainWindow::on_radioButtonConnUdp_clicked()
{
    //ui->pushButtonRegistr->setEnabled(true);
}

void MainWindow::sendUdpCmd(const char* s)
{
    QString ipStr = ui->lineEditDstAddr->text();
    QString portStr = ui->lineEditDstPort->text();
    udpSocket->writeDatagram(s, QHostAddress(ipStr), portStr.toInt());
}
