#include "extra_tasks.h"
#include "main.h"
#include "stm32f4xx_hal_i2c.h"
#include "fonts.h"
extern I2C_HandleTypeDef hi2c1;
int8_t dtBuf[7]={[DAY]=1,[DATE]=1,[MON]=1};
uint8_t date_invalid_flag=0;

/************** PRIVATE FUNCTION DECLARATIONS***************/
static void extiDeinit(void);
static void switchInit(void);
static int isValidDate(unsigned char date,unsigned char mon, unsigned char year);
static void scan_dt(int8_t dt_indx);
static void show_DT(uint8_t);
static void show_DT_instruction(const char *heading,const char *body);
static void up_down_arrow(uint8_t x_pos,uint8_t y_pos);

static void update_RTC(void);
/***********************************************************/

void Task5_SWMS_Config(void)
{
	//INIT
	extiDeinit();
	switchInit();

	//FIRST DISPLAY MESSAGE
	ST7735_FillScreen(ST7735_CYAN);
	ST7735_WriteString(9,7,"CONFIGURATION",Font_11x18,ST7735_LICORICE,DISABLE);
	ST7735_WriteString(55,27,"MODE",Font_11x18,ST7735_LICORICE,DISABLE);
	HAL_Delay(500);
	//SECOND DISPLAY MESSAGE
	drawRect(7, 50, 145, 70, ST7735_ONYX);
	ST7735_FillRectangle(8, 69, 143, 50, ST7735_CREAM);
	ST7735_FillRectangle(8, 51, 143, 18, ST7735_GREY);

	/************************ INSTRUCTION PART ********************************/
	ST7735_WriteString(15,53,"INSTRUCTIONS:",Font_10x14,ST7735_RED,ST7735_GREY);
	ST7735_WriteString(9,75,"Press SW_ENT-> Enter",Font_7x10,ST7735_DARK_PURPLE,ST7735_CREAM);
	ST7735_WriteString(9,90,"Press RESET-> Exit",Font_7x10,ST7735_DARK_PURPLE,ST7735_CREAM);

	/********* WAITING FOR ENTER BUTTON ********/
	while(GPIOC->IDR & BIT(10));

	ST7735_FillRectangle(8, 69, 143, 50, ST7735_CREAM);
	ST7735_WriteString(9,75,"SW_UP-> Increase",Font_7x10,ST7735_DARK_PURPLE,ST7735_CREAM);
	ST7735_WriteString(9,90,"SW_DN-> Decrease",Font_7x10,ST7735_DARK_PURPLE,ST7735_CREAM);
	ST7735_WriteString(9,105,"SW_ENT-> Confirm",Font_7x10,ST7735_DARK_PURPLE,ST7735_CREAM);
	HAL_Delay(3000);

	/******* This function shows date and time instruction and exit if presses enter ****************/
	show_DT_instruction("ENTER TIME:","hh : mm : ss");
	show_DT(TIME);
	up_down_arrow(X_POS1,Y_POS);
	while(1)
	{
		if(date_invalid_flag)
		{
			date_invalid_flag=0;
			scan_dt(DATE);
		}
		else
			scan_dt(TIME);

		update_RTC();

	}
}

