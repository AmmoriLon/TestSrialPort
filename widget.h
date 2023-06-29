#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_2_clicked();
    void receiveMessage();
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::Widget *ui;
    QSerialPort serialPort;
    QSerialPortInfo info;
    QString buffer;
    QString code;
    int codeSize;
    QByteArray m_bytes;
    QFile m_file;
    bool rtsActivated;//для отслеживания состояния сигнала RTS
};
#endif // WIDGET_H
