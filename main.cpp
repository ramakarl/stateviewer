
// State Viewer
// NVIDIA
// Rama Hoetzlein, 2013
// 

// THIS PROGRAM USES A MINIMAL HEADER TO ALLOW
// SAMPLE APPS WITH EITHER DIRECTX OR OPENGL
#ifdef USE_DX
	#include "app_directx.h"
#else
	#include "app_opengl.h"
#endif
#include "app_perf.h"
#include "app_util.h"

#include <string>
#include <algorithm>
#include <vector>
#include <conio.h>


Camera3D	cam;
int			mMode;
int			frame = 0;
int			mMaxSize = 1;
int			mMaxPrim = 1;
int			mSelectID = 0;
int			mSelectBin = 0;

int				startFrame = 0;
int				maxDraw = 0;
std::string		fileName;

bool		bShow[10];

Vector3DF	glide;

#define NUM_BIN			25

#define BIN_DRAW		25
#define BIN_SIZE		26
#define MAX_BIN			27

#define BIN_NOTUSED		0
#define BIN_CREATE		1
#define BIN_CHANGE		2
#define BIN_SWITCH		3
#define BIN_REUSE		4

struct Event {
	std::string		name;
	int				name_id;
	int				bin_id[MAX_BIN];
	int				bin_change[MAX_BIN];
	int				bin_size[MAX_BIN];
	int				size;
	int				count;
	int				frame;
	unsigned long	call_start;
	int				call_num;
};
struct Call {
	std::string		name;
	int				name_id;
	int				bin_id;
	int				size;
	unsigned long long	obj_id;
	unsigned long long	val_id;
};
struct Frame {	
	Frame()	{ clear (); }
	void clear () { totalDraw = 0; totalTransfer = 0; totalPrim = 0; for (int n=0; n <MAX_BIN; n++) { binChange[n]=0; binSwitch[n]=0; binReuse[n]=0; binUnique[n]=0;}  }
	int				binChange[MAX_BIN];
	int				binSwitch[MAX_BIN];
	int				binReuse[MAX_BIN];
	int				binUnique[MAX_BIN];
	int				totalDraw;
	int				totalTransfer;
	int				totalPrim;
};

std::vector<Event>		mEvents;
std::vector<Call>		mCalls;
std::vector<Frame>		mFrames;

std::string			binNames[MAX_BIN];
std::string			callNames[200];

Vector3DF getClr ( unsigned long id )
{
	srand ( id );	
	for (int n=0; n < 256; n++ ) {
		if ( n % 64 == 0 ) srand( rand() );
		rand();
	}
	return Vector3DF(float(rand())/RAND_MAX, float(rand())/RAND_MAX, float(rand())/RAND_MAX );
}