static void scan_dt(int8_t dt_indx)
{
	uint8_t x_pos_diff=0,btn_prs_flag=0,inr_flag=0;

	//structure variable that holds limits of date & time
	limit_t dt;

	/* this lookup table holds the date & time value limits e.g.,
	 * dt_lookup[]={sec_Hi_Lim,sec_Lw_Lim,min_Hi_Lim,min_Lw_Lim,..........yr_Hi_Lim,yr_Lw_Lim};
	 */
	uint8_t const dt_lookup[]={59,0,59,0,23,0,7,1,31,1,12,1,99,0};

	char dispNum[4];

	/* Assigning index 2 because HOUR value should be stored first*/
	if(dt_indx==TIME)
		dt_indx=2;

	/******* THIS OUTER LOOP ITERATES TO STORE THE VALUE OF DATE & TIME **********************/
	while(dt_indx<7)
	{
		// below expression doesn't work for index 3 that's why written at top
		x_pos_diff=(50*(dt_indx%4));
		if(dt_indx < 0)
		{
			//setting dt_index to day index
			dt_indx=3;
			inr_flag=1;

			//setting hardcode value of x_postion when index is 3
			x_pos_diff=50;
		}

		//calculating upper boundaries and lower boundaries of each date/time indexes
		dt.upr_lim = dt_indx << 1U;
		dt.lwr_lim = (dt_indx << 1U)+1;

		/*********** SHOWING INSTRUCTIONS FOR DAY AND DATE SETTINGS*************/
		if(dt_indx == DAY)
		{

			/******* This function shows date and time instruction and exit if presses enter ****************/
			show_DT_instruction("ENTER DAY:","    day");

			show_DT(DAY);
			up_down_arrow(X_POS2,Y_POS);
		}
		else if(dt_indx == DATE)
		{
			inr_flag=1;
			/******* This function shows date and time instruction and exit if presses enter ****************/
			show_DT_instruction("ENTER DATE:","dd / mm / yy");

			show_DT(DATE);
			up_down_arrow(X_POS1,Y_POS);

		}
		/**************************************************************************/

		//Position up and down arrows according to the INDEX
		if(dt_indx < 3)
			up_down_arrow(125-x_pos_diff,Y_POS);
		else
			up_down_arrow(25+x_pos_diff,Y_POS);

		//Loop is true until ENT_Sw is not pressed
		while(GPIOC->IDR & BIT(10))
		{
			//use SW_UP to increase value
			if(!(GPIOC->IDR & BIT(9)))
			{
				btn_prs_flag=1;
				dtBuf[dt_indx]--;

				//debounce delay for SW_UP
				HAL_Delay(170);

				//error checking for value limits
				if(dtBuf[dt_indx] < dt_lookup[dt.lwr_lim])
					dtBuf[dt_indx]=dt_lookup[dt.upr_lim];
			}

			//use SW_DN to decrease value
			else if(!(GPIOC->IDR & BIT(8)))
			{
				btn_prs_flag=1;
				dtBuf[dt_indx]++;

				//debounce delay for SW_DN
				HAL_Delay(170);

				//error checking for value limits
				if(dtBuf[dt_indx] > dt_lookup[dt.upr_lim])
					dtBuf[dt_indx]=dt_lookup[dt.lwr_lim];
			}
			if(btn_prs_flag)
			{
				btn_prs_flag=0;
				snprintf(dispNum,5,"%02d",dtBuf[dt_indx]);
				if(dt_indx < 3)
					ST7735_WriteString(120-x_pos_diff,86,dispNum,Font_10x14,ST7735_ORANGE,ST7735_CREAM);
				else
					ST7735_WriteString(20+x_pos_diff,86,dispNum,Font_10x14,ST7735_ORANGE,ST7735_CREAM);
			}
		}
		//debounce delay for SW_ENT
		HAL_Delay(170);
		if(inr_flag)
			dt_indx++;
		else
			dt_indx--;
	}
	HAL_Delay(200);
}
//
//
static void update_RTC(void)
{
	int i;

	//CLEARING BOX
	ST7735_FillRectangle(8, 51, 143, 68, ST7735_CREAM);
	ST7735_WriteString(12,60,"Checking Date",Font_10x14,ST7735_PURPLE,ST7735_CREAM);
	ST7735_WriteString(55,75,"....",Font_10x14,ST7735_PURPLE,ST7735_CREAM);
	HAL_Delay(1000);
	if(!(isValidDate(dtBuf[DATE],dtBuf[MON],dtBuf[YEAR])))
	{
		for(i=0;i<7;i++)
		// CONVERTING DECIMAL TO BCD
			dtBuf[i]=(dtBuf[i]%10)|(dtBuf[i]/10<<4);
		ST7735_WriteString(12,60,"Updating RTC ",Font_10x14,ST7735_PURPLE,ST7735_CREAM);
		ST7735_WriteString(55,75,"....",Font_10x14,ST7735_PURPLE,ST7735_CREAM);
		HAL_Delay(3000);

		//Writing data to RTC
		HAL_I2C_Mem_Write(&hi2c1,RTC_ADDR,0x00,I2C_MEMADD_SIZE_8BIT,(uint8_t *)dtBuf,7,100);

		//CLEARING BOX
		ST7735_FillRectangle(8, 51, 143, 68, ST7735_CREAM);

		ST7735_WriteString(12,60,"RTC Updated!!",Font_10x14,ST7735_GREEN,ST7735_CREAM);
		HAL_Delay(1500);
		ST7735_WriteString(12,80,"Press RESET To ",Font_7x10,ST7735_PURPLE,ST7735_CREAM);
		ST7735_WriteString(12,95,"Restart The Program",Font_7x10,ST7735_PURPLE,ST7735_CREAM);
		while(1);

	}
	else
	{
		//CLEARING BOX
		ST7735_FillRectangle(8, 51, 143, 68, ST7735_CREAM);

		ST7735_WriteString(10,60,"INVALID DATE!!",Font_10x14,ST7735_RED,ST7735_CREAM);
		ST7735_WriteString(12,80,"TRY AGAIN...",Font_10x14,ST7735_RED,ST7735_CREAM);
		date_invalid_flag=1;
		HAL_Delay(2000);
	}
}

