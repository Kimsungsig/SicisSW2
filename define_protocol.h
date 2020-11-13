#ifndef DEFINE_PROTOCOL_H
#define DEFINE_PROTOCOL_H
#include <QtGlobal>
#include <QString>

#pragma pack(push, 1)

typedef struct trainMacro{
    unsigned char trainnum1 = 0x00;
    unsigned char trainnum2 = 0x00;
    unsigned char trainspeed = 0x00;
    unsigned char curcode = 0x00;
    unsigned char nxtcode = 0x00;
    unsigned char dstcode = 0x00;
    unsigned char rundist1 = 0x00;
    unsigned char rundist2 = 0x00;
    uchar DCW:1;
    uchar DOW2:1;
    uchar DOW1:1;
    uchar DIR:1;
}trainMacro;


typedef struct SDR{
    uchar stx;			     /*Start data = 0x02*/
    uchar toAddress;         /*To Address = 0x70*/
    uchar fromAddress;       /*From TCMSAddress = 0x10*/
    uchar dataType;          /*datatype*/
    uchar year;              /*year*/
    uchar month;             /*month*/
    uchar day;               /*day*/
    uchar hour;              /*hour*/
    uchar min;               /*minute*/
    uchar sec;               /*second*/
    uchar trainNum1;         /*train number 1*/
    uchar trainNum2;         /*train number 2*/
    uchar speed;             /*train speed*/
    uchar curCode;           /*now station code*/
    uchar nxtCode;           /*next station code*/
    uchar dstCode;           /*final station code*/
    uchar runDist1;           /*distance driven1*/
    uchar runDist2;           /*distance driven2*/

    uchar sp_byte0;          /* Spare BYTE 0 */

    uchar test_start_req:1;  /*Test start request*/
    uchar sp_bit0:7;         /* Spare bit 0 */

    uchar drive_DM:1;        /*Drive mode Driverless Mode*/
    uchar drive_AM:1;        /*Drive mode Auto Mode*/
    uchar drive_MM:1;        /*Drive mode Manual Mode*/
    uchar drive_EM:1;        /*Drive mode Emergency Mode*/
    uchar ros:1;             /*Rescue Operation Switch*/
    uchar zvr:1;             /*Zero Velocity Relay*/
    uchar mc2Hcr:1;          /*Mc2 side HCR ON*/
    uchar mc1Hcr:1;          /*Mc1 side HCR ON*/

    uchar fd3:1;             /*Fire Detection of Car 3*/
    uchar fd2:1;             /*Fire Detection of Car 2*/
    uchar fd1:1;             /*Fire Detection of Car 1*/
    uchar dir:1;             /*Door Interlock Relay*/
    uchar dor2_3:1;          /*Door Open Relay3 of Car 1*/
    uchar dor1_3:1;          /*Door Open Relay1 of Car 3*/
    uchar dor2_1:1;          /*Door Open Relay2 of Car 1*/
    uchar dor1_1:1;          /*Door Open Relay1 of Car 1*/

    uchar sp_bit1:5;         /* Spare bit 1 */
    uchar dcw:1;               /*Door closw Warning*/
    uchar dow2:1;              /*Door Open Warning2*/
    uchar dow1:1;              /*Door Open Warning1*/

    uchar lifeCnt;           /*Life counter*/
    uchar etx;               /*end data = 0x03*/
    uchar bcc1;              /*Block check character1*/
    uchar bcc2;              /*Block check character2*/
}SDR;

