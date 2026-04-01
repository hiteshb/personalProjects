 
#include <stdio.h>
 /**
* @brief  SET CURRENT DATE AND TIME
* @param  void
* @retval void
*/
 void Task5_SWMS_Config(void);
 /*------------------------------------------TASK-05----------------------------------------------*/
 
 /************************* STRUCTURE********************/

typedef struct
{
	unsigned char upr_lim:4;
	unsigned char lwr_lim:4;
}limit_t;
/************************************************************************/

/******************************* MACROS*************************/
#define SEC		0U
#define MIN		1U
#define HR		2U
#define DAY		3U
#define DATE	4U
#define MON		5U
#define YEAR	6U

#define TIME 	0U
#define DAY  	3U
#define DATE 	4U

#define Y_POS	72U

#define X_POS1	25U
#define X_POS2	75U
#define X_POS3	125U
//GPIO MACROS
#define PULLUP08			0x00010000UL
#define PULLUP09			0x00040000UL
#define PULLUP10			0x00100000UL

//RCC MACROS
#define GPIOCEN_BIT		BIT(2)
#define SYSCFGEN_BIT	BIT(14)
/*----------------------------------------------------------------------------------------------*/


