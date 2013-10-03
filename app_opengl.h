
#include "glew.h"

#define GLCOMPAT

#include <stdio.h>
#include <stdlib.h>
#include "glew.h"
#ifdef _WIN32
	#include "wglew.h"
#else
	#include "glxew.h"
#endif

#include <windows.h>
#include <windowsx.h>
#include <io.h>
#include <fcntl.h>	
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <GL/gl.h>

extern void display ();
extern void keyboard_func (unsigned char, int, int);
extern void reshape (int, int);
extern void mouse_click_func (int,int,int,int);
extern void mouse_drag_func (int, int);
extern void mouse_move_func (int, int);
extern void idle_func ();
extern char* initialize ( char* cmdline );
extern void shutdown ();

#define GLUT_UP				0
#define GLUT_DOWN			1
#define GLUT_LEFT_BUTTON	1
#define GLUT_RIGHT_BUTTON	2

//------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HDC         g_hDC       = NULL;
HGLRC       g_hRC       = NULL;
HWND        g_hWnd      = NULL;
bool		g_bCtrl	= false;
bool		g_bShift = false;
float		window_width  = 1900;
float		window_height = 1024;
int			mState;	
FILE*		m_OutCons = 0x0;


void checkGL( char* msg )
{
	GLenum errCode;
	errCode = glGetError();
	if (errCode != GL_NO_ERROR) {
		printf ( "%s, ERROR: %s\n", msg, gluErrorString(errCode) );
		_getch();
		exit( errCode );
	}
}

void app_printf ( char* format, ... )
{
	// Note: This is the >only< way to do this. There is no general way to
	// pass on all the arguments from one ellipsis function to another.
	// The function vfprintf was specially designed to allow this.
	va_list argptr;
	va_start (argptr, format);				
	vfprintf ( m_OutCons, format, argptr);			
	va_end (argptr);			
	fflush ( m_OutCons );
}



//------------------------------------------------------------------------------

void APIENTRY glErrorCallback (  GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
    char *strSource = "0";
    char *strType = strSource;
    char *strSeverity = strSource;
    switch(source) {
	case GL_DEBUG_SOURCE_API_ARB:     strSource = "API";       break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:       strSource = "WINDOWS";       break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:     strSource = "SHADER COMP.";  break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:         strSource = "3RD PARTY";     break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:         strSource = "APP";           break;
    case GL_DEBUG_SOURCE_OTHER_ARB:               strSource = "OTHER";         break;
    }
    switch(type) {
	case GL_DEBUG_TYPE_ERROR_ARB:		        strType = "ERROR";        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:   strType = "Deprecated";     break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB :   strType = "Undefined";      break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:           strType = "Portability";    break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:           strType = "Performance";    break;
    case GL_DEBUG_TYPE_OTHER_ARB:                 strType = "Other";          break;
    }
    switch(severity) {
    case GL_DEBUG_SEVERITY_HIGH_ARB:	        strSeverity = "High";      break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:          strSeverity = "Medium";    break;
    case GL_DEBUG_SEVERITY_LOW_ARB:             strSeverity = "Low";       break;
    }
    app_printf ("GLError: %s - %s - %s : %s\n", strSeverity, strSource, strType, message); 
}

bool InitGL ()
{
    GLuint PixelFormat;
    
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    
    g_hDC = GetDC( g_hWnd );
    PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
    SetPixelFormat( g_hDC, PixelFormat, &pfd);
    g_hRC = wglCreateContext( g_hDC );
    wglMakeCurrent( g_hDC, g_hRC );

	// calling glewinit NOW because the inside glew, there is mistake to fix...
    // This is the joy of using Core. The query glGetString(GL_EXTENSIONS) is deprecated from the Core profile.
    // You need to use glGetStringi(GL_EXTENSIONS, <index>) instead. Sounds like a "bug" in GLEW.

    if(!wglCreateContextAttribsARB) wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (wglCreateContextAttribsARB) {
        HGLRC hRC = NULL;
        int attribList[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			#ifdef GLCOMPAT
						WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			#else
						WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			#endif
            WGL_CONTEXT_FLAGS_ARB,
            //WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB|
			#ifndef GLCOMPAT
						WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB|
			#endif
			#ifdef _DEBUG
						WGL_CONTEXT_DEBUG_BIT_ARB
			#else
						0
			#endif
            ,0, 0
        };
        if (!(hRC = wglCreateContextAttribsARB(g_hDC, 0, attribList)))
        {
            app_printf ("wglCreateContextAttribsARB() failed for OpenGL context.\n");
            return false;            
        }
        if (!wglMakeCurrent(g_hDC, hRC)) { 
			app_printf ("wglMakeCurrent() failed for OpenGL context.\n"); 
		} else {
            wglDeleteContext( g_hRC );
            g_hRC = hRC;
#ifdef _DEBUG
            if(!glDebugMessageCallbackARB)
            {
                glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
                glDebugMessageControlARB =  (PFNGLDEBUGMESSAGECONTROLARBPROC)wglGetProcAddress("glDebugMessageControlARB");
            }
            if(glDebugMessageCallbackARB)
            {
                glDebugMessageCallbackARB( glErrorCallback, NULL );
                glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_ARB, 0, NULL, GL_TRUE);
            }
#endif
        }
    }        

	glewInit();

    return true;
}