static void show_DT(uint8_t choice)
{
	char disp[20];
	if(choice==TIME)
		snprintf(disp,20,"%02d : %02d : %02d",dtBuf[SEC],dtBuf[MIN],dtBuf[HR]);
	else if(choice==DAY)
		snprintf(disp,20,"     %02d",dtBuf[DAY]);
	else
		snprintf(disp,20,"%02d / %02d / %02d",dtBuf[DATE],dtBuf[MON],dtBuf[YEAR]);
	ST7735_WriteString(20,86,disp,Font_10x14,ST7735_ORANGE,ST7735_CREAM);
}
static void show_DT_instruction(const char *heading,const char *body)
{
	ST7735_FillRectangle(8, 51, 143, 18, ST7735_GREY);
	ST7735_WriteString(25,53,heading,Font_10x14,ST7735_PURPLE,ST7735_GREY);
	ST7735_FillRectangle(8, 69, 143, 50, ST7735_CREAM);
	ST7735_WriteString(20,86,body,Font_10x14,ST7735_ORANGE,ST7735_CREAM);
	while(GPIOC->IDR & BIT(10));

	//debounce delay for SW_UP
	HAL_Delay(170);
}
static void up_down_arrow(uint8_t x_pos,uint8_t y_pos)
{
	/************** Clearing arrows in previous position***************************/
	if(x_pos>X_POS1)
	{
		ST7735_WriteString(x_pos-50, y_pos, "^" ,Font_10x14,ST7735_CREAM,DISABLE);
		ST7735_WriteString(x_pos-50, y_pos+1, "^" ,Font_10x14,ST7735_CREAM,DISABLE);
		ST7735_WriteString(x_pos-50, y_pos+2, "^" ,Font_10x14,ST7735_CREAM,DISABLE);
		ST7735_WriteString(x_pos+1-50, y_pos+28, "v" ,Font_10x14,ST7735_CREAM,DISABLE);
	}
	/*******************************************************************/
	/************** Writing arrows in new position***************************/
	ST7735_WriteString(x_pos, y_pos, "^" ,Font_10x14,ST7735_DARKGREY,DISABLE);
	ST7735_WriteString(x_pos, y_pos+1, "^" ,Font_10x14,ST7735_DARKGREY,DISABLE);
	ST7735_WriteString(x_pos, y_pos+2, "^" ,Font_10x14,ST7735_DARKGREY,DISABLE);
	ST7735_WriteString(x_pos+1, y_pos+28, "v" ,Font_10x14,ST7735_DARKGREY,ST7735_CREAM);
}

static void switchInit(void)
{
	//ENABLE GPIOC CLK
	if(!(RCC->AHB1ENR & GPIOCEN_BIT))
		RCC->AHB1ENR|=GPIOCEN_BIT;
	//GPIO CONFIG
	GPIOC->PUPDR|=PULLUP08;
	GPIOC->PUPDR|=PULLUP09;
	GPIOC->PUPDR|=PULLUP10;
}

