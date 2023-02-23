/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <math.h>
#include <memory.h>
#include "digits.h"
#include "softuart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DOT_DELAY 8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// delay has to constant expression
static void inline __attribute__((always_inline)) delay(volatile uint32_t delay) {
  while (delay--) {
	  ;
  }
}

void setXY(uint16_t x, uint16_t y) {
  GPIOA->ODR = x;
  GPIOB->ODR = y << 3;
}

uint8_t second_id = 0;
uint8_t satellite_sync = 0;
uint32_t last_tick = 0;

void processMessage(uint8_t* buffer, uint32_t length) {
  if (length < 13) {
    return;
  }
  uint8_t* buffer_raw = buffer;
  if (memcmp("$GPRMC", buffer, 6u)) {
    return;
  }
  buffer += 7;

  uint8_t hours = *buffer - '0';
  hours *= 10;
  buffer++;
  hours += *buffer - '0';
  buffer++;

  uint8_t minutes = *buffer - '0';
  minutes *= 10;
  buffer++;
  minutes += *buffer - '0';
  buffer++;

  uint8_t seconds = *buffer - '0';
  seconds *= 10;
  buffer++;
  seconds += *buffer - '0';
  buffer++;

  buffer = buffer_raw + length - 1;
  while (buffer > buffer_raw) {
    if (*buffer == ',') {
      break;
    }
    buffer--;
  }
  buffer--;
  while (buffer > buffer_raw) {
    if (*buffer == ',') {
      break;
    }
    buffer--;
  }
  buffer--;
  while (buffer > buffer_raw) {
    if (*buffer == ',') {
      break;
    }
    buffer--;
  }
  if (buffer - buffer_raw < 6) {
    return;
  }

  buffer--;
  uint8_t year = *buffer - '0';
  buffer--;
  year += (*buffer - '0') * 10;
  buffer--;
  uint8_t month = *buffer - '0';
  buffer--;
  month += (*buffer - '0') * 10;
  buffer--;
  uint8_t day = *buffer - '0';
  buffer--;
  day += (*buffer - '0') * 10;

  RTC_TimeTypeDef time;
  time.Hours = (hours + 3) % 24;
  time.Minutes = minutes;
  time.Seconds = seconds;

  RTC_DateTypeDef date;
  date.Date = day;
  date.Month = month;
  date.Year = year;

  HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
}

