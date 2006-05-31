// direct sound example for playing wave and midi files
// may be used for phonon win32 support


//In VC++, under the C/C++ tab within the Project->Settings menu, within
//the category of Code Generation, change the option under "Use run-time
//library:" from Single-threaded* to Multithreaded.


#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include <dmusici.h> // need the arrow thingies, but they don't display in html
#include <windows.h>
#include <windowsx.h>
//#include <fstream.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// if arrays are deleted, be sure to use delete [] array; first.

// defines
#define MAINWINDOWNAME "Clone of Game"
#define MAINWINDOWTITLE "Main Window"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Globals
HWND mainwindowhandle = NULL; // for later, the main window handle. (needed for wrappers and stuff)

// DX globals
IDirectMusicLoader8*      g_pLoader       = NULL;
IDirectMusicPerformance8* g_pPerformance  = NULL;
IDirectMusicSegment8*     g_pSegment      = NULL;

// Game functions
void deInit(void)
{
   IDirectMusicPerformance8_Stop(g_pPerformance,
        NULL,   // Stop all segments.
        NULL,   // Stop all segment states.
        0,      // Do it immediately.
        0       // Flags.
    );
    IDirectMusicPerformance8_CloseDown(g_pPerformance);
 
    IDirectMusicLoader8_Release(g_pLoader);
    IDirectMusicPerformance8_Release(g_pPerformance);
    IDirectMusicSegment8_Release(g_pSegment);

}
  int xx = 360, yy = 600;
  RECT washwall = {0,0, 800, 600};
  HBRUSH brushblack;// will be init later = CreateSolidBrush(RGB(0,0,0));

