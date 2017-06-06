#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "stm32f10x.h"
#include "Configuration.h"

#ifdef USE_GPS
extern GPS *pGPS1;
extern GPS *pGPS2;
extern GPS *pGPS3;
#endif
#ifdef USE_USART1
extern USART *pCOM1;
#endif
#ifdef USE_USART1_DMA
extern USART *pCOM1;
#endif

#ifdef USE_USART2
extern USART *pCOM2;
#endif
#ifdef USE_USART2_DMA
extern USART *pCOM2;
#endif

#ifdef USE_USART3
extern USART *pCOM3;
#endif
#ifdef USE_USART3_DMA
extern USART *pCOM3;
#endif

#ifdef USE_I2C1
extern I2C *pI2C1;
#endif

#ifdef USE_I2C2
extern I2C *pI2C2;
#endif

#ifdef USE_CAPTURE_TIM1
extern InputCapture_TIM *pICP1;
#endif

#ifdef USE_CAPTURE_TIM2
extern InputCapture_TIM *pICP2;
#endif

#ifdef USE_CAPTURE_TIM3
extern InputCapture_TIM *pICP3;
#endif

#ifdef USE_CAPTURE_TIM4
extern InputCapture_TIM *pICP4;
#endif
#ifdef USE_CAPTURE_EXIT0
extern InputCapture_EXIT *pICPExt0;
#endif
#ifdef USE_CAPTURE_EXIT1
extern InputCapture_EXIT *pICPExt1;
#endif
#ifdef USE_CAPTURE_EXIT2
extern InputCapture_EXIT *pICPExt2;
#endif
#ifdef USE_CAPTURE_EXIT3
extern InputCapture_EXIT *pICPExt3;
#endif
#ifdef USE_CAPTURE_EXIT4
extern InputCapture_EXIT *pICPExt4;
#endif
#ifdef USE_CAPTURE_EXIT5
extern InputCapture_EXIT *pICPExt5;
#endif
#ifdef USE_CAPTURE_EXIT6
extern InputCapture_EXIT *pICPExt6;
#endif
#ifdef USE_CAPTURE_EXIT7
extern InputCapture_EXIT *pICPExt7;
#endif
#ifdef USE_CAPTURE_EXIT8
extern InputCapture_EXIT *pICPExt8;
#endif
#ifdef USE_CAPTURE_EXIT9
extern InputCapture_EXIT *pICPExt9;
#endif
#ifdef USE_CAPTURE_EXIT10
extern InputCapture_EXIT *pICPExt10;
#endif
#ifdef USE_CAPTURE_EXIT11
extern InputCapture_EXIT *pICPExt11;
#endif
#ifdef USE_CAPTURE_EXIT12
extern InputCapture_EXIT *pICPExt12;
#endif
#ifdef USE_CAPTURE_EXIT13
extern InputCapture_EXIT *pICPExt13;
#endif
#ifdef USE_CAPTURE_EXIT14
extern InputCapture_EXIT *pICPExt14;
#endif
#ifdef USE_CAPTURE_EXIT15
extern InputCapture_EXIT *pICPExt15;
#endif

#ifdef USE_TIMER1
extern void Timer1_IRQ();
#endif

#ifdef USE_TIMER2
extern void Timer2_IRQ();
#endif

#ifdef USE_TIMER3
extern void Timer3_IRQ();
#endif

#ifdef USE_TIMER4
extern void Timer4_IRQ();
#endif


#ifdef USE_EXTI0
extern void	EXTI0_IRQ();
#endif
#ifdef USE_EXTI1
extern void	EXTI1_IRQ();
#endif
#ifdef USE_EXTI2
extern void	EXTI2_IRQ();
#endif
#ifdef USE_EXTI3
extern void	EXTI3_IRQ();
#endif
#ifdef USE_EXTI4
extern void	EXTI4_IRQ();
#endif
#ifdef USE_EXTI5
extern void	EXTI5_IRQ();
#endif
#ifdef USE_EXTI6
extern void	EXTI6_IRQ();
#endif
#ifdef USE_EXTI7
extern void	EXTI7_IRQ();
#endif
#ifdef USE_EXTI8
extern void	EXTI8_IRQ();
#endif
#ifdef USE_EXTI9
extern void	EXTI9_IRQ();
#endif
#ifdef USE_EXTI10
extern void	EXTI10_IRQ();
#endif
#ifdef USE_EXTI11
extern void	EXTI11_IRQ();
#endif
#ifdef USE_EXTI12
extern void	EXTI12_IRQ();
#endif
#ifdef USE_EXTI13
extern void	EXTI13_IRQ();
#endif
#ifdef USE_EXTI14
extern void	EXTI14_IRQ();
#endif
#ifdef USE_EXTI15
extern void	EXTI15_IRQ();
#endif


#ifdef USE_ULTRASONIC_EXTI0
extern Ultrasonic	*pUltExt0;
#endif
#ifdef USE_ULTRASONIC_EXTI1
extern Ultrasonic	*pUltExt1;
#endif
#ifdef USE_ULTRASONIC_EXTI2
extern Ultrasonic	*pUltExt2;
#endif
#ifdef USE_ULTRASONIC_EXTI3
extern Ultrasonic	*pUltExt3;
#endif
#ifdef USE_ULTRASONIC_EXTI4
extern Ultrasonic	*pUltExt4;
#endif
#ifdef USE_ULTRASONIC_EXTI5
extern Ultrasonic	*pUltExt5;
#endif
#ifdef USE_ULTRASONIC_EXTI6
extern Ultrasonic	*pUltExt6;
#endif
#ifdef USE_ULTRASONIC_EXTI7
extern Ultrasonic	*pUltExt7;
#endif
#ifdef USE_ULTRASONIC_EXTI8
extern Ultrasonic	*pUltExt8;
#endif
#ifdef USE_ULTRASONIC_EXTI9
extern Ultrasonic	*pUltExt9;
#endif
#ifdef USE_ULTRASONIC_EXTI10
extern Ultrasonic	*pUltExt10;
#endif
#ifdef USE_ULTRASONIC_EXTI11
extern Ultrasonic	*pUltExt11;
#endif
#ifdef USE_ULTRASONIC_EXTI12
extern Ultrasonic	*pUltExt12;
#endif
#ifdef USE_ULTRASONIC_EXTI13
extern Ultrasonic	*pUltExt13;
#endif
#ifdef USE_ULTRASONIC_EXTI14
extern Ultrasonic	*pUltExt14;
#endif
#ifdef USE_ULTRASONIC_EXTI15
extern Ultrasonic	*pUltExt15;
#endif


#endif

