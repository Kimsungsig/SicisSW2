#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qserialport.h>
#include <qserialportinfo.h>
#include <stdio.h>
#include <QLabel>
#include <QMessageBox>
#include <qDebug>
#include <Qstring>
//#include <studio.h>
#include <QtGui>
#include <QTime>
#include <windows.h>
#include <Tlhelp32.h>

QString year;
QString month;
QString day;
QString hour;
QString min;
QString sec;
QString sec2;
//unsigned char SDRDATA[27];
//unsigned char SDDATA[24];
//unsigned char *SDRDATA2 = new unsigned char[27];

//SDR sendData;
//SDR *sendData2 = new SDR;
QSerialPort *serial;
int readDataCheck=0;
unsigned char* Inhex = new unsigned char[30];
unsigned char rundist = 0x00;
//SD *recData = new SD;

int pushWhile = 1;


void MainWindow::closeEvent(QCloseEvent *e){
    qDebug() << "CloseEvent.while over";
    pushWhile=0;
}

QString getStringFromUnsignedChar( unsigned char str ){
    QString result = "";

    QString s;
        s = QString( "%1" ).arg( str, 0, 16 );
        if( s.length() == 1 )
            result.append( "0" );

        result.append( s );

    return result;
}

void delay(double sec)
{
    QTime dieTime= QTime::currentTime().addSecs(sec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
        ui->setupUi(this);
        ui->lcd_temp->setPalette(Qt::red);
        serial = new QSerialPort(this);
        connect(ui->rescan_Button, SIGNAL(clicked()), this, SLOT(serial_rescan()));
        connect(ui->connect_button, SIGNAL(clicked()), this, SLOT (serial_connect()));
        connect(ui->slider1, SIGNAL(valueChanged(int)), this, SLOT(widget_changed()));
        connect(ui->slider2, SIGNAL(valueChanged(int)), this, SLOT(widget_changed()));
        connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(widget_changed()));
        connect(serial, SIGNAL(readyRead()), this, SLOT(serial_received()));
        serial_rescan();
}

MainWindow::~MainWindow()
{
    pushWhile=0;
    delete ui;
    serial->close();
}

void MainWindow::serial_connect()
{
        serial->setPortName(ui->port_box->currentText());
        serial->setBaudRate(QSerialPort::Baud19200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::EvenParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        this->ui->textEdit->insertHtml("port connect OK");

        if(!serial->open(QIODevice::ReadWrite)){
            qDebug() << "Serial port open error";
        }
}

void MainWindow::serial_rescan()
{
    ui->port_box->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        ui->port_box->addItem(serialPortInfo.portName());
    }
}

