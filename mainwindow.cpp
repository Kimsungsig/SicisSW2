#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qserialport.h>
#include <qserialportinfo.h>
#include <stdio.h>
#include <QLabel>
#include <QMessageBox>
#include <qDebug>
#include <Qstring>
#include <QtGui>
#include <QTime>
#include <windows.h>

QString year;
QString month;
QString day;
QString hour;
QString min;
QString sec;
QString sec2;
unsigned char SDRDATA[27];
unsigned char *SDRDATA2 = new unsigned char[27];

SDR sendData;

SDR *sendData2 = new SDR;

SD recData;

SD *recData2 = new SD;

QSerialPort *serial;

QString getStringFromUnsignedChar( unsigned char str ){
    QString result = "";

    QString s;
        s = QString( "%1" ).arg( str, 0, 16 );
        if( s.length() == 1 )
            result.append( "0" );

        result.append( s );

    return result;
}

void delay(int sec)
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
        connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(serial_received()));
        serial_rescan();
}

MainWindow::~MainWindow()
{
    delete ui;
    serial->close();
}

void MainWindow::serial_connect()
{
        serial->setPortName(ui->port_box->currentText());
        serial->setBaudRate(QSerialPort::Baud115200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
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
    //delay();
    this->ui->textEdit_3->clear();
    unsigned char bcc1 = 0x00;
    unsigned char bcc2 = 0x00;
    qDebug() << "Read ready data ↓";
    QByteArray read_Data;
    read_Data = serial->readAll();
    unsigned char* hex = new unsigned char[read_Data.size()];
    memcpy(hex, read_Data.constData(), read_Data.size());
    memcpy(&recData, read_Data.constData(),sizeof(SD));
    qDebug() << "Read data ↓";
    qDebug() << read_Data.size();
    qDebug() << hex;

    if (read_Data.size() < 24){
        qDebug() << "test2 ";
        QByteArray read_Data2;
        int resize = 24-read_Data.size();
        read_Data2 = serial->readLine();
        unsigned char* hex2 = new unsigned char[resize];
        memcpy(hex2, read_Data2.constData(), resize);
        qDebug() << "test3 " << resize;
        for (int j=0; j<resize; j++){
            hex[j] = 1;// 수정
        }
        for(int i=0; i<resize; i++){
            hex[i+read_Data2.size()] = hex2[i];
            qDebug() << hex[i+read_Data.size()];
            qDebug() << hex2[i];
        }
    }

    for(int i=0; i<24; i++){
        this->ui->textEdit_3->insertHtml(getStringFromUnsignedChar(hex[i]));
    }

    bcc1 = 0x00;
    bcc2 = 0x00;
    for(int z=1; z<=24; z++)
    {
        if(z%2==0){
            bcc1 = bcc1^hex[z];
        }
        else{
            bcc2 = bcc2^hex[z];
            }
    }

    if (bcc1 != recData.bcc1){
        qDebug() << "Not bcc1 samsam";
        qDebug() << recData.bcc1;
        qDebug() << bcc1;
    }
    if (bcc2 != recData.bcc2){
        qDebug() << "Not bcc2 samsam";
        qDebug() << recData.bcc2;
        qDebug() << bcc2;
    }

    if (recData.stx == 0x02)
        {
            ui->spinBox_20->setValue(recData.toAddress);
            ui->spinBox_21->setValue(recData.fromAddress);
            ui->spinBox_22->setValue(recData.dataType);
            if(recData.onbrc==1)
                ui->checkBox_30->setChecked(true);
            else
                ui->checkBox_30->setChecked(false);
            if(recData.slave==1)
                ui->checkBox_31->setChecked(true);
            else
                ui->checkBox_31->setChecked(false);
            if(recData.master==1)
                ui->checkBox_32->setChecked(true);
            else
                ui->checkBox_32->setChecked(false);
            if(recData.pa_Ok==1)
                ui->checkBox_33->setChecked(true);
            else
                ui->checkBox_33->setChecked(false);
            if(recData.pa_Ng==1)
                ui->checkBox_34->setChecked(true);
            else
                ui->checkBox_34->setChecked(false);
            if(recData.pis_Ok==1)
                ui->checkBox_35->setChecked(true);
            else
                ui->checkBox_35->setChecked(false);
            if(recData.pis_Ng==1)
                ui->checkBox_36->setChecked(true);
            else
                ui->checkBox_36->setChecked(false);
            if(recData.onTest==1)
                ui->checkBox_96->setChecked(true);
            else
                ui->checkBox_96->setChecked(false);

            ui->spinBox_31->setValue(recData.sp_byte1);

            if(recData.fcam1f==1)
                ui->checkBox_37->setChecked(true);
            else
                ui->checkBox_37->setChecked(false);
            if(recData.scam2_1f==1)
                ui->checkBox_38->setChecked(true);
            else
                ui->checkBox_38->setChecked(false);
            if(recData.scam1_1f==1)
                ui->checkBox_39->setChecked(true);
            else
                ui->checkBox_39->setChecked(false);
            if(recData.pei2_1f==1)
                ui->checkBox_40->setChecked(true);
            else
                ui->checkBox_40->setChecked(false);
            if(recData.pei1_1f==1)
                ui->checkBox_41->setChecked(true);
            else
                ui->checkBox_41->setChecked(false);
            if(recData.pamp1f==1)
                ui->checkBox_42->setChecked(true);
            else
                ui->checkBox_42->setChecked(false);
            if(recData.cop1f==1)
                ui->checkBox_43->setChecked(true);
            else
                ui->checkBox_43->setChecked(false);
            if(recData.avc1f==1)
                ui->checkBox_44->setChecked(true);
            else
                ui->checkBox_44->setChecked(false);

            if(recData.esw2_1f==1)
                ui->checkBox_45->setChecked(true);
            else
                ui->checkBox_45->setChecked(false);
            if(recData.esw1_1f==1)
                ui->checkBox_46->setChecked(true);
            else
                ui->checkBox_46->setChecked(false);
            if(recData.lcd4_1f==1)
                ui->checkBox_47->setChecked(true);
            else
                ui->checkBox_47->setChecked(false);
            if(recData.lcd3_1f==1)
                ui->checkBox_48->setChecked(true);
            else
                ui->checkBox_48->setChecked(false);
            if(recData.lcd2_1f==1)
                ui->checkBox_49->setChecked(true);
            else
                ui->checkBox_49->setChecked(false);
            if(recData.lcd1_1f==1)
                ui->checkBox_50->setChecked(true);
            else
                ui->checkBox_50->setChecked(false);
            if(recData.led2_1f==1)
                ui->checkBox_51->setChecked(true);
            else
                ui->checkBox_51->setChecked(false);
            if(recData.led1_1f==1)
                ui->checkBox_52->setChecked(true);
            else
                ui->checkBox_52->setChecked(false);

            if(recData.sp_Bit4==1)
                ui->checkBox_25->setChecked(true);
            else
                ui->checkBox_25->setChecked(false);
            if(recData.scam2_2f==1)
                ui->checkBox_53->setChecked(true);
            else
                ui->checkBox_53->setChecked(false);
            if(recData.scam1_2f==1)
                ui->checkBox_54->setChecked(true);
            else
                ui->checkBox_54->setChecked(false);
            if(recData.pei2_2f==1)
                ui->checkBox_55->setChecked(true);
            else
                ui->checkBox_55->setChecked(false);
            if(recData.pei1_2f==1)
                ui->checkBox_56->setChecked(true);
            else
                ui->checkBox_56->setChecked(false);
            if(recData.pamp2f==1)
                ui->checkBox_57->setChecked(true);
            else
                ui->checkBox_57->setChecked(false);

            if(recData.esw2_2f==1)
                ui->checkBox_58->setChecked(true);
            else
                ui->checkBox_58->setChecked(false);
            if(recData.esw1_2f==1)
                ui->checkBox_59->setChecked(true);
            else
                ui->checkBox_59->setChecked(false);
            if(recData.lcd4_2f==1)
                ui->checkBox_60->setChecked(true);
            else
                ui->checkBox_60->setChecked(false);
            if(recData.lcd3_2f==1)
                ui->checkBox_61->setChecked(true);
            else
                ui->checkBox_61->setChecked(false);
            if(recData.lcd2_2f==1)
                ui->checkBox_62->setChecked(true);
            else
                ui->checkBox_62->setChecked(false);
            if(recData.lcd1_2f==1)
                ui->checkBox_63->setChecked(true);
            else
                ui->checkBox_63->setChecked(false);
            if(recData.led2_2f==1)
                ui->checkBox_64->setChecked(true);
            else
                ui->checkBox_64->setChecked(false);
            if(recData.led1_2f==1)
                ui->checkBox_65->setChecked(true);
            else
                ui->checkBox_65->setChecked(false);

            if(recData.fcam3f==1)
                ui->checkBox_66->setChecked(true);
            else
                ui->checkBox_66->setChecked(false);
            if(recData.scan2_3f==1)
                ui->checkBox_67->setChecked(true);
            else
                ui->checkBox_67->setChecked(false);
            if(recData.scan1_3f==1)
                ui->checkBox_68->setChecked(true);
            else
                ui->checkBox_68->setChecked(false);
            if(recData.pei2_3f==1)
                ui->checkBox_69->setChecked(true);
            else
                ui->checkBox_69->setChecked(false);
            if(recData.pei1_3f==1)
                ui->checkBox_70->setChecked(true);
            else
                ui->checkBox_70->setChecked(false);
            if(recData.pemp3f==1)
                ui->checkBox_71->setChecked(true);
            else
                ui->checkBox_71->setChecked(false);
            if(recData.cop3f==1)
                ui->checkBox_72->setChecked(true);
            else
                ui->checkBox_72->setChecked(false);
            if(recData.avc3f==1)
                ui->checkBox_73->setChecked(true);
            else
                ui->checkBox_73->setChecked(false);

            if(recData.esw2_3f==1)
                ui->checkBox_74->setChecked(true);
            else
                ui->checkBox_74->setChecked(false);
            if(recData.esw1_3f==1)
                ui->checkBox_75->setChecked(true);
            else
                ui->checkBox_75->setChecked(false);
            if(recData.lcd4_3f==1)
                ui->checkBox_76->setChecked(true);
            else
                ui->checkBox_76->setChecked(false);
            if(recData.lcd3_3f==1)
                ui->checkBox_77->setChecked(true);
            else
                ui->checkBox_77->setChecked(false);
            if(recData.lcd2_3f==1)
                ui->checkBox_78->setChecked(true);
            else
                ui->checkBox_78->setChecked(false);
            if(recData.lcd1_3f==1)
                ui->checkBox_79->setChecked(true);
            else
                ui->checkBox_79->setChecked(false);
            if(recData.led2_3f==1)
                ui->checkBox_80->setChecked(true);
            else
                ui->checkBox_80->setChecked(false);
            if(recData.led1_3f==1)
                ui->checkBox_81->setChecked(true);
            else
                ui->checkBox_81->setChecked(false);

            ui->spinBox_25->setValue(recData.sp_byte2);
            ui->spinBox_26->setValue(recData.sp_byte3);

            //if(recData.sp_Bit6 > 0)
            //    ui->radioButton_57->setChecked(true);
            if(recData.pei2_3c==1)
                ui->checkBox_82->setChecked(true);
            else
                ui->checkBox_82->setChecked(false);
            if(recData.pei1_3c==1)
                ui->checkBox_83->setChecked(true);
            else
                ui->checkBox_83->setChecked(false);
            if(recData.pei2_2c==1)
                ui->checkBox_84->setChecked(true);
            else
                ui->checkBox_84->setChecked(false);
            if(recData.pei1_2c==1)
                ui->checkBox_85->setChecked(true);
            else
                ui->checkBox_85->setChecked(false);
            if(recData.pei2_1c==1)
                ui->checkBox_86->setChecked(true);
            else
                ui->checkBox_86->setChecked(false);
            if(recData.pei1_1c==1)
                ui->checkBox_87->setChecked(true);
            else
                ui->checkBox_87->setChecked(false);

            //if(recData.sp_Bit7 > 0)
            //    ui->radioButton_57->setChecked(true);
            if(recData.pei2_3t==1)
                ui->checkBox_88->setChecked(true);
            else
                ui->checkBox_88->setChecked(false);
            if(recData.pei1_3t==1)
                ui->checkBox_89->setChecked(true);
            else
                ui->checkBox_89->setChecked(false);
            if(recData.pei2_2t==1)
                ui->checkBox_90->setChecked(true);
            else
                ui->checkBox_90->setChecked(false);
            if(recData.pei1_2t==1)
                ui->checkBox_91->setChecked(true);
            else
                ui->checkBox_90->setChecked(false);
            if(recData.pei2_1t==1)
                ui->checkBox_92->setChecked(true);
            else
                ui->checkBox_92->setChecked(false);
            if(recData.pei1_1t==1)
                ui->checkBox_93->setChecked(true);
            else
                ui->checkBox_93->setChecked(false);

        ui->spinBox_27->setValue(recData.sp_byte4);
        ui->spinBox_28->setValue(recData.sp_byte5);

        ui->spinBox_29->setValue(recData.VUD);
        ui->spinBox_30->setValue(recData.VDD);

        ui->spinBox_32->setValue(recData.bcc1);
        ui->spinBox_33->setValue(recData.bcc2);
    }
    else{
        this->ui->textEdit_3->insertHtml("not data start 02");
    }
}

//void MainWindow::readDateSet(QString data, int j)
//{
//    QString test = "0x01" ;
//    unsigned char test2[1];
//    memcpy( test2, data.toStdString().c_str() ,data.size());

//    QByteArray ba = data.toLocal8Bit();
//    //unsigned char check1 = 0x80;
//    unsigned char check = 0x00;

//     //헥사변환해야되는경우 확인필요.
//    if (j==0)      ui->spinBox_20->setValue(data.toInt());//02
//    else if (j==1) ui->spinBox_20->setValue(data.toInt());//10 TCMS주소
//    else if (j==2)   ui->spinBox_21->setValue(data.toInt());//70 통합제어장치주소
//    else if (j==3)   ui->spinBox_22->setValue(data.toInt());//20 데이터 타입

//    if(j==4){
//        qDebug() <<test2;
//        if(test2[1] & check)
//        {

//        }
//    }
//    //ui->radioButton->click();//방송중
//    //ui->radioButton_2->click();

//}

void MainWindow::on_PUSH_clicked()
{
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

    QThread thread;
    while(1)
    {
        qDebug() << "ONE";
        // 여기서 샛강~서울대 시나리오값 인자로 넘겨서 각각 값에 따라 처리하도록 한다.
        TM.trainnum1 = 0x03;
        TM.trainnum2 = 0x76;// 열차번호정보
        TM.dstcode = 0x6F; // 종착역 111
        qDebug() << countdata;
        if(countdata==0) {TM.trainspeed = 0x00; // 첫번째역 출발
                TM.curcode = 0x65;
                TM.nxtcode = 0x66;
                TM.rundist2 = 0x00;
                TM.DIR=1;
        }
        else if(countdata==1) {TM.trainspeed = 0x0a; // 속도 10
                TM.curcode = 0x65;
                TM.nxtcode = 0x66;
                TM.rundist2 = 0x0a;
                TM.DIR=1;}
        else if(countdata==2) {TM.trainspeed = 0x14; // 속도 15
                TM.curcode = 0x65;
                TM.nxtcode = 0x66;
                TM.rundist2 = 0x0b;
                TM.DIR=1;}
        else if(countdata==3) {TM.trainspeed = 0x0a; // 속도 10
                TM.curcode = 0x65;
                TM.nxtcode = 0x66;
                TM.rundist2 = 0x0c;
                TM.DIR=1;}
        else if(countdata==4) {TM.trainspeed = 0x00; // 멈춤, 도어열림
                TM.curcode = 0x65;
                TM.nxtcode = 0x66;
                TM.rundist2 = 0x0d;
                TM.DIR=0;}
        else if(countdata==5) {TM.trainspeed = 0x00; // 도어닫힘
                TM.curcode = 0x65;
                TM.nxtcode = 0x66;
                TM.rundist2 = 0x0d;
                TM.DIR=1;}
        else if(countdata==6) {TM.trainspeed = 0x0a; // 속도 10
                TM.curcode = 0x66;
                TM.nxtcode = 0x67;
                TM.rundist2 = 0x0e;
                TM.DIR=1;}
        else if(countdata==7) {TM.trainspeed = 0x14; // 속도 15
                TM.curcode = 0x66;
                TM.nxtcode = 0x67;
                TM.rundist2 = 0x0f;
                TM.DIR=1;}
        else if(countdata==8) {TM.trainspeed = 0x0a; // 속도 10
                TM.curcode = 0x66;
                TM.nxtcode = 0x67;
                TM.rundist2 = 0x10;
                TM.DIR=1;}
        else if(countdata==9) {TM.trainspeed = 0x00; // 멈춤, 도어열림
                TM.curcode = 0x66;
                TM.nxtcode = 0x67;
                TM.rundist2 = 0x11;
                TM.DIR=0;}
        else if(countdata==10) {TM.trainspeed = 0x00; // 도어닫힘
                TM.curcode = 0x66;
                TM.nxtcode = 0x67;
                TM.rundist2 = 0x11;
                TM.DIR=1;}
        else if(countdata==11) {TM.trainspeed = 0x0a; // 속도 10
                TM.curcode = 0x67;
                TM.nxtcode = 0x68;
                TM.rundist2 = 0x11;
                TM.DIR=1;}
        else if(countdata==12) {TM.trainspeed = 0x14; // 속도 15
                TM.curcode = 0x67;
                TM.nxtcode = 0x68;
                TM.rundist2 = 0x11;
                TM.DIR=1;}
        else if(countdata==13) {TM.trainspeed = 0x14; // 속도 15
                TM.curcode = 0x67;
                TM.nxtcode = 0x68;
                TM.rundist2 = 0x11;
                TM.DIR=1;}
        else if(countdata==14) {TM.trainspeed = 0x14; // 속도 10
                TM.curcode = 0x67;
                TM.nxtcode = 0x68;
                TM.rundist2 = 0x11;
                TM.DIR=1;}
        else if(countdata==15) {TM.trainspeed = 0x00; // 멈춤, 도어열림
                TM.curcode = 0x67;
                TM.nxtcode = 0x68;
                TM.rundist2 = 0x11;
                TM.DIR=0;}
        else if(countdata==16) {TM.trainspeed = 0x00; // 도어닫힘
                TM.curcode = 0x67;
                TM.nxtcode = 0x68;
                TM.rundist2 = 0x11;
                TM.DIR=1;}


        if (TM.trainspeed == 0x00){
             TM.DCW=0;
             TM.DOW1=0;
        }
        else{
             TM.DCW=1;
             TM.DOW1=1;
        }
        on_textEdit_destroyed(TM);
        countdata++;
         qDebug() << "two";
         delay(3);
    }
}

void MainWindow::on_textEdit_destroyed(struct trainMacro data)
{
    unsigned char inData2 = 0x00;
    unsigned char inData3 = 0x00;
    unsigned char inData4 = 0x00;
    unsigned char bcc1 = 0x00;
    unsigned char bcc2 = 0x00;

    this->ui->textEdit_2->clear();
    QString time2 = QDateTime::currentDateTime().toString(Qt::TextDate);
    qDebug() << time2;
    time2.remove(0,2);
    month = time2.left(2);
    ui->spinBox_5->setValue(month.toInt());
    time2.remove(0,3);
    day = time2.left(2);
    ui->spinBox_6->setValue(day.toInt());
    time2.remove(0,3);
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
    sendData2->dataType = ui->spinBox_3->value();// Data타입  Data 요청 과 응답이 같네...?
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
            //ui->spinBox_10->value();
    sendData2->trainNum2 = data.trainnum2;
    ui->spinBox_11->setValue(data.trainnum1);
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
    ui->spinBox_16->setValue(data.rundist2);

    sendData2->sp_byte0 = ui->spinBox_18->value(); // SPARE
    SDRDATA[18] = ui->spinBox_18->value();

    if (ui->checkBox_16->isChecked() == true) // 시험요청여부
        sendData2->test_start_req=1;
    sendData2->sp_bit0 = ui->spinBox_23->value(); // spare
    if (sendData2->test_start_req==1)
        SDRDATA[19] = sendData2->sp_bit0 + 0x80;
    else
        SDRDATA[19] = sendData2->sp_bit0;

    int countdata = ui->comboBox->count();//운전모드
    if (countdata == 0){sendData2->drive_DM=1;
                inData2 += 0x80;}
    else if (countdata == 1)   {sendData2->drive_AM=1;
                inData2 += 0x40;}
    else if (countdata == 2)   {sendData2->drive_MM=1;
                inData2 += 0x20;}
    else if (countdata == 1)   {sendData2->drive_EM=1;
                inData2 += 0x10;}

    //QCheckBox
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

    SDRDATA[25] = bcc1;
    sendData2->bcc1 = bcc1;
    SDRDATA[26] = bcc2; //BCC2
    sendData2->bcc2 = bcc2;

    auto send = reinterpret_cast<char *>(sendData2);
    serial->write(send,sizeof(SDR));
    QByteArray byteArray(send,sizeof(SDR)); // 용도가?
    qDebug() << "write" << byteArray.toHex();
    for(int i=0; i<=26; i++)
    {
        this->ui->textEdit_2->insertHtml(getStringFromUnsignedChar(SDRDATA[i]));
    }

}


void MainWindow::on_pushButton_2_clicked()
{
    recData2->stx = 0x02;
    recData2->toAddress = 0x10;
    recData2->fromAddress = 0x70;
    recData2->dataType= 0x20;
    recData2->onbrc=1;
    recData2->slave=0;
    recData2->master=1;

    recData2->pa_Ok=1;
    recData2->pa_Ng=0;
    recData2->pis_Ok=0;
    recData2->pis_Ng=1;
    recData2->onTest=1;

    //recData2->sp_byte;

    recData2->fcam1f=1;
    recData2->scam2_1f=1;
    recData2->scam1_1f=1;
    recData2->pei2_1f=1;
    recData2->pei1_1f=1;
    recData2->pamp1f=1;
    recData2->cop1f=1;
    recData2->avc1f=1;

    recData2->esw2_1f=1;
    recData2->esw1_1f=1;
    recData2->lcd4_1f=1;
    recData2->lcd3_1f=1;
    recData2->lcd2_1f=1;
    recData2->lcd1_1f=1;
    recData2->led2_1f=1;
    recData2->led1_1f=1;

    recData2->sp_Bit4=1;
    recData2->scam2_2f=1;
    recData2->scam1_2f=1;
    recData2->pei2_2f=1;
    recData2->pei1_2f=1;
    recData2->pamp2f=1;

    recData2->esw2_2f=1;
    recData2->esw1_2f=1;
    recData2->lcd4_2f=1;
    recData2->lcd3_2f=1;
    recData2->lcd2_2f=1;
    recData2->lcd1_2f=1;
    recData2->led2_2f=1;
    recData2->led1_2f=1;

    recData2->fcam3f=1;
    recData2->scan2_3f=1;
    recData2->scan1_3f=1;
    recData2->pei2_3f=1;
    recData2->pei1_3f=1;
    recData2->pemp3f=1;
    recData2->cop3f=1;
    recData2->avc3f=1;

    recData2->esw2_3f=1;
    recData2->esw1_3f=1;
    recData2->lcd4_3f=1;
    recData2->lcd3_3f=1;
    recData2->lcd2_3f=1;
    recData2->lcd1_3f=1;
    recData2->led2_3f=1;
    recData2->led1_3f=1;

    recData2->pei2_3c=1;
    recData2->pei1_3c=1;
    recData2->pei2_2c=1;
    recData2->pei1_2c=1;
    recData2->pei2_1c=1;
    recData2->pei1_1c=1;

    recData2->pei2_3t=1;
    recData2->pei1_3t=1;
    recData2->pei2_2t=1;
    recData2->pei1_2t=1;
    recData2->pei2_1t=1;
    recData2->pei1_1t=1;

    recData2->VUD=0x20;
    recData2->VDD=0x10;

    //   for(int z=1; z<=24; z++)
    //   {
    //   if(z%2=0){
    //   bcc1 = bcc1^recData[z];
    //   }
    //   else{
    //   bcc2 = bcc2^recData[z];
    //   }
    //   }
    recData2->etx = 0x03;

    recData2->bcc1=0xBD;
    recData2->bcc2=0x3C;

    auto send = reinterpret_cast<char *>(recData2);
    serial->write(send,sizeof(SD));
    QByteArray byteArray(send,sizeof(SD)); // 용도가?
    qDebug() << "write" << byteArray.toHex();
    this->ui->textEdit_2->clear();
    this->ui->textEdit_2->insertHtml(byteArray.toHex());

}
