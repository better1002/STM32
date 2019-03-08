#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "can.h"
#include "key.h"
#include "adc.h"
#include "includes.h"


//*************定义任务的优先级***********************/
#define START_TASK_PRIO      			12 	//开始任务的优先级设置为最低
#define LED0_TASK_PRIO       			9 	//LED0任务的优先级
#define LED1_TASK_PRIO       			8 	//LED1任务的优先级
#define CAN_TASK_PRIO       			7 	//CAN任务的优先级
#define KEY_TASK_PRIO       			6 	//KEY任务的优先级
#define ADC_TASK_PRIO       			5 	//ADC任务的优先级


/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务------------------------------------
//设置任务堆栈大小
#define START_STK_SIZE  				128
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//LED0任务--------------------------------------
//设置任务堆栈大小
#define LED0_STK_SIZE  		    	512
//任务堆栈	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//任务函数
void led0_task(void *pdata);


//LED1任务--------------------------------------
//设置任务堆栈大小
#define LED1_STK_SIZE  					512
//任务堆栈
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//任务函数
void led1_task(void *pdata);


//CAN任务--------------------------------------
//设置任务堆栈大小
#define CAN_STK_SIZE  					512
//任务堆栈
OS_STK CAN_TASK_STK[CAN_STK_SIZE];
//任务函数
void can_task(void *pdata);


//KEY任务--------------------------------------
//设置任务堆栈大小
#define KEY_STK_SIZE  					512
//任务堆栈
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);


//ADC任务--------------------------------------
//设置任务堆栈大小
#define ADC_STK_SIZE  					1024
//任务堆栈
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
//任务函数
void adc_task(void *pdata);

OS_EVENT *EventSemKey;
u8 led0cnt = 0;

//主函数-----------------------------------------
int main(void)
{ 
	EventSemKey = OSSemCreate(0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);    //初始化延时函数
	uart_init(115200);	//初始化串口波特率为115200
	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化 
	KEY_Init(); 				//按键初始化  
	Adc_Init();         //初始化ADC
	CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_LoopBack);	//CAN初始化环回模式,波特率500Kbps
    
 	POINT_COLOR=RED;				//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"CUHK(SZ)");	
	LCD_ShowString(30,70,200,16,16,"CCR_FOOT_CONTROL");	
	LCD_ShowString(30,90,200,16,16,"Tony@CUHK");
	LCD_ShowString(30,110,200,16,16,"2019/3/7");
	LCD_ShowString(30,130,200,16,16,"LoopBack Mode");	 
	LCD_ShowString(30,150,200,16,16,"KEY0:Send WK_UP:Mode");	//显示提示信息		
  POINT_COLOR=BLUE;				//设置字体为蓝色	   
	LCD_ShowString(30,170,200,16,16,"Count:");		  	//显示当前计数值	
	LCD_ShowString(30,190,200,16,16,"Send Data:");		//提示发送的数据	
	LCD_ShowString(30,250,200,16,16,"Receive Data:");	//提示接收到的数据	
	
	delay_init(168);		  //初始化延时函数	
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();		//系统开始
	
}


 //开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr = 0;
	pdata = pdata; 
  OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(led0_task,(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);						   
 	OSTaskCreate(led1_task,(void *)0,(OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],LED1_TASK_PRIO);
	OSTaskCreate(key_task, (void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);
	OSTaskCreate(can_task, (void *)0,(OS_STK*)&CAN_TASK_STK[CAN_STK_SIZE-1],CAN_TASK_PRIO);
  OSTaskCreate(adc_task, (void *)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO);	
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
	
} 


//LED0任务-----------------------
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
			LCD_ShowxNum(170,280,i,4,16,0);		//显示按键的值 			
		}
		LED0=!LED0;			//红色LED灯闪烁
 		OSTimeDly(150); 
//		LED0=1;
//		delay_ms(180);
	};
}


//LED1任务-------------------------
void led1_task(void *pdata)
{	  
	u8 event_err;
	while(1)
	{
		OSSemPend(EventSemKey,0,&event_err);
		LED1=!LED1;		//绿色LED灯闪烁
		OSTimeDly(150); 
//		delay_ms(300);
//		LED1=1;
//		delay_ms(300);
	};
}


//KEY任务-----------------------
void key_task(void *pdata)
{	 
	u8 KeyValue=0;
	u8 keycnt=0;
	while(1)
	{
		KeyValue = KEY_Scan(0);	
		LCD_ShowString(30,90,200,16,16,"Key_task runninig");
		LCD_ShowxNum(170,280,KeyValue,4,16,0);		//显示按键的值
		LCD_ShowxNum(180,90,keycnt,4,16,0);				//显示按键的值
		keycnt=keycnt + 2;		
		delay_ms(200);
	};
}


//ADC任务-----------------------
void adc_task(void *pdata)
{
	u16 adc_data[256];		//ADC数据缓存区1
	u16 adc_data1[256];		//ADC数据缓存区2
	u16 Mul_index=0;			//模拟开关的控制
	float temp;	
	//LCD_ShowString(30,70,200,16,16,"ADC_TASK RUNNING!!");
	LCD_ShowString(30,70,200,16,16,"ADC1_CH5_VOL:0.000V");	//先在固定位置显示小数点 
	while(1)
	{ 				  
			adc_data[Mul_index]=Get_Adc_Average(ADC_Channel_5,5);//获取通道5的转换值，20次取平均
			//LCD_ShowxNum(134,170,adc_data[0],4,16,0);    //显示ADCC采样后的原始值
			temp=(float)adc_data[Mul_index]*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
			adc_data[0]=temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形
			LCD_ShowxNum(134,70,adc_data[Mul_index],1,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3
			temp-=adc_data[Mul_index];                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111
			temp*=1000;                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。
			LCD_ShowxNum(150,70,temp,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是111.
		  delay_ms(100);	
	}
	
}


//CAN任务-----------------------
void can_task(void *pdata)
{	
	u8 i;
	u8 cnt=0,cantask_cnt=0;
	u8 canbuf[8];

	u8 can_cnt=0;
	u8 KeyValue;
	u8 mode=1;	//CAN工作模式;0,普通模式;1,环回模式
	LCD_ShowString(30,110,200,16,16,"can_task running");	
	while(1)
	{		
		
		can_cnt++;
		LCD_ShowxNum(180,110,can_cnt,4,16,0);
	//	OSSemPend(EventSemKey,0,&event_err);
		
		
		
		
		
		
		delay_ms(1000);
	};
}









