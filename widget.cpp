#include "widget.h"
#include "ui_widget.h"
#include <QCoreApplication>
#include <QDebug>
#include <unistd.h>
#include <QLabel>
#include <QTimerEvent>
#include <QFileDialog>
#include <QFile>
// Добавляем заголовочный файлы для работы с последовательным портом
#include <QSerialPort>
#include <QSerialPortInfo>


QString PORT("/dev/pts/1");//временный порт для тестирования(запускал через терминал)

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    rtsActivated = false;//инициализируем переменную для отслеживания сигнала RTS

    setFixedSize(width(), height());

    // добавляем заголовок для виджета
    QWidget::setWindowTitle("Serial Port Example");


    // Ports
    QList<QSerialPortInfo> ports = info.availablePorts();
    QList<QString> stringPorts;
    for(int i = 0 ; i < ports.size() ; i++){
        stringPorts.append(ports.at(i).portName());
    }

    //добавляет путь "эмулированного порта"
    stringPorts.append(PORT);

    ui->comboBox->addItems(stringPorts);

    // Baud Rate Ratios
    QList<qint32> baudRates = info.standardBaudRates(); //какие скорости поддерживает этот пк
    QList<QString> stringBaudRates;
    for(int i = 0 ; i < baudRates.size() ; i++){
        stringBaudRates.append(QString::number(baudRates.at(i)));
    }
    ui->comboBox_2->addItems(stringBaudRates);

    // Data Bits
    ui->comboBox_3->addItem("5");
    ui->comboBox_3->addItem("6");
    ui->comboBox_3->addItem("7");
    ui->comboBox_3->addItem("8");

    // Stop Bits
    ui->comboBox_4->addItem("1 Bit");
    ui->comboBox_4->addItem("1,5 Bits");
    ui->comboBox_4->addItem("2 Bits");

    // Parities
    ui->comboBox_5->addItem("No Parity");
    ui->comboBox_5->addItem("Even Parity");
    ui->comboBox_5->addItem("Odd Parity");
    ui->comboBox_5->addItem("Mark Parity");
    ui->comboBox_5->addItem("Space Parity");

    //Flow Controls
    ui->comboBox_6->addItem("No Flow Control");
    ui->comboBox_6->addItem("Hardware Flow Control");
    ui->comboBox_6->addItem("Software Flow Control");
    //
    ui->comboBox_6->addItem("RTS Flow Control");

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_2_clicked()
{

    QString portName = ui->comboBox->currentText();
    serialPort.setPortName(portName);

    serialPort.open(QIODevice::ReadWrite);

    if(!serialPort.isOpen()){
        ui->textBrowser->setTextColor(Qt::red);
        ui->textBrowser->append("wrong");
    }
    else {

        QString stringbaudRate = ui->comboBox_2->currentText();
        int intbaudRate = stringbaudRate.toInt();
        serialPort.setBaudRate(intbaudRate);

        QString dataBits = ui->comboBox_3->currentText();
        if(dataBits == "5 Bits") {
           serialPort.setDataBits(QSerialPort::Data5);
        }
        else if((dataBits == "6 Bits")) {
           serialPort.setDataBits(QSerialPort::Data6);
        }
        else if(dataBits == "7 Bits") {
           serialPort.setDataBits(QSerialPort::Data7);
        }
        else if(dataBits == "8 Bits"){
           serialPort.setDataBits(QSerialPort::Data8);
        }

        QString stopBits = ui->comboBox_4->currentText();
        if(stopBits == "1 Bit") {
         serialPort.setStopBits(QSerialPort::OneStop);
        }
        else if(stopBits == "1,5 Bits") {
         serialPort.setStopBits(QSerialPort::OneAndHalfStop);
        }
        else if(stopBits == "2 Bits") {
         serialPort.setStopBits(QSerialPort::TwoStop);
        }

        QString parity = ui->comboBox_5->currentText();
        if(parity == "No Parity"){
          serialPort.setParity(QSerialPort::NoParity);
        }
        else if(parity == "Even Parity"){
          serialPort.setParity(QSerialPort::EvenParity);
        }
        else if(parity == "Odd Parity"){
          serialPort.setParity(QSerialPort::OddParity);
        }
        else if(parity == "Mark Parity"){
          serialPort.setParity(QSerialPort::MarkParity);
        }
        else if(parity == "Space Parity") {
            serialPort.setParity(QSerialPort::SpaceParity);
        }


        QString flowControl = ui->comboBox_6->currentText();
        if(flowControl == "No Flow Control") {
          serialPort.setFlowControl(QSerialPort::NoFlowControl);
        }
        else if(flowControl == "Hardware Flow Control") {
          serialPort.setFlowControl(QSerialPort::HardwareControl);
        }
        else if(flowControl == "Software Flow Control") {
          serialPort.setFlowControl(QSerialPort::SoftwareControl);
        }
        //
        else if (flowControl == "RTS Flow Control") {
            serialPort.setDataTerminalReady(true);
            serialPort.setRequestToSend(true);
        }

        code = ui->lineEdit->text();
        codeSize = code.size();

        connect(&serialPort,SIGNAL(readyRead()),this,SLOT(receiveMessage()));
    }


}

/*
Далее сохраняется каждое полученное сообщение в буфере затем ищется в нем код.
Если код найден, то выводится часть сообщения до кода, после чего это сообщение удаляется из буфера.
Если код не найден, то полученное сообщение продолжает сохраняться в буфере для дальнейшей обработки.
*/


void Widget::receiveMessage()
{
    QByteArray dataBA = serialPort.readAll();
    //
    if (rtsActivated) {
        // Приостановить передачу данных, так как сигнал RTS активирован
        qDebug() << "Сигнал RTS активирован";
        return;
    }
    QString data(dataBA);
    buffer.append(data);
    int index = buffer.indexOf(code);
    if(index != -1){
       QString message = buffer.mid(0,index);
       ui->textBrowser->setTextColor(Qt::blue);
       ui->textBrowser->append(message);
       buffer.remove(0,index+codeSize);
    }

}

void Widget::on_pushButton_clicked()
{
    if (rtsActivated) {
        // Сигнал RTS активирован, оставляем передачу данных
        qDebug() << "Сигнал RTS активирован";
        return;
    }

    QByteArray dataBA = QString(m_bytes).toUtf8();
    for (int i = 0; i < dataBA.size(); ++i) {
        QByteArray byte = dataBA.mid(i, 1);
        serialPort.write(byte);

    }
}


// Кнопка отключения
void Widget::on_pushButton_3_clicked()
{
    serialPort.close();
}

// Кнопка обновления портов
void Widget::on_pushButton_4_clicked()
{
    ui->comboBox->clear();
    QList<QSerialPortInfo> ports = info.availablePorts();
    QList<QString> stringPorts;
    for(int i = 0 ; i < ports.size() ; i++){
        stringPorts.append(ports.at(i).portName());
    }
    ui->comboBox->addItems(stringPorts);
}

// очищение окна вывода состояния
void Widget::on_pushButton_5_clicked()
{
    ui->textBrowser->clear();
}


void Widget::on_pushButton_6_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите текстовый файл");

    if (!filePath.isEmpty()) {
        ui->textBrowser->append("Файл успешно открыт.");
        // Читаем содержимое файла
        QFile file(filePath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

            while (!file.atEnd()) {
                m_bytes = file.readAll();  // Читаем один байт из файла
            }
            file.close();
        } else {
            ui->textBrowser->append("Ошибка чтения");
            qDebug() << "Ошибка чтения";
        }
    }
}

