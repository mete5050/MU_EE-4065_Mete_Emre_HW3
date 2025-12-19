/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ALF.h"
#include "ALF_color.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern UART_HandleTypeDef huart3;   // <<< UART3
#define THR 64
#define IMG_SIZE (IMG_W * IMG_H)

#define IMG_SIZE_COLOR (IMG_W * IMG_H * 3)


/*-------------------- UART SEND ---------------------------*/
static void uart3_send(const uint8_t *data, uint32_t len)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)data, len, HAL_MAX_DELAY);
}
void uart_write(const char *msg)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

static void send_image_buffer(const uint8_t *buf)
{
    char hdr[64];
    int n = snprintf(hdr, sizeof(hdr), "P5\n%d %d\n255\n", IMG_W, IMG_H);
    uart3_send((uint8_t*)hdr, n);
    uart3_send(buf, IMG_SIZE);
}
static void send_color_image_buffer(const uint8_t *buf)
{
    char hdr[64];
    int n = snprintf(hdr, sizeof(hdr), "P6\n%d %d\n255\n", IMG_W, IMG_H);
    uart3_send((uint8_t*)hdr, n);
    uart3_send(buf, IMG_W * IMG_H * 3);
}
/*============================================================
=            Q1 - OTSU'S THRESHOLDING METHOD                 =
============================================================*/
uint8_t compute_otsu_threshold(const uint8_t *src) {
    uint32_t hist[256] = {0};
    for (uint32_t i = 0; i < IMG_SIZE; i++) hist[src[i]]++;

    float sum = 0;
    for (int i = 0; i < 256; i++) sum += (float)i * hist[i];

    float sumB = 0;
    uint32_t wB = 0;
    uint32_t wF = 0;
    float varMax = 0;
    uint8_t threshold = 0;

    for (int i = 0; i < 256; i++) {
        wB += hist[i];
        if (wB == 0) continue;
        wF = IMG_SIZE - wB;
        if (wF == 0) break;

        sumB += (float)(i * hist[i]);
        float mB = sumB / (float)wB;
        float mF = (sum - sumB) / (float)wF;

        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = (uint8_t)i;
        }
    }
    return threshold;
}

void apply_binary_threshold(const uint8_t *src, uint8_t *dst, uint8_t T) {
    for (uint32_t i = 0; i < IMG_SIZE; i++) {
        dst[i] = (src[i] > T) ? 255 : 0;
    }
}

/*============================================================
=                Q2 - COLOR IMAGE PROCESSING                 =
============================================================*/
uint8_t compute_otsu_threshold_color(const uint8_t *color_src, uint32_t w, uint32_t h) {
    uint32_t hist[256] = {0};
    uint32_t total_pixels = w * h;

    for (uint32_t i = 0; i < total_pixels; i++) {
        uint8_t r = color_src[i * 3];
        uint8_t g = color_src[i * 3 + 1];
        uint8_t b = color_src[i * 3 + 2];
        uint8_t gray = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
        hist[gray]++;
    }


    float sum = 0;
    for (int i = 0; i < 256; i++) sum += (float)i * hist[i];

    float sumB = 0;
    uint32_t wB = 0;
    float varMax = 0;
    uint8_t threshold = 0;

    for (int i = 0; i < 256; i++) {
        wB += hist[i];
        if (wB == 0) continue;
        uint32_t wF = total_pixels - wB;
        if (wF == 0) break;

        sumB += (float)(i * hist[i]);
        float mB = sumB / (float)wB;
        float mF = (sum - sumB) / (float)wF;
        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = (uint8_t)i;
        }
    }
    return threshold;
}

void apply_binary_threshold_color(const uint8_t *src, uint8_t *dst, uint32_t w, uint32_t h, uint8_t T) {
    for (uint32_t i = 0; i < w * h; i++) {
        uint8_t r = src[i * 3];
        uint8_t g = src[i * 3 + 1];
        uint8_t b = src[i * 3 + 2];
        uint8_t gray = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);

        dst[i] = (gray > T) ? 255 : 0;
    }
}

/*============================================================
=                Q3 - MORPHOLOGICAL OPERATIONS               =
============================================================*/
void img_erosion(const uint8_t *src, uint8_t *dst) {
    for (int y = 1; y < IMG_H - 1; y++) {
        for (int x = 1; x < IMG_W - 1; x++) {
            uint8_t min_val = 255;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    uint8_t pix = src[(y + ky) * IMG_W + (x + kx)];
                    if (pix < min_val) min_val = pix;
                }
            }
            dst[y * IMG_W + x] = min_val;
        }
    }
}

void img_dilation(const uint8_t *src, uint8_t *dst) {
    for (int y = 1; y < IMG_H - 1; y++) {
        for (int x = 1; x < IMG_W - 1; x++) {
            uint8_t max_val = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    uint8_t pix = src[(y + ky) * IMG_W + (x + kx)];
                    if (pix > max_val) max_val = pix;
                }
            }
            dst[y * IMG_W + x] = max_val;
        }
    }
}

void img_opening(const uint8_t *src, uint8_t *dst) {
    static uint8_t temp[IMG_SIZE];
    img_erosion(src, temp);
    img_dilation(temp, dst);
}

void img_closing(const uint8_t *src, uint8_t *dst) {
    static uint8_t temp[IMG_SIZE];
    img_dilation(src, temp);
    img_erosion(temp, dst);
}

void run_image_pipeline(void)
{
    static uint8_t bin_img[IMG_SIZE];
    static uint8_t morph_res[IMG_SIZE];
    uint8_t otsu_threshold;
    HAL_Delay(20000);
    otsu_threshold = compute_otsu_threshold(g_img); // [cite: 7]
    apply_binary_threshold(g_img, bin_img, otsu_threshold); //
    uart_write("--- Q1: Otsu Grayscale Result ---\n");
    send_image_buffer(bin_img);
    HAL_Delay(3000);


    otsu_threshold = compute_otsu_threshold_color(g_color_img, IMG_W, IMG_H); //
    apply_binary_threshold_color(g_color_img, bin_img, IMG_W, IMG_H, otsu_threshold); //

    uart_write("--- Q2: Otsu Color Result ---\n");
    send_image_buffer(bin_img);
    HAL_Delay(3000);


	img_erosion(bin_img, morph_res);
	uart_write("--- Q3: Erosion Result ---\n");
	send_image_buffer(morph_res);
	HAL_Delay(2000);

	img_dilation(bin_img, morph_res);
	uart_write("--- Q3: Dilation Result ---\n");
	send_image_buffer(morph_res);
	HAL_Delay(2000);


	img_opening(bin_img, morph_res);
	uart_write("--- Q3: Opening Result ---\n");
	send_image_buffer(morph_res);
	HAL_Delay(2000);

	img_closing(bin_img, morph_res);
	uart_write("--- Q3: Closing Result ---\n");
	send_image_buffer(morph_res);
	HAL_Delay(2000);
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
  HAL_Delay(100);
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1000);
    uart_write("STM32 Image Processing Pipeline Starting...\n");
     // static uint8_t buf[IMG_SIZE];
  /* USER CODE END 2 */
while(1){


	      run_image_pipeline();
	      uart_write("Cycle complete. Waiting 10 seconds...\n");
	      HAL_Delay(1000);

}
    /* USER CODE END WHILE */
}
    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */
  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */
  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_RED_Pin */
  GPIO_InitStruct.Pin = LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_RED_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
