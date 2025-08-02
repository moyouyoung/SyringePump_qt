#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPortInfo>
#include <QSerialPort>
#include <QComboBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //slot for the send button
    void on_pushButton_send_clicked();
    void on_pushButton_connect_clicked();
    void on_pushButton_disconnect_clicked();
    void on_pushButton_inject_clicked();
    void on_pushButton_stop_clicked();
    void readSerialData();


private:
    void populatePortsComboBox();
    void addToLog(const QString &message, bool isReceived = false);
    Ui::MainWindow *ui;
    // Add a QSerialPort member variable
    QSerialPort *m_serialPort;
};
#endif // MAINWINDOW_H