void setup_bins ()
{
	binNames[0] = "Shader";
	binNames[1] = "Render Target";
	binNames[2] = "Viewport";
	binNames[3] = "Raster State";
	binNames[4] = "Depth State";
	binNames[5] = "Blend State";
	binNames[6] = "Sampler State";
	binNames[7] = "Input Assembler";
	binNames[8] = "Texture";
	binNames[9] = "Vertex 0";
	binNames[10] = "Vertex 1";
	binNames[11] = "Vertex 2";
	binNames[12] = "Vertex 3";
	binNames[13] = "Vertex 4";	
	binNames[14] = "VS Const 0";
	binNames[15] = "VS Const 1";
	binNames[16] = "VS Const 2";
	binNames[17] = "VS Const 3";
	binNames[18] = "VS Const 4";
	binNames[19] = "PS Const 0";
	binNames[20] = "PS Const 1";
	binNames[21] = "PS Const 2";
	binNames[22] = "PS Const 3";
	binNames[23] = "PS Const 4";	
	binNames[24] = "Index Buffer";
	
	binNames[25] = "Draw Prims";
	binNames[26] = "Draw Bytes";

	// DirectX names
	callNames[0] = "Present";						// 0
	callNames[1] = "DrawIdx";
	callNames[2] = "DrawIst";
	callNames[3] = "Draw";
	callNames[4] = "CreateBuffer";
	callNames[5] = "CreateRenderTargetView";		// 5
	callNames[6] = "OMSetRenderTargets";
	callNames[7] = "CreateRasterizerState";
	callNames[8] = "RSSetState";
	callNames[9] = "CreateVertexShader (DX10)";
	callNames[10] = "CreateVertexShader (DX11)";	// 10
	callNames[11] = "CreatePixelShader (DX10)";
	callNames[12] = "CreatePixelShader (DX11)";
	callNames[13] = "VSSetShader";
	callNames[14] = "PSSetShader";
	callNames[15] = "Map";							// 15
	callNames[16] = "UpdateSubresource";
	callNames[17] = "IASetVertexBuffers";
	callNames[18] = "IASetIndexBuffer";
	callNames[19] = "VSSetConstantBuffers";
	callNames[20] = "PSSetConstantBuffers";			// 20
	callNames[21] = "memcpy";				

	// OpenGL names
	callNames[100] = "SwapBuffers";	
	callNames[101] = "DrawArrays";
	callNames[102] = "DrawElem";
	callNames[103] = "DrawInst";
	callNames[104] = "GenBuffers";
	callNames[105] = "BindBuffer";	
	callNames[106] = "BufferData";
	callNames[107] = "CreateShader";
	callNames[108] = "CreateProgram";
	callNames[109] = "UseProgram";
	callNames[110] = "GenTextures";	
	callNames[111] = "BindTexture";
	callNames[112] = "TexSubImage2D";
	callNames[113] = "glGetUniformLoc";
	callNames[114] = "glUniform1f";
	callNames[115] = "glUniform3f";
	callNames[116] = "glUniform4f";
	callNames[117] = "glUniformMatrix4fv";	
	callNames[118] = "ShaderSource";	
	callNames[119] = "glLoadMatrixd";	
	callNames[120] = "glLoadMatrixf";	
}

std::string getName ( char id )
{
	return callNames[id];
}


void drawMatrix ()
{
	Vector3DF id_clr;
	float ypos, yposl;
	int b, num = (int) mEvents.size();	

	Matrix4F proj, view, model;
	proj.Scale ( 2.0/window_width, -2.0/window_height, 1 );		
	model.Translate ( cam.getToPos().x, cam.getToPos().y, 1 );
	view.Scale ( cam.getToPos().z, 1, 1 );
	view *= model;
	model.Identity ();

	//----- 2D Drawing (visualizer space)		
	setview2D ( model.GetDataF(), view.GetDataF(), proj.GetDataF() );
	setorder2D ( false, 1 );

	// Event ID	bars	
	static2D ();	
	int fl = -1;
	char msg[100];
	for (int n=0; n < num; n++ ) {
		if ( mEvents[n].frame != fl ) {
			sprintf ( msg, "frame %d", mEvents[n].frame ); drawText ( n*10, -2, msg, 1,1,1,1 );
		}
		for (b=0; b < NUM_BIN; b++) {
			if ( mEvents[n].bin_id[b] == -1  ) {
				id_clr.Set ( 1, 1, 1 );
			} else if ( mEvents[n].bin_change[b] == 0 ) {
				id_clr.Set ( .2, .2, .2 );
			} else {
				id_clr = getClr ( mEvents[n].bin_id[b] + b*16384 );
			}
			drawFill ( n*10.0, b*25, n*10+10.0, b*25+24, id_clr.x, id_clr.y, id_clr.z, 1 );						
		}
		fl = mEvents[n].frame;
	}
	end2D ();

	// Change State bars
	static2D ();
	for (int n=0; n < num; n++ ) {
		for (b=0; b < NUM_BIN; b++) {
			if ( mEvents[n].bin_id[b] != -1 ) {
				switch ( mEvents[n].bin_change[b] ) {
				case BIN_NOTUSED:		id_clr.Set ( .2, .2, .2 );	break;		
				case BIN_CREATE:		id_clr.Set ( 1, 0, 0 );		break;
				case BIN_CHANGE:		id_clr.Set ( 1, 0, 0 );		break;
				case BIN_SWITCH:		id_clr.Set ( 1, .5, 0 );	break;
				case BIN_REUSE:		id_clr.Set ( 0, 1, 0 );		break;
				};				
				drawTri ( n*10, b*25, n*10+10, b*25, n*10, b*25+10, id_clr.x, id_clr.y, id_clr.z, 1 );
			}
		}
	}
	end2D ();

	int ys = NUM_BIN*25;
		
	// Prim count bars
	static2D();
	b = BIN_DRAW;
	for (int n=0; n < num; n++ ) {
		id_clr = getClr ( mEvents[n].bin_id[9] + 9*16384 );	
		ypos = mEvents[n].bin_id[ b ] * 400 / mMaxPrim;		// prim count
		if ( ypos > 100 ) ypos = 100;		
		drawFill ( n*10, ys+(125-ypos), n*10+10, ys+125, id_clr.x, id_clr.y, id_clr.z, 1 );
	}
	end2D ();
	
	// Mem transfer line
	static2D();
	yposl = 0;
	b = BIN_SIZE;
	for (int n=0; n < num; n++ ) {
		id_clr.Set(1,1,1);		
		ypos = mEvents[n].bin_size[ b ] * 100 / mMaxSize;		// size (bytes)
		if ( ypos > 100 ) ypos = 100;		
		drawLine ( n*10+5, ys+(250-ypos), n*10-5, ys+(250-yposl), 1, 1, 1, 1 );
		yposl = ypos;
	}
	drawLine ( 5, ys+250, num*10-5, ys+250, .5, .5, .5, 1 );
	end2D ();
}


