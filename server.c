/********************************************************************\
* server.c                                                           *
*                                                                    *
* Desc: example of the server-side of an application                 *
* Revised: Dag Nystrom & Jukka Maki-Turja                     *
*                                                                    *
* Based on generic.c from Microsoft.                                 *
*                                                                    *
*  Functions:                                                        *
*     WinMain      - Application entry point                         *
*     MainWndProc  - main window procedure                           *
*                                                                    *
* NOTE: this program uses some graphic primitives provided by Win32, *
* therefore there are probably a lot of things that are unfamiliar   *
* to you. There are comments in this file that indicates where it is *
* appropriate to place your code.                                    *
* *******************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "wrapper.h"

/* the server uses a timer to periodically update the presentation window */
/* here is the timer id and timer period defined                          */
#define GRAV	0.0000000000667259
#define UPDATE_FREQ     10	/* update frequency (in ms) for the timer */

							/* (the server uses a mailslot for incoming client requests) */



/*********************  Prototypes  ***************************/
/* NOTE: Windows has defined its own set of types. When the   */
/*       types are of importance to you we will write comments*/
/*       to indicate that. (Ignore them for now.)             */
/**************************************************************/



LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI mailThread(LPVOID);
void createPlanet(planet_type* pt);
void Planet(planet_type* pt);
CRITICAL_SECTION CS;
HANDLE MySemaphore = NULL;
planet_type* head = NULL;
ThreadCount = 0;

UINT_PTR RAM_TIMER;
HDC hDC;		/* Handle to Device Context, gets set 1st time in MainWndProc */
				/* we need it to access the window for printing and drawin */

/********************************************************************\
*  Function: int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)    *
*                                                                    *
*   Purpose: Initializes Application                                 *
*                                                                    *
*  Comments: Register window class, create and display the main      *
*            window, and enter message loop.                         *
*                                                                    *
*                                                                    *
\********************************************************************/