void MainWindow::widget_changed()
{
    QString buffer;
    buffer.sprintf("A%03i;%03i;%03i\n",ui->slider1->value(),ui->slider2->value(),ui->dial->value());
    qDebug() << buffer;
    serial->write( buffer.toStdString().c_str(), buffer.size());
    serial->QSerialPort::waitForBytesWritten(-1);
    ui->lcd_led1->display(ui->slider1->value());
    ui->lcd_led2->display(ui->slider2->value());
    ui->lcd_pwm->display(ui->dial->value());
}
void MainWindow::serial_received()
{
    unsigned char bcc1 = 0x00;
    unsigned char bcc2 = 0x00;

    if (ui->checkBox_97->isChecked() == true) // SDR read SD Send Mode
    {
        if(readDataCheck>0 && readDataCheck<27)
        {
            this->ui->textEdit_3->clear();
            qDebug() << "one more read data ↓";
            QByteArray read_Data2;
            read_Data2 = serial->readAll();
            qDebug() << read_Data2.size();
            int resize = 27-readDataCheck;
            unsigned char* hex2 = new unsigned char[resize];
            memcpy(hex2, read_Data2.constData(), resize);
            for(int i=0; i<resize; i++){
                Inhex[i+readDataCheck] = hex2[i];
            }

            readDataCheck = 27;
        }
        else
        {
            this->ui->textEdit_3->clear();
            QByteArray read_Data;
            read_Data = serial->readAll();
            readDataCheck = read_Data.size();
            memcpy(Inhex, read_Data.constData(), read_Data.size());
            qDebug() << "Read data ↓";
            qDebug() << read_Data.size();
        }
        if(readDataCheck==27)
        {
            this->ui->textEdit_2->clear();

            for(int i=0; i<27; i++){
                this->ui->textEdit_2->insertHtml(getStringFromUnsignedChar(Inhex[i]));
            }

            bcc1 = 0x00;
            bcc2 = 0x00;
            for(int z=1; z<=24; z++)
            {
                if(z%2==0){
                    bcc1 = bcc1^Inhex[z];
                }
                else{
                    bcc2 = bcc2^Inhex[z];
                    }
            }

            if (bcc2 != Inhex[25]){
                this->ui->textEdit_3->insertHtml("BCC1 not same");
                qDebug() << "Not bcc1 samsam";
                qDebug() << Inhex[25];
                qDebug() << bcc2;
            }
            if (bcc1 != Inhex[26]){
                this->ui->textEdit_3->insertHtml("BCC2 not same");
                qDebug() << "Not bcc2 samsam";
                qDebug() << Inhex[26];
                qDebug() << bcc1;
            }
            on_pushButton_2_clicked();
        }
    }
    else
    {
        if(readDataCheck>0 && readDataCheck<24) // 현제는 일반데이터 값 30이니까. 이후에 필요시 변경.
        {
            this->ui->textEdit_3->clear(); // 일단 지우고
            qDebug() << "one more read data ↓";
            QByteArray read_Data2;
            read_Data2 = serial->readAll();
            qDebug() << read_Data2.size();
            int resize = 24-readDataCheck;
            unsigned char* hex2 = new unsigned char[resize];
            memcpy(hex2, read_Data2.constData(), resize);
            for(int i=0; i<resize; i++){
                Inhex[i+readDataCheck] = hex2[i];
            }

            readDataCheck = 24; // 이건 다시읽어온 값인데.필요시 바꿔준다.
        }
        else
        {
            this->ui->textEdit_3->clear();
            QByteArray read_Data;
            read_Data = serial->readAll();
            readDataCheck = read_Data.size();
            memcpy(Inhex, read_Data.constData(), read_Data.size());
            qDebug() << "Read data ↓";
            qDebug() << read_Data.size();
        }

        for(int i=0; i<24; i++){
            this->ui->textEdit_3->insertHtml(getStringFromUnsignedChar(Inhex[i]));
        }

        bcc1 = 0x00;
        bcc2 = 0x00;
        for(int z=1; z<=21; z++)
        {
            if(z%2==0){
                bcc1 = bcc1^Inhex[z];
            }
            else{
                bcc2 = bcc2^Inhex[z];
                }
        }

        if (bcc2 != Inhex[22]){
            this->ui->textEdit_3->insertHtml("BCC1 not same");
            qDebug() << "Not bcc1 samsam";
            qDebug() << Inhex[22];
            qDebug() << bcc2;
        }
        if (bcc1 != Inhex[23]){
            this->ui->textEdit_3->insertHtml("BCC2 not same");
            qDebug() << "Not bcc2 samsam";
            qDebug() << Inhex[23];
            qDebug() << bcc1;
        }
    }

        if (Inhex[0] == 0x02 && ui->checkBox_97->isChecked() == false)
        {
            ui->spinBox_20->setValue(Inhex[1]);
            ui->spinBox_21->setValue(Inhex[2]);
            ui->spinBox_22->setValue(Inhex[3]);
            if(	Inhex[4] & 0x04)
                ui->checkBox_30->setChecked(true);
            else
                ui->checkBox_30->setChecked(false);
            if(Inhex[4] & 0x02)
                ui->checkBox_31->setChecked(true);
            else
                ui->checkBox_31->setChecked(false);
            if(Inhex[4] & 0x01)
                ui->checkBox_32->setChecked(true);
            else
                ui->checkBox_32->setChecked(false);

            if(Inhex[5] & 0x10)
                ui->checkBox_33->setChecked(true);
            else
                ui->checkBox_33->setChecked(false);
            if(Inhex[5] & 0x08)
                ui->checkBox_34->setChecked(true);
            else
                ui->checkBox_34->setChecked(false);
            if(Inhex[5] & 0x04)
                ui->checkBox_35->setChecked(true);
            else
                ui->checkBox_35->setChecked(false);
            if(Inhex[5] & 0x02)
                ui->checkBox_36->setChecked(true);
            else
                ui->checkBox_36->setChecked(false);
            if(Inhex[5] & 0x01)
                ui->checkBox_96->setChecked(true);
            else
                ui->checkBox_96->setChecked(false);

            ui->spinBox_31->setValue(Inhex[6]);

            if(Inhex[7] & 0x80)
                ui->checkBox_37->setChecked(true);
            else
                ui->checkBox_37->setChecked(false);
            if(Inhex[7] & 0x40)
                ui->checkBox_38->setChecked(true);
            else
                ui->checkBox_38->setChecked(false);
            if(Inhex[7] & 0x20)
                ui->checkBox_39->setChecked(true);
            else
                ui->checkBox_39->setChecked(false);
            if(Inhex[7] & 0x10)
                ui->checkBox_40->setChecked(true);
            else
                ui->checkBox_40->setChecked(false);
            if(Inhex[7] & 0x08)
                ui->checkBox_41->setChecked(true);
            else
                ui->checkBox_41->setChecked(false);
            if(Inhex[7] & 0x04)
                ui->checkBox_42->setChecked(true);
            else
                ui->checkBox_42->setChecked(false);
            if(Inhex[7] & 0x02)
                ui->checkBox_43->setChecked(true);
            else
                ui->checkBox_43->setChecked(false);
            if(Inhex[7] & 0x01)
                ui->checkBox_44->setChecked(true);
            else
                ui->checkBox_44->setChecked(false);

            if(Inhex[8] & 0x80)
                ui->checkBox_45->setChecked(true);
            else
                ui->checkBox_45->setChecked(false);
            if(Inhex[8] & 0x40)
                ui->checkBox_46->setChecked(true);
            else
                ui->checkBox_46->setChecked(false);
            if(Inhex[8] & 0x20)
                ui->checkBox_47->setChecked(true);
            else
                ui->checkBox_47->setChecked(false);
            if(Inhex[8] & 0x10)
                ui->checkBox_48->setChecked(true);
            else
                ui->checkBox_48->setChecked(false);
            if(Inhex[8] & 0x08)
                ui->checkBox_49->setChecked(true);
            else
                ui->checkBox_49->setChecked(false);
            if(Inhex[8] & 0x04)
                ui->checkBox_50->setChecked(true);
            else
                ui->checkBox_50->setChecked(false);
            if(Inhex[8] & 0x02)
                ui->checkBox_51->setChecked(true);
            else
                ui->checkBox_51->setChecked(false);
            if(Inhex[8] & 0x01)
                ui->checkBox_52->setChecked(true);
            else
                ui->checkBox_52->setChecked(false);

            if(Inhex[9] & 0x80)
                ui->checkBox_25->setChecked(true);
            else
                ui->checkBox_25->setChecked(false);
            if(Inhex[9] & 0x40)
                ui->checkBox_53->setChecked(true);
            else
                ui->checkBox_53->setChecked(false);
            if(Inhex[9] & 0x20)
                ui->checkBox_54->setChecked(true);
            else
                ui->checkBox_54->setChecked(false);
            if(Inhex[9] & 0x10)
                ui->checkBox_55->setChecked(true);
            else
                ui->checkBox_55->setChecked(false);
            if(Inhex[9] & 0x08)
                ui->checkBox_56->setChecked(true);
            else
                ui->checkBox_56->setChecked(false);
            if(Inhex[9] & 0x04)
                ui->checkBox_57->setChecked(true);
            else
                ui->checkBox_57->setChecked(false);

            if(Inhex[10] & 0x80)
                ui->checkBox_58->setChecked(true);
            else
                ui->checkBox_58->setChecked(false);
            if(Inhex[10] & 0x40)
                ui->checkBox_59->setChecked(true);
            else
                ui->checkBox_59->setChecked(false);
            if(Inhex[10] & 0x20)
                ui->checkBox_60->setChecked(true);
            else
                ui->checkBox_60->setChecked(false);
            if(Inhex[10] & 0x10)
                ui->checkBox_61->setChecked(true);
            else
                ui->checkBox_61->setChecked(false);
            if(Inhex[10] & 0x08)
                ui->checkBox_62->setChecked(true);
            else
                ui->checkBox_62->setChecked(false);
            if(Inhex[10] & 0x04)
                ui->checkBox_63->setChecked(true);
            else
                ui->checkBox_63->setChecked(false);
            if(Inhex[10] & 0x02)
                ui->checkBox_64->setChecked(true);
            else
                ui->checkBox_64->setChecked(false);
            if(Inhex[10] & 0x01)
                ui->checkBox_65->setChecked(true);
            else
                ui->checkBox_65->setChecked(false);

            if(Inhex[11] & 0x80)
                ui->checkBox_66->setChecked(true);
            else
                ui->checkBox_66->setChecked(false);
            if(Inhex[11] & 0x40)
                ui->checkBox_67->setChecked(true);
            else
                ui->checkBox_67->setChecked(false);
            if(Inhex[11] & 0x20)
                ui->checkBox_68->setChecked(true);
            else
                ui->checkBox_68->setChecked(false);
            if(Inhex[11] & 0x10)
                ui->checkBox_69->setChecked(true);
            else
                ui->checkBox_69->setChecked(false);
            if(Inhex[11] & 0x08)
                ui->checkBox_70->setChecked(true);
            else
                ui->checkBox_70->setChecked(false);
            if(Inhex[11] & 0x04)
                ui->checkBox_71->setChecked(true);
            else
                ui->checkBox_71->setChecked(false);
            if(Inhex[11] & 0x02)
                ui->checkBox_72->setChecked(true);
            else
                ui->checkBox_72->setChecked(false);
            if(Inhex[11] & 0x01)
                ui->checkBox_73->setChecked(true);
            else
                ui->checkBox_73->setChecked(false);

            if(Inhex[12] & 0x80)
                ui->checkBox_74->setChecked(true);
            else
                ui->checkBox_74->setChecked(false);
            if(Inhex[12] & 0x40)
                ui->checkBox_75->setChecked(true);
            else
                ui->checkBox_75->setChecked(false);
            if(Inhex[12] & 0x20)
                ui->checkBox_76->setChecked(true);
            else
                ui->checkBox_76->setChecked(false);
            if(Inhex[12] & 0x10)
                ui->checkBox_77->setChecked(true);
            else
                ui->checkBox_77->setChecked(false);
            if(Inhex[12] & 0x08)
                ui->checkBox_78->setChecked(true);
            else
                ui->checkBox_78->setChecked(false);
            if(Inhex[12] & 0x04)
                ui->checkBox_79->setChecked(true);
            else
                ui->checkBox_79->setChecked(false);
            if(Inhex[12] & 0x02)
                ui->checkBox_80->setChecked(true);
            else
                ui->checkBox_80->setChecked(false);
            if(Inhex[12] & 0x01)
                ui->checkBox_81->setChecked(true);
            else
                ui->checkBox_81->setChecked(false);

            ui->spinBox_25->setValue(Inhex[13]);
            ui->spinBox_26->setValue(Inhex[14]);

            if(Inhex[15] & 0x20)
                ui->checkBox_82->setChecked(true);
            else
                ui->checkBox_82->setChecked(false);
            if(Inhex[15] & 0x10)
                ui->checkBox_83->setChecked(true);
            else
                ui->checkBox_83->setChecked(false);
            if(Inhex[15] & 0x08)
                ui->checkBox_84->setChecked(true);
            else
                ui->checkBox_84->setChecked(false);
            if(Inhex[15] & 0x04)
                ui->checkBox_85->setChecked(true);
            else
                ui->checkBox_85->setChecked(false);
            if(Inhex[15] & 0x02)
                ui->checkBox_86->setChecked(true);
            else
                ui->checkBox_86->setChecked(false);
            if(Inhex[15] & 0x01)
                ui->checkBox_87->setChecked(true);
            else
                ui->checkBox_87->setChecked(false);

            if(Inhex[16] & 0x20){
                ui->checkBox_88->setChecked(true);
                qDebug() << Inhex[16];
            }
            else{
                ui->checkBox_88->setChecked(false);
            }
            if(Inhex[16] & 0x10)
                ui->checkBox_89->setChecked(true);
            else
                ui->checkBox_89->setChecked(false);
            if(Inhex[16] & 0x08)
                ui->checkBox_90->setChecked(true);
            else
                ui->checkBox_90->setChecked(false);
            if(Inhex[16] & 0x04){
                ui->checkBox_91->setChecked(true);
            }
            else{
                ui->checkBox_91->setChecked(false);
            }
            if(Inhex[16] & 0x02)
                ui->checkBox_92->setChecked(true);
            else
                ui->checkBox_92->setChecked(false);
            if(Inhex[16] & 0x01)
                ui->checkBox_93->setChecked(true);
            else
                ui->checkBox_93->setChecked(false);

            ui->spinBox_29->setValue(Inhex[19]);
            ui->spinBox_30->setValue(Inhex[20]);

            ui->spinBox_32->setValue(Inhex[22]);
            ui->spinBox_33->setValue(Inhex[23]);
        }
        else if(Inhex[0] != 0x02){
            this->ui->textEdit_3->insertHtml("stx not 0x02");
        }
}