void parseHeader ( char* buf, char& typ, char& nameID, unsigned long long& tstart, unsigned long long& tstop )
{
	typ =		*(char*) &buf[0];
	nameID =	*(char*) &buf[1];
	tstart =	*(unsigned long long*) &buf[2];
	tstop =		*(unsigned long long*) &buf[10];
}

void parseTrace ( char* buf, char nameID, Call& cl )
{
	cl.bin_id = *(int*) &buf[0];
	cl.size   = *(int*) &buf[4];
	cl.val_id = *(int*) &buf[8];
	cl.obj_id = *(int*) &buf[12];
	cl.name_id = nameID;
	cl.name = getName ( nameID );
}
void parseTrace ( char* buf, char nameID, Event& e )
{
	int n;
	for (n=0; n < NUM_BIN; n++) {
		e.bin_id[n] =		* (int*)	&buf[n*9 + 0];
		e.bin_change[n] =	* (char*)	&buf[n*9 + 4];
		e.bin_size[n] =		* (int*)	&buf[n*9 + 5];
	}	
	n = BIN_DRAW;
	e.bin_id[n] =		* (int*)	&buf[n*9 + 0];
	e.bin_change[n] =	* (char*)	&buf[n*9 + 4];
	e.bin_size[n] =		* (int*)	&buf[n*9 + 5];
	n = BIN_SIZE;
	e.bin_id[n] =		e.bin_id[BIN_DRAW];
	e.bin_change[n] =	e.bin_change[BIN_DRAW];
	e.bin_size[n] =		e.bin_size[BIN_DRAW];

	e.name_id = nameID;
	e.name = getName ( nameID );
}
void parseTrace ( char* buf, int& frame, int& size )
{
	frame = * (int* ) &buf [ 0 ];
	size  = * (int* ) &buf [ 4 ];
}

int		fpos = 0;
int		fnum = 0;
char	fdata[524288];

void readbytes ( char* buf, int siz, int cnt, FILE* fp )
{
	if ( fpos + siz >= fnum ) {		
		int delta = fnum-fpos;
		memcpy ( buf, &fdata[fpos], delta );
		fread ( fdata, 524288, 1, fp );
		fnum = 524288;
		fpos = 0;
		memcpy ( buf+delta, &fdata[fpos], siz-delta);
		fpos += siz-delta;
		return;
	}
	memcpy ( buf, &fdata[fpos], siz );
	fpos += siz;
}

