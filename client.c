/*********************************************
* client.c
*
* Desc: lab-skeleton for the client side of an
* client-server application
* 
* Revised by Dag Nystrom & Jukka Maki-Turja
* NOTE: the server must be started BEFORE the
* client.
*********************************************/
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"

//#define MESSAGE "Hello! Fucktard"
void input(char* msg)
{
	char dummy;

	fgets(msg, 1024, stdin);

	if (*(msg + (strlen(msg) - 1)) == '\n')		//if the last char is \n. Changes it to \0
	{
		*(msg + (strlen(msg) - 1)) = '\0';
	}
	else		//if the last char is not \n
	{
		do
		{   // loop until the new-line is read to remove keyboard buffer
			dummy = getchar();
		} while (dummy != '\n');

	}
}
void main(void) {

	HANDLE mailSlot = INVALID_HANDLE_VALUE;
	DWORD bytesWritten;
	int i = 0, length;
	while (mailSlot == INVALID_HANDLE_VALUE)
	{
		Sleep(100);
		mailSlot = mailslotConnect("mailbox");

		if (mailSlot == INVALID_HANDLE_VALUE) {
			printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		}
	}
	planet_type* pt = malloc(sizeof(planet_type));
	char* MESSAGE = malloc(1024);
	while (TRUE)
	{
		do
		{
			printf("What is the planets name?\n");
			input(MESSAGE);
			length = strlen(MESSAGE);
		} while (length > 20 || length <= 0);
		while (i < length)
		{

			pt->name[i] = MESSAGE[i];
			i++;
		}
		if (length != 20)
		{
			pt->name[length] = '\0';
		}
		pt->life = 5000;

		pt->mass = 100000000.0;
		pt->next = NULL;
		*pt->pid = NULL;
		pt->sx = 300.0;
		pt->sy = 300.0;
		pt->vx = 0.001;
		pt->vy = 0.0;


		bytesWritten = mailslotWrite(mailSlot, pt, sizeof(planet_type));
		if (bytesWritten != -1)
			printf("data sent to server (bytes = %d)\n", bytesWritten);
		else
			printf("failed sending data to server\n");
	}
	mailslotClose (mailSlot);

					/* (sleep for a while, enables you to catch a glimpse of what the */
					/*  client prints on the console)                                 */
	Sleep(20000);
	return;
}