void MainWindow::train_set(struct trainMacro traindata)
{
    int countdata=0;
    for(countdata=0; countdata<14; countdata++)
    {
        if (countdata == 0) {
            traindata.trainspeed = 0x00; // 출발
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 1) {
            traindata.trainspeed = 0x05; // 속도 5
            rundist = rundist + 1;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 2) {
            traindata.trainspeed = 0x0a; // 속도 10
            rundist = rundist + 1;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 3) {
            traindata.trainspeed = 0x0F; // 속도 15
            rundist = rundist + 1;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 4) {
            traindata.trainspeed = 0x14; // 속도 20
            rundist = rundist + 2;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 5) {
            traindata.trainspeed = 0x19; // 속도 25
            rundist = rundist + 2;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 6) {
            traindata.trainspeed = 0x1E; // 속도 30
            rundist = rundist + 3;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 7) {
            traindata.trainspeed = 0x19; // 속도 25
            rundist = rundist + 2;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 8) {
            traindata.trainspeed = 0x14; // 속도 20
            rundist = rundist + 2;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 9) {
            traindata.trainspeed = 0x0F; // 속도 15
            rundist = rundist + 2;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 10) {
            traindata.trainspeed = 0x0a; // 속도 10
            rundist = rundist + 1;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 11) {
            traindata.trainspeed = 0x05; // 속도 5
            rundist = rundist + 1;
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }
        else if (countdata == 12) {
            traindata.trainspeed = 0x00; // 멈춤, 도어열림
            traindata.rundist2 = rundist;
            traindata.DIR = 0;
        }
        else if (countdata == 13) {
            traindata.trainspeed = 0x00; // 도어닫힘
            traindata.rundist2 = rundist;
            traindata.DIR = 1;
        }

        if (traindata.trainspeed == 0x00){
             traindata.DCW=0;
             traindata.DOW1=0;
        }
        else{
             traindata.DCW=1;
             traindata.DOW1=1;
        }
            on_textEdit_destroyed(traindata);
            if (rundist == 0xFF)
            {
                traindata.rundist1 = traindata.rundist1 + 1;
                rundist = 0x00;
            }
            delay(1);

            if(pushWhile==0)
                countdata=14;
    }
}
void MainWindow::on_PUSH_clicked()
{
    pushWhile=1;
    int countdata=0;
    trainMacro TM;
    TM.trainnum1 = 0x00;
    TM.trainnum2 = 0x00;
    TM.trainspeed = 0x00;
    TM.curcode = 0x00;
    TM.nxtcode = 0x00;
    TM.dstcode = 0x00;
    TM.rundist1 = 0x00;
    TM.rundist2 = 0x00;
    TM.DCW=0;
    TM.DOW2=0;
    TM.DOW1=0;
    TM.DIR=0;
    rundist=0;

    while(pushWhile==1)
    {
        qDebug() << "ONE";
        // 여기서 샛강~서울대 시나리오
        TM.trainnum1 = ui->spinBox_10->value();
        TM.trainnum2 = ui->spinBox_11->value();
        TM.dstcode = 0x6F; // 종착역 111
        TM.curcode = 0x65;
        TM.nxtcode = 0x66;
        train_set(TM);
        if(pushWhile==0)
            continue;
        qDebug() << pushWhile;
        TM.curcode = 0x66;
        TM.nxtcode = 0x67;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x67;
        TM.nxtcode = 0x68;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x68;
        TM.nxtcode = 0x69;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x69;
        TM.nxtcode = 0x6a;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x6a;
        TM.nxtcode = 0x6b;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x6b;
        TM.nxtcode = 0x6c;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x6c;
        TM.nxtcode = 0x6d;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x6d;
        TM.nxtcode = 0x6e;
        train_set(TM);
        if(pushWhile==0)
            continue;
        TM.curcode = 0x6e;
        TM.nxtcode = 0x6f;
        train_set(TM);
        if(pushWhile==0)
            continue;

        countdata++;
         qDebug() << "two";
         delay(3); // 한싸이클을 모두 돌게되면 10초 휴식
    }
}

