/////////////////////////////////////////////////////////////
// ������: Ŀ���ڵ��Ǹű�
// ��������: Ŀ���ڵ� �ǸűⰡ �Է¿� ���� Ŀ�Ǹ� ����� ���� ����
// ����� �ϵ����(���): GPIO, buzzer, switch, led
// ������: 2023. 5. 09
// ������ Ŭ����: ȭ���Ϲ�
// �й�: 2018132027       
// �̸�: �̰ǿ�
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"

void _GPIO_Init(void);
uint16_t KEY_Scan(void);
void BEEP(void);
void MixCoffee(void);
void SugarCoffee(void);
void BlackCoffee(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

int main(void)
{
	DelayMS(100);
	_GPIO_Init();

	GPIOG->ODR = ~0x00FF;	// �ʱⰪ: LED0~7 Off
        
        int flag = 0;               // coin�� �ԷµǾ����� Ȯ���ϴ� ����
        
	while(1)
	{
                switch(KEY_Scan()){
                case 0xFE00 :
                        if(flag ==0)    //Coin �Է��� �Ǿ� ���� �� �������� �ʵ��� ����
                        {
                                GPIOG -> BSRRL = 0x0001;  //  LED0 (PG0) ON
                                BEEP();                                 //buzzer
                        }
                        flag = 1;               // Coin �Է��� �� ���·� ��ȯ
                 }
                      
		if(flag == 1){  
                  switch(KEY_Scan()) {
                                case 0xFD00 : 	//SW1
                                        GPIOG->ODR |= 0x0002;   //  LED1 (PG1) ON
                                        BEEP();                         //buzzer
                                        DelayMS(1000);              // �޴� ���� �� 1�� ������
                                        BlackCoffee();                  // �� Ŀ�� cycle 
                                        flag = 0;
                                break;
                                case 0xFB00 : 	//SW2
                                        GPIOG->ODR |= 0x0004;   //  LED2 (PG2) ON
                                        BEEP();                         //buzzer
                                        DelayMS(1000);            // �޴� ���� �� 1�� ������
                                        SugarCoffee();                  // ���� Ŀ�� cycle 
                                        flag =0;
                                break;
                                case 0xF700 : 	//SW3
                                        GPIOG->ODR |= 0x0008;   //  LED3 (PG3) ON
                                        BEEP();                         //buzzer
                                        DelayMS(1000);            // �޴� ���� �� 1�� ������
                                        MixCoffee();               // �ͽ� Ŀ�� cycle 
                                        flag = 0;
                                break;
                        }                       
		}
		else{	
                        DelayMS(1000);    // 1���� ���� ���
                        GPIOG->ODR = ~0x00FF; // �ʱⰪ: LED0~7 Off
		}
	}
}

/* Switch�� �ԷµǾ������� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{	if(key_flag == 0)
			return key;
		else
		{	DelayMS(10);
			key_flag = 0;
			return key;
		}
	}
	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
			return 0xFF00;
		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);
 			return key;
		}
	}
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer))	*/
void _GPIO_Init(void)
{
	// LED (GPIO G) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
   
	// SW (GPIO H) ���� : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� : Output mode 
	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable		
        GPIOF -> MODER     &= ~0x000C0000;
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}	

void BEEP(void)			/* beep for 30 ms */
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);			// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}

void MixCoffee()
{
          GPIOG->ODR |= 0x0080;              //  Cup LED7 (PG7) ON
          DelayMS(500);
          GPIOG->ODR &= ~0x0080;            //  Cup LED7 (PG7) OFF
          DelayMS(500);
  
          for(int i=0; i<2; i++){
                  GPIOG->ODR |= 0x0040;             //  Sugar LED6 (PG6) ON
                  DelayMS(500);
                  GPIOG->ODR &= ~0x0040;           //  Sugar LED6 (PG6) OFF
                  DelayMS(500);
          }
          
          for(int i=0; i<2; i++){
                  GPIOG->ODR |= 0x0020;             //  Cream LED5 (PG5) ON
                  DelayMS(500);
                  GPIOG->ODR &= ~0x0020;          //  Cream LED5 (PG5) OFF
                  DelayMS(500);
          }
          
          for(int i =0; i<3; i++){
                  GPIOG->ODR |= 0x0010;             //  Water/Coffee LED4 (PG4) ON
                  DelayMS(500);
                  GPIOG->ODR &= ~0x0010;           //  Water/Coffee LED4 (PG4) OFF
                  DelayMS(500);
          }
          
          for(int i =0; i<3; i++){
                  BEEP();
                  DelayMS(500);
          }
}
void SugarCoffee()
{
          GPIOG->ODR |= 0x0080;              //  Cup LED7 (PG7) ON
          DelayMS(500);
          GPIOG->ODR &= ~0x0080;            //  Cup LED7 (PG7) OFF
          DelayMS(500);
          
          for(int i=0; i<2; i++){
                  GPIOG->ODR |= 0x0040;             //  Sugar LED6 (PG6) ON
                  DelayMS(500);
                  GPIOG->ODR &= ~0x0040;           //  Sugar LED6 (PG6) OFF
                  DelayMS(500);
          }
          
          for(int i =0; i<3; i++){
                  GPIOG->ODR |= 0x0010;             //  Water/Coffee LED4 (PG4) ON
                  DelayMS(500);
                  GPIOG->ODR &= ~0x0010;           //  Water/Coffee LED4 (PG4) OFF
                  DelayMS(500);
          }
          for(int i =0; i<3; i++){
                  BEEP();
                  DelayMS(500);
          }
                   
}
void BlackCoffee()
{
          GPIOG->ODR |= 0x0080;              //  Cup LED7 (PG7) ON
          DelayMS(500);
          GPIOG->ODR &= ~0x0080;            //  Cup LED7 (PG7) OFF
          DelayMS(500);
          
          for(int i =0; i<3; i++){
                  GPIOG->ODR |= 0x0010;             //  Water/Coffee LED4 (PG4) ON
                  DelayMS(500);
                  GPIOG->ODR &= ~0x0010;           //  Water/Coffee LED4 (PG4) OFF
                  DelayMS(500);
          }
          for(int i =0; i<3; i++){
                  BEEP();
                  DelayMS(500);
          }
}