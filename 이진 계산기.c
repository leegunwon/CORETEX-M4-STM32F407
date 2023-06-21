//************************************//
// 과제명: TP2 이진 연산기
// 과제개요: 2진수 연산을 해주는 연산기 제작
// 사용한 하드웨어(기능): GPIO,  EXTI, GLCD,  ...
// 제출일: 2023. 6.11
// 제출자 클래스: 화요일반
// 학번: 2018132027
// 이름: 이건원
//************************************//


#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW2_PUSH        0xFB00  
#define SW3_PUSH        0xF700  
#define SW4_PUSH        0xEF00  
#define SW5_PUSH        0xDF00  


void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t JOY_Scan(void);
uint16_t KEY_Scan(void);
void BEEP(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t   SW0_Flag, SW1_Flag;
//*********************************************
int excape_Flag = 0;

int A_right = 0;
int A_left = 0;
int Aopnd = 0;


int B_right = 0;
int B_left = 0;
int Bopnd = 0;

int Cresult = 0;

int Ope_num = 0;
char oper;

int D = 0;
int add();       
int subtract();
int multiply();
int logical_and();
int logical_or();
int logical_xor();

void select_operator();  // 연산자 선택
void Dec_Bin(int opnd);    // 10진수에서 2진수로
void J_UP();                //  연속 증가 모드
void J_RIGHT();          //  연산자 전환
int calculate();           //결과 계산 함수
//***********************************************

int main(void)
{
    LCD_Init();   // LCD 모듈 초기화
    DelayMS(10);
    _GPIO_Init();   // GPIO (LED & SW) 초기화
    _EXTI_Init();   // EXTI 초기화
    Fram_Init();   
    Fram_Status_Config();   
    DisplayInitScreen();   // LCD 초기화면

    GPIOG->ODR &= ~0x00FF;      // 초기값: LED0~7 Off
    
    if(Fram_Read(532) == 1){
      Ope_num = Fram_Read(530);
      select_operator();
      Cresult = Fram_Read(531);
      Dec_Bin(Cresult);
    }
    while (1)
    {
        switch (KEY_Scan())   // 입력된 Switch 정보 분류 
        {
        case SW2_PUSH:    //SW2
            BEEP();
            A_left++;
            Aopnd = (A_left % 2) * 2 + (A_right % 2);    // A 2진수의 값
            LCD_DisplayChar(2, 4, (A_left%2)+0x30);  // A_left  
            break;
        case SW3_PUSH:    //SW3
            BEEP();
            A_right++;
            Aopnd = (A_left % 2) * 2 + (A_right % 2);     // A 2진수의 값
            LCD_DisplayChar(4, 4, (A_right%2)+0x30);  // A_right
            break;
        case SW4_PUSH:    //SW4
            BEEP();
            B_left++;
            Bopnd = (B_left % 2) * 2 + (B_right % 2);    // B 2진수의 값
            LCD_DisplayChar(6, 4, (B_left%2)+0x30);  // B_left
            break;
        case SW5_PUSH:    //SW5
            BEEP();
            B_right++;
            Bopnd = (B_left % 2) * 2 + (B_right % 2);     // B 2진수의 값
            LCD_DisplayChar(7, 4, (B_right%2)+0x30);  // B_right
            break;
        }

    }
}

int add() {               //덧셈
    int C = Aopnd + Bopnd;
    return C;
}

int subtract() {         // 뺄셈
    if (Aopnd >= Bopnd) {
        D = 0;
        int C = Aopnd - Bopnd;
        return C;
    }
    else {
        D = 1;
        int C = Bopnd - Aopnd;
        return C;
    }
}

int multiply() {         // 곱셈
    int C = Aopnd * Bopnd;
    return C;
}

int logical_and() {       // And 연산
    int C_right = 0;
    int C_left = 0;
    if ((B_right%2) == (A_right%2)) {
        C_right = B_right;
    }

    if ((B_right%2) != (A_right%2)) {
        C_right = 0;
    }

    if ((B_left%2) == (A_left%2)) {
        C_left = B_left;
    }

    if ((B_left%2) != (A_left%2)) {
        C_left = 0;
    }
    int C = (C_left % 2) * 2 + (C_right % 2);
    return C;
}

int logical_or() {        // Or 연산

    int C_right = 0;
    int C_left = 0;
    if ((B_right%2) == (A_right%2)) {
        C_right = B_right;
    }

    if ((B_right%2) != (A_right%2)) {
        C_right = 1;
    }

    if ((B_left%2) == (A_left%2)) {
        C_left = B_left;
    }

    if ((B_left%2) != (A_left%2)) {
        C_left = 1;
    }
    
    int C = (C_left % 2) * 2 + (C_right % 2);
    return C;
}
 
int logical_xor() {     // Xor 연산
    int C_right = 0;
    int C_left = 0;
    if ((B_right%2) == (A_right%2)) {
        C_right = 0;
    }

    if ((B_right%2) != (A_right%2)) {
        C_right = 1;
    }

    if ((B_left%2) == (A_left%2)) {
        C_left = 0;
    }

    if ((B_left%2) != (A_left%2)) {
        C_left = 1;
    }
    
    int C = (C_left % 2) * 2 + (C_right % 2);
    return C;
}

void Dec_Bin(int opnd) {      // 10진수의 값을 입력 했을 때 2진수의 결과 값을 LCD에 출력해주는 코드

    switch (opnd % 16) {
    case 0:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 1:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
                            
    case 2:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 3:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    case 4:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 5:   LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    case 6:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 7:  LCD_DisplayChar(4, 15, '0');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    case 8:   LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 9:  LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    case 10: LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 11: LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '0'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    case 12: LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 13: LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '0'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    case 14: LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '0'); // 네 번째 값 
                   break;
    case 15: LCD_DisplayChar(4, 15, '1');  // 첫 번째 값 
                   LCD_DisplayChar(5, 15, '1'); // 두 번째 값 
                   LCD_DisplayChar(6, 15, '1'); // 세 번째 값  
                   LCD_DisplayChar(7, 15, '1'); // 네 번째 값 
                   break;
    }
}

