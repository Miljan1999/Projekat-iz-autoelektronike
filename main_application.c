/* Standard includes. */
#include <stdio.h>
#include <conio.h>

#include "main_application.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "extint.h"


/* Hardware simulator utility functions */
#include "HW_access.h"
void vApplicationIdleHook(void);

/* 7-SEG NUMBER DATABASE - ALL HEX DIGITS */
static const unsigned char hexnum[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
								0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };


/* GLOBAL OS-HANDLES */
static SemaphoreHandle_t RXC_BS_0, RXC_BS_1, RXC_BS_2;
static SemaphoreHandle_t TBE_BS_0, TBE_BS_1, TBE_BS_2;
static SemaphoreHandle_t binSem, LED_sem;
static TaskHandle_t serial1, serial0, serial2, ledbar;


/* TBE - TRANSMISSION BUFFER EMPTY - INTERRUPT HANDLER */
static uint32_t prvProcessTBEInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	if (get_TBE_status(0) != 0) {
		if (xSemaphoreGiveFromISR(TBE_BS_0, &xHigherPTW) != pdFALSE)
		{
			printf("potvrda\n");
		}
	}

	if (get_TBE_status(1) != 0) {
		if (xSemaphoreGiveFromISR(TBE_BS_1, &xHigherPTW) != pdFALSE)
		{
			printf("potvrda\n");
		}
	}

	if (get_TBE_status(2) != 0) {
		if (xSemaphoreGiveFromISR(TBE_BS_2, &xHigherPTW) != pdFALSE)
		{
			printf("potvrda\n");
		}
	}

	portYIELD_FROM_ISR((uint32_t)xHigherPTW);
}

/* RXC - RECEPTION COMPLETE - INTERRUPT HANDLER */
static uint32_t prvProcessRXCInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;
	if (get_RXC_status(0) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BS_0, &xHigherPTW) != pdFALSE){
			printf("get_rxc_status od 0\n");
		}
	}

	if (get_RXC_status(1) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BS_1, &xHigherPTW) != pdFALSE) {
			printf("get_rxc_status od 1\n");
		}
	}

	if (get_RXC_status(2) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BS_2, &xHigherPTW) != pdFALSE) {
			printf("get_rxc_status od 2\n");
		}
	}

	portYIELD_FROM_ISR((uint32_t)xHigherPTW);
}

void task_ukljuci_iskljuci(void* pvParams)
{
	for (;;)
	{
		if (stanje == (uint8_t)0)
		{
			if (set_LED_BAR(1, 0x00) != 0){
				printf("Greska\n");
			}
			vTaskSuspend(serial1);
			vTaskSuspend(serial0);
			vTaskSuspend(ledbar);
			vTaskSuspend(serial2);
		}
		if (stanje == (uint8_t)1)
		{
			if (set_LED_BAR(1, 0x01) != 0) {
				printf("Greska\n");
			}
			vTaskResume(serial1);
			vTaskResume(serial0);
			vTaskResume(ledbar);
			vTaskResume(serial2);
		}
	}
}