void MainWindow::on_textEdit_destroyed(struct trainMacro data)
{
    SDR *sendData2 = new SDR;
    unsigned char SDRDATA[27];
    unsigned char inData2 = 0x00;
    unsigned char inData3 = 0x00;
    unsigned char inData4 = 0x00;
    unsigned char bcc1 = 0x00;
    unsigned char bcc2 = 0x00;

    this->ui->textEdit_2->clear();
    QString time2 = QDateTime::currentDateTime().toString(Qt::TextDate);
    qDebug() << "현제시간 : "<< time2;
    time2.remove(0,2);
    month = time2.left(2);
    ui->spinBox_5->setValue(month.toInt());
    if (month.toInt()>=10)
        time2.remove(0,3);
    else
        time2.remove(0,2);
    day = time2.left(2);
    ui->spinBox_6->setValue(day.toInt());
    if (day.toInt()>=10)
        time2.remove(0,3);
    else
        time2.remove(0,2);
    hour = time2.left(2);
    ui->spinBox_7->setValue(hour.toInt());
    time2.remove(0,3);
    min = time2.left(2);
    ui->spinBox_8->setValue(min.toInt());
    time2.remove(0,3);
    sec = time2.left(2);
    ui->spinBox_9->setValue(sec.toInt());
    time2.remove(0,5);
    year = time2.left(2);
    ui->spinBox_4->setValue(year.toInt());
    time2.clear();

    this->ui->textEdit->setText( "Data set OK");

    sendData2->stx = 0x02; // STX 시작값
    SDRDATA[0] = sendData2->stx;
    sendData2->toAddress = ui->spinBox->value();//추진 제어 장치(통합제어장치)의 Data 주소는 70H 이다.
    SDRDATA[1] = ui->spinBox->value();
    sendData2->fromAddress = ui->spinBox_2->value();//열차 모니터 장치(TCMS)의 Data 주소는 10H이다.
    SDRDATA[2] = ui->spinBox_2->value();
    sendData2->dataType = ui->spinBox_3->value();// 데이터 타입
    SDRDATA[3] = ui->spinBox_3->value();
    sendData2->year = ui->spinBox_4->value();//year
    SDRDATA[4] = ui->spinBox_4->value();
    sendData2->month = ui->spinBox_5->value();//month
    SDRDATA[5] = ui->spinBox_5->value();
    sendData2->day = ui->spinBox_6->value();//day
    SDRDATA[6] = ui->spinBox_6->value();
    sendData2->hour = ui->spinBox_7->value();//hour
    SDRDATA[7] = ui->spinBox_7->value();
    sendData2->min = ui->spinBox_8->value();//minute
    SDRDATA[8] = ui->spinBox_8->value();
    sendData2->sec = ui->spinBox_9->value();//second
    SDRDATA[9] = ui->spinBox_9->value();
    sendData2->trainNum1 = data.trainnum1;
    ui->spinBox_10->setValue(data.trainnum1);
    SDRDATA[10] = data.trainnum1;
    sendData2->trainNum2 = data.trainnum2;
    ui->spinBox_11->setValue(data.trainnum2);
    SDRDATA[11] = data.trainnum2;

    sendData2->speed = data.trainspeed;// 속도 SPEED
    SDRDATA[12] = data.trainspeed;
    ui->spinBox_12->setValue(data.trainspeed);

    sendData2->curCode = data.curcode;// 현재역 코드 SURCODE
    SDRDATA[13] = data.curcode;
    ui->spinBox_13->setValue(data.curcode);

    sendData2->nxtCode = data.nxtcode;// 다음역 코드 NXTCODE
    SDRDATA[14] = data.nxtcode;
    ui->spinBox_14->setValue(data.nxtcode);

    sendData2->dstCode = data.dstcode;// 종착역 코드 DSTCODE
    SDRDATA[15] = data.dstcode;
    ui->spinBox_15->setValue(data.dstcode);

    sendData2->runDist1 = data.rundist1;// 주행거리 RUNDIST
    SDRDATA[16] = data.rundist1;
    ui->spinBox_16->setValue(data.rundist1);

    sendData2->runDist2 = data.rundist2;// 주행거리 2
    SDRDATA[17] = data.rundist2;
    ui->spinBox_17->setValue(data.rundist2);

    sendData2->sp_byte0 = ui->spinBox_18->value(); // SPARE
    SDRDATA[18] = ui->spinBox_18->value();

    if (ui->checkBox_16->isChecked() == true) // 시험요청여부
        sendData2->test_start_req=1;
    sendData2->sp_bit0 = ui->spinBox_23->value(); // spare
    if (sendData2->test_start_req==1)
        SDRDATA[19] = sendData2->sp_bit0 + 0x80;
    else
        SDRDATA[19] = sendData2->sp_bit0;

    int countdata = ui->comboBox->currentIndex();//운전모드
    if (countdata == 0){sendData2->drive_DM=1;
                inData2 += 0x80;}
    else if (countdata == 1)   {sendData2->drive_AM=1;
                inData2 += 0x40;}
    else if (countdata == 2)   {sendData2->drive_MM=1;
                inData2 += 0x20;}
    else if (countdata == 3)   {sendData2->drive_EM=1;
                inData2 += 0x10;}

    if (ui->checkBox1->isChecked() == true){ // 구원운전 스위치
        sendData2->ros=1;
        inData2 += 0x08;
    }
    if (ui->checkBox_2->isChecked() == true){ // 정지속도 검출
        sendData2->zvr=1;
        inData2 += 0x04;
    }
    if (ui->checkBox_3->isChecked() == true){ // 3호차MC2측 HCR투입
        sendData2->mc2Hcr=1;
        inData2 += 0x02;
    }
    if (ui->checkBox_4->isChecked() == true){ // 1호차MC1측 HCR투입
        sendData2->mc1Hcr=1;
        inData2 += 0x01;
    }

    SDRDATA[20] = inData2; //

    if (ui->checkBox_5->isChecked() == true){ // 3호차 화재발생
        sendData2->fd3=1;
        inData3 += 0x80;
    }
    if (ui->checkBox_6->isChecked() == true){ // 2호차 화재발생
        sendData2->fd2=1;
        inData3 += 0x40;
    }
    if (ui->checkBox_7->isChecked() == true){ // 1호차 화재발생
        sendData2->fd1=1;
        inData3 += 0x20;
    }

    if(data.DIR==1)
        ui->checkBox_8->setChecked(true);
    else
        ui->checkBox_8->setChecked(false);

    if (ui->checkBox_8->isChecked() == true){ // 전체 출입문 닫힘
        sendData2->dir=1;
        inData3 += 0x10;
    }
    if (ui->checkBox_9->isChecked() == true){ // 3호차 2위 열림 계전기
        sendData2->dor2_3=1;
        inData3 += 0x08;
    }
    if (ui->checkBox_10->isChecked() == true){ // 3호차 1위 열림 계전기
        sendData2->dor1_3=1;
        inData3 += 0x04;
    }
    if (ui->checkBox_11->isChecked() == true){ // 1호차 2위 열림 계전기
        sendData2->dor2_1=1;
        inData3 += 0x02;
    }
    if (ui->checkBox_12->isChecked() == true){ // 1호차 1위 열림 계전기
        sendData2->dor1_1=1;
        inData3 += 0x01;
    }
    SDRDATA[21] = inData3;

    //spinBox_24
    sendData2->sp_bit1 = ui->spinBox_24->value();
    inData4 += ui->spinBox_24->value();

    if(data.DCW==1){
        ui->checkBox_13->setChecked(true);}
    else{
        ui->checkBox_13->setChecked(false);}
    if(data.DOW2==1)
        ui->checkBox_14->setChecked(true);
    else
        ui->checkBox_14->setChecked(false);
    if(data.DOW1==1)
        ui->checkBox_15->setChecked(true);
    else
        ui->checkBox_15->setChecked(false);

    if (ui->checkBox_13->isChecked() == true){ //
        sendData2->dcw=1;
        inData4 += 0x04;
    }
    if (ui->checkBox_14->isChecked() == true){ //
        sendData2->dow2=1;
        inData4 += 0x02;
    }
    if (ui->checkBox_15->isChecked() == true){ //
        sendData2->dow1=1;
        inData4 += 0x04;
    }
    SDRDATA[22] = inData4;

    sendData2->lifeCnt = ui->spinBox_19->value();
    SDRDATA[23] = sendData2->lifeCnt;

    int additem= ui->spinBox_19->value();
    if (additem == 255)
        additem = 0;
    ui->spinBox_19->setValue(additem+1);

    sendData2->etx = 0x03;; // ETX 종료값
    SDRDATA[24] = sendData2->etx;

    for(int z=1; z<=24; z++)
    {
        if(z%2==0){
            bcc1 = bcc1^SDRDATA[z];
        }
        else{
            bcc2 = bcc2^SDRDATA[z];
            }
    }

    SDRDATA[25] = bcc2;
    sendData2->bcc1 = bcc2;
    SDRDATA[26] = bcc1; //BCC2
    sendData2->bcc2 = bcc1;

    auto send = reinterpret_cast<char *>(SDRDATA);

    serial->write(send,sizeof(SDR));
    QByteArray byteArray(send,sizeof(SDR)); // 용도가?
    qDebug() << "write" << byteArray.toHex();
    for(int i=0; i<=26; i++)
    {
        this->ui->textEdit_2->insertHtml(getStringFromUnsignedChar(SDRDATA[i]));
    }
    this->ui->textEdit_9->clear();
    this->ui->textEdit_9->insertHtml(getStringFromUnsignedChar(SDRDATA[ui->spinBox_34->value()]));
}

