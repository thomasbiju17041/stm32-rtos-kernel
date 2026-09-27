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

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
task_t *current_task;
task_t *next_task;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
void led_green_task(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

        for(volatile uint32_t i = 0; i < 1000000; i++);
    }
}

void led_orange_task(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

        for(volatile uint32_t i = 0; i < 1000000; i++);
    }
}


void schedule_next_task(void)
{
    task_t *prev = current_task;
    task_t *next = get_next_task();

    if(next != NULL)
    {
        if(prev->state == TASK_RUNNING)
        {
            prev->state = TASK_READY;
        }

        current_task = next;
        current_task->state = TASK_RUNNING;
    }
}

void trigger_pendsv(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void idle_task(void)
{
    while(1)
    {
        __WFI();
    }
}
void task_delay(uint32_t delay_ms)
{
    current_task->wake_tick = HAL_GetTick() + delay_ms;
    current_task->state = TASK_BLOCKED;

    trigger_pendsv();
}

task_t* get_next_task(void)
{
    int current_index = current_task - tasks;

    uint32_t highest_priority = 0;

    /* Find highest READY priority */
    for(int i = 0; i < NUM_TASKS; i++)
    {
        if(tasks[i].state == TASK_READY)
        {
            if(tasks[i].priority > highest_priority)
            {
                highest_priority = tasks[i].priority;
            }
        }
    }

    /* Search circularly starting after current task */
    for(int i = 1; i <= NUM_TASKS; i++)
    {
        int index = (current_index + i) % NUM_TASKS;

        if(tasks[index].state == TASK_READY &&
           tasks[index].priority == highest_priority)
        {
            return &tasks[index];
        }
    }

    return NULL;
}
/* USER CODE END 0 */
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
__attribute__((naked)) void save_context(void)
{
    __asm volatile(
        "MRS R0, PSP        \n"
        "STMDB R0!, {R4-R11}\n"
        "MSR PSP, R0        \n"
        "BX LR              \n"
    );
}

__attribute__((naked)) void restore_context(void)
{
    __asm volatile(
        "MRS R0, PSP        \n"
        "LDMIA R0!, {R4-R11}\n"
        "MSR PSP, R0        \n"
        "BX LR              \n"
    );
}


void save_current_task_context(void)
{
    save_context();

    current_task->sp = (uint32_t *)__get_PSP();
}

void restore_current_task_context(void)
{
    __set_PSP((uint32_t)current_task->sp);

    restore_context();
}
void start_scheduler(void)
{
    __asm volatile("SVC #0");
}

void sem_init(semaphore_t *sem, uint8_t initial)
{
    sem->available = initial;
}

void sem_take(semaphore_t *sem)
{
    while(1)
    {
        if(sem->available)
        {
            sem->available = 0;
            current_task->waiting_sem = NULL;
            return;
        }

        current_task->waiting_sem = sem;
        current_task->state = TASK_BLOCKED;

        trigger_pendsv();
    }
}

void sem_give(semaphore_t *sem)
{
    sem->available = 1;

    for(int i = 0; i < NUM_TASKS; i++)
    {
        if(tasks[i].state == TASK_BLOCKED &&
           tasks[i].waiting_sem == sem)
        {
            tasks[i].state = TASK_READY;
            tasks[i].waiting_sem = NULL;
        }
    }

    trigger_pendsv();
}
/* USER CODE END PFP */
void init_task_stack(task_t *task)
{
    uint32_t *sp;

    sp = &task->stack[STACK_SIZE - 1];

    *(sp--) = 0x01000000;               // xPSR
    *(sp--) = ((uint32_t)task->task_func) | 1U; // PC
    *(sp--) = (uint32_t)task_exit_error;             // LR (placeholder)

    *(sp--) = 0; // R12
    *(sp--) = 0; // R3
    *(sp--) = 0; // R2
    *(sp--) = 0; // R1
    *(sp--) = 0; // R0

    *(sp--) = 0; // R11
    *(sp--) = 0; // R10
    *(sp--) = 0; // R9
    *(sp--) = 0; // R8
    *(sp--) = 0; // R7
    *(sp--) = 0; // R6
    *(sp--) = 0; // R5
    *(sp--) = 0; // R4

    task->sp = sp + 1;
}

task_t tasks[NUM_TASKS] =
{
    {
        .wake_tick = 0,
        .priority = 5,
        .state = TASK_READY,
        .task_func = led_green_task
    },

    {
        .wake_tick = 0,
        .priority = 5,
        .state = TASK_READY,
        .task_func = led_orange_task
    },

    {
        .wake_tick = 0,
        .priority = 0,
        .state = TASK_READY,
        .task_func = idle_task
    }


};


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	current_task = &tasks[0];
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

  current_task = &tasks[1];   // highest priority

  start_scheduler();

//  set_psp((uint32_t)tasks[0].sp);
//
//  set_control(get_control() | 0x02);
//
//  __ISB();
//
//  save_task_context();
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
void task_exit_error(void)
{
    while(1);
}
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

void task_yield(void)
{
//    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);   // Blue

    trigger_pendsv();
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
