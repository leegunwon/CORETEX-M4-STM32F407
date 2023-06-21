//************************************//
// 과제명: HW3 엘레베이터
// 과제개요: *******************************
// 사용한 하드웨어(기능): GPIO,  EXTI, GLCD,  ...
// 제출일: 2023. 5.26
// 제출자 클래스: 화요일반
// 학번: 2018132027
// 이름: 이건원
//************************************//
#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t  KEY_Scan(void);
void BEEP(void);


void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t   SW0_Flag, SW1_Flag;

//****************************//

#define SW0_PUSH        0xFE00  
#define SW1_PUSH        0xFD00  
#define SW2_PUSH        0xFB00  
#define SW3_PUSH        0xF700  
#define SW4_PUSH        0xEF00  
#define SW5_PUSH        0xDF00  
#define SW6_PUSH        0xBF00  
#define SW7_PUSH        0x7F00  


// 내가 선언한 함수

void floor_mode(void);        // Floor mode 실행 함수
void execute_mode(void);   // execute_mode 실행 함수
void hold(void);                // hold mode 실행 함수
void display_floor(int axis_y, int axis_x, int floor);   // 좌표와 int 값 입력 시 LCD에 Display 해주는 함수
int L_elevate(int x, int y);   //  출발 위치와 도착 위치를 입력하면 이동해주는 함수
int R_elevate(int x, int y);   //  출발 위치와 도착 위치를 입력하면 이동해주는 함수
void GREEN_BOX(void);    // 1층에서 6층 길이의 초록색 바를 만들어 주는 함수
void BLUE_BOX(void);      // 1층에서 6층 길이의 파란색 바를 만들어 주는 함수
void R_SET_BOX(int x);    // 층 값을 입력시 해당 층 길이의 초록색 바를 만들어 주는 함수
void L_SET_BOX(int x);    // 층 값을 입력시 해당 층 길이의 파란색 바를 만들어 주는 함수


// 내가 선언한 변수
int floor_start = 1;     // 출발 층
int floor_dest= 1;      // 목표 층
int elevL_curr = 1;  // 엘레베이터 Left 현재 층
int elevR_curr = 1;  //엘레베이터 Right 현재 층
int comp_number = 0;   // 밑에서 숫자 비교할 때 사용할 임시 변수

int LED1_ON = 0;   // LED 1이 켜져 있는지 꺼져있는지 확인 하는 변수
int LED2_ON = 0;   // LED 2가 켜져 있는지 꺼져있는지 확인 하는 변수
int input_v = 0;     // 엘레베이터 입력 받았을 때

//****************************//
int main(void)
{
   _GPIO_Init();        
   _EXTI_Init();    
   LCD_Init();  
   
   Fram_Init();               
   Fram_Status_Config();   
 
   DelayMS(100);         
   BEEP();
   
   GPIOG->ODR &= 0xFF00;	        // 초기값: LED0~7 Off
   GPIOG->BSRRL = 0x0080;         // LED 7 ON
   
   
   DisplayInitScreen();   
   
   if(Fram_Read(2025) ==1){    // FRAM에 elevator 현재 층이 저장되어있을 경우 불러옴
    elevL_curr = Fram_Read(2023);
    elevR_curr = Fram_Read(2024);

    L_SET_BOX(80 -  (80/6)*elevL_curr);     // 엘레베이터 좌측 막대 
    R_SET_BOX(80 -  (80/6)*elevR_curr);     // 엘레베이터 우측 막대 
    
    
    display_floor(6, 7, elevL_curr);
    display_floor(6, 5, elevL_curr);
    
    LCD_DisplayChar(5,6, 'S');
    
    floor_start = elevL_curr;
    floor_dest = elevL_curr;
    
    GPIOG->ODR &= 0xFF00;	        // 초기값: LED0~7 Off
    GPIOG->BSRRL = 0x0080;         // LED 7 ON
    }   
   
   while(1){   
   floor_mode();
   }
}