typedef struct SD{
    uchar stx;			     /*Start data = 0x02*/
    uchar toAddress;         /*To TCMS Address = 0x10*/
    uchar fromAddress;       /*From Address = 0x70*/
    uchar dataType;          /*datatype*/

    uchar sp_bit2:5;         /* Spare bit 2 */
    uchar onbrc:1;           /*On Broadcastiong*/
    uchar slave:1;           /*Integrated controller slave*/
    uchar master:1;          /*Integrated controller master*/

    uchar sp_Bit3:3;         /* Spare bit 3 */
    uchar pa_Ok:1;           /*PA test result OK*/
    uchar pa_Ng:1;           /*PA test result NG*/
    uchar pis_Ok:1;          /*Pis test result OK*/
    uchar pis_Ng:1;          /*Pis test result NG*/
    uchar onTest:1;          /*On Self Test*/

    uchar sp_byte1;          /* Spare BYTE 1 */

    uchar fcam1f:1;          /*Front Camera Car1 Fault*/
    uchar scam2_1f:1;        /*Saloon Camera Car1 #2 Fault*/
    uchar scam1_1f:1;        /*Saloon Camera Car1 #1 Fault*/
    uchar pei2_1f:1;         /*Passenger Emergency interphone car1 #2 Fault*/
    uchar pei1_1f:1;         /*Passenger Emergency interphone car1 #1 Fault*/
    uchar pamp1f:1;          /*Power Amplifier Car1 Fault*/
    uchar cop1f:1;           /*Center Operation Panel Car1 Fault*/
    uchar avc1f:1;           /*Audio Video Controller Car1 Fault*/

    uchar esw2_1f:1;         /*Ethernet Switch Car1 #2 Fault*/
    uchar esw1_1f:1;         /*Ethernet Switch Car1 #1 Fault*/
    uchar lcd4_1f:1;         /*LCD Display Car1 #4 Fault*/
    uchar lcd3_1f:1;         /*LCD Display Car1 #3 Fault*/
    uchar lcd2_1f:1;         /*LCD Display Car1 #2 Fault*/
    uchar lcd1_1f:1;         /*LCD Display Car1 #1 Fault*/
    uchar led2_1f:1;         /*LED Indicator Car1 #2 fault*/
    uchar led1_1f:1;         /*LED Indicator Car1 #1 Fault*/

    uchar sp_Bit4:1;         /* Spare bit 4 */
    uchar scam2_2f:1;        /*Saloon Camera Car2 #2 Fault*/
    uchar scam1_2f:1;        /*Saloon Camera Car2 #1 Fault*/
    uchar pei2_2f:1;         /*Passenger Emergency Interphone Car2 #2 Fault*/
    uchar pei1_2f:1;         /*Passenger Emergency Interphone Car2 #1 Fault*/
    uchar pamp2f:1;          /*Power Amplifier Car2 Fault*/
    uchar sp_Bit5:2;         /* Spare bit 5 */

    uchar esw2_2f:1;         /*Ethernet switch Car2 #2 Fault*/
    uchar esw1_2f:1;         /*Ethernet switch Car2 #1 Fault*/
    uchar lcd4_2f:1;         /*LCD Display Car2 #4 Fault*/
    uchar lcd3_2f:1;         /*LCD Display Car2 #3 Fault*/
    uchar lcd2_2f:1;         /*LCD Display Car2 #2 Fault*/
    uchar lcd1_2f:1;         /*LCD Display Car2 #1 Fault*/
    uchar led2_2f:1;         /*LED Indicator Car2 #2 Fault*/
    uchar led1_2f:1;         /*LED Indicator Car2 #1 Fault*/

    uchar fcam3f:1;          /*Front Camera Car3 Fault*/
    uchar scan2_3f:1;        /*Saloon Camera Car3 #2 Fault*/
    uchar scan1_3f:1;        /*Saloon Camera Car3 #1 Fault*/
    uchar pei2_3f:1;         /*Passenger Emergency Interphone Car3 #2 Fault*/
    uchar pei1_3f:1;         /*Passenger Emergency Interphone Car3 #1 Fault*/
    uchar pemp3f:1;          /*Power Amplifier Car3 Fault*/
    uchar cop3f:1;           /*Center Operation Panel Car3 Fault*/
    uchar avc3f:1;           /*Audio Video Controller Car3 Fault*/

    uchar esw2_3f:1;         /*Ethernet switch Car3 #2 Fault*/
    uchar esw1_3f:1;         /*Ethernet switch Car3 #1 Fault*/
    uchar lcd4_3f:1;         /*LCD Display Car3 #4 Fault*/
    uchar lcd3_3f:1;         /*LCD Display Car3 #3 Fault*/
    uchar lcd2_3f:1;         /*LCD Display Car3 #2 Fault*/
    uchar lcd1_3f:1;         /*LCD Display Car3 #1 Fault*/
    uchar led2_3f:1;         /*LED Indicator Car3 #2 Fault*/
    uchar led1_3f:1;         /*LED Indicator Car3 #1 Fault*/

    uchar sp_byte2;          /* Spare BYTE 2 */
    uchar sp_byte3;          /* Spare BYTE 3 */

    uchar sp_Bit6:2;         /* Spare bit 6 */
    uchar pei2_3c:1;         /*Passenger Emergency Interphone Car3 #2 Call*/
    uchar pei1_3c:1;         /*Passenger Emergency Interphone Car3 #1 Call*/
    uchar pei2_2c:1;         /*Passenger Emergency Interphone Car2 #2 Call*/
    uchar pei1_2c:1;         /*Passenger Emergency Interphone Car2 #1 Call*/
    uchar pei2_1c:1;         /*Passenger Emergency Interphone Car1 #2 Call*/
    uchar pei1_1c:1;         /*Passenger Emergency Interphone Car1 #1 Call*/

    uchar sp_Bit7:2;         /* Spare bit 7 */
    uchar pei2_3t:1;         /*Passenger Emergency Interphone Car3 #2 Talk*/
    uchar pei1_3t:1;         /*Passenger Emergency Interphone Car3 #1 Talk*/
    uchar pei2_2t:1;         /*Passenger Emergency Interphone Car2 #2 Talk*/
    uchar pei1_2t:1;         /*Passenger Emergency Interphone Car2 #1 Talk*/
    uchar pei2_1t:1;         /*Passenger Emergency Interphone Car1 #2 Talk*/
    uchar pei1_1t:1;         /*Passenger Emergency Interphone Car1 #1 Talk*/

    uchar sp_byte4;          /* Spare BYTE 4 */
    uchar sp_byte5;          /* Spare BYTE 5 */

    uchar VUD;          /*version upper digit*/
    uchar VDD;          /*version down digit*/

    uchar etx;

    uchar bcc1;              /*Block check character1*/
    uchar bcc2;              /*Block check character2*/

}SD;





#pragma pack(pop)

#endif // DEFINE_PROTOCOL_H