void J_RIGHT() {           // Operator 전환
    BEEP();
    Ope_num++;
    select_operator(Ope_num);
}

void J_UP() {              //  연속 증가 모드

    excape_Flag = 1;

    while (excape_Flag) {
        Cresult++;
        Dec_Bin(Cresult);
        DelayMS(500);
    }
}

int calculate(){        // 연산 결과 출력
  D = 0;
  int res = 0;
  switch(Ope_num%6){
    
  case 0: res = add(); break;
  case 1: res = subtract(); break;
  case 2: res = multiply(); break;
  case 3: res = logical_and(); break;
  case 4: res = logical_or(); break;
  case 5: res = logical_xor(); break;
  
  }
  if (D==0){
      LCD_DisplayChar(2, 15, '+');  // 부호  
  }
   else if (D==1){
      LCD_DisplayChar(2, 15, '-');  // 부호  
  }
  
  Fram_Write(530, Ope_num);           
  Fram_Write(531, res);  
  Fram_Write(532, 1); 
  
  return res;
}

void select_operator() {            // 부호를 바꾸고 LCD에 출력
    LCD_SetBackColor(GET_RGB(255, 192, 203));
    switch (Ope_num % 6) {

    case 0: LCD_DisplayChar(4, 10, '+'); break;  // operator break;
    case 1: LCD_DisplayChar(4, 10, '-'); break;
    case 2: LCD_DisplayChar(4, 10, 'x'); break;
    case 3: LCD_DisplayChar(4, 10, '&'); break;
    case 4: LCD_DisplayChar(4, 10, '|'); break;
    case 5: LCD_DisplayChar(4, 10, '^'); break;

    }
    LCD_SetBackColor(RGB_YELLOW);
    Fram_Write(530, Ope_num); 
    Fram_Write(532, 1); 
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
    LCD_Clear(RGB_WHITE);      // 화면 클리어
    LCD_SetFont(&Gulim8);      // 폰트 : 굴림 8
    LCD_SetBackColor(RGB_YELLOW);   // 글자배경색 : Green
    LCD_SetTextColor(RGB_BLACK);   // 글자색 : Black

    LCD_SetBrushColor(RGB_YELLOW);
    LCD_DrawFillRect(30, 25, 13, 13);   // A_left  
    LCD_DrawFillRect(30, 51, 13, 13);  // A_right
    LCD_DrawFillRect(30, 77, 13, 13);  // B_left
    LCD_DrawFillRect(30, 90, 13, 13);  // B_right
    
    LCD_DrawFillRect(118, 25, 13, 13);  // 부호
    LCD_DrawFillRect(118, 51, 13, 13);  // 첫 번째 결과
    LCD_DrawFillRect(118, 64, 13, 13);  // 두 번째 결과 
    LCD_DrawFillRect(118, 77, 13, 13);  // 세 번째 결과
    LCD_DrawFillRect(118, 90, 13, 13);  // 네 번째 결과
    
    LCD_DrawFillRect(67, 90, 23, 13);   // 연속 증가 모드
    
    LCD_SetBrushColor(GET_RGB(255, 192, 203)); // 보라색 코드
    LCD_DrawFillRect(76, 51, 13, 13);    // 연산자
    
    LCD_SetPenColor(RGB_BLACK);
    LCD_DrawRectangle(30, 25, 13, 13);   // A_left  
    LCD_DrawRectangle(30, 51, 13, 13);  // A_right
    LCD_DrawRectangle(30, 77, 13, 13);  // B_left
    LCD_DrawRectangle(30, 90, 13, 13);  // B_right
    
    LCD_DrawRectangle(118, 25, 13, 13);  // 부호
    LCD_DrawRectangle(118, 51, 13, 13);  // 첫 번째 결과
    LCD_DrawRectangle(118, 64, 13, 13);  // 두 번째 결과 
    LCD_DrawRectangle(118, 77, 13, 13);  // 세 번째 결과
    LCD_DrawRectangle(118, 90, 13, 13);  // 네 번째 결과
    
    LCD_DrawRectangle(67, 90, 23, 13);  // 연속 증가 모드
    LCD_DrawRectangle(76, 51, 13, 13);   // 연산자
      
    LCD_SetPenColor(RGB_BLUE);
    LCD_DrawHorLine(43, 33, 17);
    LCD_DrawHorLine(43, 57, 17);
    LCD_DrawHorLine(43, 85, 17);
    LCD_DrawHorLine(43, 95, 17);

    LCD_DrawHorLine(100, 33, 18);
    LCD_DrawHorLine(100, 60, 18);
    LCD_DrawHorLine(100, 70, 18);
    LCD_DrawHorLine(100, 85, 18);
    LCD_DrawHorLine(100, 95, 18);


    LCD_SetPenColor(RGB_GREEN);
    LCD_DrawRectangle(13, 37, 14, 14);   // A 상자
    LCD_DrawRectangle(13, 90, 14, 14);   // B 상자
    LCD_DrawRectangle(133, 50, 14, 14);  // C 상자
    LCD_DrawRectangle(60, 20, 40, 90);   // 메인 상자


    LCD_DisplayChar(2, 4, '0');  // A_left  
    LCD_DisplayChar(4, 4, '0');  // A_right
    LCD_DisplayChar(6, 4, '0');  // B_left
    LCD_DisplayChar(7, 4, '0');  // B_right
    LCD_DisplayChar(2, 15, '+');  // 부호    
    LCD_DisplayChar(4, 15, '0');  // 첫 번째 결과 
    LCD_DisplayChar(5, 15, '0'); // 두 번째 결과
    LCD_DisplayChar(6, 15, '0'); // 세 번째 결과 
    LCD_DisplayChar(7, 15, '0'); // 네 번째 결과 
    LCD_DisplayText(7, 9, "+0");  // 연속 증가 모드

    LCD_SetBackColor(GET_RGB(255, 192, 203));
    LCD_DisplayText(4, 10, "+");   // operator  


    LCD_SetBackColor(RGB_WHITE);
    LCD_DisplayChar(3, 2, 'A');
    LCD_DisplayChar(7, 2, 'B');
    LCD_DisplayChar(4, 17, 'C');
    LCD_SetBackColor(RGB_YELLOW); 
    LCD_DrawRectangle(60, 20, 40, 90);
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정   */
void _GPIO_Init(void)
{
   // LED (GPIO G) 설정
    RCC->AHB1ENR   |=  0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
   GPIOG->MODER    |=  0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
   GPIOG->OTYPER   &= ~0x00FF;   // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
    GPIOG->OSPEEDR    |=  0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
   // SW (GPIO H) 설정 
   RCC->AHB1ENR    |=  0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
   GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
   GPIOH->PUPDR    &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

   // Buzzer (GPIO F) 설정 
    RCC->AHB1ENR   |=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
   GPIOF->MODER    |=  0x00040000;   // GPIOF 9 : Output mode (0b01)                  
   GPIOF->OTYPER    &= ~0x0200;   // GPIOF 9 : Push-pull     
    GPIOF->OSPEEDR    |=  0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 
}   

/* EXTI (EXTI8(GPIOH.8, SW0), EXTI9(GPIOH.9, SW1)) 초기 설정  */
void _EXTI_Init(void)
{
   RCC->AHB1ENR |= 0x00000080;   // RCC_AHB1ENR GPIOH Enable
   RCC->APB2ENR |= 0x00004000;   // Enable System Configuration Controller Clock
   RCC->AHB1ENR |= 0x00000100;      // RCC_AHB1ENR GPIOI Enable
   
   GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH PIN8~PIN15 Input mode (reset state)             
   
// EXTI6,8,9 설정
   SYSCFG->EXTICR[1] |= 0x0800;    // EXTI6 -> GPIOI
   SYSCFG->EXTICR[2] |= 0x0078;    // EXTI8 -> GPIOI, EXTI9-> GPIOH
   SYSCFG->EXTICR[3] |= 0x0700;    // EXTI14 -> GPIOH

   EXTI->FTSR |= 0x000140;      // EXTI8: Falling Trigger Enable
   EXTI->RTSR |= 0x000200;      // EXTI9: Rising Trigger  Enable
   EXTI->IMR  |= 0x000340;     // EXTI8,9 인터럽트 mask (Interrupt Enable) 설정
      
   NVIC->ISER[0] |= (1 << 23);   // 0x00800000
               // Enable 'Global Interrupt EXTI8,9'
               // Vector table Position 참조

// EXTI14 설정
   SYSCFG->EXTICR[3] |= 0x0700;    // EXTI14에 대한 소스 입력은 GPIOH로 설정  

    EXTI->FTSR |= 0x004000;      // EXTI14: Falling Trigger Enable
    EXTI->IMR  |= 0x004000;     // EXTI14 인터럽트 mask (Interrupt Enable) 설정
      
   NVIC->ISER[1] |= (1 << (40-32));// 0x00000100

  NVIC->IP[23]= 0xF0;  // Low Priority 
  NVIC->IP[40]= 0xE0;  // High Priority 

}

/* EXTI5~9 ISR */
void EXTI9_5_IRQHandler(void)      
{
   if (EXTI->PR & 0x0040) {  //EXTI 6 발생  NAVI_UP
      EXTI->PR &= 0x0040;
      GPIOG->ODR |= 0x0080; //LED 7 ON
      LCD_DisplayText(7, 9, "+1");  // 연속 증가 모드
      BEEP();
      J_UP();
      LCD_DisplayText(7, 9, "+0");
      BEEP();
      DelayMS(500);
      BEEP();
      DelayMS(500);
      BEEP();
      GPIOG->ODR &= ~0x0080; //LED 7 OFF
      Fram_Write(531, Cresult);    //Cresult Fram 저장
      Fram_Write(532, 1);
   }

   if (EXTI->PR & 0x0100) {  //EXTI 8 발생  NAVI_RIGHT
      EXTI->PR &= 0x0100;
      J_RIGHT();
   }
   
   if (EXTI->PR & 0x0200) {  //EXTI 9 발생  SW 1
      EXTI->PR &= 0x0200;
      BEEP();
      Cresult = calculate();
      Dec_Bin(Cresult);
   }
}

/* EXTI15 ISR */
void EXTI15_10_IRQHandler(void)      
{
     if(EXTI->PR & 0x4000)                   // EXTI14 Interrupt Pending(발생) 여부?
   {
      EXTI->PR |= 0x4000;       // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
      excape_Flag = 0;
      DelayMS(1000);
    }     

}
                
/* Switch가 입력되었는지 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)   // input key SW0 - SW7 
{ 
   uint16_t key;
   key = GPIOH->IDR & 0xFF00;   // any key pressed ?
   if(key == 0xFF00)      // if no key, check key off
   {     if(key_flag == 0)
              return key;
            else
      {   DelayMS(10);
              key_flag = 0;
              return key;
           }
       }
     else            // if key input, check continuous key
   {   if(key_flag != 0)   // if continuous key, treat as no key input
              return 0xFF00;
            else         // if new key,delay for debounce
      {   key_flag = 1;
         DelayMS(10);
          return key;
           }
   }
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)         
{    
   GPIOF->ODR |=  0x0200;   // PF9 'H' Buzzer on
   DelayMS(30);      // Delay 30 ms
   GPIOF->ODR &= ~0x0200;   // PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
    register unsigned short i;
    for (i=0; i<wMS; i++)
        DelayUS(1000);               // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
    volatile int Dly = (int)wUS*17;
    for(; Dly; Dly--);
} 