void floor_mode()
{
        GPIOG->ODR &= 0xFF00;	        // 초기값: LED0~7 Off
        GPIOG->BSRRL = 0x0080;         // LED 7 ON
        
        while(1){       
                LCD_DisplayText(2, 5, "FL");
                switch(KEY_Scan())  
                {
                        case SW1_PUSH :          // SW1
                                if(floor_start == 6){                                             
                                            GPIOG->ODR &= ~0x0004;   // LED 2 OFF
                                            GPIOG->BSRRL = 0x0002;    // LED 1 ON
                                            BEEP();
                                            floor_start = 1;
                                            display_floor(6, 5,floor_start);
                                        }
                                 else{                                         
                                            GPIOG->ODR &= ~0x0004;   // LED2 OFF
                                            GPIOG->BSRRL = 0x0002;    // LED1 ON
                                            BEEP();
                                            floor_start ++;
                                            display_floor(6, 5,floor_start);
                                        }
                                 break;

                        case SW2_PUSH :          // SW2
                                    if(floor_dest == 6){
                                            GPIOG->ODR &= ~0x0002;  //LED 1 OFF
                                            GPIOG->BSRRL = 0x0004;   // LED2 ON
                                            BEEP();
                                            floor_dest = 1;
                                            display_floor(6, 7, floor_dest);
                                    }
                                    else{
                                            GPIOG->ODR &= ~0x0002;  // LED 1 OFF
                                                GPIOG->BSRRL = 0x0004;   // LED 2 ON
                                                BEEP();         
                                                floor_dest ++;
                                                display_floor(6, 7, floor_dest);
                                        }
                                break;
                }  
                 
                
                if(SW0_Flag ==1){
                        GPIOG->ODR &= ~0x0006;
                        execute_mode();
                        SW0_Flag =0;
                }
        }
}

void execute_mode(){
        GPIOG->BSRRL = 0x0001;
         LCD_DisplayText(2, 5, "EX");
        BEEP();  
              
        if(abs(elevL_curr - floor_start) > abs(elevR_curr - floor_start)){     // 호출 층과 가까운 엘레베이터 정함
                
                LCD_SetTextColor(RGB_GREEN);
                LCD_DisplayText(4, 5, "R-E");
                LCD_SetTextColor(RGB_RED);
                
                elevR_curr = R_elevate(elevR_curr, floor_start);   // 엘레베이터 현재 위치에서 호출 층까지 이동
                DelayMS(500);
                LCD_DisplayChar(5,6, 'S');
                elevR_curr = R_elevate(elevR_curr, floor_dest);   // 호출 층에서 목적 층까지 이동
                DelayMS(500);
                LCD_DisplayChar(5,6, 'S');
        }
        
        else if(abs(elevL_curr - floor_start) < abs(elevR_curr - floor_start)){ // 호출 층과 가까운 엘레베이터 정함
                LCD_SetTextColor(RGB_BLUE);
                LCD_DisplayText(4, 5, "L-E");
                LCD_SetTextColor(RGB_RED);
                elevL_curr = L_elevate(elevL_curr, floor_start);   // 엘레베이터 현재 위치에서 호출 층까지 이동
                DelayMS(500);
                LCD_DisplayChar(5,6, 'S');
                elevL_curr = L_elevate(elevL_curr, floor_dest);  // 호출 층에서 목적 층까지 이동
                DelayMS(500);
                LCD_DisplayChar(5,6, 'S');
        }
        else if(abs(elevL_curr - floor_start) == abs(elevR_curr - floor_start)){ // 호출 층과 가까운 엘레베이터 정함
                LCD_SetTextColor(RGB_BLUE);
                LCD_DisplayText(4, 5, "L-E"); 
                LCD_SetTextColor(RGB_RED);
                elevL_curr = L_elevate(elevL_curr, floor_start);  // 엘레베이터 현재 위치에서 호출 층까지 이동
                LCD_DisplayChar(5,6, 'S');
                DelayMS(500);
                elevL_curr = L_elevate(elevL_curr, floor_dest);  // 호출 층에서 목적 층까지 이동
                DelayMS(500);
                LCD_DisplayChar(5,6, 'S');
          
        }
        DelayMS(500);
        BEEP();
        DelayMS(500);
        BEEP();
        DelayMS(500);
        BEEP();
        GPIOG->ODR &= ~0x0001;          // LED 0 OFF
        GPIOG->ODR |= 0x0080;             // LED 7 ON
        
	Fram_Write(2023, elevL_curr);           // FRAM 2023 주소에 Left elevator
	Fram_Write(2024, elevR_curr);           // FRAM 2024 주소에 Left elevator
        Fram_Write(2025, 1);                      // FRAM 2023과 2024에 주소가 저장되어있을 경우 불러오기 위해  2025에 1을 flag로 사용
        
}









int L_elevate(int x, int y)
{            
  if ( x < y){
    int k= y - x;
    for(int i=0; i < k; i++){      // 출발 층과 도착 층의 층 차이 만큼 반복
      LCD_DisplayText(2, 5, "EX");
      GPIOG->ODR &= ~0x0040;   // LED6 OFF
      GPIOG->ODR &= ~0x0080;   // LED7 OFF
      GPIOG->ODR |= 0x0001;   // LED0 ON
      DelayMS(500);
      x++;
      L_SET_BOX(80 -  (80/6)*x);
      LCD_DisplayChar(5,6, 'U');
    }
  }
  else if(x>y){
    int k= y - x;
     for(int i =0; i>k ; i--){   // 출발 층과 도착 층의 층 차이 만큼 반복
       
        LCD_DisplayText(2, 5, "EX");
        GPIOG->ODR &= ~0x0040;   // LED6 OFF
        GPIOG->ODR &= ~0x0080;   // LED7 OFF
        GPIOG->ODR |= 0x0001;   // LED0 ON
        DelayMS(500);
        x--;
        L_SET_BOX(80 -  (80/6)*x);
        LCD_DisplayChar(5,6, 'D');
    }
  }
   else if(x==y){      // 출발 층과 도착 층이 같을 경우 1초 Delay
        DelayMS(1000);
   }   
return x;
}