void load_trace_raw ( char* fname )
{
	unsigned long totalBytes = getFileSize ( fname );
	unsigned long currBytes = 0;

	FILE* fp = fopen ( fname, "rb" );
	char header[2048];
	char buf[2048];

	Call cl;
	Event e;
	char typ, nameID;
	unsigned long long tstart, tstop;
	int fnum, size = 0;	
	int cstart = 0, cnum = 0;
	mMaxSize = 1;

	int num_frame = 0;
	int num_draw = 0;

	Frame f;
	f.clear ();
	frame = 0;

	while ( !feof(fp) && (num_draw < maxDraw || maxDraw==0)) {
			
		readbytes ( header, 18, 1, fp );		// 18 byte header
		parseHeader ( header, typ, nameID, tstart, tstop );
		switch ( typ ) {
		case 'C': {
			readbytes ( buf, 20, 1, fp );			
			if ( num_frame >= startFrame ) {				
				parseTrace ( buf, nameID, cl );
				mCalls.push_back ( cl );
				cnum++;
			}
			} break;
		case 'D': {
			
				currBytes = getFilePos ( fp );
				if ( f.totalDraw % 100 == 0 ) {
					if ( maxDraw == 0 ) {
						app_printf ( "%dk read, %.2f%% of file, %.2f%% done\n", currBytes/1024, currBytes*100.0f/totalBytes, currBytes*100.0f/totalBytes );	
					} else {
						app_printf ( "%dk read, %.2f%% of file, %.2f%% done\n", currBytes/1024, currBytes*100.0f/totalBytes, num_draw*100.0f/maxDraw );	
					}
				}				
				readbytes ( buf, NUM_BIN*9 + 9, 1, fp );
				
				if ( num_frame >= startFrame ) {
					parseTrace ( buf, nameID, e );
				
					e.frame = frame;
					e.call_num = cnum;
					e.call_start = cstart;
					mEvents.push_back ( e );
			
					cstart += cnum;
					cnum = 0;
					if ( e.bin_size[BIN_DRAW] > mMaxSize ) mMaxSize = e.bin_size[BIN_DRAW];
					if ( e.bin_id[BIN_DRAW] > mMaxPrim ) mMaxPrim = e.bin_id[BIN_DRAW];
					num_draw++;
		
					for (int n=0; n < NUM_BIN; n++) {
						f.binChange[n] += (e.bin_change[n]==BIN_CREATE || e.bin_change[n]==BIN_CHANGE) ? 1 : 0;
						f.binSwitch[n] += (e.bin_change[n]==BIN_SWITCH ) ? 1 : 0;
						f.binReuse[n] += (e.bin_change[n]==BIN_REUSE ) ? 1 : 0;
						f.binUnique[n] = (e.bin_id[n] > f.binUnique[n] ) ? e.bin_id[n] : f.binUnique[n];
					}
					f.totalDraw++;
					f.totalTransfer += e.bin_size[BIN_DRAW];
					f.totalPrim += e.bin_id[BIN_DRAW];		
				}
			} break;
		case 'F': {
			
			readbytes ( buf, 8, 1, fp );
			
			if ( num_frame >= startFrame ) {
				mFrames.push_back ( f );		// record frame data
				f.clear ();
				frame++;
				parseTrace ( buf, fnum, size );
				e.name_id = nameID;
				e.name = "Present";
				e.call_num = cnum;
				e.call_start = cstart;			
				e.count = 1;
				for (int n=0; n < NUM_BIN; n++ ) {
					e.bin_id[n] = -1;
					e.bin_change[n] = -1;
				}
				mEvents.push_back ( e );
				cstart += cnum;			
				cnum = 0;				
			}
			num_frame++;

			} break;
		};
	}
	// read may not have gotten to end of frame
	mFrames.push_back ( f );

	if ( mFrames.size() == 0 || mEvents.size() == 0 ) {
		app_printf ( "Error: No frames or events detected.\n" );
		app_printf ( "Try running trace again. \n" );
		_getch();
		exit(-1);
	}

	fclose ( fp );

}

