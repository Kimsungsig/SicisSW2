#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qserialport.h>
#include <qserialportinfo.h>
#include <stdio.h>
#include <QLabel>
//#include <Qtime>
#include <QMessageBox>
#include <qDebug>
#include <Qstring>
#include <QtGui>
#include <QThread>
#include <windows.h>
#include <define_protocol.h>

QString year;
QString month;
QString day;
QString hour;
QString min;
QString sec;
QString sec2;
bool secCheck=false;
//unsigned char *SDRDATA = new unsigned char[27]; // SDR 내보내는 데이터
unsigned char SDRDATA[27];
unsigned char *SDRDATA2 = new unsigned char[27];
//unsigned char SDDATA[27];
//unsigned char *SD = new unsigned char[27]; // SD 받는 값 저장용

SDR sendData;

SDR *sendData2 = new SDR;

SD recData;

SD *recData2 = new SD;

QSerialPort *serial;

QString getStringFromUnsignedChar( unsigned char str ){
    QString result = "";

    QString s;
        s = QString( "%1" ).arg( str, 0, 16 );
        // account for single-digit hex values (always must serialize as two digits)
        if( s.length() == 1 )
            result.append( "0" );

        result.append( s );

    return result;
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

void MainWindow::send_test()
{
    serial->write("test!\n");
}

void MainWindow::serial_received()
{
//    usleep(10000);
//    QString received = serial->readAll();
    //QString received( reinterpret_cast< char* >( SD ) );

    /*QString received;
    received = serial->readLine(); // 한번에 전체를 읽어오는것으로 보임. 확인필요
    QStringList splitted = received.split(";");*/
    QString received;
    QString receivedData[27];
    unsigned char bcc1 = 0x00;
    unsigned char bcc2 = 0x00;
    //unsigned char sds[27];
    //serial->readLine(sds,27);
    //serial->readBytes(dataBuf, 15);

            //나중에 데이터가 짤려들어오는 내용에 대한 처리도 해줄것.. 길이체크, 03마지막내용 체크,,

    QByteArray read_Data;
    read_Data = serial->readAll();

    //y::p SDDATA = new y::t[read_Data.size()];
    unsigned char *SDDATA = new unsigned char[read_Data.size()];
    memcpy(SDDATA, read_Data, read_Data.size());

    received = serial->readLine();
    //현제 들어오는 데이터가 없으므로 임시데이터를 생성.
    received = "02107020061500FFAA0405550000001215000001005353"; // -> 이거 안먹혀 02가 48?
    //SDR recData;
    memcpy(&recData, received.toLocal8Bit().constData(), sizeof(SDR));
    //memcpy(&SDDATA, received.toLocal8Bit().constData(), sizeof(SDR));
    this->ui->textEdit_3->insertHtml(received);

//    for (int k=0;k<27;k++) // 1. 일단 전체값을 모두 1바이트단위로 쪼개서 넣는다. 이후 비트단위로 계산?
//    {
//        receivedData[k]=received.left(2);
//        received.remove(0,2);
//    }

    for(int z=1; z<=24; z++)
    {
        if(z%2==0){
            bcc1 = bcc1^SDDATA[z];
        }
        else{
            bcc2 = bcc2^SDDATA[z];
            }
    }

    qDebug() << received;

    if (bcc1 != recData.bcc1){
        qDebug() << "Not bcc1 samsam";
        qDebug() << recData.bcc1;
    }
    if (bcc2 != recData.bcc2){
        qDebug() << "Not bcc2 samsam";
        qDebug() << recData.bcc2;
    }

    qDebug() << recData.stx; //48
    qDebug() << recData.toAddress;
    //qDebug() << QByteArray;
    //qDebug() << SDDATA[1];

    if (recData.stx == 0x02)
    {
        ui->spinBox_20->setValue(recData.toAddress);
        ui->spinBox_21->setValue(recData.fromAddress);
        ui->spinBox_22->setValue(recData.dataType);
        if(recData.onbrc==1)
            ui->radioButton->click();
        if(recData.slave==1)
            ui->radioButton_2->click();
        if(recData.master==1)
            ui->radioButton_3->click();

        if(recData.pa_Ok==1)
            ui->radioButton_4->click();
        if(recData.pa_Ng==1)
            ui->radioButton_5->click();
        if(recData.pis_Ok==1)
            ui->radioButton_6->click();
        if(recData.pis_Ng==1)
            ui->radioButton_7->click();
        if(recData.onTest==1)
            ui->radioButton_8->click();

        ui->spinBox_31->setValue(recData.sp_byte1);

        if(recData.fcam1f==1)
            ui->radioButton_9->click();
        if(recData.scam2_1f==1)
            ui->radioButton_10->click();
        if(recData.scam1_1f==1)
            ui->radioButton_11->click();
        if(recData.pei2_1f==1)
            ui->radioButton_12->click();
        if(recData.pei1_1f==1)
            ui->radioButton_13->click();
        if(recData.pamp1f==1)
            ui->radioButton_14->click();
        if(recData.cop1f==1)
            ui->radioButton_15->click();
        if(recData.avc1f==1)
            ui->radioButton_16->click();

        if(recData.esw2_1f==1)
            ui->radioButton_17->click();
        if(recData.esw1_1f==1)
            ui->radioButton_18->click();
        if(recData.lcd4_1f==1)
            ui->radioButton_19->click();
        if(recData.lcd3_1f==1)
            ui->radioButton_20->click();
        if(recData.lcd2_1f==1)
            ui->radioButton_21->click();
        if(recData.lcd1_1f==1)
            ui->radioButton_22->click();
        if(recData.led2_1f==1)
            ui->radioButton_23->click();
        if(recData.led1_1f==1)
            ui->radioButton_24->click();

        if(recData.sp_Bit4==1)
            ui->radioButton_25->click();
        if(recData.scam2_2f==1)
            ui->radioButton_25->click();
        if(recData.scam1_2f==1)
            ui->radioButton_26->click();
        if(recData.pei2_2f==1)
            ui->radioButton_27->click();
        if(recData.pei1_2f==1)
            ui->radioButton_28->click();
        if(recData.pamp2f==1)
            ui->radioButton_29->click();

        if(recData.esw2_2f==1)
            ui->radioButton_33->click();
        if(recData.esw1_2f==1)
            ui->radioButton_34->click();
        if(recData.lcd4_2f==1)
            ui->radioButton_35->click();
        if(recData.lcd3_2f==1)
            ui->radioButton_36->click();
        if(recData.lcd2_2f==1)
            ui->radioButton_37->click();
        if(recData.lcd1_2f==1)
            ui->radioButton_38->click();
        if(recData.led2_2f==1)
            ui->radioButton_39->click();
        if(recData.led1_2f==1)
            ui->radioButton_40->click();

        if(recData.fcam3f==1)
            ui->radioButton_41->click();
        if(recData.scan2_3f==1)
            ui->radioButton_42->click();
        if(recData.scan1_3f==1)
            ui->radioButton_43->click();
        if(recData.pei2_3f==1)
            ui->radioButton_44->click();
        if(recData.pei1_3f==1)
            ui->radioButton_45->click();
        if(recData.pemp3f==1)
            ui->radioButton_46->click();
        if(recData.cop3f==1)
            ui->radioButton_47->click();
        if(recData.avc3f==1)
            ui->radioButton_48->click();

        if(recData.esw2_3f==1)
            ui->radioButton_49->click();
        if(recData.esw1_3f==1)
            ui->radioButton_50->click();
        if(recData.lcd4_3f==1)
            ui->radioButton_51->click();
        if(recData.lcd3_3f==1)
            ui->radioButton_52->click();
        if(recData.lcd2_3f==1)
            ui->radioButton_53->click();
        if(recData.lcd1_3f==1)
            ui->radioButton_54->click();
        if(recData.led2_3f==1)
            ui->radioButton_55->click();
        if(recData.led1_3f==1)
            ui->radioButton_56->click();

        ui->spinBox_25->setValue(recData.sp_byte2);
        ui->spinBox_26->setValue(recData.sp_byte3);

        //if(recData.sp_Bit6 > 0)
        //    ui->radioButton_57->click();
        if(recData.pei2_3c==1)
            ui->radioButton_57->click();
        if(recData.pei1_3c==1)
            ui->radioButton_58->click();
        if(recData.pei2_2c==1)
            ui->radioButton_59->click();
        if(recData.pei1_2c==1)
            ui->radioButton_60->click();
        if(recData.pei2_1c==1)
            ui->radioButton_61->click();
        if(recData.pei1_1c==1)
            ui->radioButton_62->click();

        //if(recData.sp_Bit7 > 0)
        //    ui->radioButton_57->click();
        if(recData.pei2_3t==1)
            ui->radioButton_63->click();
        if(recData.pei1_3t==1)
            ui->radioButton_64->click();
        if(recData.pei2_2t==1)
            ui->radioButton_65->click();
        if(recData.pei1_2t==1)
            ui->radioButton_66->click();
        if(recData.pei2_1t==1)
            ui->radioButton_67->click();
        if(recData.pei1_1t==1)
            ui->radioButton_68->click();

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

void MainWindow::readDateSet(QString data, int j)
{
    QString test = "0x01" ;
    unsigned char test2[1];
    memcpy( test2, data.toStdString().c_str() ,data.size());

    QByteArray ba = data.toLocal8Bit();
    unsigned char check1 = 0x80;
    unsigned char check = 0x00;

    switch (j){ //헥사변환해야되는경우 확인필요.
    //case 0:   ui->spinBox_20->setValue(data.toInt());//02
    case 1:   ui->spinBox_20->setValue(data.toInt());//10 TCMS주소
    case 2:   ui->spinBox_21->setValue(data.toInt());//70 통합제어장치주소
    case 3:   ui->spinBox_22->setValue(data.toInt());//20 데이터 타입
    }
    if(j==4){
        qDebug() <<test2;
        if(test2[1] & check)
        {

        }
    }
    //ui->radioButton->click();//방송중
    //ui->radioButton_2->click();

}

void MainWindow::on_PUSH_clicked()
{
    //while(1)
    //{
        on_textEdit_destroyed();
        //QThread::sleep(1*10000);
/*        if (secCheck == false) // 1s 통신단위 찾으면 그것으로 딜레이 넣어줄것
            sec2 = time2.left(2);
        else{
            sec2 = time2.left(2);
            if(sec2!=sec)
                on_textEdit_destroyed();
        }*/
        //on_textEdit_destroyed();
    //};
}

void MainWindow::on_textEdit_destroyed()
{
    //unsigned char indata1 = 0x00;
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

    secCheck = true;

    this->ui->textEdit->setText( "Data set OK");

    sendData2->stx = 0x02; // STX 시작값
    SDRDATA[0] = sendData2->stx;
    // 1~ N text 데이터값 부근.
    sendData2->toAddress = ui->spinBox->value();//추진 제어 장치(통합제어장치)의 Data 주소는 70H 이다.
    SDRDATA[1] = ui->spinBox->value();
    sendData2->fromAddress = ui->spinBox_2->value();//열차 모니터 장치(TCMS)의 Data 주소는 10H이다.
        SDRDATA[2] = ui->spinBox->value();
        sendData2->dataType = ui->spinBox_3->value();// Data타입  Data 요청 과 응답이 같네...?
        SDRDATA[3] = ui->spinBox_3->value();
        //날짜정보↓ 나중에 실제값으로 넣을것
        //SDR[4] = ui->spinBox_4->value();
        //unsigned char da22 = ui->spinBox_4->value();
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
        sendData2->trainNum1 = ui->spinBox_10->value(); // 열차번호정보 TRNO MSB
        SDRDATA[10] = ui->spinBox_10->value();
        sendData2->trainNum2 = ui->spinBox_11->value(); // 열차번호정보 TRNO LSB
        SDRDATA[11] = ui->spinBox_11->value();
        sendData2->speed = ui->spinBox_12->value(); // 속도 SPEED
        SDRDATA[12] = ui->spinBox_12->value();
        sendData2->curCode = ui->spinBox_13->value(); // 현재역 코드 SURCODE
        SDRDATA[13] = ui->spinBox_13->value();
        sendData2->nxtCode = ui->spinBox_14->value(); // 다음역 코드 NXTCODE
        SDRDATA[14] = ui->spinBox_14->value();
        sendData2->dstCode = ui->spinBox_15->value(); // 종착역 코드 DSTCODE
        SDRDATA[15] = ui->spinBox_15->value();
        sendData2->runDist1 = ui->spinBox_16->value(); // 주행거리 RUNDIST
        SDRDATA[16] = ui->spinBox_16->value();
        sendData2->runDist2 = ui->spinBox_17->value(); // 주행거리 2
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

        int countdata = ui->comboBox->count();
        switch (countdata){ //운전모드
        case 0:   sendData2->drive_DM=1;
                    inData2 += 0x80;
        case 1:   sendData2->drive_AM=1;
                    inData2 += 0x40;
        case 2:   sendData2->drive_MM=1;
                    inData2 += 0x20;
        case 3:   sendData2->drive_EM=1;
                    inData2 += 0x10;
        }
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
    // send 할것
        /*
        QString writeData;
        //QString receivedData[27];
        memcpy(&SDRDATA, writeData.toLocal8Bit().constData(), sizeof(writeData));
        */
    QByteArray *write_Data = new QByteArray[27];
    write_Data->insert(26,1);
    //memcpy(&write_Data, SDRDATA2, write_Data->size());
    for(int i=0; i<=26; i++)
    {
        memcpy(write_Data, SDRDATA2, write_Data->size());
        serial->write(write_Data[i]);
//        QString chData = getStringFromUnsignedChar(SDRDATA[i]);

//        serial->write(chData.toStdString().c_str(), chData.size());

        this->ui->textEdit_2->insertHtml(write_Data[i]);
//        this->ui->textEdit_2->insertHtml(getStringFromUnsignedChar(SDRDATA[i]));
//        this->ui->textEdit_2->insertHtml("\n ");
//        qDebug() << getStringFromUnsignedChar(SDRDATA[i]);
//        qDebug() << chData.toStdString().c_str();
        qDebug() << i;
        qDebug() << write_Data[i];
        qDebug() << SDRDATA2[i];
    }

//    auto sendData3 = reinterpret_cast<char*>(sendData2);
//    qDebug() << sendData2->stx;
//    qDebug() << sendData2;
//    serial->write(sendData3,sizeof(SDR));


}