// Functions
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//  PAINTSTRUCT ps; // used in WM_PAINT
  HDC hdc; // handle to device context, used a lot

  switch(msg)
  {
  case WM_CREATE:
    {

      // called when window is created
      return(0);
    } break;

  case WM_PAINT: // called when repaintint is needed (refreshing)
    {
      // validate the window
      hdc= GetDC(hwnd);
      while(yy > -20) 
      {
      FillRect(hdc, &washwall, brushblack);
      FrameRect(hdc, &washwall, brushblack);
    
      // here is where the game is drawn :) A wrapper or so would come in real handy.
      SetBkMode(hdc, TRANSPARENT);
      SetTextColor(hdc, RGB(0,0,255));
      SetBkColor(hdc, RGB(0,0,0));
      TextOut(hdc, xx,yy, "CREDITS", strlen("CREDITS"));

      Sleep(5);
        yy--; 
     ReleaseDC(hwnd, hdc);
      } 
      
      return(0);
    } break;
    
  case WM_DESTROY: // called when window is killed
    {
      // kill the app
      // do not release DX objects in here, causes errors.

      PostQuitMessage(0);
 
      return(0);
    } break;
  default: break;
  } // end switch
  // process any other messages...
  return (DefWindowProc(hwnd, msg, wParam, lParam));
} // end WindowProc, the message processing function, since Windows is a message-based OS




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lbCmdLine, int nShowCmd)
{
  brushblack= CreateSolidBrush(RGB(0,0,0));

  //DMusic and COM setup
  CoInitialize(NULL);
//CoCreateInstance(REFCLSID,LPUNKNOWN,DWORD,REFIID,PVOID*);

    CoCreateInstance(&CLSID_DirectMusicLoader, NULL, 
                     CLSCTX_INPROC, &IID_IDirectMusicLoader8,
                     (void**)&g_pLoader);

    CoCreateInstance(&CLSID_DirectMusicPerformance, NULL,
                     CLSCTX_INPROC, &IID_IDirectMusicPerformance8,
                     (void**)&g_pPerformance );


  // end of COM and DMusic setup

  WNDCLASS mainwindow;
  HWND hwnd; // generic window handle
  MSG msg; // generic message

  mainwindow.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  mainwindow.lpfnWndProc = WindowProc;
  mainwindow.cbClsExtra = 0;
  mainwindow.cbWndExtra = 0;
  mainwindow.hInstance = hInstance;
  mainwindow.hIcon = LoadIcon(NULL, IDI_APPLICATION); // can use resources, IDB_BITMAP and MAKEINTRESOURCE()
  mainwindow.hCursor = LoadCursor(NULL, IDC_ARROW);
  mainwindow.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  mainwindow.lpszMenuName = NULL;  // can add menu later
  mainwindow.lpszClassName = MAINWINDOWNAME; // can be changed using define above...
  // now that the window class has been styled/filled, we register it
  if(!RegisterClass(&mainwindow)) // check for errors
    return (0); // if an error.
  // now to make the window itself...
  if (!(hwnd = CreateWindow(MAINWINDOWNAME, MAINWINDOWTITLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT,CW_USEDEFAULT, // x, y
    WINDOW_WIDTH, WINDOW_HEIGHT,
    NULL, // no parent window handle, since only one window
    NULL, // no handle to menu, currently
    hInstance, // instance
    NULL))) // creation parameters. (might be more useful in CreateWindowEx()?)
  return(0); // if no errors.
// save window handle in a global handle variable
  mainwindowhandle = hwnd;

  // make DMusic initialization
      IDirectMusicPerformance8_InitAudio(g_pPerformance,
        NULL,                  // IDirectMusic interface not needed.
        NULL,                  // IDirectSound interface not needed.
        hwnd,                  // Window handle.
        DMUS_APATH_SHARED_STEREOPLUSREVERB,  // Default audiopath type.
        64,                    // Number of performance channels.
        DMUS_AUDIOF_ALL,       // Features on synthesizer.
        NULL                   // Audio parameters; use defaults.
    );
// end init audio

// DMusic
    // Find the Windows media directory.
 
    CHAR strPath[MAX_PATH];
    GetWindowsDirectory( strPath, MAX_PATH );
    strcat( strPath, "\\Media" );
 

   // Convert to Unicode.
    WCHAR wstrSearchPath[MAX_PATH];
    MultiByteToWideChar( CP_ACP, 0, strPath, -1, 
                         wstrSearchPath, MAX_PATH );
 
    // Set the search directory.
 
    IDirectMusicLoader8_SetSearchDirectory(g_pLoader,
        &GUID_DirectMusicAllTypes,   // Types of files sought.
          /*L"C:\\Program Files\\KaZaA\\My Shared Folder"*/ wstrSearchPath, //Null for default
                                    // Where to look. was wstrSearchPath
        FALSE                       // Don't clear object data.
    );

  WCHAR wstrFileName[MAX_PATH] = L"TOWN.MID";
//  WCHAR wstrFileName[MAX_PATH] = L"Windows XP-Startvorgang.wav";
 
    if (FAILED(IDirectMusicLoader8_LoadObjectFromFile(g_pLoader,
        &CLSID_DirectMusicSegment,   // Class identifier.
        &IID_IDirectMusicSegment8,   // ID of desired interface.
        wstrFileName,               // Filename.
        (LPVOID*) &g_pSegment       // Pointer that receives interface.
    )))
    {
    
        MessageBox( NULL, "Media not found, sample will now quit.", 
                          "DMusic Tutorial", MB_OK );
        return 0;
    }


    IDirectMusicSegment8_Download(g_pSegment,g_pPerformance );


    IDirectMusicPerformance8_PlaySegmentEx(g_pPerformance,
        g_pSegment,  // Segment to play.
        NULL,        // Used for songs; not implemented.
        NULL,        // For transitions. 
        0,           // Flags.
        0,           // Start time; 0 is immediate.
        NULL,        // Pointer that receives segment state.
        NULL,        // Object to stop.
        NULL         // Audiopath, if not default.
    );      
/*
   // unneeded, but helps for splash screens. 
    Sleep(6000); // makes a segment of sound play for a bit.



 IDirectMusicPerformance8_Stop(g_pPerformance,
        NULL,   // Stop all segments.
        NULL,   // Stop all segment states.
        0,      // Do it immediately.
        0       // Flags.
    );

    deInit(); // releases stuff.
*/
    CoUninitialize();
  // Entering main event loop
// in winMain() the msg pump
 while(1)
  if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) 
    {
      if(!GetMessage(&msg,NULL,0,0)) return msg.wParam;
      TranslateMessage(&msg); DispatchMessage(&msg);
      // main game goes here
  } 
} 