void task_obrada(void* pvParams)
{
	for (;;)
	{
		procentualna_vr1 = (uint8_t)100 * (senzor1 - donja_granica) / (gornja_granica - donja_granica);
		procentualna_vr2 = (uint8_t)100 * (senzor2 - donja_granica) / (gornja_granica - donja_granica);

		if (senzor2 <= donja_granica)
		{
			zona2 = 1;
			period2 = 250;
		}

		else
		{
			if (procentualna_vr2 <= (uint8_t)50 && procentualna_vr2 >(uint8_t)0)
			{
				zona2 = 2;
				period2 = 500;
			}

			else if (procentualna_vr2 < (uint8_t)100 && procentualna_vr2 >(uint8_t)50)
			{
				zona2 = 3;
				period2 = 1000;
			}

			else
			{
				zona2 = 4;
				period2 = 1100;      //problem sa nulom, preuyme prioritet printanja
			}
		}



		if (senzor1 <= donja_granica)
		{
			zona1 = 1;
			period1 = 250;
		}

		else
		{
			if (procentualna_vr1 <= (uint8_t)50 && procentualna_vr1 >(uint8_t)0)
			{
				zona1 = 2;
				period1 = 500;
			}

			else if (procentualna_vr1 < (uint8_t)100 && procentualna_vr1 >(uint8_t)50)
			{
				zona1 = 3;
				period1 = 1000;
			}

			else
			{
				zona1 = 4;
				period1 = 1100;
			}
		}

		if (xSemaphoreGive(LED_sem) != pdFALSE) {
			printf("Greska\n");
		}


		/*Rasclanivanje procentualne vrednosti na 3 broja i konvertovanje u nizove karaktera*/
		s1_c1 = procentualna_vr1 / (uint8_t)100;
		niz1[0] = s1_c1 + '0';
		procentualna_vr1 -= (uint8_t)100 * s1_c1;

		s1_c2 = procentualna_vr1 / (uint8_t)10;
		niz1[1] = s1_c2 + '0';
		procentualna_vr1 -= (uint8_t)10 * s1_c2;

		s1_c3 = procentualna_vr1;
		niz1[2] = s1_c3 + '0';

		/*Prebacivanje vrednosti2 u niz*/
		s2_c1 = procentualna_vr2 / (uint8_t)100;
		niz2[0] = s2_c1 + '0';
		procentualna_vr2 -= (uint8_t)100 * s2_c1;

		s2_c2 = procentualna_vr2 / (uint8_t)10;
		niz2[1] = s2_c2 + '0';
		procentualna_vr2 -= (uint8_t)10 * s2_c2;

		s2_c3 = procentualna_vr2;
		niz2[2] = s2_c3 + '0';

		if (xSemaphoreGive(binSem) != pdFALSE) {
			printf("Greska\n");
		}
		if (xSemaphoreGive(TBE_BS_2) != pdFALSE) {
			printf("Greska\n");
		}
	}
}

