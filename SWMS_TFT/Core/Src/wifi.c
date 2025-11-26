
#include "main.h"
#include <stdio.h>
#include <string.h>
#define RX_BUF_SIZE 690

#define SSID	"No Devices Found..."
#define SSID_PASSWORD	"24681012"
#define SERVER_IP	"142.93.218.33"

short timeout_flag=0;
extern UART_HandleTypeDef huart6;
int wifi_flag=0;


short int wifi_feedback(char sub[],uint16_t timeout)
{
	uint8_t rx_buffer[RX_BUF_SIZE]={0};
	HAL_UART_Receive(&huart6,rx_buffer,RX_BUF_SIZE,timeout);
	register int i;
    for (i=0; i<RX_BUF_SIZE; i++)
    {
        //logic to find substring in string
        if (sub[0]==rx_buffer[i]) 
        {
            register int s1Ind=1,strInd=i+1;
            while (sub[s1Ind]) 
            {
                if (sub[s1Ind]!=rx_buffer[strInd]) 
                {
                    break;
                }
                else 
                {
                    s1Ind++;
                    strInd++;
                }
            }
            if(!sub[s1Ind])
            {   
                return SUCCESS;
            }
        }
    }
		return FAIL;
}




int wifi_init(void)
{
	enum wifi_cmd{AT,AT_RST,AT_CWMODE,AT_CWJAP,AT_CIFSR,AT_CIPMUX,AT_CIPMODE};
	const char *const wifi_cmds[]={"AT\r\n","AT+RST\r\n","AT+CWMODE=3\r\n","AT+CWJAP=\""SSID"\",\""SSID_PASSWORD"\r\n",
						"AT+CIFSR\r\n","AT+CIPMUX=0\r\n","AT+CIPMODE=0\r\n"};
	int test=1;
	int count=1;
	while(1)
	{
		if(test==1)
		{
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT],strlen(wifi_cmds[AT]),100); //basic communication
			wifi_flag=wifi_feedback("OK",100);
			if(wifi_flag==SUCCESS)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON green LED
				ST7735_WriteString(10,86,"WIFI Found",Font_6x8,ST7735_PURPLE,DISABLE);
				HAL_Delay(500);
				test++;
				count=1;
			}
			else if(wifi_flag==FAIL)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF green LED
				HAL_Delay(1000);
				count++;

			}
			if(count==5)
			{
				count=0;
				return WIFI_NOT_FOUND;
			}
			HAL_Delay(500);
		}
		if(test==2)
		{
			ST7735_FillRectangle(7, 66, 148, 43, ST7735_WHITE);// this fill doesn't clear heading
			ST7735_WriteString(10,66,"RestartinG WiFi...",Font_6x8,ST7735_PURPLE,DISABLE);
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT_RST],strlen(wifi_cmds[AT_RST]),100);
			wifi_flag=wifi_feedback("WIFI CONNECTED",8000);
			ST7735_WriteString(10,76,"Wifi Restarted",Font_6x8,ST7735_PURPLE,DISABLE);
			HAL_Delay(500);
			if(wifi_flag==SUCCESS)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON green LED
				ST7735_WriteString(10,86,"Wifi Connected",Font_6x8,ST7735_PURPLE,DISABLE);
				HAL_Delay(500);
				test=5;

			}
			else if(wifi_flag==FAIL)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_RESET); //set 13th bit postion to turn ON red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_SET); //clear 13th bit postion to turn OFF green LED
				ST7735_WriteString(10,86,"ConnectinG to WiFi...",Font_6x8,ST7735_PURPLE,DISABLE);
				test++;
			}

		}
		if(test==3)
		{
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT_CWMODE],strlen(wifi_cmds[AT_CWMODE]),100);
			HAL_Delay(500);
			test++;
		}
		if(test==4)
		{
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT_CWJAP],strlen(wifi_cmds[AT_CWJAP]),100);
			wifi_flag=wifi_feedback("OK",7000);
			if(wifi_flag==SUCCESS)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON green LED
				ST7735_WriteString(10,96,"Wifi Connected",Font_6x8,ST7735_PURPLE,DISABLE);
				HAL_Delay(500);
				test++;
			}
			else if(wifi_flag==FAIL)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF green LED
				HAL_Delay(1000);
				count++;
			}
		  if(count==5)
			{
				count=0;
				return PASSWORD_ERROR;
			}
		}
		if(test==5)
		{
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT_CIFSR],strlen(wifi_cmds[AT_CIFSR]),100);
			wifi_flag=wifi_feedback("OK",100);
			if(wifi_flag==SUCCESS)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON green LED
				HAL_Delay(500);
				test++;
			}
		}
		if(test==6)
		{
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT_CIPMUX],strlen(wifi_cmds[AT_CIPMUX]),100);
			wifi_flag=wifi_feedback("OK",100);
			if(wifi_flag==SUCCESS)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON green LED
				HAL_Delay(500);
			  test++;
			}
		}

		if(test==7)
		{
			HAL_UART_Transmit(&huart6,(uint8_t *)wifi_cmds[AT_CIPMODE],strlen(wifi_cmds[AT_CIPMODE]),100);
			wifi_flag=wifi_feedback("OK",100);
			if(wifi_flag==SUCCESS)
			{
				HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET); //set 13th bit postion to turn OFF red LED
				HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET); //clear 13th bit postion to turn ON green LED
				HAL_Delay(500);
				return SUCCESS;
			}
		}
	}
}

int8_t checkInt(void)
{
	HAL_UART_Transmit(&huart6,(uint8_t *)"AT+CIPSTATUS\r\n",sizeof("AT+CIPSTATUS\r\n"),100);
	wifi_flag=wifi_feedback("STATUS:5",100); //STATUS:5 -> DISCONNECTION
	if(wifi_flag==SUCCESS)
	{
		return STATUS_ERROR;
	}
	return SUCCESS;
}

int wifi_transmit(uint8_t *dht_buffer)
{
	char Fetch[50];
	HAL_UART_Transmit(&huart6,(uint8_t *)"AT+CIPSTART=\"TCP\",\""SERVER_IP"\",80\r\n",sizeof("AT+CIPSTART=\"TCP\",\"142.93.218.33\",80\r\n"),100);
	osDelay(50);

	snprintf(Fetch,50,"GET /page.php?temp=%02d.%1d&hum=%02d.%1d&dev=08\r\n\r\n",dht_buffer[2],dht_buffer[3],dht_buffer[0],dht_buffer[1]); //ESP8266_SendTcp // @suppress("Float formatting support")
	HAL_UART_Transmit(&huart6,(uint8_t *)"AT+CIPSEND=50\r\n",sizeof("AT+CIPSEND=50\r\n"),100);
	osDelay(50);

	HAL_UART_Transmit(&huart6,(uint8_t *)Fetch,sizeof(Fetch),100);
	return SUCCESS;
}