void load_trace_txt ( char* fname ) 
{
	std::string str, word;
	char buf[1024];

	unsigned long totalBytes = getFileSize ( fname );
	unsigned long currBytes = 0;

	FILE* fp = fopen ( fname, "rt" );
	int c;
	std::vector<std::string> changestates;
	changestates.push_back ( "x" );			// 0 = BIN_NOTUSED
	changestates.push_back ( "c" );			// 1 = BIN_CREATE
	changestates.push_back ( "u" );			// 2 = BIN_CHANGE
	changestates.push_back ( "s" );			// 3 = BIN_SWITCH
	changestates.push_back ( "-" );			// 4 = BIN_REUSE
	changestates.push_back ( "D" );

	Call cl;
	Event e;
	int lin = 0;
	int max_lin = 5000;
	std::string szstr;
	int sz;
	unsigned long cstart = 0;
	int cnum = 0;

	while (!feof(fp) && lin < max_lin) {
		
		currBytes = getFilePos ( fp );
		printf ( "%d (%.2f%%)\n", currBytes, currBytes*100.0f/totalBytes );

		fgets ( buf, 1024, fp );
		str = buf;
		int bin = 0;		
		str = strTrim ( str );

		e.name = word;
		
		if ( str.compare (0, 2, "C:") == 0 ) {
			/*word = strSplit ( str, " " );	
			word = strSplit ( str, " " );	cl.bin_id = strToI(word);
			word = strSplit ( str, " " );	cl.size = strToI(word); 
			word = strSplit ( str, " " );	cl.obj_id = strToLI(word);
			word = strSplit ( str, " " );	cl.val_id = strToLI(word);
			word = strSplit ( str, " " );	cl.name = word;
			mCalls.push_back ( cl );*/
			cnum++;			
		} else if ( str.compare ( 0, 2, "FR" ) == 0 ) {
			e.count = 1;
			for (int n=0; n < NUM_BIN; n++ ) {
				e.bin_id[n] = -1;
				e.bin_change[n] = -1;
			}			
			mEvents.push_back ( e );
		} else if ( str.compare ( 0, 2, "Dr" ) == 0 ) {
			e.count = 1;
			int bin = 0;
			word = strLeft ( str, 8 );
			str = strTrim ( str );			
			while ( str.length() > 0 ) {
				word = strSplit ( str, " " );				
				c = strExtract ( word, changestates );
				szstr = strParse ( word, "[", "]" );
				e.bin_id[bin] = strToI ( word );
				e.bin_change[bin] = c;		
				e.bin_size[bin] = strToI ( szstr );
				bin++;				
			}	
			e.call_start = cstart;
			e.call_num = cnum;
			if ( e.bin_size[BIN_DRAW] > mMaxSize ) mMaxSize = e.bin_size[BIN_DRAW];
			mEvents.push_back ( e );			
			cstart += cnum;
		}	
		
		lin++;
	}

	fclose ( fp );
}