void MainWindow::on_pushButton_2_clicked()
{
    unsigned char SDDATA[24] = {0,};
    SD *recData2 = new SD;
    memset(recData2,0,sizeof(SD));
    unsigned char bcc1= 0x00;
    unsigned char bcc2= 0x00;
    unsigned char inData1 = 0x00;
    unsigned char inData2 = 0x00;
    unsigned char inData3 = 0x00;
    unsigned char inData4 = 0x00;
    unsigned char inData5 = 0x00;
    unsigned char inData6 = 0x00;
    unsigned char inData7 = 0x00;
    unsigned char inData8 = 0x00;
    unsigned char inData9 = 0x00;
    unsigned char inData10 = 0x00;

    recData2->stx = 0x02;
    SDDATA[0]=0x02;
    recData2->toAddress = ui->spinBox_20->value(); //spinBox_20
    SDDATA[1]=ui->spinBox_20->value();
    recData2->fromAddress = ui->spinBox_21->value();
    SDDATA[2]=ui->spinBox_21->value();
    recData2->dataType= ui->spinBox_22->value();
    SDDATA[3]=ui->spinBox_22->value();

    if (ui->checkBox_30->isChecked() == true){
        recData2->onbrc=1;
        inData1 = inData1 | 0x04;
    }
    if (ui->checkBox_31->isChecked() == true){
        recData2->slave=1;
        inData1 = inData1 | 0x02;
    }
    if (ui->checkBox_32->isChecked() == true){
        recData2->master=1;
        inData1 = inData1 | 0x01;
    }
    SDDATA[4] = inData1;

    if (ui->checkBox_33->isChecked() == true){
        recData2->pa_Ok=1;
        inData2 = inData2 | 0x10;
    }
    else
        recData2->pa_Ok=0;
    if (ui->checkBox_34->isChecked() == true){
        recData2->pa_Ng=1;
        inData2 = inData2 | 0x08;
    }
    else
        recData2->pa_Ng=0;
    if (ui->checkBox_35->isChecked() == true){
        recData2->pis_Ok=1;
        inData2 = inData2 | 0x04;
    }
    else
        recData2->pis_Ok=0;
    if (ui->checkBox_36->isChecked() == true){
        recData2->pis_Ng=1;
        inData2 = inData2 | 0x02;
    }
    else
        recData2->pis_Ng=0;
    if (ui->checkBox_96->isChecked() == true){
        recData2->onTest=1;
        inData2 = inData2 | 0x01;
    }
    else
        recData2->onTest=0;
    SDDATA[5] = inData2;

    //recData2->sp_byte;
    if (ui->checkBox_37->isChecked() == true){
        recData2->fcam1f=1;
        inData3 += 0x80;
    }
    else
        recData2->fcam1f=0;
    if (ui->checkBox_38->isChecked() == true){
        recData2->scam2_1f=1;
        inData3 +=0x40;
    }
    else
        recData2->scam2_1f=0;
    if (ui->checkBox_39->isChecked() == true){
        recData2->scam1_1f=1;
        inData3 +=0x20;
    }
    else
        recData2->scam1_1f=0;
    if (ui->checkBox_40->isChecked() == true){
        recData2->pei2_1f=1;
        inData3 += 0x10;
    }
    else
        recData2->pei2_1f=0;
    if (ui->checkBox_41->isChecked() == true){
        recData2->pei1_1f=1;
        inData3 += 0x08;
    }
    else
        recData2->pei1_1f=0;
    if (ui->checkBox_42->isChecked() == true){
        recData2->pamp1f=1;
        inData3 += 0x04;
    }
    else
        recData2->pamp1f=0;
    if (ui->checkBox_43->isChecked() == true){
        recData2->cop1f=1;
        inData3 += 0x02;
    }
    else
        recData2->cop1f=0;
    if (ui->checkBox_44->isChecked() == true){
        recData2->avc1f=1;
        inData3 += 0x01;
    }
    else
        recData2->avc1f=0;

    SDDATA[7] = inData3;

    if (ui->checkBox_45->isChecked() == true){
        recData2->esw2_1f=1;
        inData4 = inData4 | 0x80;
    }
    else
        recData2->esw2_1f=0;
    if (ui->checkBox_46->isChecked() == true){
        recData2->esw1_1f=1;
        inData4 = inData4 | 0x40;
    }
    else
        recData2->esw1_1f=0;
    if (ui->checkBox_47->isChecked() == true){
        recData2->lcd4_1f=1;
        inData4 = inData4 | 0x20;
    }
    else
        recData2->lcd4_1f=0;
    if (ui->checkBox_48->isChecked() == true){
        recData2->lcd3_1f=1;
        inData4 = inData4 | 0x10;
    }
    else
        recData2->lcd3_1f=0;
    if (ui->checkBox_49->isChecked() == true){
        recData2->lcd2_1f=1;
        inData4 = inData4 | 0x08;
    }
    else
        recData2->lcd2_1f=0;
    if (ui->checkBox_50->isChecked() == true){
        recData2->lcd1_1f=1;
        inData4 = inData4 | 0x04;
    }
    else
        recData2->lcd1_1f=0;
    if (ui->checkBox_51->isChecked() == true){
        recData2->led2_1f=1;
        inData4 = inData4 | 0x02;
    }
    else
        recData2->led2_1f=0;
    if (ui->checkBox_52->isChecked() == true){
        recData2->led1_1f=1;
        inData4 = inData4 | 0x01;
    }
    else
        recData2->led1_1f=0;

    SDDATA[8] = inData4;

    if (ui->checkBox_25->isChecked() == true){
        recData2->sp_Bit4=1;
        inData5 = inData5 | 0x80;
    }
    else
        recData2->sp_Bit4=0;
    if (ui->checkBox_53->isChecked() == true){
        recData2->scam2_2f=1;
        inData5 = inData5 | 0x40;
    }
    else
        recData2->scam2_2f=0;
    if (ui->checkBox_54->isChecked() == true){
        recData2->scam1_2f=1;
        inData5 = inData5 | 0x20;
    }
    else
        recData2->scam1_2f=0;
    if (ui->checkBox_55->isChecked() == true){
        recData2->pei2_2f=1;
        inData5 = inData5 | 0x10;
    }
    else
        recData2->pei2_2f=0;
    if (ui->checkBox_56->isChecked() == true){
        recData2->pei1_2f=1;
        inData5 = inData5 | 0x08;
    }
    else
        recData2->pei1_2f=0;
    if (ui->checkBox_57->isChecked() == true){
        recData2->pamp2f=1;
        inData5 = inData5 | 0x04;
    }
    else
        recData2->pamp2f=0;

    SDDATA[9] = inData5;

    if (ui->checkBox_58->isChecked() == true){
        recData2->esw2_2f=1;
        inData6 = inData6 | 0x80;
    }
    else
        recData2->esw2_2f=0;
    if (ui->checkBox_59->isChecked() == true){
        recData2->esw1_2f=1;
        inData6 = inData6 | 0x40;
    }
    else
        recData2->esw1_2f=0;
    if (ui->checkBox_60->isChecked() == true){
        recData2->lcd4_2f=1;
        inData6 = inData6 | 0x20;
    }
    else
        recData2->lcd4_2f=0;
    if (ui->checkBox_61->isChecked() == true){
        recData2->lcd3_2f=1;
        inData6 = inData6 | 0x10;
    }
    else
        recData2->lcd3_2f=0;
    if (ui->checkBox_62->isChecked() == true){
        recData2->lcd2_2f=1;
        inData6 = inData6 | 0x08;
    }
    else
        recData2->lcd2_2f=0;
    if (ui->checkBox_63->isChecked() == true){
        recData2->lcd1_2f=1;
        inData6 = inData6 | 0x04;
    }
    else
        recData2->lcd1_2f=0;
    if (ui->checkBox_64->isChecked() == true){
        recData2->led2_2f=1;
        inData6 = inData6 | 0x02;
    }
    else
        recData2->led2_2f=0;
    if (ui->checkBox_65->isChecked() == true){
        recData2->led1_2f=1;
        inData6 = inData6 | 0x01;
    }
    else
        recData2->led1_2f=0;

    SDDATA[10] = inData6;

    if (ui->checkBox_66->isChecked() == true){
        recData2->fcam3f=1;
        inData7 = inData7 | 0x80;
    }
    else
        recData2->fcam3f=0;
    if (ui->checkBox_67->isChecked() == true){
        recData2->scan2_3f=1;
        inData7 = inData7 | 0x40;
    }
    else
        recData2->scan2_3f=0;
    if (ui->checkBox_68->isChecked() == true){
        recData2->scan1_3f=1;
        inData7 = inData7 | 0x20;
    }
    else
        recData2->scan1_3f=0;
    if (ui->checkBox_69->isChecked() == true){
        recData2->pei2_3f=1;
        inData7 = inData7 | 0x10;
    }
    else
        recData2->pei2_3f=0;
    if (ui->checkBox_70->isChecked() == true){
        recData2->pei1_3f=1;
        inData7 = inData7 | 0x08;
    }
    else
        recData2->pei1_3f=0;
    if (ui->checkBox_71->isChecked() == true){
        recData2->pemp3f=1;
        inData7 = inData7 | 0x04;
    }
    else
        recData2->pemp3f=0;
    if (ui->checkBox_72->isChecked() == true){
        recData2->cop3f=1;
        inData7 = inData7 | 0x02;
    }
    else
        recData2->cop3f=0;
    if (ui->checkBox_73->isChecked() == true){
        recData2->avc3f=1;
        inData7 = inData7 | 0x01;
    }
    else
        recData2->avc3f=0;

    SDDATA[11] = inData7;

    if (ui->checkBox_74->isChecked() == true){
        recData2->esw2_3f=1;
        inData8 = inData8 | 0x80;
    }
    else
        recData2->esw2_3f=0;
    if (ui->checkBox_75->isChecked() == true){
        recData2->esw1_3f=1;
        inData8 = inData8 | 0x40;
    }
    else
        recData2->esw1_3f=0;
    if (ui->checkBox_76->isChecked() == true){
        recData2->lcd4_3f=1;
        inData8 = inData8 | 0x20;
    }
    else
        recData2->lcd4_3f=0;
    if (ui->checkBox_77->isChecked() == true){
        recData2->lcd3_3f=1;
        inData8 = inData8 | 0x10;
    }
    else
        recData2->lcd3_3f=0;
    if (ui->checkBox_78->isChecked() == true){
        recData2->lcd2_3f=1;
        inData8 = inData8 | 0x08;
    }
    else
        recData2->lcd2_3f=0;
    if (ui->checkBox_79->isChecked() == true){
        recData2->lcd1_3f=1;
        inData8 = inData8 | 0x04;
    }
    else
        recData2->lcd1_3f=0;
    if (ui->checkBox_80->isChecked() == true){
        recData2->led2_3f=1;
        inData8 = inData8 | 0x02;
    }
    else
        recData2->led2_3f=0;
    if (ui->checkBox_81->isChecked() == true){
        recData2->led1_3f=1;
        inData8 = inData8 | 0x01;
    }
    else
        recData2->led1_3f=0;

    SDDATA[12] = inData8;

    if (ui->checkBox_82->isChecked() == true){
        recData2->pei2_3c=1;
        inData9 = inData9 | 0x20;
    }
    else
        recData2->pei2_3c=0;
    if (ui->checkBox_83->isChecked() == true){
        recData2->pei1_3c=1;
        inData9 = inData9 | 0x10;
    }
    else
        recData2->pei1_3c=0;
    if (ui->checkBox_84->isChecked() == true){
        recData2->pei2_2c=1;
        inData9 = inData9 | 0x08;
    }
    else
        recData2->pei2_2c=0;
    if (ui->checkBox_85->isChecked() == true){
        recData2->pei1_2c=1;
        inData9 = inData9 | 0x04;
    }
    else
        recData2->pei1_2c=0;
    if (ui->checkBox_86->isChecked() == true){
        recData2->pei2_1c=1;
        inData9 = inData9 | 0x02;
    }
    else
        recData2->pei2_1c=0;
    if (ui->checkBox_87->isChecked() == true){
        recData2->pei1_1c=1;
        inData9 = inData9 | 0x01;
    }
    else
        recData2->pei1_1c=0;

    SDDATA[15] = inData9;

    if (ui->checkBox_88->isChecked() == true){
        recData2->pei2_3t=1;
        inData10 += 0x20;
        qDebug() << inData10 << "값을 알려줘";
    }
    else
        recData2->pei2_3t=0;
    if (ui->checkBox_89->isChecked() == true){
        recData2->pei1_3t=1;
        inData10 = inData10 | 0x10;
    }
    else
        recData2->pei1_3t=0;
    if (ui->checkBox_90->isChecked() == true){
        recData2->pei2_2t=1;
        inData10 = inData10 | 0x08;
    }
    else
        recData2->pei2_2t=0;
    if (ui->checkBox_91->isChecked() == true){
        recData2->pei1_2t=1;
        inData10 = inData10 | 0x04;
    }
    else
        recData2->pei1_2t=0;
    if (ui->checkBox_92->isChecked() == true){
        recData2->pei2_1t=1;
        inData10 = inData10 | 0x02;
    }
    else
        recData2->pei2_1t=0;
    if (ui->checkBox_93->isChecked() == true){
        recData2->pei1_1t=1;
        inData10 += 0x01;
    }
    else
        recData2->pei1_1t=0;

    SDDATA[16] = inData10;

    recData2->VUD = ui->spinBox_29->value();
    SDDATA[19] = ui->spinBox_29->value();
    recData2->VDD = ui->spinBox_30->value();
    SDDATA[20] = ui->spinBox_30->value();
    recData2->etx = 0x03;
    SDDATA[21] = 0x03;

    if(ui->spinBox_32->value() > 0x00){
        recData2->bcc1=ui->spinBox_32->value();
        recData2->bcc2=ui->spinBox_33->value();
    }
    recData2->sp_Bit3=0;
    recData2->sp_Bit4=0;
    recData2->sp_Bit5=0;
    recData2->sp_Bit6=0;
    recData2->sp_Bit7=0;
    recData2->sp_byte1=0;
    recData2->sp_byte2=0;
    recData2->sp_byte3=0;
    recData2->sp_byte4=0;
    recData2->sp_byte5=0;


    for(int z=1; z<=21; z++)
    {
        if(z%2==0){
            bcc1 = bcc1^SDDATA[z];
        }
        else{
            bcc2 = bcc2^SDDATA[z];
            }
    }


    recData2->bcc1 = bcc1;
    recData2->bcc2 = bcc2;
    SDDATA[22] = bcc1;
    SDDATA[23] = bcc2;
    auto send = reinterpret_cast<char *>(SDDATA);
    QByteArray byteArray(send,sizeof(SD)); // 용도가?
    qDebug() << "write" << byteArray.toHex();
    serial->write(send,sizeof(SD));

    this->ui->textEdit_10->clear();
    this->ui->textEdit_10->insertHtml(byteArray.toHex());
}

