NVIDIA STATEVIEWER 
===================
NVIDIA (c) 2013

Contact: Rama Hoetzlein, rhoetzlein@nvidia.com

The StateViewer tracks state changes in DX11 and OpenGL graphics apps, 
using apitrace to initially capture and record traces to a .trace file.
During retrace the state bins are tracked, and output to a .raw file 
which is visualized using the StateViewer.

The StateViewer consists of two parts:

1) d3dretrace/glretrace - State tracing, now part of apitrace. (https://github.com/apitrace/apitrace)

2) StateViewer - This visualizer for viewing state trace data.

OVERVIEW
====================
- Install apitrace
- Run apitrace to produce .trace file
- Run the d3dretrace or glretrace with new -r and -o options (by Nvidia) to replay the trace 
  with state tracking enabled for output to a .raw file
- Run this StateViewer to visualize the state trace data from .raw files

TRACING
====================
* Trace an application to produce a replayable .trace file

Step 1. Start a command line, and modify path to specify 32 or 64-bit trace, 
        depending on application to be traced.

   > path=%path%;c:\codes\stateview\x64\bin     

Step 2. Run application trace 

   > apitrace64 trace -a dxgi -o demo.trace nvinstdemo.exe

   > apitrace86 trace -a gl -o demo.trace opengldemo.exe

   See apitrace website for additional trace options: http://apitrace.github.io/ 


RETRACING 
=====================
* Retrace the application with the new state tracking option (by Nvidia)

Step 3. Retrace the application

   > d3dretrace demo.trace
   Normal usage of retrace. Just replays the trace file on the screen. 
   Useful to confirm a good trace.

   > d3dretrace -r -o demo.raw demo.trace
   Perform state tracking in raw binary mode (-r), with output to demo.raw (-o)
   Due to the state track algorithm the replay will occur twice on the screen.

   > d3dretrace -t -o demo.txt demo.trace
   Perform state tracking in text mode (-t), to view detailed state information.
   This does the full state track algorithm, and outputs all tracked calls to txt file.

   > d3dretrace -r -f 10000 -o demo.raw demo.trace
   Perform state tracking, but start tracking after 10000th frame. 
   Useful to reduce the size of .raw files.

VISUALIZATION
======================
* Visualize the state tracking results

Step 4. Run the StateViewer

   > stateview demo.raw
   Visualize all state data in the .raw file

   > stateview -d 5000 demo.raw
   Limit playback to 5000 draw calls. Useful when .raw file is large.

   > stateview -f 100 -d 5000 demo.raw
   Skip first 100 frames of visualization, and also limit playback to 5000 draw calls.


BINS CURRENTLY TRACKED
======================
The StateViewier assigns all graphics API calls to a particular bin in order
to track and compare values. The following bin names are used:
 * NA = not yet tracked

0	Shader
1	Render Target
2	Viewport
3	Rasterizer State
4	Depth State *NA*
5	Blend State *NA*
6	Sampler State *NA*
7	Input  *NA*
8	Texture
9	Vertex Buffer (IA Slot 0)
10	Vertex Buffer (IA Slot 1)
11	Vertex Buffer (IA Slot 2)
12	Vertex Buffer (IA Slot 3)
13	Vertex Buffer (IA Slot 4)
14	VS Const Buffer 0
15	VS Const Buffer 1
16	VS Const Buffer 2
17	VS Const Buffer 3
18	VS Const Buffer 4
19	PS Const Buffer 0
20	PS Const Buffer 1
21	PS Const Buffer 2
22	PS Const Buffer 3
23	PS Const Buffer 4
24	Index Buffer


FUNCTIONS CURRENTLY TRACKED
===========================
This is the list of API functions currently tracked 
by the Nvidia StateViewer (c) 2013. 
It does not cover all the functions which apitrace supports, which is a
much larger set. 

Updated 9/24/2013.

OpenGL:
 memcpy
 wglSwapBuffers
 glDrawArrays
 glDrawElements
 glGenBuffers
 glBindBuffer
 glBufferData
 glCreateShader
 glCreateProgram
 glUseProgram
 glGenTextures
 glBindTexture
 glTexSubImage2D
 glGetUniformLocation
 glUniform1f/3f/4f
 glUniformMatrix4fv
 glShaderSource
 glVertexPointer
 glNormalPointer
 glLoadMatrixd
 glLoadMatrixf

DX10/11:
 memcpy
 DXGISwapChain::Present
 D3D11Buffer:Map
 D3D11DeviceContext:Map
 D3D11Device:DrawIndexed
 D3D11Device:DrawInstanced
 D3D11Device:Draw
 D3D11Device:CreateBuffer
 D3D11Device:CreateRenderTargetView
 D3D11Device:OMSetRenderTargets
 D3D11Device:CreateRasterizerState
 D3D11Device:RSSetState
 D3D11Device:CreateVertexShader
 D3D11Device:CreatePixelShader
 D3D11Device:VSSetShader
 D3D11Device:PSSetShader
 D3D11Device:UpdateSubresource
 D3D11Device:IASetVertexBuffers
 D3D11Device:VSSetConstantBuffers
 D3D11Device:PSSetConstantBuffers
 

BUILDING APITRACE
==================
Building APITRACE is only needed if the goal is also to rebuild the StateViewer,
or to add functionality to the StateViewer.

INSTALL:

1. Requires VS 2010 64-bit Service Pack 1

2. Install cmake and Python 2.7.5 (do not install 2.7.3 or 3.3)
    * Make sure that later version of Python were not previously installed

3. Get apitrace: 
      git clone git://github.com/apitrace/apitrace.git

CMAKE BUILD:

4. Rename retrieved folder as: /apitrace_v5

5. Run VS 2010 Command Prompt - to get correct VC env variables

6. Include cmake-gui in path: path=%path%;c:\Program Files (x86)\CMake 2.8\bin

7. Run cmake-gui from command line (CMake 2.8): 
	cmake-gui -H%cd% -B%cd%\build

8. Source path: c:/codes/apitrace_v5/
   Build path:  c:/codes/apitrace_v5/build
   Configure:   VS 2010 64-bit 

9. Confirm PYTHON_EXECUTABLE	C:/Python27/python.exe  (NOT /Python33 or other ver)

11. Click Configure. Click Generate.

BUILD SOLUTION:

10. Build entire solution (34 projects). Rebuild again to resolve dependencies.


11. If you get this issue:
	Issue:	Any .py file error which says
		print "..." ^
		SyntaxError: invalid syntax.
	Resolve: You must use Python 2.7.5. Cannot use Python 3.x. 
  		Later version of pythong require () for print statements.
		http://stackoverflow.com/questions/12713648/python-3-3-0-syntax-err
12. Trace/retrace build to:	\build\Release
    Injector builds to:		\build\Release
    API wrappers build to:	\build\wrappers\Release