int R_elevate(int x, int y)
{            
  if ( x < y){
    int k= y - x;
    for(int i=0; i < k; i++){     // 출발 층과 도착 층의 층 차이 만큼 반복
      LCD_DisplayText(2, 5, "EX");
      GPIOG->ODR &= ~0x0040;   // LED6 OFF
      GPIOG->ODR &= ~0x0080;   // LED7 OFF
      GPIOG->ODR |= 0x0001;   // LED0 ON
      DelayMS(500);
      x++;
      R_SET_BOX(80 -  (80/6)*x);
      LCD_DisplayChar(5,6, 'U');
    }
  }
  else if(x>y){
    int k= y - x;
     for(int i =0; i>k ; i--){     // 출발 층과 도착 층의 층 차이 만큼 반복
        LCD_DisplayText(2, 5, "EX");
        GPIOG->ODR &= ~0x0040;   // LED6 OFF
        GPIOG->ODR &= ~0x0080;   // LED7 OFF
        GPIOG->ODR |= 0x0001;   // LED0 ON
        DelayMS(500);
        x--;
        R_SET_BOX(80 -  (80/6)*x);
        LCD_DisplayChar(5,6, 'D');
    }
  }
   else if(x==y){     // 출발 층과 도착 층이 같을 경우 1초 Delay
        DelayMS(1000);
   }   
return x;
}


void display_floor(int axis_y, int axis_x, int floor){
    if(floor == 0){
       LCD_DisplayChar(axis_y, axis_x, '0');
  }
  else if(floor == 1){
       LCD_DisplayChar(axis_y, axis_x, '1');
  }
  else if(floor == 2){
        LCD_DisplayChar(axis_y, axis_x, '2');
  }
  else if(floor == 3){
        LCD_DisplayChar(axis_y, axis_x, '3');
  }
  else if(floor == 4){
        LCD_DisplayChar(axis_y, axis_x, '4');
  }
  else if(floor == 5){
        LCD_DisplayChar(axis_y, axis_x, '5');
  }
  else if(floor == 6){
        LCD_DisplayChar(axis_y, axis_x, '6');
  }
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
   LCD_Clear(RGB_WHITE);      // 화면 하얀색
   LCD_SetFont(&Gulim8);      // 폰트 : 굴림 8
   LCD_SetBackColor(RGB_WHITE);   //글자배경색 : 하얀색

   LCD_SetTextColor(RGB_BLACK);   // 글자색 : Blue

   LCD_DisplayText(0, 0, " MC-Elevator(LGW)");     // Title

   LCD_SetTextColor(RGB_BLUE);   // 글자색 : Black

   LCD_DisplayChar(2, 2, '6');
   LCD_DisplayChar(3, 2, '5');
   LCD_DisplayChar(4, 2, '4');
   LCD_DisplayChar(5, 2, '3');
   LCD_DisplayChar(6, 2, '2');
   LCD_DisplayChar(7, 2, '1');

   LCD_DisplayText(4, 5, "L-E");
   LCD_SetTextColor(RGB_GREEN);

   LCD_DisplayChar(2, 10, '6');
   LCD_DisplayChar(3, 10, '5');
   LCD_DisplayChar(4, 10, '4');
   LCD_DisplayChar(5, 10, '3');
   LCD_DisplayChar(6, 10, '2');
   LCD_DisplayChar(7, 10, '1');

   LCD_SetTextColor(RGB_BLACK);   // 글자색 : 
   LCD_DisplayChar(6, 6, '>');

   LCD_SetTextColor(RGB_RED);   // 글자색 : RED
   LCD_DisplayText(2, 5, "FL");
   LCD_DisplayChar(6, 5, '1');
   LCD_DisplayChar(6, 7, '1');
   LCD_DisplayChar(5, 6, 'S');

   L_SET_BOX(70);    // 
   R_SET_BOX(70);
   
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정   */
void _GPIO_Init(void)
{
   // LED (GPIO G) ???? : Output mode
   RCC->AHB1ENR   |=  0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
   GPIOG->MODER    |=  0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
   GPIOG->OTYPER   &= ~0x00FF;      // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
   GPIOG->OSPEEDR    |=  0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
   // SW (GPIO H) ???? : Input mode 
   RCC->AHB1ENR    |=  0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
   GPIOH->MODER    &= ~0xFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
   GPIOH->PUPDR    &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

   // Buzzer (GPIO F) ???? : Output mode
   RCC->AHB1ENR   |=  0x00000020;   // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
   GPIOF->MODER    |=  0x00040000;   // GPIOF 9 : Output mode (0b01)                  
   GPIOF->OTYPER    &= ~0x0200;      // GPIOF 9 : Push-pull     
   GPIOF->OSPEEDR    |=  0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 
}

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
   RCC->AHB1ENR |= 0x00000080;   // RCC_AHB1ENR GPIOH Enable
   RCC->APB2ENR |= 0x00004000;   // Enable System Configuration Controller Clock

   GPIOH->MODER &= ~0xFFFF0000;   // GPIOH PIN8~PIN15 Input mode (reset state)             

   SYSCFG->EXTICR[2] |= 0x0007;
   SYSCFG->EXTICR[3] |= 0x7000;   // EXTI15에 대한 소스 입력은 GPIOH로 설정      

   EXTI->RTSR |= 0x008100;      // EXTI15: Rising Trigger  Enable 
   EXTI->IMR |= 0x008100;      // EXTI15 인터럽트 mask (Interrupt Enable) 설정

   NVIC->ISER[0] |= 1 << (23);
   NVIC->ISER[1] |= 1 << (40 - 32);
   // Enable 'Global Interrupt EXTI8,9'
            // Vector table Position 참조
}