void Ispis7Seg(void* pvParams)
{
	for (;;)
	{
		if (xSemaphoreTake(binSem, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		if (select_7seg_digit(0)!=pdFALSE) {
			printf("greska\n");
		};
		if (set_7seg_digit(hexnum[s1_c1]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		if (select_7seg_digit(1) != pdFALSE) {
			printf("greska\n");
		};
		if (set_7seg_digit(hexnum[s1_c2]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		if (select_7seg_digit(2) != pdFALSE) {
			printf("greska\n");
		};
		if (set_7seg_digit(hexnum[s1_c3]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		if (select_7seg_digit(4) != pdFALSE) {
			printf("greska\n");
		};
		if (set_7seg_digit(hexnum[s2_c1]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		if (select_7seg_digit(5) != pdFALSE) {
			printf("greska\n");
		};
		if (set_7seg_digit(hexnum[s2_c2]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		if (select_7seg_digit(6) != pdFALSE) {
			printf("greska\n");
		};
		if (set_7seg_digit(hexnum[s2_c3]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void RXC_isr_0(void* pvParams) {

	uint8_t cc;
	uint8_t rec_buffer[1];
	uint8_t loc = 0;

	for (;;) {
		if (xSemaphoreTake(RXC_BS_0, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		if (get_serial_character(0, &cc) != pdFALSE) {
			printf("Greska\n");
		}

		if (cc == (uint8_t)0xef)
		{
			loc = 0;
		}
		else if (cc == (uint8_t)0x0d)
		{
			senzor1 = rec_buffer[0];
			printf("Stigao je s1 %d\n", rec_buffer[0]);

		}
		else {
			rec_buffer[loc++] = cc;
		}
	}
}

void RXC_isr_1(void* pvParams) {

	uint8_t cc;
	uint8_t rec_buffer[1];
	uint8_t loc = 0;

	for (;;) {
		if (xSemaphoreTake(RXC_BS_1, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		if (get_serial_character(1, &cc) != pdFALSE) {
			printf("Greska\n");
		}

		if (cc == (uint8_t)0xef)
		{
			loc = 0;
		}
		else if (cc == (uint8_t)0x0d)
		{
			senzor2 = rec_buffer[0];
			printf("Stigao je s2 %d\n", rec_buffer[0]);

		}
		else {
			rec_buffer[loc++] = cc;
		}
	}
}

void PC_Receive_task(void* pvParams) {

	uint8_t rec_buffer[5];
	uint8_t i = 0;

	for (;;) {
		if (xSemaphoreTake(RXC_BS_2, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		if (get_serial_character(2, &rec_buffer[i]) != pdFALSE) {
			printf("Greska\n");
		}

		i++;
		if (i == (uint8_t)3 && rec_buffer[2] == (uint8_t)0x0d) //poruka oblika \donja_granica\gornja\CR
		{
			printf("kanal2: %d %d \n", rec_buffer[0], rec_buffer[1]); //provera pristiglih vrednosti
			donja_granica = rec_buffer[0];
			gornja_granica = rec_buffer[1];
			i = 0;
		}

		if (i == (uint8_t)5)
		{
			i = 0;

			if (rec_buffer[0] == (uint8_t)115 && rec_buffer[1] == (uint8_t)116 && rec_buffer[2] == (uint8_t)97 && rec_buffer[3] == (uint8_t)114 && rec_buffer[4] == (uint8_t)116)
			{
				printf("Start \n");
				stanje = 1;

			}

			if (rec_buffer[0] == (uint8_t)115 && rec_buffer[1] == (uint8_t)116 && rec_buffer[2] == (uint8_t)111 && rec_buffer[3] == (uint8_t)112 && rec_buffer[4] == (uint8_t)112)
			{
				printf("Stopp \n");
				stanje = 0;
			}
		}
	}
}

void Serial0Send_Task(void* pvParameters)
{
	uint8_t brojac = 0;

	unsigned char trigger1[] = "S1\n";
	int niz_len = 3;

	for (;;)
	{
		for (size_t i = 0; i < sizeof(trigger1) / sizeof(unsigned char); i++)
		{
			if (send_serial_character(COM_CH0, trigger1[i]) != pdFALSE) {
				printf("Greska\n");
			}

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void Serial1Send_Task(void* pvParameters)
{
	uint8_t brojac = 0;

	unsigned char trigger2[] = "S2\n";
	int niz_len = 3;

	for (;;)
	{
		for (size_t i = 0; i < sizeof(trigger2) / sizeof(unsigned char); i++)
		{
			if (send_serial_character(COM_CH1, trigger2[i]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void Serial2Send_Task(void* pvParameters)
{
	uint8_t t_point = 0;
	char poruka1[] = "Procentualna vrednost prvog: ";
	char poruka2[] = ", drugog :";
	char z1[] = " KONTAKT_DETEKCIJA";
	char z2[] = " BLISKA_DETEKCIJA";
	char z3[] = " UDALJENA_DETEKCIJA";
	char z4[] = " NEMA_DETEKCIJE";

	for (;;)
	{
		while (t_point < (uint8_t)29)
		{
			if (send_serial_character((uint8_t)COM_CH2, (uint8_t)poruka1[t_point]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(50));
			t_point++;
		}
		t_point = 0;

		if (xSemaphoreTake(TBE_BS_2, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)niz1[0]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));
		vTaskDelay(pdMS_TO_TICKS(50));
		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)niz1[1]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));
		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)niz1[2]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));
		vTaskDelay(pdMS_TO_TICKS(50));

		if (zona1 == (uint8_t)1)
		{
			while (t_point < (uint8_t)18)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z1[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		else if (zona1 == (uint8_t)2)
		{
			while (t_point < (uint8_t)17)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z2[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		else if (zona1 == (uint8_t)3)
		{
			while (t_point < (uint8_t)20)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z3[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		else
		{
			while (t_point < (uint8_t)15)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z4[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		while (t_point < (uint8_t)10)
		{
			if (send_serial_character((uint8_t)COM_CH2, (uint8_t)poruka2[t_point]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(50));
			t_point++;
		}
		t_point = 0;

		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)niz2[0]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));
		vTaskDelay(pdMS_TO_TICKS(50));
		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)niz2[1]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));
		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)niz2[2]) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		if (zona2 == (uint8_t)1)
		{
			while (t_point < (uint8_t)18)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z1[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		else if (zona2 == (uint8_t)2)
		{
			while (t_point < (uint8_t)17)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z2[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		else if (zona2 == (uint8_t)3)
		{
			while (t_point < (uint8_t)20)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z3[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		else
		{
			while (t_point < (uint8_t)15)
			{
				if (send_serial_character((uint8_t)COM_CH2, (uint8_t)z4[t_point]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(50));
				t_point++;
			}
			t_point = 0;
		}

		if (send_serial_character((uint8_t)COM_CH2, (uint8_t)13) != pdFALSE) {
			printf("Greska\n");
		}
		vTaskDelay(pdMS_TO_TICKS(50));

		vTaskDelay(pdMS_TO_TICKS(450));             //na ispis poruke se trosi 4550ms, zato se ovaj period smanjuje, i uukupno je 5sekundi.
	}
}

void led_task(void* pvParams) {

	for (;;)
	{
		if (xSemaphoreTake(LED_sem, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		if (period1 <= period2)
		{
			period = period1;
		}
		else
		{
			period = period2;
		}
		//printf("period1: %d", period1);
		//printf("period2: %d", period2);

		if (period == (uint16_t)1100)	{
			if (set_LED_BAR(2, 0x00) != 0) {
				printf("Greska\n");
			}
		}

		else
		{
			if (set_LED_BAR(2, 0xff) != 0) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS((TickType_t)period));

			if (set_LED_BAR(2, 0x00) != 0) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS((TickType_t)period));
		}

	}

}

/* MAIN - SYSTEM STARTUP POINT */
void main_demo(void)
{
	if (init_LED_comm() != 0) {

		printf("Neuspesna inicijalizacija\n");
	}
	if (init_7seg_comm() != 0) {

		printf("Neuspesna inicijalizacija\n");
	}
	if (init_serial_downlink(COM_CH0) != 0) {

		printf("Neuspesna inicijalizacija\n");
	}
	if (init_serial_downlink(COM_CH1) != 0) {

		printf("Neuspesna inicijalizacija\n");
	}

	if (init_serial_downlink(COM_CH2) != 0) {

		printf("Neuspesna inicijalizacija\n");
	}
	if (init_serial_uplink(COM_CH0) != 0) {

		printf("Neuspesna inicijalizacija\n");
	}
	if (init_serial_uplink(COM_CH1) != 0) {

		printf("Neuspesna inicijalizacija\n");
	}
	if (init_serial_uplink(COM_CH2) != 0) {

		printf("Neuspesna inicijalizacija\n");
	}

	/* SERIAL RECEPTION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);
	/* SERIAL TRANSMISSION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_TBE, prvProcessTBEInterrupt);

	RXC_BS_0 = xSemaphoreCreateBinary();
	RXC_BS_1 = xSemaphoreCreateBinary();
	RXC_BS_2 = xSemaphoreCreateBinary();

	TBE_BS_0 = xSemaphoreCreateBinary();
	TBE_BS_1 = xSemaphoreCreateBinary();
	TBE_BS_2 = xSemaphoreCreateBinary();

	binSem = xSemaphoreCreateBinary();
	LED_sem = xSemaphoreCreateBinary();

	//LED_q = xQueueCreate(5, sizeof(int));
	BaseType_t status; 
	status = xTaskCreate(
		RXC_isr_0,
		"serial rec 0",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		RXC_isr_1,
		"serial rec 1",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		PC_Receive_task,
		"serial rec 2",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		Serial0Send_Task,
		"serial send 0",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		&serial0
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		Serial1Send_Task,
		"serial send 1",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		&serial1
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		Serial2Send_Task,
		"serial send 2",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		&serial2
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		led_task,
		"led task",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		&ledbar
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		task_obrada,
		"task koji obradjuje podatke",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(
		task_ukljuci_iskljuci,
		"task koji pali i gasi",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);

	status = xTaskCreate(
		Ispis7Seg,
		"task koji ispisuje na 7Seg",
		configMINIMAL_STACK_SIZE,
		NULL,
		2,
		NULL
	);
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	vTaskStartScheduler();

	for (;;) {}
}

void vApplicationIdleHook(void) {

	//idleHookCounter++;
}