void drawOverlay ()
{
	Vector4DF clr;

	Matrix4F proj, view, model;
	proj.Scale ( 2.0/window_width, -2.0/window_height, 1 );		
	model.Translate ( cam.getToPos().x, cam.getToPos().y, 1 );
	view.Scale ( cam.getToPos().z, 1, 1 );
	view *= model;
	model.Identity ();

	//----- 2D Drawing (visualizer space)	
	setview2D ( model.GetDataF(), view.GetDataF(), proj.GetDataF() );
	setorder2D ( false, 1 );
	updatestatic2D ( 0 );		// update model/view/proj matrices of each static draw layer
	updatestatic2D ( 1 );
	updatestatic2D ( 2 );
	updatestatic2D ( 3 );
		
	// Selector Bar
	start2D ();					
	  drawRect ( mSelectID*10, 0, mSelectID*10+10, NUM_BIN*25, 1,1,1,1 );

	// Same-State Highlights (yellow)
	int minE = (((0-window_width/2.0)/cam.getToPos().z) - cam.getToPos().x) / 10;
	int maxE = (((window_width - window_width/2.0)/cam.getToPos().z) - cam.getToPos().x) / 10;
	minE = ( minE < 0 ) ? 0 : minE;
	maxE = ( maxE >= mEvents.size() ) ? mEvents.size() : maxE;
	for (int n=0; n < mEvents.size(); n++ ) {
		if ( mEvents[n].bin_id[mSelectBin] == mEvents[mSelectID].bin_id[mSelectBin] && n > minE && n < maxE ) {
			drawRect ( n*10, mSelectBin*25, n*10+10, mSelectBin*25+24, 1,1,0,1 );
			drawRect ( n*10+1, mSelectBin*25+1, n*10+9, mSelectBin*25+23, 1,1,0,1 );
		}
	}
	end2D ();
	
	//----- 2D Drawing (screen space)	
	setview2D ( window_width, window_height );	// change to screen space
	setorder2D ( true, -0.00001 );
	
	start2D ();					// dynamic draw
	int panel_width = 200;
	float xoff = window_width - panel_width;
	float yoff = cam.getToPos().y + (window_height/2);

	// Left panel - Bin Text 
	char name[128];
	char msg[1024];

	int frame = mEvents[mSelectID].frame; 
	frame = (frame < 0 || frame >= mFrames.size() ) ? 0 : frame;
	Frame& f = mFrames[ frame ];

	drawFill ( 0, yoff, +panel_width, yoff + NUM_BIN*25+250, 0.15,0.15,0.2,0.75 );
	
	drawText ( 10, yoff-85, "Frame #:", 1,1,1,1 );
	drawText ( 10, yoff-70, "Frame Draws:", 1,1,1,1 );
	drawText ( 10, yoff-55, "Frame Prims:", 1,1,1,1 );
	drawText ( 10, yoff-40, "Frame Transfer:", 1,1,1,1 );	
	drawText ( 10, yoff-20, "Frame States:", 1,1,1,1 );	
	sprintf ( msg, "%d", frame );					drawText ( 100, yoff - 85, msg, 1,1,1,1 );	
	sprintf ( msg, "%d", f.totalDraw );				drawText ( 100, yoff - 70, msg, 1,1,1,1 );
	sprintf ( msg, "%d", f.totalPrim );				drawText ( 100, yoff - 55, msg, 1,1,1,1 );
	sprintf ( msg, "%d bytes", f.totalTransfer );	drawText ( 100, yoff - 40, msg, 1,1,1,1 );

	setText ( 0.8, 0 );
	drawText ( 100, yoff - 20, "Modify", 1,0,0,1 );
	drawText ( 125, yoff - 10, "Switch", 1,0.5,0,1 );
	drawText ( 150, yoff - 20, "Reuse", 0,1,0,1 );
	drawText ( 175, yoff - 10, "Unique", 1,1,1,1 );
	drawTri ( 100, yoff-10, 100+10, yoff-10, 100, yoff, 1, 0,0, 1);
	drawTri ( 125, yoff-10, 125+10, yoff-10, 125, yoff, 1,.5,0, 1);
	drawTri ( 150, yoff-10, 150+10, yoff-10, 150, yoff, 0, 1,0, 1);
	setText ( 1.0, -0.5 );

	for (int b=0; b < NUM_BIN; b++ ) {
		strncpy ( name, binNames[b].c_str(), 128 );
		drawText ( 8, yoff + b*25+12, name, 1,1,1,1 );

		clr.x = float(f.binChange[b]) / f.totalDraw;	if ( clr.x > 1 ) clr.x = 1;
		sprintf ( msg, "%d", f.binChange[b] );	drawText ( 100, yoff + b*25+12-2, msg, clr.x*0.7+0.3, 0,0,1 );	
		
		clr.x = float(f.binSwitch[b]) / f.totalDraw;	if ( clr.x > 1 ) clr.x= 1;
		sprintf ( msg, "%d", f.binSwitch[b] );	drawText ( 125, yoff + b*25+12+2, msg, clr.x*0.7+0.3,clr.x*.4+0.2,0,1 );
		
		clr.x = float(f.binReuse[b]) / f.totalDraw; 	if ( clr.x > 1 ) clr.x= 1;
		sprintf ( msg, "%d", f.binReuse[b] );	drawText ( 150, yoff + b*25+12-2, msg, 0, clr.x*0.7+0.3, 0,1 );

		sprintf ( msg, "%d", f.binUnique[b] );	drawText ( 175, yoff + b*25+12+2, msg, .7, .7, .7,1 );
	}

	int ys = NUM_BIN*25;
	float ypos; 
	sprintf ( msg, "%d", mMaxPrim/4 ); 
	drawLine ( 0, yoff + ys+25, 200, yoff+ys+25, .6, .6, .6, 1);	
	drawText ( 10, yoff+ys+25+15, msg, .8,.8,.8,1 );
	drawText ( 10, yoff + ys+80, "# Prims", 1, 1, 1, 1 );
	drawText ( 10, yoff+ys+125, "0", .8,.8,.8,1 );
	drawLine ( 0, yoff + ys+125, 200, yoff+ys+125, .6, .6, .6, 1);
	ypos = mEvents[ mSelectID ].bin_id[ BIN_DRAW ] * 400 / mMaxPrim;		// prim count	
	clr = getClr ( mEvents[ mSelectID ].bin_id[9] + 9*16384 );
	if ( ypos > 100 ) ypos = 100;			
	sprintf ( msg, "%d", mEvents[ mSelectID ].bin_id[BIN_DRAW] );
	drawLine ( 150, yoff+ys+(125-ypos), 200, yoff+ys+(125-ypos), clr.x,clr.y,clr.z,1 );
	drawText ( 150, yoff+ys+(125-ypos), msg, clr.x,clr.y,clr.z,1 );

	sprintf ( msg, "%d", mMaxSize ); 
	drawLine ( 0, yoff + ys+150, 200, yoff+ys+150, .6, .6, .6, 1);
	drawText ( 10, yoff+ys+150+15, msg, .8,.8,.8,1 );
	drawText ( 10, yoff + ys+200, "Transfer (bytes)" , 1, 1, 1, 1);
	drawText ( 10, yoff+ys+250, "0", .8,.8,.8,1 );
	drawLine ( 0, yoff + ys+250, 200, yoff+ys+250, .6, .6, .6, 1);
	ypos = mEvents[ mSelectID ].bin_size [ BIN_SIZE ] * 100 / mMaxSize;		// prim count
	if ( ypos > 100 ) ypos = 100;	
	sprintf ( msg, "%d", mEvents[ mSelectID ].bin_size [BIN_SIZE] );
	drawLine ( 150, yoff+ys+(250-ypos), 200, yoff+ys+(250-ypos), 1,1,1,1 );
	drawText ( 150, yoff+ys+(250-ypos), msg, 1,1,1,1 );
		
	// Right panel - Call Text		
	drawFill ( xoff, yoff, xoff + panel_width, yoff + NUM_BIN*25+250, 0.15,0.15,0.2,0.75 );	
	int cid;
	glColor3f ( 1, 1, 1);
	
	for (int c = 0; c < mEvents[ mSelectID ].call_num; c++ ) {	
		cid = mEvents[ mSelectID ].call_start + c;		
		clr = getClr ( mCalls[cid].val_id + mCalls[cid].bin_id*16384 );
		clr.w = 1;
		drawFill ( xoff+100, yoff + c*15, xoff+125, yoff+c*15+13, clr.x, clr.y, clr.z, clr.w );

		sprintf ( msg, "%s", mCalls[cid].name.c_str() );  
		drawText ( xoff+5, yoff + c*15+15, msg, 1,1,1,1 );

		sprintf ( msg, "%02d  %d %d", mCalls[cid].bin_id, mCalls[cid].size, mCalls[cid].val_id );
		drawText ( xoff+130, yoff + c*15+15, msg, 1,1,1,1 );
	}
	end2D ();
	
	

	#ifdef USE_DX
		g_pContext->OMSetDepthStencilState( g_pDepthStencilState, 1 );
	#endif
}

