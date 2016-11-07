/**
  ******************************************************************************
  * @file    ADC/ADC_DMA_Transfer/Src/main.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    05-Dec-2014
  * @brief   This example describes how to use Timer to convert continuously data.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "contiki.h"



/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

PROCINIT(NULL);

void led_init(void)
{
	GPIO_InitTypeDef  gpioST;

	//io clock init 
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	__SYSCFG_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	

	//gpioST.GPIO_Pin = GPIO_Pin_5;
	//gpioST.GPIO_Mode = GPIO_Mode_OUT;
	//gpioST.GPIO_PuPd = GPIO_PuPd_UP;
	//gpioST.GPIO_OType = GPIO_OType_PP;
	//gpioST.GPIO_Speed = GPIO_Speed_Level_3;

	gpioST.Pin = GPIO_PIN_5;
	gpioST.Mode = GPIO_MODE_OUTPUT_PP;
	gpioST.Pull = GPIO_PULLUP;
	gpioST.Speed = GPIO_SPEED_HIGH;

	//GPIO_Init(GPIOC,&gpioST);
	//GPIO_Init(GPIOA,&gpioST);
	HAL_GPIO_Init(GPIOA, &gpioST);
}



void LED_On(void) //点亮led 灯子函数
{
	//GPIO_ResetBits(GPIOC, GPIO_Pin_8);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_9);
	//GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}



void LED_Off(void) //点亮led 灯子函数
{
	//GPIO_ResetBits(GPIOC, GPIO_Pin_8);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_9);
	//GPIO_SetBits(GPIOA, GPIO_Pin_5);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}



/*
* \brief This process thread is used to test plantform io end usarart
*/
PROCESS(os_test_process, "os test process");
PROCESS_THREAD(os_test_process, ev, data)
{
	static struct etimer et_T; 
	PROCESS_BEGIN();
	led_init( );
	while(1)
	{
		etimer_set(&et_T, 500);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_T));
		//debug_send_one_byte('a');
		//u_printf("abcdef \r\n");
		LED_On();
		etimer_set(&et_T, 500);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_T));
		//debug_send_one_byte('b');
		//XPRINTF((0,"out time is %d\r\n", clock_time()));
		LED_Off( );
	}
	PROCESS_END();
}



/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
 
  HAL_Init();
  /* Configure the system clock to 48 MHz */
  SystemClock_Config();
  /* Infinite Loop */

  //clock init
  clock_init( );
  process_init( );
  process_start(&etimer_process, NULL);
  process_start(&os_test_process, NULL);

  
  while (1)
  { 
	  do 
	  {
	  } while(process_run() > 0);
  }
}





/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI/2)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 8000000
  *            PREDIV                         = 1
  *            PLLMUL                         = 12
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* No HSE Oscillator on Nucleo, Activate PLL with HSI/2 as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  //RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    Error_Handler();
  }
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
