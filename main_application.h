#ifndef MAIN_APP
#define MAIN_APP

#include "stdint.h"

/* SERIAL SIMULATOR CHANNEL TO USE */
#define COM_CH0 (0)
#define COM_CH1 (1)
#define COM_CH2 (2)

void task_ukljuci_iskljuci(void* pvParams);
void task_obrada(void* pvParams);
void Ispis7Seg(void* pvParams);
void RXC_isr_0(void* pvParams);
void RXC_isr_1(void* pvParams);
void PC_Receive_task(void* pvParams);
void Serial0Send_Task(void* pvParameters);
void Serial1Send_Task(void* pvParameters);
void Serial2Send_Task(void* pvParameters);
void led_task(void* pvParams);
void main_demo(void);

//Globalne promenljive
extern uint16_t period;
extern uint16_t period1;
extern uint16_t period2;
extern uint8_t senzor1;
extern uint8_t senzor2;
extern uint8_t donja_granica;
extern uint8_t gornja_granica;
extern uint8_t procentualna_vr1;
extern uint8_t procentualna_vr2;
extern uint8_t stanje;
extern uint8_t s1_c1, s1_c2, s1_c3, s2_c1, s2_c2, s2_c3;
extern char niz1[3];
extern char niz2[3];
extern uint8_t zona1;
extern uint8_t zona2;

uint16_t period = 10;
uint16_t period1 = 10;
uint16_t period2 = 10;
uint8_t senzor1 = 20;
uint8_t senzor2 = 30;
uint8_t donja_granica = 10;
uint8_t gornja_granica = 100;
uint8_t procentualna_vr1;
uint8_t procentualna_vr2;
uint8_t stanje = 1;
uint8_t s1_c1, s1_c2, s1_c3, s2_c1, s2_c2, s2_c3 = 0;
char niz1[3];
char niz2[3];
uint8_t zona1;
uint8_t zona2;

#endif //MAIN_APP
