/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usb_host.h"
#include "gpio.h"
#include "core_cm4.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_TASKS 2
#define STACK_SIZE 128




typedef enum
{
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED
} task_state_t;

typedef struct
{
    uint32_t *sp;

    uint32_t stack[STACK_SIZE];

    uint32_t wake_tick;

    uint32_t priority;

    task_state_t state;

    void (*task_func)(void);

} task_t;
/* USER CODE END PD */
extern task_t tasks[];
void task_delay(uint32_t delay_ms);
void init_task_stack(task_t *task);
task_t* get_next_task(void);
/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
task_t *current_task;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
static inline void set_control(uint32_t control)
{
    __set_CONTROL(control);
}

static inline uint32_t get_control(void)
{
    return __get_CONTROL();
}

static inline void set_psp(uint32_t psp)
{
    __set_PSP(psp);
}

static inline uint32_t get_psp(void)
{
    return __get_PSP();
}

static inline uint32_t* get_current_sp(void)
{
    return (uint32_t*)get_psp();
}

static inline void save_current_task_sp(void)
{
    current_task->sp = get_current_sp();
}

void led_green_task(void)
{
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

    task_delay(500);
}

void led_orange_task(void)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

	task_delay(1000);
}

void task_delay(uint32_t delay_ms)
{
    current_task->wake_tick =
        HAL_GetTick() + delay_ms;

    current_task->state = TASK_BLOCKED;
}
/* USER CODE END PFP */
task_t tasks[NUM_TASKS] =
{
    {
        .wake_tick = 0,
        .priority = 2,
        .state = TASK_READY,
        .task_func = led_green_task
    },

    {
        .wake_tick = 0,
        .priority = 5,
        .state = TASK_READY,
        .task_func = led_orange_task
    }
};

void init_task_stack(task_t *task)
{
    uint32_t *sp;

    sp = &task->stack[STACK_SIZE - 1];

    *(sp--) = 0x01000000;               // xPSR
    *(sp--) = (uint32_t)task->task_func; // PC
    *(sp--) = 0xFFFFFFFD;               // LR (placeholder)

    *(sp--) = 0; // R12
    *(sp--) = 0; // R3
    *(sp--) = 0; // R2
    *(sp--) = 0; // R1
    *(sp--) = 0; // R0

    task->sp = sp + 1;
}
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
task_t* get_next_task(void)
{
    task_t *highest_task = NULL;

    for(int i = 0; i < NUM_TASKS; i++)
    {
        if(tasks[i].state == TASK_READY)
        {
            if((highest_task == NULL) ||
               (tasks[i].priority > highest_task->priority))
            {
                highest_task = &tasks[i];
            }
        }
    }

    return highest_task;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	current_task = &tasks[0];
	volatile uint32_t psp_val;
	volatile uint32_t control_val;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//  MX_I2C1_Init();
//  MX_I2S3_Init();
//  MX_SPI1_Init();
//  MX_USB_HOST_Init();
  /* USER CODE BEGIN 2 */


  for(int i = 0; i < NUM_TASKS; i++)
  {
      init_task_stack(&tasks[i]);
  }

  set_psp((uint32_t)tasks[0].sp);

  set_control(get_control() | 0x02);

  __ISB();

  psp_val = get_psp();
  control_val = get_control();

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
//    MX_USB_HOST_Process();

	    uint32_t now = HAL_GetTick();

	    /* Wake blocked tasks */
	    for(int i = 0; i < NUM_TASKS; i++)
	    {
	        if((tasks[i].state == TASK_BLOCKED) &&
	           (now >= tasks[i].wake_tick))
	        {
	            tasks[i].state = TASK_READY;
	        }
	    }

	    /* Select highest priority READY task */
	    task_t *next_task = get_next_task();

	    if(next_task != NULL)
	    {
		    current_task = next_task;
		    current_task->state = TASK_RUNNING;
	    	current_task->task_func();
	    }

	    if(current_task->state == TASK_RUNNING)
	    {
	        current_task->state = TASK_READY;
	    }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
