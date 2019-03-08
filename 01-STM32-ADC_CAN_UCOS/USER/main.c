#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "can.h"
#include "key.h"
#include "adc.h"
#include "includes.h"


//*************������������ȼ�***********************/
#define START_TASK_PRIO      			12 	//��ʼ��������ȼ�����Ϊ���
#define LED0_TASK_PRIO       			9 	//LED0��������ȼ�
#define LED1_TASK_PRIO       			8 	//LED1��������ȼ�
#define CAN_TASK_PRIO       			7 	//CAN��������ȼ�
#define KEY_TASK_PRIO       			6 	//KEY��������ȼ�
#define ADC_TASK_PRIO       			5 	//ADC��������ȼ�


/////////////////////////UCOSII��������///////////////////////////////////
//START ����------------------------------------
//���������ջ��С
#define START_STK_SIZE  				128
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//LED0����--------------------------------------
//���������ջ��С
#define LED0_STK_SIZE  		    	512
//�����ջ	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//������
void led0_task(void *pdata);


//LED1����--------------------------------------
//���������ջ��С
#define LED1_STK_SIZE  					512
//�����ջ
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//������
void led1_task(void *pdata);


//CAN����--------------------------------------
//���������ջ��С
#define CAN_STK_SIZE  					512
//�����ջ
OS_STK CAN_TASK_STK[CAN_STK_SIZE];
//������
void can_task(void *pdata);


//KEY����--------------------------------------
//���������ջ��С
#define KEY_STK_SIZE  					512
//�����ջ
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);


//ADC����--------------------------------------
//���������ջ��С
#define ADC_STK_SIZE  					1024
//�����ջ
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
//������
void adc_task(void *pdata);

OS_EVENT *EventSemKey;
u8 led0cnt = 0;

//������-----------------------------------------
int main(void)
{ 
	EventSemKey = OSSemCreate(0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);    //��ʼ����ʱ����
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200
	LED_Init();					//��ʼ��LED 
 	LCD_Init();					//LCD��ʼ�� 
	KEY_Init(); 				//������ʼ��  
	Adc_Init();         //��ʼ��ADC
	CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_LoopBack);	//CAN��ʼ������ģʽ,������500Kbps
    
 	POINT_COLOR=RED;				//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"CUHK(SZ)");	
	LCD_ShowString(30,70,200,16,16,"CCR_FOOT_CONTROL");	
	LCD_ShowString(30,90,200,16,16,"Tony@CUHK");
	LCD_ShowString(30,110,200,16,16,"2019/3/7");
	LCD_ShowString(30,130,200,16,16,"LoopBack Mode");	 
	LCD_ShowString(30,150,200,16,16,"KEY0:Send WK_UP:Mode");	//��ʾ��ʾ��Ϣ		
  POINT_COLOR=BLUE;				//��������Ϊ��ɫ	   
	LCD_ShowString(30,170,200,16,16,"Count:");		  	//��ʾ��ǰ����ֵ	
	LCD_ShowString(30,190,200,16,16,"Send Data:");		//��ʾ���͵�����	
	LCD_ShowString(30,250,200,16,16,"Receive Data:");	//��ʾ���յ�������	
	
	delay_init(168);		  //��ʼ����ʱ����	
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();		//ϵͳ��ʼ
	
}


 //��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr = 0;
	pdata = pdata; 
  OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
 	OSTaskCreate(led0_task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(led1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);
	OSTaskCreate(key_task, (void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);
	OSTaskCreate(can_task, (void *)0,(OS_STK*)&CAN_TASK_STK[CAN_STK_SIZE-1],CAN_TASK_PRIO);
  OSTaskCreate(adc_task, (void *)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO);	
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
	
} 


//LED0����-----------------------
void led0_task(void *pdata)
{	 	
	u8 i=0;	
	while(1)
	{
		led0cnt++;
		if(led0cnt==10)
		{
			led0cnt=0;
//			OSSemPost(EventSemKey);
			LCD_ShowxNum(170,280,i,4,16,0);		//��ʾ������ֵ 			
		}
		LED0=!LED0;			//��ɫLED����˸
 		OSTimeDly(150); 
//		LED0=1;
//		delay_ms(180);
	};
}


//LED1����-------------------------
void led1_task(void *pdata)
{	  
	u8 event_err;
	while(1)
	{
		OSSemPend(EventSemKey,0,&event_err);
		LED1=!LED1;		//��ɫLED����˸
		OSTimeDly(150); 
//		delay_ms(300);
//		LED1=1;
//		delay_ms(300);
	};
}


//KEY����-----------------------
void key_task(void *pdata)
{	 
	u8 KeyValue=0;
	u8 keycnt=0;
	while(1)
	{
		KeyValue = KEY_Scan(0);	
		LCD_ShowString(30,90,200,16,16,"Key_task runninig");
		LCD_ShowxNum(170,280,KeyValue,4,16,0);		//��ʾ������ֵ
		LCD_ShowxNum(180,90,keycnt,4,16,0);				//��ʾ������ֵ
		keycnt=keycnt + 2;		
		delay_ms(200);
	};
}


//ADC����-----------------------
void adc_task(void *pdata)
{
	u16 adc_data[256];		//ADC���ݻ�����1
	u16 adc_data1[256];		//ADC���ݻ�����2
	u16 Mul_index=0;			//ģ�⿪�صĿ���
	float temp;	
	//LCD_ShowString(30,70,200,16,16,"ADC_TASK RUNNING!!");
	LCD_ShowString(30,70,200,16,16,"ADC1_CH5_VOL:0.000V");	//���ڹ̶�λ����ʾС���� 
	while(1)
	{ 				  
			adc_data[Mul_index]=Get_Adc_Average(ADC_Channel_5,5);//��ȡͨ��5��ת��ֵ��20��ȡƽ��
			//LCD_ShowxNum(134,170,adc_data[0],4,16,0);    //��ʾADCC�������ԭʼֵ
			temp=(float)adc_data[Mul_index]*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
			adc_data[0]=temp;                            //��ֵ�������ָ�adcx��������ΪadcxΪu16����
			LCD_ShowxNum(134,70,adc_data[Mul_index],1,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3
			temp-=adc_data[Mul_index];                           //���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111
			temp*=1000;                           //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����
			LCD_ShowxNum(150,70,temp,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���111.
		  delay_ms(100);	
	}
	
}


//CAN����-----------------------
void can_task(void *pdata)
{	
	u8 i;
	u8 cnt=0,cantask_cnt=0;
	u8 canbuf[8];

	u8 can_cnt=0;
	u8 KeyValue;
	u8 mode=1;	//CAN����ģʽ;0,��ͨģʽ;1,����ģʽ
	LCD_ShowString(30,110,200,16,16,"can_task running");	
	while(1)
	{		
		
		can_cnt++;
		LCD_ShowxNum(180,110,can_cnt,4,16,0);
	//	OSSemPend(EventSemKey,0,&event_err);
		
		
		
		
		
		
		delay_ms(1000);
	};
}









