#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

namespace Ui {
class MainWindow;
}

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
signals:
    void showStatusBarMessage(const QString &message, int timeout = 0);


private:
    Ui::MainWindow *ui;
    QSerialPort serial;
};

#endif // MAINWINDOW_H