LRESULT CALLBACK WinProc( HWND   hWnd, UINT   msg, WPARAM wParam, LPARAM lParam )
{  
	PAINTSTRUCT ps;
    HDC hdc;
    switch( msg )
	{
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
        case WM_KEYUP:
			switch( wParam ) {
            case VK_CONTROL:	g_bCtrl = false;    break;
            case VK_SHIFT:		g_bShift = false;   break;
            } break;
		case WM_KEYDOWN:
            switch( wParam )  {
            case VK_CONTROL:    g_bCtrl = true;    break;
            case VK_SHIFT:      g_bShift = true;   break;
			default: {
				WPARAM param = wParam;
				char c = MapVirtualKey ( (UINT) param, MAPVK_VK_TO_CHAR );
				keyboard_func ( c, 0, 0 );  // invoke GLUT-style mouse events
				} break;
			} break;
		case WM_LBUTTONDOWN: {
			mState = GLUT_DOWN;
			mouse_click_func ( GLUT_LEFT_BUTTON, GLUT_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );		// invoke GLUT-style mouse move
			} break;
		case WM_RBUTTONDOWN: {
			mState = GLUT_DOWN;
			mouse_click_func ( GLUT_RIGHT_BUTTON, GLUT_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );		// invoke GLUT-style mouse move
			} break;
		case WM_LBUTTONUP: {
			mState = GLUT_UP;
			mouse_click_func ( GLUT_LEFT_BUTTON, GLUT_UP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );		// invoke GLUT-style mouse move
			} break;
		case WM_RBUTTONUP: {
			mState = GLUT_UP;
			mouse_click_func ( GLUT_RIGHT_BUTTON, GLUT_UP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );		// invoke GLUT-style mouse move
			} break;
		case WM_MOUSEMOVE: {
			int xpos = GET_X_LPARAM(lParam);
			int ypos = GET_Y_LPARAM(lParam);
			if ( mState == GLUT_DOWN )  mouse_drag_func ( xpos, ypos );// invoke GLUT-style mouse events
			else						mouse_move_func ( xpos, ypos ); 
			} break;
        case WM_SIZE:
            reshape(LOWORD(lParam), HIWORD(lParam));
            break;
        default:
            return DefWindowProc( hWnd, msg, wParam, lParam );            
			break;
	};
    return 0;
}


int InitWindow ( HINSTANCE hInstance )
{
	WNDCLASSEX winClass;
    MSG        uMsg;

	memset(&uMsg,0,sizeof(uMsg));

    winClass.lpszClassName = "MY_WINDOWS_CLASS";
    winClass.cbSize        = sizeof(WNDCLASSEX);
    winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    winClass.lpfnWndProc   = WinProc;
    winClass.hInstance     = hInstance;
    winClass.hIcon         = 0x0;
    winClass.hIconSm       = 0x0;
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;
    
    if(!RegisterClassEx(&winClass) )
        return E_FAIL;

    g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", "", WS_OVERLAPPEDWINDOW, 0, 0, (int) window_width, (int) window_height, NULL, NULL, hInstance, NULL );
    if( g_hWnd == NULL )
        return E_FAIL;

	return 1;
}

//------------------------------------------------------------------------------
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{ 
	MSG msg = {0};

	// Console window for printf
	AllocConsole ();
	long lStdHandle = (long) GetStdHandle( STD_OUTPUT_HANDLE );
	int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	m_OutCons = _fdopen( hConHandle, "w" );

	InitWindow ( hInstance );				// Init OS window

    if ( InitGL() ) {						// Init OpenGL context

		char* title = initialize ( lpCmdLine );			// User-init

		SetWindowText ( g_hWnd, title );

		ShowWindow( g_hWnd, nCmdShow );		// Show main window
	    UpdateWindow( g_hWnd );
		
        // Message pump        
		while( WM_QUIT != msg.message ) 
		{ 
			display ();        

			if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
        
		}
    }
    shutdown ();
    if( g_hRC != NULL ) {
        ReleaseDC( g_hWnd, g_hDC );
        g_hDC = NULL;
    }
    UnregisterClass( "MY_WINDOWS_CLASS", hInstance );

    return (int) msg.wParam;
}


#ifdef USE_GLUT

int main ( int argc, char **argv )
{
	#ifdef BUILD_CUDA
		// Initialize CUDA
		cudaInit( argc, argv );
	#endif

	// set up the window	
	glutInit( &argc, argv ); 
	glutInitDisplayMode( GLUT_RGBA ); //| GLUT_DEPTH | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 100, 100 );
	glutInitWindowSize( (int) window_width, (int) window_height );
	glutCreateWindow ( "" );

	// callbacks
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutMouseFunc( mouse_click_func );  
	glutKeyboardFunc( keyboard_func );
	glutMotionFunc( mouse_drag_func );
	glutPassiveMotionFunc ( mouse_move_func );
	
	glewInit ();

	if ( !g_txt.init ( "baub_16", window_width, window_height ) )  {
		printf ( "ERROR: Could not load font.\n" );
		exit ( -1 );
	}

	initialize ();

	//wglSwapIntervalEXT(0);			// no vsync
	
	glutIdleFunc( idle_func );
	glutMainLoop();

	return 0;
}

#endif