// Main display loop
void display () 
{
	cam.moveToPos ( glide.x, 0, 0  );
	glide.x *= 0.8;	
		
	PERF_PUSH ( "frame" );

	// Clear framebuffers. OpenGL
	glClearColor( 0.1, 0.1, 0.1, 0.0 );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	drawOverlay ();

	drawGui ();

	draw2D ();

	#ifdef USE_DX
		// DirectX - Swap buffers		
		checkHR ( g_pSwapChain->Present ( 0, 0 ) );
	#else
		// OpenGL - Swap buffers
		SwapBuffers ( g_hDC );  		
	#endif

	PERF_POP ();

	frame++;
}

void reshape ( int width, int height ) 
{
	// set window height and width
	window_width  = (float) width;
	window_height = (float) height;
	glViewport( 0, 0, width, height );  
	setview2D ( window_width, window_height );
}

// This function called by both OpenGL (GLUT) and DirectX
void keyboard_func ( unsigned char key, int x, int y )
{
	switch ( key ) {		
	case 27:
		exit ( 0 ); 
		break;
	case 'v':	bShow[0] = !bShow[0];	break;
	case 'c':	bShow[1] = !bShow[1];	break;		
	};
}


Vector3DF cangs;
Vector3DF ctp;
float cdist;
int dragging;
#define DRAG_OFF	0
#define DRAG_LEFT	1
#define DRAG_RIGHT	2
float last_x, last_y;