const uint16_t satellite[512][2] = { { 3207, 889 }, { 3148, 834 }, { 3087, 781 }, { 3024, 732 }, { 2958, 686 }, { 2890, 643 }, { 2820, 603 }, { 2749, 567 }, { 2675, 534 }, { 2600, 505 }, { 2524, 480 }, { 2446, 459 }, { 2368, 441 }, { 2288, 427 }, { 2209, 417 }, { 2128, 412 }, { 2048, 410 }, { 1968, 412 }, { 1887, 417 }, { 1808, 427 }, { 1728, 441 }, { 1650, 459 }, { 1572, 480 }, { 1496, 505 }, { 1421, 534 }, { 1347, 567 }, { 1276, 603 }, { 1206, 643 }, { 1138, 686 }, { 1072, 732 }, { 1009, 781 }, { 948, 834 }, { 889, 889 }, { 834, 948 }, { 781, 1009 }, { 732, 1072 }, { 686, 1138 }, { 643, 1206 }, { 603, 1276 }, { 567, 1347 }, { 534, 1421 }, { 505, 1496 }, { 480, 1572 }, { 459, 1650 }, { 441, 1728 }, { 427, 1808 }, { 417, 1887 }, { 412, 1968 }, { 410, 2048 }, { 412, 2128 }, { 417, 2209 }, { 427, 2288 }, { 441, 2368 }, { 459, 2446 }, { 480, 2524 }, { 505, 2600 }, { 534, 2675 }, { 567, 2749 }, { 603, 2820 }, { 643, 2890 }, { 686, 2958 }, { 732, 3024 }, { 781, 3087 }, { 834, 3148 }, { 3207, 889 }, { 3170, 926 }, { 3134, 962 }, { 3098, 998 }, { 3062, 1034 }, { 3026, 1070 }, { 2989, 1107 }, { 2953, 1143 }, { 2917, 1179 }, { 2881, 1215 }, { 2844, 1252 }, { 2808, 1288 }, { 2772, 1324 }, { 2736, 1360 }, { 2700, 1396 }, { 2663, 1433 }, { 2627, 1469 }, { 2591, 1505 }, { 2555, 1541 }, { 2519, 1577 }, { 2482, 1614 }, { 2446, 1650 }, { 2410, 1686 }, { 2374, 1722 }, { 2338, 1758 }, { 2301, 1795 }, { 2265, 1831 }, { 2229, 1867 }, { 2193, 1903 }, { 2157, 1939 }, { 2120, 1976 }, { 2084, 2012 }, { 2048, 2048 }, { 2012, 2084 }, { 1976, 2120 }, { 1939, 2157 }, { 1903, 2193 }, { 1867, 2229 }, { 1831, 2265 }, { 1795, 2301 }, { 1758, 2338 }, { 1722, 2374 }, { 1686, 2410 }, { 1650, 2446 }, { 1614, 2482 }, { 1577, 2519 }, { 1541, 2555 }, { 1505, 2591 }, { 1469, 2627 }, { 1433, 2663 }, { 1396, 2700 }, { 1360, 2736 }, { 1324, 2772 }, { 1288, 2808 }, { 1252, 2844 }, { 1215, 2881 }, { 1179, 2917 }, { 1143, 2953 }, { 1107, 2989 }, { 1070, 3026 }, { 1034, 3062 }, { 998, 3098 }, { 962, 3134 }, { 926, 3170 }, { 1536, 2560 }, { 1552, 2560 }, { 1568, 2560 }, { 1584, 2560 }, { 1600, 2560 }, { 1616, 2560 }, { 1632, 2560 }, { 1648, 2560 }, { 1664, 2560 }, { 1680, 2560 }, { 1696, 2560 }, { 1712, 2560 }, { 1728, 2560 }, { 1744, 2560 }, { 1760, 2560 }, { 1776, 2560 }, { 1792, 2560 }, { 1808, 2560 }, { 1824, 2560 }, { 1840, 2560 }, { 1856, 2560 }, { 1872, 2560 }, { 1888, 2560 }, { 1904, 2560 }, { 1920, 2560 }, { 1936, 2560 }, { 1952, 2560 }, { 1968, 2560 }, { 1984, 2560 }, { 2000, 2560 }, { 2016, 2560 }, { 2032, 2560 }, { 2048, 2560 }, { 2064, 2560 }, { 2080, 2560 }, { 2096, 2560 }, { 2112, 2560 }, { 2128, 2560 }, { 2144, 2560 }, { 2160, 2560 }, { 2176, 2560 }, { 2192, 2560 }, { 2208, 2560 }, { 2224, 2560 }, { 2240, 2560 }, { 2256, 2560 }, { 2272, 2560 }, { 2288, 2560 }, { 2304, 2560 }, { 2320, 2560 }, { 2336, 2560 }, { 2352, 2560 }, { 2368, 2560 }, { 2384, 2560 }, { 2400, 2560 }, { 2416, 2560 }, { 2432, 2560 }, { 2448, 2560 }, { 2464, 2560 }, { 2480, 2560 }, { 2496, 2560 }, { 2512, 2560 }, { 2528, 2560 }, { 2544, 2560 }, { 2662, 2918 }, { 2687, 2917 }, { 2712, 2913 }, { 2737, 2907 }, { 2760, 2899 }, { 2783, 2888 }, { 2805, 2875 }, { 2825, 2860 }, { 2843, 2843 }, { 2860, 2825 }, { 2875, 2805 }, { 2888, 2783 }, { 2899, 2760 }, { 2907, 2737 }, { 2913, 2712 }, { 2917, 2687 }, { 2918, 2662 }, { 2917, 2637 }, { 2913, 2612 }, { 2907, 2588 }, { 2899, 2564 }, { 2888, 2542 }, { 2875, 2520 }, { 2860, 2500 }, { 2843, 2481 }, { 2825, 2465 }, { 2805, 2450 }, { 2783, 2437 }, { 2760, 2426 }, { 2737, 2417 }, { 2712, 2411 }, { 2687, 2408 }, { 2662, 2406 }, { 2637, 2408 }, { 2612, 2411 }, { 2588, 2417 }, { 2564, 2426 }, { 2542, 2437 }, { 2520, 2450 }, { 2500, 2465 }, { 2481, 2481 }, { 2465, 2500 }, { 2450, 2520 }, { 2437, 2542 }, { 2426, 2564 }, { 2417, 2588 }, { 2411, 2612 }, { 2408, 2637 }, { 2406, 2662 }, { 2408, 2687 }, { 2411, 2712 }, { 2417, 2737 }, { 2426, 2760 }, { 2437, 2783 }, { 2450, 2805 }, { 2465, 2825 }, { 2481, 2843 }, { 2500, 2860 }, { 2520, 2875 }, { 2542, 2888 }, { 2564, 2899 }, { 2588, 2907 }, { 2612, 2913 }, { 2637, 2917 }, { 2560, 2560 }, { 2560, 2544 }, { 2560, 2528 }, { 2560, 2512 }, { 2560, 2496 }, { 2560, 2480 }, { 2560, 2464 }, { 2560, 2448 }, { 2560, 2432 }, { 2560, 2416 }, { 2560, 2400 }, { 2560, 2384 }, { 2560, 2368 }, { 2560, 2352 }, { 2560, 2336 }, { 2560, 2320 }, { 2560, 2304 }, { 2560, 2288 }, { 2560, 2272 }, { 2560, 2256 }, { 2560, 2240 }, { 2560, 2224 }, { 2560, 2208 }, { 2560, 2192 }, { 2560, 2176 }, { 2560, 2160 }, { 2560, 2144 }, { 2560, 2128 }, { 2560, 2112 }, { 2560, 2096 }, { 2560, 2080 }, { 2560, 2064 }, { 2560, 2048 }, { 2560, 2032 }, { 2560, 2016 }, { 2560, 2000 }, { 2560, 1984 }, { 2560, 1968 }, { 2560, 1952 }, { 2560, 1936 }, { 2560, 1920 }, { 2560, 1904 }, { 2560, 1888 }, { 2560, 1872 }, { 2560, 1856 }, { 2560, 1840 }, { 2560, 1824 }, { 2560, 1808 }, { 2560, 1792 }, { 2560, 1776 }, { 2560, 1760 }, { 2560, 1744 }, { 2560, 1728 }, { 2560, 1712 }, { 2560, 1696 }, { 2560, 1680 }, { 2560, 1664 }, { 2560, 1648 }, { 2560, 1632 }, { 2560, 1616 }, { 2560, 1600 }, { 2560, 1584 }, { 2560, 1568 }, { 2560, 1552 }, { 2048, 410 }, { 2048, 403 }, { 2048, 397 }, { 2048, 390 }, { 2048, 384 }, { 2048, 378 }, { 2048, 371 }, { 2048, 365 }, { 2048, 358 }, { 2048, 352 }, { 2048, 346 }, { 2048, 339 }, { 2048, 333 }, { 2048, 326 }, { 2048, 320 }, { 2048, 314 }, { 2048, 307 }, { 2048, 301 }, { 2048, 294 }, { 2048, 288 }, { 2048, 282 }, { 2048, 275 }, { 2048, 269 }, { 2048, 262 }, { 2048, 256 }, { 2048, 250 }, { 2048, 243 }, { 2048, 237 }, { 2048, 230 }, { 2048, 224 }, { 2048, 218 }, { 2048, 211 }, { 2048, 205 }, { 2048, 198 }, { 2048, 192 }, { 2048, 186 }, { 2048, 179 }, { 2048, 173 }, { 2048, 166 }, { 2048, 160 }, { 2048, 154 }, { 2048, 147 }, { 2048, 141 }, { 2048, 134 }, { 2048, 128 }, { 2048, 122 }, { 2048, 115 }, { 2048, 109 }, { 2048, 102 }, { 2048, 96 }, { 2048, 90 }, { 2048, 83 }, { 2048, 77 }, { 2048, 70 }, { 2048, 64 }, { 2048, 58 }, { 2048, 51 }, { 2048, 45 }, { 2048, 38 }, { 2048, 32 }, { 2048, 26 }, { 2048, 19 }, { 2048, 13 }, { 2048, 6 }, { 2048, 0 }, { 2024, 0 }, { 2000, 0 }, { 1976, 0 }, { 1952, 0 }, { 1928, 0 }, { 1904, 0 }, { 1880, 0 }, { 1856, 0 }, { 1832, 0 }, { 1808, 0 }, { 1784, 0 }, { 1760, 0 }, { 1736, 0 }, { 1712, 0 }, { 1688, 0 }, { 1664, 0 }, { 1640, 0 }, { 1616, 0 }, { 1592, 0 }, { 1568, 0 }, { 1544, 0 }, { 1520, 0 }, { 1496, 0 }, { 1472, 0 }, { 1448, 0 }, { 1424, 0 }, { 1400, 0 }, { 1376, 0 }, { 1352, 0 }, { 1328, 0 }, { 1304, 0 }, { 1280, 0 }, { 1256, 0 }, { 1232, 0 }, { 1208, 0 }, { 1184, 0 }, { 1160, 0 }, { 1136, 0 }, { 1112, 0 }, { 1088, 0 }, { 1064, 0 }, { 1040, 0 }, { 1016, 0 }, { 992, 0 }, { 968, 0 }, { 944, 0 }, { 920, 0 }, { 896, 0 }, { 872, 0 }, { 848, 0 }, { 824, 0 }, { 800, 0 }, { 776, 0 }, { 752, 0 }, { 728, 0 }, { 704, 0 }, { 680, 0 }, { 656, 0 }, { 632, 0 }, { 608, 0 }, { 584, 0 }, { 560, 0 }, { 536, 0 }, { 512, 0 }, { 518, 14 }, { 524, 28 }, { 530, 42 }, { 536, 56 }, { 541, 69 }, { 547, 83 }, { 553, 97 }, { 559, 111 }, { 565, 125 }, { 571, 139 }, { 577, 153 }, { 583, 167 }, { 589, 181 }, { 595, 195 }, { 600, 208 }, { 606, 222 }, { 612, 236 }, { 618, 250 }, { 624, 264 }, { 630, 278 }, { 636, 292 }, { 642, 306 }, { 648, 320 }, { 654, 334 }, { 659, 347 }, { 665, 361 }, { 671, 375 }, { 677, 389 }, { 683, 403 }, { 689, 417 }, { 695, 431 }, { 701, 445 }, { 707, 459 }, { 713, 473 }, { 718, 486 }, { 724, 500 }, { 730, 514 }, { 736, 528 }, { 742, 542 }, { 748, 556 }, { 754, 570 }, { 760, 584 }, { 766, 598 }, { 772, 612 }, { 777, 625 }, { 783, 639 }, { 789, 653 }, { 795, 667 }, { 801, 681 }, { 807, 695 }, { 813, 709 }, { 819, 723 }, { 825, 737 }, { 830, 750 }, { 836, 764 }, { 842, 778 }, { 848, 792 }, { 854, 806 }, { 860, 820 }, { 866, 834 }, { 872, 848 }, { 878, 862 }, { 884, 876 } };

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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  SoftUartInit(GPIOA, GPIO_PIN_15);
  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  RTC_TimeTypeDef current_time;
  RTC_DateTypeDef current_date;

  uint8_t message_buffer[4096];
  uint32_t message_len = 0;

  while (1) {
    if (HAL_GetTick() - last_tick > 3000) {
      satellite_sync = 0;
    }

    HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    while (SoftUartRxAlavailable()) {
      uint8_t result = SoftUartReadRxBuffer(message_buffer + message_len);
      uint32_t new_len = message_len + result;
      for (uint32_t i = message_len; i < message_len + result; i++) {
        if (message_buffer[i] == '\n') {
          processMessage(message_buffer, i - 1);
          for (uint32_t j = i + 1; j < message_len + result; j++) {
            message_buffer[j - (i + 1)] = message_buffer[i];
          }
          new_len = (message_len + result) - (i + 1);
          break;
        }
      }
      message_len = new_len;
    }

    for (int j = 0; j < 256; j++) {
      int digit = current_time.Seconds / 10;
      setXY(digits[digit][j][0] / 8 + 2048 - 384, digits[digit][j][1] / 8 + 2048 + 1024 + 32);
      delay(DOT_DELAY);
    }
    for (int j = 0; j < 256; j++) {
      int digit = current_time.Seconds % 10;
      setXY(digits[digit][j][0] / 8 + 2048, digits[digit][j][1] / 8 + 2048 + 1024 + 32);
      delay(DOT_DELAY);
    }

    for (int j = 0; j < 256; j++) {
      int digit = current_time.Hours / 10;
      setXY(digits[digit][j][0] / 4 + 256, digits[digit][j][1] / 4 + 2048 - 512);
      delay(DOT_DELAY);
    }
    for (int j = 0; j < 256; j++) {
      int digit = current_time.Hours % 10;
      setXY(digits[digit][j][0] / 4 + 256 + 768, digits[digit][j][1] / 4 + 2048 - 512);
      delay(DOT_DELAY);
    }

    if (second_id || !satellite_sync) {
      setXY(2048 + 64, 2048 - 512 + 256);
      delay(DOT_DELAY * 40);
      setXY(2048 + 64, 2048 - 512 + 768);
      delay(DOT_DELAY * 40);
    }

    for (int j = 0; j < 256; j++) {
      int digit = current_time.Minutes / 10;
      setXY(digits[digit][j][0] / 4 + 256 + 1152 + 768, digits[digit][j][1] / 4 + 2048 - 512);
      delay(DOT_DELAY);
    }
    for (int j = 0; j < 256; j++) {
      int digit = current_time.Minutes % 10;
      setXY(digits[digit][j][0] / 4 + 256 + 1152 + 768 + 768, digits[digit][j][1] / 4 + 2048 - 512);
      delay(DOT_DELAY);
    }

    for (int j = 0; j < 256; j++) {
      int digit = current_date.Date / 10;
      setXY(digits[digit][j][0] / 8 + 2048 - 384 - 768 - 64, digits[digit][j][1] / 8 + 2048 - 1024 - 32 - 512);
      delay(DOT_DELAY);
    }
    for (int j = 0; j < 256; j++) {
      int digit = current_date.Date % 10;
      setXY(digits[digit][j][0] / 8 + 2048 - 768 - 64, digits[digit][j][1] / 8 + 2048 - 1024 - 32 - 512);
      delay(DOT_DELAY);
    }

    setXY(2048 - 256 - 96, 2048 - 1024 - 32 - 512);
    delay(DOT_DELAY * 40);

    for (int j = 0; j < 256; j++) {
      int digit = current_date.Month / 10;
      setXY(digits[digit][j][0] / 8 + 2048 - 384, digits[digit][j][1] / 8 + 2048 - 1024 - 32 - 512);
      delay(DOT_DELAY);
    }
    for (int j = 0; j < 256; j++) {
      int digit = current_date.Month % 10;
      setXY(digits[digit][j][0] / 8 + 2048, digits[digit][j][1] / 8 + 2048 - 1024 - 32 - 512);
      delay(DOT_DELAY);
    }

    setXY(2048 + 128 + 256 + 96, 2048 - 1024 - 32 - 512);
    delay(DOT_DELAY * 40);

    for (int j = 0; j < 256; j++) {
      int digit = current_date.Year / 10 % 10;
      setXY(digits[digit][j][0] / 8 + 2048 - 384 + 768 + 64, digits[digit][j][1] / 8 + 2048 - 1024 - 32 - 512);
      delay(DOT_DELAY);
    }
    for (int j = 0; j < 256; j++) {
      int digit = current_date.Year % 10;
      setXY(digits[digit][j][0] / 8 + 2048 + 768 + 64, digits[digit][j][1] / 8 + 2048 - 1024 - 32 - 512);
      delay(DOT_DELAY);
    }

    if (satellite_sync || (HAL_GetTick() % 500 >= 250)) {
      for (int i = 0; i < 16; i++) {
        setXY(satellite[0][0] / 8 + 2048 + 768, satellite[0][1] / 8 + 2048 + 1024);
        delay(DOT_DELAY);
      }
      for (int i = 0; i < 512; i++) {
        setXY(satellite[i][0] / 8 + 2048 + 768, satellite[i][1] / 8 + 2048 + 1024);
        delay(DOT_DELAY / 2);
      }
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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

  /** Initializes the CPU, AHB and APB buses clocks
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 49;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 59;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA8 PA9 PA10 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB12 PB13
                           PB14 PB3 PB4 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_1PPS_Pin */
  GPIO_InitStruct.Pin = INT_1PPS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(INT_1PPS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_RX_Pin */
  GPIO_InitStruct.Pin = SW_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW_RX_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
  if (pin != INT_1PPS_Pin) {
    return;
  }
  second_id = !second_id;
  last_tick = HAL_GetTick();
  satellite_sync = 1;
  delay(10000);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance != TIM2) {
    return;
  }
  SoftUartHandler();
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
  while (1) {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