static int isValidDate(unsigned char date,unsigned char mon, unsigned char year)
{
	int leapYear=0;
	if(year>=0 && mon>0 && mon<=12)
	{
		if(year%400==0 || (year%100!=0 && year%4==0))
			leapYear=1;
		if((mon==4 || mon ==6 || mon==9 || mon==11) && (date>0 && date<=30))
			return 0;
		else if(mon==2 && ((leapYear==1 &&(date>0 && date<=29)) || (leapYear==0 &&(date>0 && date<=28))))
			return 0;
		else if(!(mon==4 || mon==6 || mon==9 || mon==11 || mon==2) && (date>0 && date<=31))
			return 0;
		else
			return 1;
	}
	return 1;
}
static void extiDeinit(void)
{
	//DISABLE SYSCFG CLK
	RCC->APB2ENR&=~BIT(14);

	//SYSCFG DECONFIG
	SYSCFG->EXTICR[2]&=~(0x00000200UL);

	//EXTI DECONFIG
	EXTI->FTSR&=~BIT(10);
	EXTI->IMR&=~BIT(10);

	//NVIC DECONFIG
	NVIC->ISER[1]&=~BIT(8);

}
///**************************************************************************************************
//void min(void)
//{
//	char dispNum[2];
//	KM_LCD_Write_Cmd(0x83);
//
//	//Loop is true until ENT_Sw is not pressed
//	while(GPIOC->IDR & BIT(10))
//	{
//		if(!(GPIOC->IDR & BIT(8)) && dtBuf[MIN]>0)
//		{
//			dtBuf[MIN]--;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0x83);
//			sprintf(dispNum,"%02d",dtBuf[MIN]);
//			KM_LCD_Write_Str(dispNum);
//		}
//		else if(!(GPIOC->IDR & BIT(9)) && dtBuf[MIN]<59)
//		{
//			dtBuf[MIN]++;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0x83);
//			sprintf(dispNum,"%02d",dtBuf[MIN]);
//			KM_LCD_Write_Str(dispNum);
//		}
//	}
//	entSwCount++;
//	KM_Delay_ms(200);
//}
//
//void hour(void)
//{
//	char dispNum[2];
//	KM_LCD_Write_Cmd(0x86);
//
//	//Loop is true until ENT_Sw is not pressed
//	while(GPIOC->IDR & BIT(10))
//	{
//		if(!(GPIOC->IDR & BIT(8)) && dtBuf[HR]>0)
//		{
//			dtBuf[HR]--;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0x86);
//			sprintf(dispNum,"%02d",dtBuf[HR]);
//			KM_LCD_Write_Str(dispNum);
//		}
//		else if(!(GPIOC->IDR & BIT(9)) && dtBuf[HR]<23)
//		{
//			dtBuf[HR]++;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0x86);
//			sprintf(dispNum,"%02d",dtBuf[HR]);
//			KM_LCD_Write_Str(dispNum);
//		}
//	}
//	entSwCount++;
//	KM_Delay_ms(200);
//}
//
//void day(void)
//{
//	char dispNum[2];
//	KM_LCD_Write_Cmd(0x89);
//
//	//Loop is true until ENT_Sw is not pressed
//	while(GPIOC->IDR & BIT(10))
//	{
//		if(!(GPIOC->IDR & BIT(8)) && dtBuf[DAY]>0)
//		{
//			dtBuf[DAY]--;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0x89);
//			sprintf(dispNum,"%02d",dtBuf[DAY]);
//			KM_LCD_Write_Str(dispNum);
//		}
//		else if(!(GPIOC->IDR & BIT(9)) && dtBuf[DAY]<7)
//		{
//			dtBuf[DAY]++;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0x89);
//			sprintf(dispNum,"%02d",dtBuf[DAY]);
//			KM_LCD_Write_Str(dispNum);
//		}
//	}
//	entSwCount++;
//	KM_Delay_ms(200);
//}
//
//void date(void)
//{
//	char dispNum[2];
//	KM_LCD_Write_Cmd(0xC0);
//
//	//Loop is true until ENT_Sw is not pressed
//	while(GPIOC->IDR & BIT(10))
//	{
//		if(!(GPIOC->IDR & BIT(8)) && dtBuf[DATE]>0)
//		{
//			dtBuf[DATE]--;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0xC0);
//			sprintf(dispNum,"%02d",dtBuf[DATE]);
//			KM_LCD_Write_Str(dispNum);
//		}
//		else if(!(GPIOC->IDR & BIT(9)) && dtBuf[DATE]<31)
//		{
//			dtBuf[DATE]++;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0xC0);
//			sprintf(dispNum,"%02d",dtBuf[DATE]);
//			KM_LCD_Write_Str(dispNum);
//		}
//	}
//	entSwCount++;
//	KM_Delay_ms(200);
//}
//
//void mon(void)
//{
//	char dispNum[2];
//	KM_LCD_Write_Cmd(0xC3);
//
//	//Loop is true until ENT_Sw is not pressed
//	while(GPIOC->IDR & BIT(10))
//	{
//		if(!(GPIOC->IDR & BIT(8)) && dtBuf[MON]>0)
//		{
//			dtBuf[MON]--;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0xC3);
//			sprintf(dispNum,"%02d",dtBuf[MON]);
//			KM_LCD_Write_Str(dispNum);
//		}
//		else if(!(GPIOC->IDR & BIT(9)) && dtBuf[MON]<12)
//		{
//			dtBuf[MON]++;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0xC3);
//			sprintf(dispNum,"%02d",dtBuf[MON]);
//			KM_LCD_Write_Str(dispNum);
//		}
//	}
//	entSwCount++;
//	KM_Delay_ms(200);
//}
//
//void year(void)
//{
//	char dispNum[2];
//	KM_LCD_Write_Cmd(0xC6);
//
//	//Loop is true until ENT_Sw is not pressed
//	while(GPIOC->IDR & BIT(10))
//	{
//		if(!(GPIOC->IDR & BIT(8)) && dtBuf[YEAR]>0)
//		{
//			dtBuf[YEAR]--;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0xC6);
//			sprintf(dispNum,"%02d",dtBuf[YEAR]);
//			KM_LCD_Write_Str(dispNum);
//		}
//		else if(!(GPIOC->IDR & BIT(9)) && dtBuf[YEAR]<99)
//		{
//			dtBuf[YEAR]++;
//			KM_Delay_ms(170);
//			KM_LCD_Write_Cmd(0xC6);
//			sprintf(dispNum,"%02d",dtBuf[YEAR]);
//			KM_LCD_Write_Str(dispNum);
//		}
//	}
//	entSwCount++;
//	KM_Delay_ms(200);
//}
//
//***********************************************************************************************************/