void MainWindow::on_PUSH_2_clicked()
{
    pushWhile=0;
    qDebug()<< "pushWhile=" << pushWhile;
}

void MainWindow::on_PUSH_3_clicked()
{
    unsigned char SDRDATA[27];
    SDR *sendData2 = new SDR;
    unsigned char inData2 = 0x00;
    unsigned char inData3 = 0x00;
    unsigned char inData4 = 0x00;
    unsigned char bcc1 = 0x00;
    unsigned char bcc2 = 0x00;

    this->ui->textEdit_2->clear();
    QString time2 = QDateTime::currentDateTime().toString(Qt::TextDate);
    qDebug() << "현제시간 : "<< time2;
    time2.remove(0,2);
    month = time2.left(2);
    ui->spinBox_5->setValue(month.toInt());
    if (month.toInt()>=10)
        time2.remove(0,3);
    else
        time2.remove(0,2);
    day = time2.left(2);
    ui->spinBox_6->setValue(day.toInt());
    if (day.toInt()>=10)
        time2.remove(0,3);
    else
        time2.remove(0,2);
    hour = time2.left(2);
    ui->spinBox_7->setValue(hour.toInt());
    time2.remove(0,3);
    min = time2.left(2);
    ui->spinBox_8->setValue(min.toInt());
    time2.remove(0,3);
    sec = time2.left(2);
    ui->spinBox_9->setValue(sec.toInt());
    time2.remove(0,5);
    year = time2.left(2);
    ui->spinBox_4->setValue(year.toInt());
    time2.clear();

    this->ui->textEdit->setText( "Data set OK");

    sendData2->stx = 0x02; // STX 시작값
    SDRDATA[0] = sendData2->stx;
    sendData2->toAddress = ui->spinBox->value();//추진 제어 장치(통합제어장치)의 Data 주소는 70H 이다.
    SDRDATA[1] = ui->spinBox->value();
    sendData2->fromAddress = ui->spinBox_2->value();//열차 모니터 장치(TCMS)의 Data 주소는 10H이다.
    SDRDATA[2] = ui->spinBox_2->value();
    sendData2->dataType = ui->spinBox_3->value();// 데이터 타입
    SDRDATA[3] = ui->spinBox_3->value();
    sendData2->year = ui->spinBox_4->value();//year
    SDRDATA[4] = ui->spinBox_4->value();
    sendData2->month = ui->spinBox_5->value();//month
    SDRDATA[5] = ui->spinBox_5->value();
    sendData2->day = ui->spinBox_6->value();//day
    SDRDATA[6] = ui->spinBox_6->value();
    sendData2->hour = ui->spinBox_7->value();//hour
    SDRDATA[7] = ui->spinBox_7->value();
    sendData2->min = ui->spinBox_8->value();//minute
    SDRDATA[8] = ui->spinBox_8->value();
    sendData2->sec = ui->spinBox_9->value();//second
    SDRDATA[9] = ui->spinBox_9->value();
    sendData2->trainNum1 = ui->spinBox_10->value();
    SDRDATA[10] = ui->spinBox_10->value();
    sendData2->trainNum2 = ui->spinBox_11->value();
    SDRDATA[11] = ui->spinBox_11->value();
    sendData2->speed = ui->spinBox_12->value();// 속도 SPEED
    SDRDATA[12] = ui->spinBox_12->value();

    sendData2->curCode = ui->spinBox_13->value();// 현재역 코드 SURCODE
    SDRDATA[13] = ui->spinBox_13->value();

    sendData2->nxtCode = ui->spinBox_14->value();// 다음역 코드 NXTCODE
    SDRDATA[14] = ui->spinBox_14->value();

    sendData2->dstCode = ui->spinBox_15->value();// 종착역 코드 DSTCODE
    SDRDATA[15] = ui->spinBox_15->value();

    sendData2->runDist1 = ui->spinBox_16->value();// 주행거리 RUNDIST
    SDRDATA[16] = ui->spinBox_16->value();

    sendData2->runDist2 = ui->spinBox_17->value();// 주행거리 2
    SDRDATA[17] = ui->spinBox_17->value();

    sendData2->sp_byte0 = ui->spinBox_18->value(); // SPARE
    SDRDATA[18] = ui->spinBox_18->value();

    if (ui->checkBox_16->isChecked() == true) // 시험요청여부
        sendData2->test_start_req=1;
    sendData2->sp_bit0 = ui->spinBox_23->value(); // spare
    if (sendData2->test_start_req==1)
        SDRDATA[19] = sendData2->sp_bit0 + 0x80;
    else
        SDRDATA[19] = sendData2->sp_bit0;

    int countdata = ui->comboBox->currentIndex();//운전모드
    if (countdata == 0){sendData2->drive_DM=1;
                inData2 += 0x80;}
    else if (countdata == 1)   {sendData2->drive_AM=1;
                inData2 += 0x40;}
    else if (countdata == 2)   {sendData2->drive_MM=1;
                inData2 += 0x20;}
    else if (countdata == 3)   {sendData2->drive_EM=1;
                inData2 += 0x10;}

    if (ui->checkBox1->isChecked() == true){ // 구원운전 스위치
        sendData2->ros=1;
        inData2 += 0x08;
    }
    if (ui->checkBox_2->isChecked() == true){ // 정지속도 검출
        sendData2->zvr=1;
        inData2 += 0x04;
    }
    if (ui->checkBox_3->isChecked() == true){ // 3호차MC2측 HCR투입
        sendData2->mc2Hcr=1;
        inData2 += 0x02;
    }
    if (ui->checkBox_4->isChecked() == true){ // 1호차MC1측 HCR투입
        sendData2->mc1Hcr=1;
        inData2 += 0x01;
    }
    SDRDATA[20] = inData2; //

    if (ui->checkBox_5->isChecked() == true){ // 3호차 화재발생
        sendData2->fd3=1;
        inData3 += 0x80;
    }
    if (ui->checkBox_6->isChecked() == true){ // 2호차 화재발생
        sendData2->fd2=1;
        inData3 += 0x40;
    }
    if (ui->checkBox_7->isChecked() == true){ // 1호차 화재발생
        sendData2->fd1=1;
        inData3 += 0x20;
    }

    if (ui->checkBox_8->isChecked() == true){ // 전체 출입문 닫힘
        sendData2->dir=1;
        inData3 += 0x10;
        qDebug() << inData3 << "보낸값확인";
    }
    if (ui->checkBox_9->isChecked() == true){ // 3호차 2위 열림 계전기
        sendData2->dor2_3=1;
        inData3 += 0x08;
    }
    if (ui->checkBox_10->isChecked() == true){ // 3호차 1위 열림 계전기
        sendData2->dor1_3=1;
        inData3 += 0x04;
    }
    if (ui->checkBox_11->isChecked() == true){ // 1호차 2위 열림 계전기
        sendData2->dor2_1=1;
        inData3 += 0x02;
    }
    if (ui->checkBox_12->isChecked() == true){ // 1호차 1위 열림 계전기
        sendData2->dor1_1=1;
        inData3 += 0x01;
    }
    SDRDATA[21] = inData3;

    sendData2->sp_bit1 = ui->spinBox_24->value();
    inData4 += ui->spinBox_24->value();

    if (ui->checkBox_13->isChecked() == true){ //
        sendData2->dcw=1;
        inData4 += 0x04;
    }
    if (ui->checkBox_14->isChecked() == true){ //
        sendData2->dow2=1;
        inData4 += 0x02;
    }
    if (ui->checkBox_15->isChecked() == true){ //
        sendData2->dow1=1;
        inData4 += 0x04;
    }
    SDRDATA[22] = inData4;

    sendData2->lifeCnt = ui->spinBox_19->value();
    SDRDATA[23] = sendData2->lifeCnt;

    int additem= ui->spinBox_19->value();
    if (additem == 255)
        additem = 0;
    ui->spinBox_19->setValue(additem+1);

    sendData2->etx = 0x03;; // ETX 종료값
    SDRDATA[24] = sendData2->etx;

    for(int z=1; z<=24; z++)
    {
        if(z%2==0){
            bcc1 = bcc1^SDRDATA[z];
        }
        else{
            bcc2 = bcc2^SDRDATA[z];
            }
    }

    SDRDATA[25] = bcc2;
    sendData2->bcc1 = bcc2;
    SDRDATA[26] = bcc1; //BCC2
    sendData2->bcc2 = bcc1;


    auto send = reinterpret_cast<char *>(SDRDATA);
    serial->write(send,sizeof(SDR));

    QByteArray byteArray(send,sizeof(SDR)); // 용도가?
    qDebug() << "write" << byteArray.toHex();
    for(int i=0; i<=26; i++)
    {
        this->ui->textEdit_2->insertHtml(getStringFromUnsignedChar(SDRDATA[i]));
    }
    this->ui->textEdit_9->clear();
    this->ui->textEdit_9->insertHtml(getStringFromUnsignedChar(SDRDATA[ui->spinBox_34->value()]));
}
