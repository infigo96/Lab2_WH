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

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
DWORD WINAPI mailThread(LPVOID);



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

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	HWND hWnd;
	DWORD threadID;
	MSG msg;
	

							/* Create the window, 3 last parameters important */
							/* The tile of the window, the callback function */
							/* and the backgrond color */

	hWnd = windowCreate (hPrevInstance, hInstance, nCmdShow, "Himmel", MainWndProc, COLOR_WINDOW+1);

							/* start the timer for the periodic update of the window    */
							/* (this is a one-shot timer, which means that it has to be */
							/* re-set after each time-out) */
							/* NOTE: When this timer expires a message will be sent to  */
							/*       our callback function (MainWndProc).               */
  
	windowRefreshTimer (hWnd, UPDATE_FREQ);
  

							/* create a thread that can handle incoming client requests */
							/* (the thread starts executing in the function mailThread) */
							/* NOTE: See online help for details, you need to know how  */ 
							/*       this function does and what its parameters mean.   */
							/* We have no parameters to pass, hence NULL				*/
  

	threadID = threadCreate (mailThread, NULL); 
  

							/* (the message processing loop that all windows applications must have) */
							/* NOTE: just leave it as it is. */
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
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

							/* create a mailslot that clients can use to pass requests through   */
							/* (the clients use the name below to get contact with the mailslot) */
							/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	
	mailbox = mailslotCreate ("mailbox");


	for(;;) {				
							/* (ordinary file manipulating functions are used to read from mailslots) */
							/* in this example the server receives strings from the client side and   */
							/* displays them in the presentation window                               */
							/* NOTE: binary data can also be sent and received, e.g. planet structures*/
 
	bytesRead = mailslotRead (mailbox, buffer, strlen(buffer)); 

	if(bytesRead!= 0) {
							/* NOTE: It is appropriate to replace this code with something */
							/*       that match your needs here.                           */
		posY++;  
							/* (hDC is used reference the previously created window) */							
		TextOut(hDC, 10, 50+posY%200, buffer, bytesRead);
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
	double total_time, a = 0, ax = 0, ay = 0, r = 0;
	clock_t time2, time = clock();
	while (pt->life > 0)

	{
		if (pt->next != NULL)
		{
			while (tmp != pt)
			{
				r = sqrt(pow(((tmp->sx) - (pt->sx)), 2) + pow((tmp->sy) - (pt->sy), 2));
				a = (GRAV*(tmp->mass)) / pow(r, 2);
				ax = ax + (a*((tmp->sx) - (pt->sx))) / r;
				ay = ay + (a*((tmp->sy) - (pt->sy))) / r;
				tmp = tmp->next;
			}
			time2 = clock();
			total_time = (double)(time2 - time) / CLOCKS_PER_SEC;
			pt->vx = pt->vx + ax*10;
			pt->vy = pt->vy + ay*10;
			pt->sx = pt->sx + pt->vx*10;
			pt->sy = pt->sy + pt->vy*10;
			time = clock();
			(pt->life)--;
			Sleep(10);
		}
	}
}
void createPlanet(planet_type** head, char name[20], double mass, double posX, double posY, double velX, double velY, int life)
{	
		planet_type* tmp_new = malloc(sizeof(planet_type));
		int i = 0; 
		int len = strlen(name);
		for (i = 0; i<= len ; i++)
		{
			tmp_new->name[i] = name[i];
		}
		tmp_new->mass = mass;
		tmp_new->sx = posX;
		tmp_new->sy = posY;
		tmp_new->vx = velX;
		tmp_new->vy = velY;
		tmp_new->life = life;
		*tmp_new->pid = 0; //change later

		if (*head == NULL)
		{
			*head = tmp_new;
			(*head)->next = NULL;
		}
		else if ((*head)->next == NULL)
		{
			tmp_new->next = *head;
			(*head)->next = tmp_new;
		}
		else if ((*head)->next != NULL)
		{
			tmp_new->next = (*head)->next;
			(*head)->next = tmp_new;
		}
		//threadCreate();
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

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
  
	PAINTSTRUCT ps;
	static int posX = 10;
	int posY;
	HANDLE context;
	static DWORD color = 0;
	planet_type* pt = NULL;
	char name[20] = "Excalibur";
	double mass = 100000000;
	double SX = 300;
	double SY = 300;
	double velX = 0;
	double velY = 0;
	double life = 100000;
	createPlanet(&pt, name, mass, SX, SY, velX, velY, life);
	//For testing---------
	char name2[20] = "Pluto";
	mass = 1000;
	SX = 200;
	SY = 300;
	velX = 0;
	velY = 0.008;
	life = 3000;
	createPlanet(&pt, name2, mass, SX, SY, velX, velY, life);
	Planet(pt->next);
	//----------


	createPlanet(pt, name, mass, SX ,SY, velX, velY, life);
	switch( msg ) {
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
							/* just to show how pixels are drawn                  */
			posX += 4;
			posY = (int) (10 * sin(posX / (double) 30) + 20);
			SetPixel(hDC, posX % 547, posY, (COLORREF) color);
			color += 12;
			windowRefreshTimer (hWnd, UPDATE_FREQ);
			break;
							/****************************************************************\
							*     WM_PAINT: (received when the window needs to be repainted, *
							*               e.g. when maximizing the window)                 *
							\****************************************************************/

		case WM_PAINT:
							/* NOTE: The code for this message can be removed. It's just */
							/*       for showing something in the window.                */
			context = BeginPaint( hWnd, &ps ); /* (you can safely remove the following line of code) */
			TextOut( context, 10, 10, "Hello, World!", 13 ); /* 13 is the string length */
			EndPaint( hWnd, &ps );
			break;
							/**************************************************************\
							*     WM_DESTROY: PostQuitMessage() is called                  *
							*     (received when the user presses the "quit" button in the *
							*      window)                                                 *
							\**************************************************************/
		case WM_DESTROY:
			PostQuitMessage( 0 );
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
			return( DefWindowProc( hWnd, msg, wParam, lParam )); 
   }
   return 0;
}




