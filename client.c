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

void mailThread(char* pid)
{
	int bytesRead;
	HANDLE mailbox = mailslotCreate(pid);
	char* MESSAGE;
	for (;;)
	{
		GetMailslotInfo(mailbox, 0, &bytesRead, 0, 0);
		if (bytesRead != -1)		//Run if there is a meesage inte hte mailbox
		{
			MESSAGE = malloc(bytesRead);
			mailslotRead(mailbox, MESSAGE, bytesRead);		//read and print the message
			printf("%s", MESSAGE);
		}
	}
}
void sort_number(char *input)
{
	int i, j = 0;

	for (i = 0; i < 10 && input[i] != '\0'; i++)		//removes all non numbers exept '.' and '-' for use with atoi or atof later
	{
		if (isdigit(input[i]) != 0 || input[i] == '.' || input[i] == '-')
		{
			input[j] = input[i];
			j++;
		}
	}
	
	input[j] = '\0';
}

void input(char* msg)		//my input function from user
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
	DWORD pid = GetCurrentProcessId();
	//char pid[30];
	
	HANDLE mailSlot = INVALID_HANDLE_VALUE;
	DWORD bytesWritten;
	int i = 0, choice, length;
	while (mailSlot == INVALID_HANDLE_VALUE)		//Run until we can connect to the server mailbox
	{
		Sleep(100);
		mailSlot = mailslotConnect("mailbox");

		if (mailSlot == INVALID_HANDLE_VALUE) {
			printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		}
	}
	planet_type* pt = malloc(sizeof(planet_type));
	char* MESSAGE = malloc(1024);


	sprintf(pt->pid, "%d", pid);
	threadCreate((LPTHREAD_START_ROUTINE)mailThread, pt->pid);		//mailthread

	while (TRUE)
	{
		i = 0;
		do
		{
			printf("	Enter what you want to do:\n0: create a planet.\n1: Use the standard setup.\n3: Create a ship.\n", pt->name);		//life expecancy of the new planet
			input(MESSAGE);
			sort_number(MESSAGE);
			choice = atoi(MESSAGE);
		} while (choice < 1);

		if (choice == 1)
		{
			do
			{
				printf("What is the planets name?\n");			//name of the new planet
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


			//pt->life = 4000;
			do
			{
				printf("enter the expected life of %s in seconds\n", pt->name);		//life expecancy of the new planet
				input(MESSAGE);
				sort_number(MESSAGE);
				pt->life = 300 * atoi(MESSAGE);
			} while (pt->life <= 0);

			do
			{
				printf("enter the mass of %s\n", pt->name);			//mass
				input(MESSAGE);
				sort_number(MESSAGE);
				pt->mass = atof(MESSAGE);
				//pt->mass = 100000000.0;
			} while (pt->mass < 0);

			pt->next = NULL;

			do
			{
				printf("enter the x-position of %s\n", pt->name);		//x-position
				input(MESSAGE);
				sort_number(MESSAGE);
				pt->sx = atof(MESSAGE);
			} while (pt->sx < 0 || pt->sx >800);


			do
			{
				printf("enter the y-position of %s\n", pt->name);		//y-position
				input(MESSAGE);
				sort_number(MESSAGE);
				pt->sy = atof(MESSAGE);
			} while (pt->sy < 0 || pt->sy >600);


			printf("enter the x-speed of %s\n", pt->name);		//x-speed
			input(MESSAGE);
			sort_number(MESSAGE);
			pt->vx = atof(MESSAGE);


			printf("enter the y-speed of %s\n", pt->name);		//y-speed
			input(MESSAGE);
			sort_number(MESSAGE);
			pt->vy = atof(MESSAGE);



			bytesWritten = mailslotWrite(mailSlot, pt, sizeof(planet_type));		//writing the new planet to the server
			if (bytesWritten != -1)
				printf("data sent to server (bytes = %d)\n", bytesWritten);
			else
				printf("failed sending data to server\n");

		}
		else if (choice == 2)
		{
			strcpy(pt->name, "Solen");
			pt->mass = 100000000;
			pt->next = NULL;
			pt->sx = 300;
			pt->sy = 300;
			pt->vx = 0;
			pt->vy = 0;
			do
			{
				printf("enter the expected life of %s in seconds\n", pt->name);		//life expecancy of the new planet
				input(MESSAGE);
				sort_number(MESSAGE);
				pt->life = 300 * atoi(MESSAGE);

			} while (pt->life <= 0);
			bytesWritten = mailslotWrite(mailSlot, pt, sizeof(planet_type));		//writing the new planet to the server
			if (bytesWritten != -1)
				printf("data sent to server (bytes = %d)\n", bytesWritten);
			else
				printf("failed sending data to server\n");


			strcpy(pt->name, "Jorden");
			pt->mass = 1000;
			pt->next = NULL;
			pt->sx = 200;
			pt->sy = 300;
			pt->vx = 0;
			pt->vy = 0.008;
			do
			{
				printf("enter the expected life of %s in seconds\n", pt->name);		//life expecancy of the new planet
				input(MESSAGE);
				sort_number(MESSAGE);
				pt->life = 300 * atoi(MESSAGE);

			} while (pt->life <= 0);
			bytesWritten = mailslotWrite(mailSlot, pt, sizeof(planet_type));		//writing the new planet to the server
			if (bytesWritten != -1)
				printf("data sent to server (bytes = %d)\n", bytesWritten);
			else
				printf("failed sending data to server\n");

		}
	}
	mailslotClose (mailSlot);

					/* (sleep for a while, enables you to catch a glimpse of what the */
					/*  client prints on the console)                                 */
	Sleep(20000);
	return;
}
