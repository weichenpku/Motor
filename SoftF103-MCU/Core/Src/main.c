/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define EXTERN_MEM
#define MEM_INITIATOR
#include "softf103.h"
extern void usb_fifo_transmit(void);
SoftF103_Mem_t mem;
SoftIO_t sio;
void __aeabi_assert(const char *expr, const char *file, int line) {
  printf("assert failed: %s(%d): %s\n", file, line, expr); while(1);
}
int fputc(int ch, FILE *f) {
  while (fifo_full(&mem.logging));  // wait others to send
  fifo_enque(&mem.logging, ch);
  USART2->CR1 |= USART_CR1_TXEIE;  // enable tx empty interrupt
	return ch;
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void my_before(void* softio, SoftIO_Head_t* head) {
  uint8_t need_disable_irq = 0;
  if (head->type == SOFTIO_HEAD_TYPE_READ || head->type == SOFTIO_HEAD_TYPE_WRITE) {
    if (softio_is_variable_included(sio, *head, mem.gpio_count)) {  // atomic write
      need_disable_irq = 1;
    }
    if (softio_is_variable_included(sio, *head, mem.gpio_count_add)) {  // atomic write
      need_disable_irq = 1;
    }
  }
  if (head->type == SOFTIO_HEAD_TYPE_READ) {
    if (softio_is_variable_included(sio, *head, mem.gpio_in)) {  // wanna read variable
      mem.gpio_in = GPIOB->IDR >> 8;  // PB15 ~ PB8
    }
  }
  if (need_disable_irq) __disable_irq();
}
void my_after(void* softio, SoftIO_Head_t* head) {
  uint8_t need_enable_irq = 0;
  if (head->type == SOFTIO_HEAD_TYPE_WRITE) {
    if (softio_is_variable_included(sio, *head, mem.gpio_out)) {
      GPIOB->BSRR = mem.gpio_out | ( ((uint32_t)(~mem.gpio_out & 0x0ff))<<16 );  // atomic write
    }
    if (softio_is_variable_included(sio, *head, mem.led)) {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, mem.led);
    }
    if (softio_is_variable_included(sio, *head, mem.tim1_prescaler)) TIM1->PSC = mem.tim1_prescaler;
    if (softio_is_variable_included(sio, *head, mem.tim1_period)) TIM1->ARR = mem.tim1_period;
    if (softio_is_variable_included(sio, *head, mem.tim1_pulse)) TIM1->CCR1 = mem.tim1_pulse;
    if (softio_is_variable_included(sio, *head, mem.tim1_PWM)) {
      if (mem.tim1_PWM) {
        TIM1->CCER |= (uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_1);  // enable pwm1
        TIM1->BDTR |= TIM_BDTR_MOE;  // only TIM1 needs this
        TIM1->CR1 |= TIM_CR1_CEN;  // enable peripheral
      } else {
        TIM1->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_1);  // disable pwm1
      }
    }
    if (softio_is_variable_included(sio, *head, mem.tim1_IT)) {
      if (mem.tim1_IT) {
        TIM1->DIER |= TIM_IT_UPDATE;  // enable interrupt
        TIM1->CR1 |= TIM_CR1_CEN;  // enable peripheral
      } else {
        TIM1->DIER &= ~TIM_IT_UPDATE;  // disable interrupt
      }
    }
    if (softio_is_variable_included(sio, *head, mem.tim2_prescaler)) TIM2->PSC = mem.tim2_prescaler;
    if (softio_is_variable_included(sio, *head, mem.tim2_period)) TIM2->ARR = mem.tim2_period;
    if (softio_is_variable_included(sio, *head, mem.tim2_pulse)) TIM2->CCR1 = mem.tim2_pulse;
    if (softio_is_variable_included(sio, *head, mem.tim2_PWM)) {
      if (mem.tim2_PWM) {
        TIM2->CCER |= (uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_1);  // enable pwm1
        TIM2->CR1 |= TIM_CR1_CEN;  // enable peripheral
      } else {
        TIM2->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_1);  // disable pwm1
      }
    }
    if (softio_is_variable_included(sio, *head, mem.tim2_IT)) {
      if (mem.tim2_IT) {
        TIM2->DIER |= TIM_IT_UPDATE;  // enable interrupt
        TIM2->CR1 |= TIM_CR1_CEN;  // enable peripheral
      } else {
        TIM2->DIER &= ~TIM_IT_UPDATE;  // disable interrupt
      }
    }
  }
  if (head->type == SOFTIO_HEAD_TYPE_READ || head->type == SOFTIO_HEAD_TYPE_WRITE) {
    if (softio_is_variable_included(sio, *head, mem.gpio_count)) {  // atomic write
      need_enable_irq = 1;
    }
    if (softio_is_variable_included(sio, *head, mem.gpio_count_add)) {  // atomic write
      need_enable_irq = 1;
    }
  }
  if (need_enable_irq) __enable_irq();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  memory_init_user_code_begin_sys_init();
  sio.before = my_before;
	sio.after = my_after;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	print_info("SoftF103 on STM32F103C8T6, version 0x%08x, pid 0x%04X, compiled at %s, %s", mem.version, mem.pid, __TIME__, __DATE__);
	print_debug("mem size=%u, check this with your host to ensure struct correctness", sizeof(mem));
  SPI1->CR2 |= SPI_CR2_TXEIE;  // enable tx empty interrupt
  SPI1->CR2 |= SPI_CR2_RXNEIE;  // enable rx not empty interrupt
  SPI1->CR1 |= SPI_CR1_SPE;  // enable peripheral
  USART1->CR1 |= USART_CR1_TXEIE;  // enable tx empty interrupt
  USART1->CR1 |= USART_CR1_RXNEIE;  // enable rx not empty interrupt
  USART1->CR1 |= USART_CR1_UE;  // enable peripheral
  USART2->CR1 |= USART_CR1_TXEIE;  // enable tx empty interrupt
  USART2->CR1 |= USART_CR1_RXNEIE;  // enable rx not empty interrupt
  USART2->CR1 |= USART_CR1_UE;  // enable peripheral
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    softio_try_handle_all(sio);  // handle commands and return, non-blocking function
    usb_fifo_transmit();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
