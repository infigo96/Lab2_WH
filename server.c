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
planet_type* head = NULL;
LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI mailThread(LPVOID);
void createPlanet(planet_type* pt);
void Planet(planet_type* pt);
CRITICAL_SECTION CS;


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
	InitializeCriticalSection(&CS);
	HWND hWnd;
	DWORD threadID;
	MSG msg;


	/* Create the window, 3 last parameters important */
	/* The tile of the window, the callback function */
	/* and the backgrond color */

	hWnd = windowCreate(hPrevInstance, hInstance, nCmdShow, "Himmel", MainWndProc, COLOR_WINDOW + 3);

	/* start the timer for the periodic update of the window    */
	/* (this is a one-shot timer, which means that it has to be */
	/* re-set after each time-out) */
	/* NOTE: When this timer expires a message will be sent to  */
	/*       our callback function (MainWndProc).               */

	windowRefreshTimer(hWnd, UPDATE_FREQ);


	/* create a thread that can handle incoming client requests */
	/* (the thread starts executing in the function mailThread) */
	/* NOTE: See online help for details, you need to know how  */
	/*       this function does and what its parameters mean.   */
	/* We have no parameters to pass, hence NULL				*/


	threadID = threadCreate(mailThread, NULL);


	/* (the message processing loop that all windows applications must have) */
	/* NOTE: just leave it as it is. */
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


/********************************************************************\
* Function: mailThread                                               *
* Purpose: Handle incoming requests from clients                     *
* NOTE: This function is important to you.                           *
/********************************************************************/
DWORD WINAPI mailThread(LPVOID arg) {

	char buffer[1024];
	DWORD bytesRead;
	static int posY = 0;
	HANDLE mailbox;
	planet_type* tmp = NULL;
	/* create a mailslot that clients can use to pass requests through   */
	/* (the clients use the name below to get contact with the mailslot) */
	/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */



	mailbox = mailslotCreate("mailbox");

	for (;;) {

		/* (ordinary file manipulating functions are used to read from mailslots) */
		/* in this example the server receives strings from the client side and   */
		/* displays them in the presentation window                               */
		/* NOTE: binary data can also be sent and received, e.g. planet structures*/


		GetMailslotInfo(mailbox, 0, &bytesRead, 0, 0);


		if (bytesRead != -1)
		{
			tmp = malloc(sizeof(planet_type));

			bytesRead = mailslotRead(mailbox, tmp, sizeof(planet_type));		//read the new planet from the client
			createPlanet(tmp);													//Put it in the database by "creating it"
			threadCreate((LPTHREAD_START_ROUTINE)Planet, tmp);					//start the calculation thread for that planet

			
			/* NOTE: It is appropriate to replace this code with something */
			/*       that match your needs here.                           */
		//posY++;
			/* (hDC is used reference the previously created window) */
		//bytesRead = strlen(tmp->name)+1;
		//TextOut(hDC, 10, 50 + posY % 200, tmp->name, bytesRead);
		}
		else {
			/* failed reading from mailslot                              */
			/* (in this example we ignore this, and happily continue...) */
		}
	}

	return 0;
}
void Planet(planet_type* pt)
{
	planet_type* tmp = pt->next;
	planet_type* tmp2;
	char message[256] = "Your planet ";
	strcat(message, pt->name);
	
	double total_time, a = 0, ax = 0, ay = 0, r = 1000;
	clock_t time2, time = clock();

	while (pt->life > 0)
	{
		HANDLE mailbox = INVALID_HANDLE_VALUE;
		
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
		else if(pt->life <= 0)		//if life is 0 it dies 
		{
			strcat(message, " died because life went to 0\n");
		}
		
		if (pt->life <= 0)		//handeling of the death
		{
			EnterCriticalSection(&CS);
			do
			{
				mailbox = mailslotConnect(pt->pid);
			} while (mailbox == INVALID_HANDLE_VALUE);

			if (pt->next == NULL)		//if the planet is alone in the database
			{

				head = NULL;
				mailslotWrite(mailbox, message, strlen(message)+1);
				mailslotClose(mailbox);
				//Sleep(4000);			//wait is the best medicine, CS won�t work well
				
				free(pt);
				pt = NULL;
				LeaveCriticalSection(&CS);
				return;
			}
			else if (pt->next != NULL) //if there is more than one planet in the database
			{

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
				
				mailslotWrite(mailbox, message, strlen(message)+1);
				mailslotClose(mailbox);
				//Sleep(4000);
				free(pt);
				LeaveCriticalSection(&CS);
				return;
			}

		}
		//WaitForSingleObject()
		EnterCriticalSection(&CS);
		tmp = pt->next;
		if (tmp == NULL)
		{
			tmp = pt->next;
		}
		while (tmp != NULL && tmp != pt)
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
		
		
		LeaveCriticalSection(&CS);
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

	PAINTSTRUCT ps;
	static int posX = 10;
	int posY;
	HANDLE context;
	static DWORD color = 0;
	int blub = 0;
	planet_type* pt = NULL;
	planet_type* tmp = NULL;

	switch (msg) {
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

		/* NOTE: replace code below for periodic update of the window */
		/*       e.g. draw a planet system)                           */
		/* NOTE: this is referred to as the 'graphics' thread in the lab spec. */

		/* here we draw a simple sinus curve in the window    */
		tmp = NULL;
		while (TRUE)
		{
			Sleep(1);
			EnterCriticalSection(&CS);
			if (tmp != NULL)
			{
				
				posX = (int)tmp->sx;
				posY = (int)tmp->sy;

				/*Instead of one pixel, the planet is a cross made out of 5 pixels*/

				int posX1 = posX - 1;
				int posX2 = posX + 1;
				int posY1 = posY - 1;
				int posY2 = posY + 1;
				SetPixel(hDC, posX, posY, (COLORREF)color);
				SetPixel(hDC, posX1, posY, (COLORREF)color);
				SetPixel(hDC, posX2, posY, (COLORREF)color);
				SetPixel(hDC, posX, posY1, (COLORREF)color);
				SetPixel(hDC, posX, posY2, (COLORREF)color);

				SetPixel(hDC, posX, posY, (COLORREF)color);
				color += 1;										//Color changes with each calculation to show speed of planet better
				windowRefreshTimer(hWnd, UPDATE_FREQ);
				if (tmp->next != NULL)
				{
					
					tmp = tmp->next;
					
				}
				
			}
			if (head == NULL || head->next == NULL)
			{
				tmp = head;
			}
			LeaveCriticalSection(&CS);
		}

		/****************************************************************\
		*     WM_PAINT: (received when the window needs to be repainted, *
		*               e.g. when maximizing the window)                 *
		\****************************************************************/

	case WM_PAINT:
		/* NOTE: The code for this message can be removed. It's just */
		/*       for showing something in the window.                */
		context = BeginPaint(hWnd, &ps); /* (you can safely remove the following line of code) */
		TextOut(context, 10, 10, "Hello, World!", 13); /* 13 is the string length */
		EndPaint(hWnd, &ps);
		break;
		/**************************************************************\
		*     WM_DESTROY: PostQuitMessage() is called                  *
		*     (received when the user presses the "quit" button in the *
		*      window)                                                 *
		\**************************************************************/
	case WM_DESTROY:
		PostQuitMessage(0);
		/* NOTE: Windows will automatically release most resources this */
		/*       process is using, e.g. memory and mailslots.           */
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