// This function called by both OpenGL (GLUT) and DirectX
void mouse_click_func ( int button, int state, int x, int y )
{
  cangs = cam.getAng();
  ctp = cam.getToPos();
  cdist = cam.getOrbitDist();

  if ( state==GLUT_DOWN && guiMouseDown ( x, y ) ) return;		// event handling for nv2D GUIs

  if( state == GLUT_DOWN ) {
    if ( button == GLUT_LEFT_BUTTON )		dragging = DRAG_LEFT;
    else if ( button == GLUT_RIGHT_BUTTON ) dragging = DRAG_RIGHT;	
    last_x = x;
    last_y = y;	
  } else if ( state==GLUT_UP ) {
    dragging = DRAG_OFF;
  }
}

// This function called by both OpenGL (GLUT) and DirectX
void mouse_move_func ( int x, int y )
{
	mSelectID = (((x-window_width/2.0)/cam.getToPos().z) - cam.getToPos().x) / 10;
	if ( mSelectID < 0 ) mSelectID = 0;
	if ( mSelectID > mEvents.size()-1 ) mSelectID = mEvents.size() -1;

	mSelectBin = ((y-window_height/2.0) - cam.getToPos().y) / 25;
	if ( mSelectBin < 0 ) mSelectBin = 0;
	if ( mSelectBin >= NUM_BIN ) mSelectBin = NUM_BIN-1;
}

// This function called by both OpenGL (GLUT) and DirectX
void mouse_drag_func ( int x, int y )
{
	if ( guiMouseDrag ( x, y ) ) return;	// event handling for nv2D GUIs

	int dx = x - last_x;
	int dy = y - last_y;

	float deltx = window_width / cam.getToPos().z;
	float delty = window_height / cam.getToPos().z;
	
	// Camera interaction
	int mode = 0;
	switch ( mode ) {
	case 0:
		if ( dragging == DRAG_LEFT ) {
			glide += Vector3DF( dx / cam.getToPos().z, 0, 0);		
			cam.moveToPos ( 0, dy, 0 );
		} else if ( dragging == DRAG_RIGHT ) {	
			cam.moveToPos ( 0, 0, -dy*0.001 );
			if ( cam.to_pos.z < 0.001 ) cam.to_pos.z = 0.001;
		}
		break;	
	}
	last_x = x;
	last_y = y;
}


void idle_func ()
{
}

char* initialize ( char* cmdline )
{
	// Get comnmand line
	std::string str = cmdline;
	std::vector<std::string>	args;
	while ( str.length() > 0) {
		args.push_back ( strSplit ( str, " " ) );		
	}
	fileName = "";
	for (int n=0; n < args.size(); n++ ) {
		if ( args[n].compare ( "-f" ) == 0 ) {
			startFrame = strToI ( args[n+1] );
		}
		if ( args[n].compare ( "-d" ) == 0 ) {		// max_draw
			maxDraw = strToI ( args[n+1] );
		}
		if ( args[n].find_first_of ( "." ) != std::string::npos ) {
			fileName = args[n];
		}
	}
	if ( fileName.length()== 0 || args.size()==0 ) {
		app_printf ( "USAGE:  state_view [-f #] [-d #] filename.raw\n\n" );
		app_printf ( "  -f #   Start at frame \n" );
		app_printf ( "  -d #   Maximum number of draw calls to read \n" );
		_getch();
		exit(-1);
	}

	for (int n=0; n < 10; n++ ) bShow[n] = true;

	// Initialize camera
	cam.setPos ( 0, 0, 1 );
	cam.setToPos ( -window_width*3/8, -window_height*3/8, 1 );
	cam.updateMatricies ();

	// Initialize bin and call names
	setup_bins ();

	// Load trace file	
	char fname[256];
	strcpy ( fname, fileName.c_str() );
	load_trace_raw ( fname );	

	// required init functions
	init2D ( "arial_12" );		// specify font file (.bin/tga)
	setText ( 1.0, -0.5 );		// scale by 0.5, kerning adjust -0.5 pixels
	setview2D ( window_width, window_height );

	// draw visualization layer
	drawMatrix ();
	
	PERF_INIT ( false );						// Enable CPU perf (do not require nv-perfmarker DLL)
	PERF_SET ( false, 2, false, "" );		// Perf CPU?, CPU level, NV Perfmarkers, Log filename

	return "NVIDIA State Viewer, by Rama Hoetzlein";	// Return Window title
}


void shutdown ()
{
}