void GREEN_BOX(void)
{
   LCD_SetBrushColor(RGB_GREEN);
   LCD_DrawFillRect(90, 25, 10, 80);  // y 축 좌표는 90이 적당
}

void BLUE_BOX(void)
{
   LCD_SetBrushColor(RGB_BLUE);
   LCD_DrawFillRect(5, 25, 10, 80);  // y 축 좌표는 5가 적당   y좌표 + y 축 길이의 합이 105가 되게
}

void L_SET_BOX(int x)
{
   BLUE_BOX();
   LCD_SetBrushColor(RGB_WHITE);
   LCD_DrawFillRect(5, 25, 10, x);
}

void R_SET_BOX(int x)
{
   GREEN_BOX();
   LCD_SetBrushColor(RGB_WHITE);
   LCD_DrawFillRect(90, 25, 10, x);
}


void EXTI9_5_IRQHandler(void)
{		
	if(EXTI->PR & 0x0100)			// EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI-> PR |= 0x0100;		// Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
		SW0_Flag = 1;			// SW0_Flag: EXTI8이 발생되었음을 알리기 위해 만든 변수(main문의 mission에 사용) 
	}
}

void EXTI15_10_IRQHandler(void)
{
  if(SW0_Flag == 1){         // EXTI8이 발생 하고 난 후에만 hold 동작하기 위해 flag 변수 조건 사용
   if (EXTI->PR & 0x8000)      // EXTI15 Interrupt Pending(발생) 여부?
   {
          hold();
    }
  }
  EXTI->PR |= 0x8000;       // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
  GPIOG->ODR &= ~0x0040;   // LED6 On
}

void hold() {   //HOLDING 동작
      LCD_DisplayText(2, 5, "HD");
      GPIOG->ODR |= 0x0040;   // LED6 On
      GPIOG->ODR &= ~0x0080;   // LED7 OFF
      GPIOG->ODR &= ~0x0001;   // LED0 OFF
      for(int i=0; i<10; i++)
      {
          DelayMS(500);
          BEEP();
      }

}


/* Switch가 입력되었는지 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)   // input key SW0 - SW7 
{
   uint16_t key;
   key = GPIOH->IDR & 0xFF00;   // any key pressed ?
   if (key == 0xFF00)      // if no key, check key off
   {
      if (key_flag == 0)
         return key;
      else
      {
         DelayMS(10);
         key_flag = 0;
         return key;
      }
   }
   else            // if key input, check continuous key
   {
      if (key_flag != 0)   // if continuous key, treat as no key input
         return 0xFF00;
      else         // if new key,delay for debounce
      {
         key_flag = 1;
         DelayMS(10);
         return key;
      }
   }
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)
{
   GPIOF->ODR |= 0x0200;   // PF9 'H' Buzzer on
   DelayMS(10);      // Delay 30 ms
   GPIOF->ODR &= ~0x0200;   // PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
   register unsigned short i;
   for (i = 0; i < wMS; i++)
      DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
   volatile int Dly = (int)wUS * 17;
   for (; Dly; Dly--);
}