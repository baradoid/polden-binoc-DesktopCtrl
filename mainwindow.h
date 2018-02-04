#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QtCore/QTimer>
#include <QtCharts/QChart>
#include <QTime>

#include <QUdpSocket>
#include <QNetworkDatagram>

namespace Ui {
class MainWindow;
}

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QLineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_audioOn_clicked();
    void on_audioOff_clicked();
    void on_pushButton_refreshCom_clicked();
    void on_pushButtonComOpen_clicked();
    void handleReadyRead();
    void handleTimeout();
    void uiUpdate();
    void on_pushButtonPwrOn_clicked();

    void on_pushButtonPwrOff_clicked();

    void on_pushButtonFanOn_clicked();

    void on_pushButtonFanOff_clicked();

    void on_pushButtonUsbPwrOn_clicked();

    void on_pushButtonUsbPwrOff_clicked();
    void readPendingDatagrams();

signals:
    void showStatusBarMessage(const QString &message, int timeout = 0);


private:
    Ui::MainWindow *ui;
    QSerialPort serial;

    QTimer m_timer, uiUpdateTimer;
    QChart *chart;
    QLineSeries *m_series;
    QStringList m_titles;
    QValueAxis *m_axis;
    qreal m_step;
    qreal m_x;
    qreal m_y;
    qreal max;
    void processStr(QString str);
    QString recvStr;
    void appendPosToGraph(int pt);
    QTime startRecvTime;
    int recvdComPacks;
    int comPacksPerSec;

    int bytesRecvd;
    int bytesSec;
    int lastByteSecFixTime;

    QUdpSocket *udpSocket;
};

#endif // MAINWINDOW_H
