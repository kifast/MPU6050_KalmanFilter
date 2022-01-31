#include "Kalman_Filter.h"
#include "math.h"


#define Q_ROLL       0.0001
#define Q_PITCH      0.0001
#define R_ROLL       2.5
#define R_PITCH      2.5

/* ˵���� */
/* 1.������y��ָ����������y��Ľ���Roll�ǣ���x��Ľ���Pitch�� */

float Pitch_Kalman,Roll_Kalman;                     //�������˲���������ĽǶ�

extern signed short  aacx,aacy,aacz;	            //���ٶ�ԭʼ����
extern signed short  gyrox,gyroy,gyroz;             //������ԭʼ����
float aacx_real,aacy_real,aacz_real;                //���ٶȻ���ֵ
float gyrox_real,gyroy_real,gyroz_real;             //���ٶȻ���ֵ
float Roll_z,Pitch_z;                               //���ٶȼƼ���ĽǶ�ֵ


float Roll_hat_pri=0,Pitch_hat_pri=0;               // �Ƕ��������
float Roll_hat_pos=0,Pitch_hat_pos=0;               // �ǶȺ������
float P_Roll_hat_pri=0,P_Pitch_hat_pri=0;           // �����������
float P_Roll_hat_pos=0,P_Pitch_hat_pos=0;           // ����������
float KalmanGain_Pitch,KalmanGain_Roll;             // ����������


float DeltaPitch=0;                                 // �Ƕȱ仯 ����PID����

float integ_angle_x=0;integ_angle_y=0;integ_angle_z=0;  //�ǶȻ���ֵ


void GetAndWashData(void)
{
    // ��ȡԭʼ����
    MPU_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);
    

    /*-----------------------------------------------����Ԥ��----------------------------------------------*/
    // ��ԭʼ���ݽ��е�λת��,�����ʵ�ļ��ٶȺͽ��ٶȣ��������ǵĳ�ʼ��Ϊ+-2000dps�����ٶȼ�+-2g��
    aacx_real = aacx/16384.0;
    aacy_real = aacy/16384.0;
    aacz_real = aacz/16384.0;
    gyrox_real = gyrox/16.4;
    gyroy_real = gyroy/16.4;
    gyroz_real = gyroz/16.4;

    // ���ٶȼƼ���(�۲�ֵ)
    Pitch_z = (atan(aacy_real/(sqrt(aacx_real*aacx_real+aacz_real*aacz_real)))) * 180 / 3.1415;   // ����y����ˮƽ��н�
    Roll_z = (atan(aacx_real/(sqrt(aacy_real*aacy_real+aacz_real*aacz_real)))) * 180 / 3.1415;    // ����x����ˮƽ��н�
}

void KalmanCalculation(void)
{        
    /*-----------------------------------------�������˲�--------------------------------------------------*/
    // ����˵����A=1��B=dt��������=���ٶȣ��۲�ֱֵ�Ӳ�ýǶ�H=1��

    // Ԥ�⣺���νǶ�������� = ת�ƾ��� * �ϴνǶȺ������ + ������
    Pitch_hat_pri = 1 * Pitch_hat_pos + gyrox_real*0.005;
    Roll_hat_pri = 1 * Roll_hat_pos +  gyroy_real*0.005;

    // Ԥ�⣺���η���������� = ת�ƾ��� * �ϴη��������� * ת�ƾ���ת�� + Q
    P_Pitch_hat_pri = P_Pitch_hat_pos + Q_PITCH;
    P_Roll_hat_pri = P_Roll_hat_pos + Q_ROLL;

    // ���£����㿨��������
    KalmanGain_Pitch = (P_Pitch_hat_pri)/(P_Pitch_hat_pri+R_PITCH);
    KalmanGain_Roll = (P_Roll_hat_pri)/(P_Roll_hat_pri+R_ROLL);

    // ���£�����������
    Pitch_hat_pos = Pitch_hat_pri + KalmanGain_Pitch*(Pitch_z-Pitch_hat_pri);
    Roll_hat_pos = Roll_hat_pri + KalmanGain_Roll*(Roll_z-Roll_hat_pri);

    // ���£������µķ���������
    P_Pitch_hat_pos = (1-KalmanGain_Pitch)*P_Pitch_hat_pri;
    P_Roll_hat_pos = (1-KalmanGain_Roll)*P_Roll_hat_pri;


    DeltaPitch = (-Pitch_hat_pos)-Pitch_Kalman;                         // Ϊ�˷���PID�����е�΢������
    Pitch_Kalman = -Pitch_hat_pos;
    Roll_Kalman = Roll_hat_pos;

}



void TestKalman(void)
{
    GetAndWashData();
    KalmanCalculation();   
}