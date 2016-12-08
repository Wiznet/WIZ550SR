
#ifndef __SPIHANDLER_H__
#define __SPIHANDLER_H__

#include "common.h"

#if (WIZ550SR_ENABLE == 1)
#define W5500_SPI                       SPI1
#define W5500_SPI_CLK                   RCC_APB2Periph_SPI1

#define W5500_SPI_SCK_PIN               GPIO_Pin_5                  /* PA.5 */
#define W5500_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */
#define W5500_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOA

#define W5500_SPI_MISO_PIN              GPIO_Pin_6                  /* PA.6 */
#define W5500_SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */
#define W5500_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOA

#define W5500_SPI_MOSI_PIN              GPIO_Pin_7                  /* PA.7 */
#define W5500_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
#define W5500_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOA

#define W5500_CS_PIN                    GPIO_Pin_4                  /* PA.4 */
#define W5500_CS_GPIO_PORT              GPIOA                       /* GPIOA */
#define W5500_CS_GPIO_CLK               RCC_APB2Periph_GPIOA

#define W5500_DUMMY_BYTE				0xFF
#define W5500_RESET_PIN                 GPIO_Pin_4
#define W5500_RESET_PORT                GPIOC
#define INT_W5500_PIN					GPIO_Pin_5	//in
#define INT_W5500_PORT					GPIOC
#else
#define W5500_SPI                       SPI2
#define W5500_SPI_CLK                   RCC_APB1Periph_SPI2

#define W5500_SPI_SCK_PIN               GPIO_Pin_13                  /* PB.13 */
#define W5500_SPI_SCK_GPIO_PORT         GPIOB                       /* GPIOB */
#define W5500_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOB

#define W5500_SPI_MISO_PIN              GPIO_Pin_14                  /* PB.14 */
#define W5500_SPI_MISO_GPIO_PORT        GPIOB                       /* GPIOB */
#define W5500_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOB

#define W5500_SPI_MOSI_PIN              GPIO_Pin_15                  /* PB.15 */
#define W5500_SPI_MOSI_GPIO_PORT        GPIOB                       /* GPIOB */
#define W5500_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOB

#define W5500_CS_PIN                    GPIO_Pin_12                  /* PB.12 */
#define W5500_CS_GPIO_PORT              GPIOB                       /* GPIOB */
#define W5500_CS_GPIO_CLK               RCC_APB2Periph_GPIOB

#define W5500_DUMMY_BYTE				0xFF
#if !defined(CHINA_BOARD)
#define W5500_RESET_PIN                 GPIO_Pin_0
#define W5500_RESET_PORT                GPIOB
#define INT_W5500_PIN					GPIO_Pin_1	//in
#define INT_W5500_PORT					GPIOB
#else
#define W5500_RESET_PIN                 GPIO_Pin_9
#define W5500_RESET_PORT                GPIOB
#define INT_W5500_PIN					GPIO_Pin_0	//in
#define INT_W5500_PORT					GPIOB
#endif
#endif

void W5500_SPI_Init(void);
void W5500_Init(void);
//void Net_Conf();

#endif
