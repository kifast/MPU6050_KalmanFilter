#include "Kalman_Filter.h"
#include "fireTools.h"
#include "math.h"
#include "LQ_MPU6050_DMP.h"

#define Q_ROLL       0.0001
#define Q_PITCH      0.0001
#define R_ROLL       2.5
#define R_PITCH      2.5

extern float Pitch,Roll;
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


float DeltaPitch=0;

float integ_angle_x=0;integ_angle_y=0;integ_angle_z=0;  //�ǶȻ���ֵ




// ------------------------------------------------���˵Ĵ���---------------------------------------------
float Q_angle = 0.002;		//�Ƕ��������Ŷȣ��Ƕ�������Э����
float Q_gyro  = 0.002;		//���ٶ��������Ŷȣ����ٶ�������Э����  
float R_angle = 0.6;		//���ٶȼƲ���������Э����
float dt      = 0.02;		//�˲��㷨�������ڣ��ɶ�ʱ����ʱ20ms
char  C_0     = 1;			//H����ֵ
float Q_bias, Angle_err;	//Q_bias:�����ǵ�ƫ��  Angle_err:�Ƕ�ƫ�� 
float PCt_0, PCt_1, E;		//����Ĺ�����
float K_0, K_1, t_0, t_1;	//����������  K_0:���ڼ������Ź���ֵ  K_1:���ڼ������Ź���ֵ��ƫ�� t_0/1:�м����
float P[4] ={0,0,0,0};	//����Э��������΢�־����м����
float PP[2][2] = { { 1, 0 },{ 0, 1 } };//����Э�������P
float Angle_Y_Final; 		//Y������б�Ƕ� 
float Gyro_y;        		//Y�������������ݴ�
void Kalman_Filter_Y(float Accel,float Gyro) 		
{
	Angle_Y_Final += (Gyro - Q_bias) * dt;
	P[0]=Q_angle - PP[0][1] - PP[1][0]; 
	P[1]=-PP[1][1];
	P[2]=-PP[1][1];
	P[3]=Q_gyro;	
	PP[0][0] += P[0] * dt; 
	PP[0][1] += P[1] * dt;  
	PP[1][0] += P[2] * dt;
	PP[1][1] += P[3] * dt;	
	Angle_err = Accel - Angle_Y_Final;		
	PCt_0 = C_0 * PP[0][0];
	PCt_1 = C_0 * PP[1][0];	
	E = R_angle + C_0 * PCt_0;	
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;	
	t_0 = PCt_0;
	t_1 = C_0 * PP[0][1];
	PP[0][0] -= K_0 * t_0;		
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;		
	Angle_Y_Final	+= K_0 * Angle_err;
	Q_bias	+= K_1 * Angle_err;	 
	Gyro_y   = Gyro - Q_bias;	 
}
//----------------------------------------------------���˵Ĵ������----------------------------------------



void GetAndWashData(void)
{
    // ��ȡԭʼ����
    MPU_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);
    

    /*-----------------------------------------------����Ԥ��----------------------------------------------*/
    // ��ԭʼ���ݽ��е�λת��,�����ʵ�ļ��ٶȺͽ��ٶ�
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
    //MPU6050_Init();
    GetAndWashData();
    //Kalman_Filter_Y(Pitch_z,gyroy_real);
    KalmanCalculation();
    //LQ_DMP_Init();
    //LQ_DMP_Read();
    
    // int pitchh,rolll,pitchhh,pitchzz,Angle_Y_Finall;
    // pitchh = Pitch_Kalman*100;
    // rolll = Roll_Kalman*100;
    // pitchhh = -Pitch * 100;
    // pitchzz = Pitch_z * 100;
    // Angle_Y_Finall = Angle_Y_Final * 100;
    // set_computer_value(0x02,0x01,&pitchh,1);  
    // set_computer_value(0x02,0x02,&pitchhh,1);
    // set_computer_value(0x02,0x03,&pitchzz,1);
    // set_computer_value(0x02,0x04,&Angle_Y_Finall,1);
}