/* NOTE: This function is not too important to you, it only */
/*       initializes a bunch of things.                     */
/* NOTE: In windows WinMain is the start function, not main */

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {

	HWND hWnd;
	DWORD threadID;
	MSG msg;
	InitializeCriticalSection(&CS);
	/* Create the window, 3 last parameters important */
	/* The tile of the window, the callback function */
	/* and the backgrond color */

	hWnd = windowCreate(hPrevInstance, hInstance, nCmdShow, "Server", MainWndProc, COLOR_WINDOW + 2);

	/* start the timer for the periodic update of the window    */
	/* (this is a one-shot timer, which means that it has to be */
	/* re-set after each time-out) */
	/* NOTE: When this timer expires a message will be sent to  */
	/*       our callback function (MainWndProc).               */

	windowRefreshTimer(hWnd, UPDATE_FREQ);

	SetTimer(hWnd, RAM_TIMER, 500, (TIMERPROC)NULL);		//Display of the RAM usage every 500ms

	/* create a thread that can handle incoming client requests */
	/* (the thread starts executing in the function mailThread) */
	/* NOTE: See online help for details, you need to know how  */
	/*       this function does and what its parameters mean.   */
	/* We have no parameters to pass, hence NULL				*/


	threadID = threadCreate(mailThread, NULL);

	/* (the message processing loop that all windows applications must have) */
	/* NOTE: just leave it as it is. */
	/*while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}*/

	while (GetMessage(&msg, (HWND)NULL, 0, 0))
	{
		if (TranslateAccelerator(MainWndProc, hWnd, &msg) == 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	return msg.wParam;

}

/********************************************************************\
* Function: mailThread                                               *
* Purpose: Handle incoming requests from clients                     *
* NOTE: This function is important to you.                           *
/********************************************************************/
DWORD WINAPI mailThread(LPVOID arg) {


	long count = 0;
	char buffer[1024];
	DWORD bytesRead;
	HANDLE mailbox;
	planet_type* tmp = NULL;
	BOOL Waited = FALSE, ResetSemaphore = FALSE; 
	/* create a mailslot that clients can use to pass requests through   */
	/* (the clients use the name below to get contact with the mailslot) */
	/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */
	ThreadCount++;

	mailbox = mailslotCreate("mailbox");

	for (;;)
	{

		/* (ordinary file manipulating functions are used to read from mailslots) */
		/* in this example the server receives strings from the client side and   */
		/* displays them in the presentation window                               */
		/* NOTE: binary data can also be sent and received, e.g. planet structures*/


		GetMailslotInfo(mailbox, 0, &bytesRead, 0, 0);

		/*if (MySemaphore != NULL && Waited == FALSE)
		{
			WaitForSingleObject(MySemaphore, INFINITE);
			
			Waited = TRUE;
		}
		else if (MySemaphore == NULL && (Waited == TRUE || ResetSemaphore == TRUE))
		{
			Waited = FALSE;
			ResetSemaphore == FALSE;
		}*/
		if (bytesRead != -1)
		{
			tmp = malloc(sizeof(planet_type));

			bytesRead = mailslotRead(mailbox, tmp, sizeof(planet_type));		//read the new planet from the client
			ThreadCount++;
			createPlanet(tmp);													//Put it in the database by "creating it"
			threadCreate((LPTHREAD_START_ROUTINE)Planet, tmp);					//start the calculation thread for that planet

			
			
			/* NOTE: It is appropriate to replace this code with something */
			/*       that match your needs here.                           */

			/* (hDC is used reference the previously created window) */
		}
		else {
			/* failed reading from mailslot                              */
			/* (in this example we ignore this, and happily continue...) */
		}
		if (MySemaphore != NULL && ResetSemaphore == FALSE)
		{
			ReleaseSemaphore(MySemaphore, 1, &count);
			ResetSemaphore = TRUE;
		}
		else if (MySemaphore == NULL && ResetSemaphore == TRUE)
		{
			ResetSemaphore = FALSE;
		}
	}

	return 0;
}
void Planet(planet_type* pt)
{
	BOOL Waited = FALSE;
	BOOL ResetSemaphore = FALSE;
	planet_type* tmp = pt->next;
	char message[256] = "Your planet ";
	strcat(message, pt->name);

	long count = 0;
	int runs = 0;
	double total_time, a = 0, ax = 0, ay = 0, r = 1000;

	clock_t time2, time = clock();

	for (;;)
	{
		HANDLE mailbox = INVALID_HANDLE_VALUE;
		runs++;		//for testing
		ax = 0;
		a = 0;
		ay = 0;
		(pt->life)--;
		if (pt->sx < 0 || pt->sx > 800 || pt->sy < 0 || pt->sy > 600)			//if the planet goes out of bounds it dies 
		{
			pt->life = 0;
			strcat(message, " died by going out of bounds\n");
		}
		else if (r < 3)			//if the planet "collides" by going to close to another planet it dies
		{
			pt->life = 0;
			strcat(message, " died by colliding with another planet\n");
		}
		else if (pt->life <= 0)		//if life is 0 it dies 
		{
			strcat(message, " died because life went to 0\n");
		}

		if (pt->life <= 0)		//handeling of the death
		{

			ThreadCount--;
			EnterCriticalSection(&CS);

			do
			{
				mailbox = mailslotConnect(pt->pid);
			} while (mailbox == INVALID_HANDLE_VALUE);

			if (pt->next == NULL)		//if the planet is alone in the database
			{

				head = NULL;
				do
				{
					Sleep(3);
				} while (MySemaphore != NULL);
				MySemaphore = CreateSemaphore(
					NULL,           // default security attributes
					1,  // initial count
					(ThreadCount + 2),  // maximum count
					NULL);          // unnamed semaphore
				mailslotWrite(mailbox, message, strlen(message) + 1);
				mailslotClose(mailbox);
				
				do
				{
					WaitForSingleObject(MySemaphore, INFINITE);
					ReleaseSemaphore(MySemaphore, 1, &count);
				} while ((count) < ThreadCount);

				free(pt);
				pt = NULL;
				CloseHandle(MySemaphore);
				MySemaphore = NULL;
				LeaveCriticalSection(&CS);
				return;
			}
			else if (pt->next != NULL) //if there is more than one planet in the database
			{
				tmp = pt->next;
				while (tmp->next != pt)		//goes until we are on the one before our pt
				{
					tmp = tmp->next;
				}
				if (pt->next == tmp)		//if the one after the pt is the one before, aka iff there is 2 planets in the database 
				{
					tmp->next = NULL;
					head = tmp;
				}
				else						//more than two planets in the database
				{

					tmp->next = pt->next;
					if (head == pt)
					{
						head = tmp->next;
					}
				}
				
				do
				{
					Sleep(3);
				} while (MySemaphore != NULL);
				MySemaphore = CreateSemaphore(
					NULL,           // default security attributes
					1,  // initial count
					(ThreadCount + 2),  // maximum count
					NULL);          // unnamed semaphore
				mailslotWrite(mailbox, message, strlen(message) + 1);
				mailslotClose(mailbox);
				do
				{
					WaitForSingleObject(MySemaphore, INFINITE);
					ReleaseSemaphore(MySemaphore, 1, &count);
				} while ((count) < ThreadCount);
				
				free(pt);
				//LeaveCriticalSection(&CS);
				CloseHandle(MySemaphore);
				MySemaphore = NULL;
				LeaveCriticalSection(&CS);
				return;
			}

		}

		/*if (MySemaphore != NULL && Waited == FALSE)
		{
			WaitForSingleObject(MySemaphore, INFINITE);
			Waited = TRUE;
		}
		else if (MySemaphore == NULL && (Waited == TRUE || ResetSemaphore == TRUE))
		{
			Waited = FALSE;
			ResetSemaphore == FALSE;
		}*/

		tmp = pt->next;
		if (tmp == NULL)
		{
			tmp = pt->next;
		}
		while (tmp != NULL && tmp != pt && r >= 3 && pt->life > 0)
		{
			
			r = sqrt(pow(((tmp->sx) - (pt->sx)), 2) + pow((tmp->sy) - (pt->sy), 2));		//radie between planets
			a = (GRAV*(tmp->mass)) / pow(r, 3);				//accleration
			ax = ax + (a*((tmp->sx) - (pt->sx)));
			ay = ay + (a*((tmp->sy) - (pt->sy)));
			
			tmp = tmp->next;

		}
		
		time2 = clock();
		total_time = (double)(time2 - time) / CLOCKS_PER_SEC;
		pt->vx = pt->vx + (ax * 10);		//new velocity...
		pt->vy = pt->vy + (ay * 10);
		pt->sx = pt->sx + (pt->vx * 10);
		pt->sy = pt->sy + (pt->vy * 10);
		time = clock();

		if (MySemaphore != NULL && ResetSemaphore == FALSE)
		{
			ReleaseSemaphore(MySemaphore, 1, NULL);
			ResetSemaphore = TRUE;
		}
		else if (MySemaphore == NULL && ResetSemaphore == TRUE)
		{
			ResetSemaphore = FALSE;
		}
		Sleep(3);
	}
}
void createPlanet(planet_type* pt)
{
	if (head == NULL)		//if the database is empty
	{
		head = pt;
		(head)->next = NULL;
	}
	else if ((head)->next == NULL)		//if there is only one planet in the database
	{
		pt->next = head;
		(head)->next = pt;
	}
	else if ((head)->next != NULL)		//more than two planets in the database
	{
		pt->next = (head)->next;
		(head)->next = pt;
	}
}

/********************************************************************\
* Function: LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM) *
*                                                                    *
* Purpose: Processes Application Messages (received by the window)   *
* Comments: The following messages are processed                     *
*                                                                    *
*           WM_PAINT                                                 *
*           WM_COMMAND                                               *
*           WM_DESTROY                                               *
*           WM_TIMER                                                 *
*                                                                    *
\********************************************************************/
/* NOTE: This function is called by Windows when something happens to our window */

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	long count = 0;
	PAINTSTRUCT ps;
	static int posX, posY;
	HANDLE context;
	HANDLE databaseMutex = (NULL, FALSE, "accessToDatabase");
	planet_type* tmp = NULL;

	planet_type* FirstPlanet = NULL;
	planet_type* ship = NULL;
	char speedx[64], speedy[64], life[32];
	BOOL Waited = FALSE;
	BOOL ResetSemaphore = FALSE;


	switch (msg)
	{
		/**************************************************************/
		/*    WM_CREATE:        (received on window creation)
		/**************************************************************/
	case WM_CREATE:
		hDC = GetDC(hWnd);
		break;
		/**************************************************************/
		/*    WM_TIMER:         (received when our timer expires)
		/**************************************************************/
	case WM_TIMER:
		if (wParam == RAM_TIMER)
		{
			char tradar[24];
			sprintf(tradar, "%d", ThreadCount - 1);
			char RAM[24];
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);
			DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
			physMemUsed = physMemUsed / 1000;
			sprintf(RAM, "%I64u", physMemUsed);		//%I64u is the %d for unsigned_int64 (DWORDLONG)
			TextOut(hDC, 820, 100, RAM, strlen(RAM));
			TextOut(hDC, 820, 80, tradar, strlen(tradar));
		}
		else
		{
			Rectangle(hDC, 0, 0, 800, 600);
			Rectangle(hDC, 810, 0, 940, 70);
			TextOut(hDC, 820, 5, "Ship Status", 11);

			WaitForSingleObject(databaseMutex, INFINITE);
			FirstPlanet = head;
			tmp = FirstPlanet;

			if (FirstPlanet != NULL && tmp != NULL)
			{

				do {

					posX = (int)tmp->sx;
					posY = (int)tmp->sy;

					if (!strcmp(tmp->name, "SHIP"))
					{
						Rectangle(hDC, posX - 8, posY - 8, posX + 8, posY + 8);
						sprintf(speedx, "%lf", tmp->vx);
						sprintf(speedy, "%lf", tmp->vy);
						sprintf(life, "%d", tmp->life);
						TextOut(hDC, 820, 20, "X: ", 3);
						TextOut(hDC, 820, 35, "Y: ", 3);
						TextOut(hDC, 820, 50, "Life:", 5);
						TextOut(hDC, 850, 20, speedx, lstrlen(speedx));
						TextOut(hDC, 850, 35, speedy, lstrlen(speedy));
						TextOut(hDC, 850, 50, life, lstrlen(life));
					}
					else
					{
						int size = log10((int)tmp->mass);
						Ellipse(hDC, posX - size, posY - size, posX + size, posY + size);
						CreateSolidBrush(3);
					}
					tmp = tmp->next;


				} while (tmp != FirstPlanet && tmp != NULL && FirstPlanet != NULL);
			}


			if (head == NULL || head->next == NULL)
			{
				tmp = head;
			}
			if (MySemaphore != NULL && ResetSemaphore == FALSE)
			{
				ReleaseSemaphore(MySemaphore, 1, &count);
				ResetSemaphore = TRUE;
			}
			else if (MySemaphore == NULL && ResetSemaphore == TRUE)
			{
				ResetSemaphore = FALSE;
			}


			ReleaseMutex(databaseMutex);
			windowRefreshTimer(hWnd, UPDATE_FREQ);
		}
		break;
		
	case WM_PAINT:

		context = BeginPaint(hWnd, &ps);
		Rectangle(hDC, 0, 0, 800, 600);
		EndPaint(hWnd, &ps);

		break;

	case WM_KEYDOWN:

		

		ship = head;
		if (ship == NULL)
			break;
		while (strcmp(ship->name, "SHIP"))
		{
			ship = ship->next;
			if (ship == head && strcmp(head->name, "SHIP")) 
				break;
		}

		switch (wParam)
		{
		case VK_LEFT: // Process the LEFT ARROW key.

			ship->vx = ship->vx - 0.001;
			break;

		case VK_RIGHT: // Process the RIGHT ARROW key. 


			ship->vx = ship->vx + 0.001;
			break;

		case VK_UP: // Process the UP ARROW key. 

			ship->vy = ship->vy - 0.001;
			break;

		case VK_DOWN: // Process the DOWN ARROW key. 

			ship->vy = ship->vy + 0.001;
			break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		/* NOTE: Windows will automatically release most resources this */
		///*       process is using, e.g. memory and mailslots.           */
		/*       (So even though we don't free the memory which has been*/
		/*       allocated by us, there will not be memory leaks.)      */

		ReleaseDC(hWnd, hDC); /* Some housekeeping */
		break;

		/**************************************************************\
		*     Let the default window proc handle all other messages    *
		\**************************************************************/


	default:
		return(DefWindowProc(hWnd, msg, wParam, lParam));
	}
	return 0;
}