#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QList>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Instantiate the serial port object
    m_serialPort = new QSerialPort(this);

    // Connect the readyRead signal to our slot for receiving data
    connect(m_serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

    //set inital UI state
    ui->pushButton_disconnect->setEnabled(false);
    ui->pushButton_send->setEnabled(false);
    ui->pushButton_inject->setEnabled(false);
    ui->pushButton_stop->setEnabled(false);
    ui->radioButton_inject->setChecked(true);
    //populate the serial select box
    populatePortsComboBox();

    // The connection from the button to the slot is handled automatically
    // by Qt's UI system because the slot is named on_WidgetName_signalName().
}

MainWindow::~MainWindow()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    delete ui;
    // The m_serialPort is a child of MainWindow, so it will be deleted automatically.
}

// Connect to the selected serial port
void MainWindow::on_pushButton_connect_clicked()
{
    // 1. Check if a port is selected
    if (ui->comboBox_ports->currentIndex() == -1) {
        QMessageBox::critical(this, "Error", "No serial port selected.");
        return;
    }

    // 2. Get the port name
    QString portName = ui->comboBox_ports->currentText();

    // 3. Configure the serial port
    m_serialPort->setPortName(portName);
    // m_serialPort->setBaudRate(QSerialPort::Baud9600);
    if (ui->comboBox_port_baudrate->currentIndex() == 0) {
        m_serialPort->setBaudRate(QSerialPort::Baud9600);
    } else if (ui->comboBox_port_baudrate->currentIndex() == 1) {
        m_serialPort->setBaudRate(QSerialPort::Baud19200);
    } else if (ui->comboBox_port_baudrate->currentIndex() == 2){
        m_serialPort->setBaudRate(QSerialPort::Baud38400);
    } else if (ui->comboBox_port_baudrate->currentIndex() == 3){
        m_serialPort->setBaudRate(QSerialPort::Baud57600);
    } else {
        m_serialPort->setBaudRate(QSerialPort::Baud115200);
    }
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    // 4. Connect Serial Port
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        addToLog("Connected to " + portName);
        ui->pushButton_connect->setEnabled(false);
        ui->pushButton_disconnect->setEnabled(true);
        ui->pushButton_send->setEnabled(true);
        ui->comboBox_ports->setEnabled(false);
        ui->comboBox_port_baudrate->setEnabled(false);
        ui->pushButton_inject->setEnabled(true);
        ui->pushButton_stop->setEnabled(true);
    } else {
        QMessageBox::critical(this, "Error", "Failed to open port: " + m_serialPort->errorString());
    }
}

// Disconnect from the serial port
void MainWindow::on_pushButton_disconnect_clicked()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        addToLog("Disconnected");
        ui->pushButton_connect->setEnabled(true);
        ui->pushButton_disconnect->setEnabled(false);
        ui->pushButton_send->setEnabled(false);
        ui->comboBox_ports->setEnabled(true);
        ui->comboBox_port_baudrate->setEnabled(true);
        ui->pushButton_inject->setEnabled(false);
        ui->pushButton_stop->setEnabled(false);
    }
}

void MainWindow::on_pushButton_inject_clicked()
{
    qint16 stepIndex = ui->comboBox_step->currentIndex();
    qint16 direction = 0;
    if (ui->radioButton_retract->isChecked()) {
        direction = 1;
    }
    float distance = ui->lineEdit_distance->text().toFloat();

    const qint16 stepsPerRound = 200; // 200 steps equal to a full 360 turn
    const qreal distancePerRound = 1; // each turn goes 1mm
    qreal stepsPerMille = distancePerRound/(pow(2,4-stepIndex) * stepsPerRound);
    qint16 motorSteps = round(distance / stepsPerMille);

    QString commandToSend = "c" + QString::number(direction) + QString::number(stepIndex) + QString::number(motorSteps) + "e";
    QByteArray data = commandToSend.toUtf8();
    qint64 bytesWritten = m_serialPort->write(data);

    // Check the bytes written is valid
    if (bytesWritten == -1) {
        QMessageBox::critical(this, "Write Error", "Failed to write data to port: " + m_serialPort->errorString());
    }
    //qDebug() << "bytes send" << bytesWritten << "actual byte sent" << data << "steps per mille" << stepsPerMille << "motorsteps" << motorSteps;
    addToLog(commandToSend); // log the sent message
}

// Slot implementation for the "Send" button
void MainWindow::on_pushButton_send_clicked()
{


    // Get the data to send
    QString dataToSend = ui->lineEdit_dataToSend->text();

    if (dataToSend.isEmpty()) {
        QMessageBox::information(this, "Info", "Nothing to send. Please enter text.");
        return;
    }

    QByteArray data = dataToSend.toUtf8();
    qint64 bytesWritten = m_serialPort->write(data);

    // Check the bytes written is valid
    if (bytesWritten == -1) {
        QMessageBox::critical(this, "Write Error", "Failed to write data to port: " + m_serialPort->errorString());
    } else {
        ui->lineEdit_dataToSend->clear(); // Clear the line edit after sending
    }

    addToLog(dataToSend); // log the sent message
}

void MainWindow::on_pushButton_stop_clicked()
{
    QString stopData = "s";
    QByteArray stopByte = stopData.toUtf8();
    m_serialPort->write(stopByte);
    addToLog(stopData);
}

// This slot is called whenever the serial port has new data to be read
void MainWindow::readSerialData()
{
    // Read all available data from the port
    QByteArray data = m_serialPort->readAll();

    // The received data might not be a valid string, so we show it as hex
    // or you can use QString::fromLatin1(data) if you expect ASCII/Latin1 text.
    QString stringData = QString::fromLatin1(data);

    addToLog(stringData, true); // Log the received message
}

// Helper function to add messages to the log with a timestamp
void MainWindow::addToLog(const QString &message, bool isReceived)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString direction = isReceived ? "RX" : "TX"; // RX for received, TX for transmitted
    QString formattedMessage = QString("[%1] %2: %3").arg(timestamp, direction, message);

    ui->plainTextEdit_log->appendPlainText(formattedMessage);
}


void MainWindow::populatePortsComboBox()
{
    // clear combo box
    ui->comboBox_ports->clear();

    // get the list of ports
    const QList<QSerialPortInfo> portInfos = QSerialPortInfo::availablePorts();

    // Add each port name to the combo box
    for (const QSerialPortInfo &portInfo : portInfos) {
        ui->comboBox_ports->addItem(portInfo.portName());
        qDebug() << "Found port:" << portInfo.portName();
    }
}
