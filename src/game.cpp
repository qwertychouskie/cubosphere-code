/**
Copyright (C) 2010 Chriddo
Copyright (C) 2018 Valeri Ochinski <v19930312@gmail.com>

This program is free software;
you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, see <http://www.gnu.org/licenses/>.
**/

#include "game.hpp"

#include "console.hpp"


#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <SDL.h>

#include "vectors.hpp"
#include "cuboboxes.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include "globals.hpp"
#include "shaderutil.hpp"
#include "models.hpp"
#include "cuboenemies.hpp"
#include "luautils.hpp"
#include "posteffects.hpp"
#include "cubopathfind.hpp"
#include "spriteutils.hpp"
#include "sounds.hpp"
#include "glutils.hpp"


#ifdef PARALLELIZE
#include <omp.h>
#endif

static CuboGame *TheGame=NULL;

CuboGame *g_Game() {return TheGame;}


static void keyhandle(int key,int down,int special) {
	if (!TheGame) { return; }
	else { TheGame->KeyHandle(key,down,special); }
	}

static void texthandle(std::string& str) {
	if (!TheGame) { return; }
	else { TheGame->TextInputHandle(str); }
	}
/*
static void joyaxishandle(int joys,int axis,float val,float pval)
{
 if (!TheGame) return;
  else TheGame->JoyAxisHandle(joys,axis,val,pval);

}
*/
static void joyhandle(int joys,int button,int dir,int down,int toggle) {
	if (!TheGame) { return; }
	else { TheGame->DiscreteJoyHandle(joys,button,dir,down,toggle); }

	}

//const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
//const GLfloat light_ambient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
const GLfloat light_ambient[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
//const GLfloat light_ambient[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_emission[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
//const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_diffuse[]  = { 0.5,0.5,0.5,1.0 };
//const GLfloat light_specular[] = { 0,0,0,0};
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//const GLfloat light_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//const GLfloat light_specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
//const GLfloat light_position[] = { 0.0f, 0.0f, 1.0f, 0.0f };
const GLfloat light_position[] = { 0.0f, 0.0f, 1.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 10.0f };


void Game::End() {
	font.StopFontEngine();
	joysticks.Free();
	}

void Game::CheckNeededExtensions() {

	if ((const char *) glGetString(GL_VERSION)) {
			std::string version = (const char *) glGetString(GL_VERSION);
			coutlog("INIT: OpenGL version "+version);
			}
	else { coutlog("INIT: unknown OpenGL version "); }

	if (glewIsSupported("GL_VERSION_2_0")) {
			std::string slangvers=((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
			coutlog("      Shaders with the GLSL version "+slangvers+" found");
			float vers=atof(slangvers.c_str());
			//  ostringstream oss; oss << vers;
			//  coutlog(oss.str());
			if (vers>=1.2999) {
					supportingShaders=1;
					}
			else {   coutlog("       > GLSL version not supported"); supportingShaders=0; }

			}

	else {

			supportingShaders=0;
			}


	}

/*
void OutVideoInfo()
{
  const SDL_VideoInfo *inf=SDL_GetVideoInfo();




}*/

/*
 * 0 = No VSync
 * 1 = VSync
 * -1 = Adaptive VSync
*/
/*
void Game::SetVSync(bool type) {
	if (type) {
		if (SDL_GL_SetSwapInterval(-1) == -1) { // Try adaptive VSync first
			SDL_GL_SetSwapInterval(1);
		}
	} else {
		SDL_GL_SetSwapInterval(0);
	}
};*/

bool Game::SetHWRender(bool hw) {
	if (sdlWindow and (not sdlRenderer or hw != IsHWRender)) {
			IsHWRender = hw;
			if (sdlRenderer) { SDL_DestroyRenderer(sdlRenderer); }
			sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, (hw ? (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) : SDL_RENDERER_SOFTWARE));
			glewInit();
			CheckNeededExtensions();

			glClearColor(1,1,1,1);
			//glClearColor(0,0,0,1);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			glEnable(GL_LIGHT0);
			glEnable(GL_NORMALIZE);
			glDisable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
			glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
			glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
//    glLightfv(GL_LIGHT0, GL_EMISSION, light_emission);
			glLightfv(GL_LIGHT0, GL_POSITION, light_position);

			glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
			glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			InvalidateMaterial();
			if (SDL_GL_SetSwapInterval(-1) == -1) { SDL_GL_SetSwapInterval(1); } // Use (Adaptive) VSync
			glReady=1;
			return sdlRenderer;
			}
	return false;
	}


bool Game::InitGL(int w, int h, int hw, int fs) {
//  PrintModes();
	textures.clear();
	Uint32 flags = SDL_WINDOW_OPENGL  ;
	if (fs) { flags|= SDL_WINDOW_FULLSCREEN; }
//	if (hw) { flags|= SDL_HWSURFACE | SDL_DOUBLEBUF; } // TODO: anything like this in sdl2?
//	else { flags|=SDL_SWSURFACE; }

	if (AntiAliasing) {
// SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			int naa=4;
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, naa);
			}
	else {
// SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			}

	//OutVideoInfo();
	screenwidth=w;
	screenheight=h;
	//SDL_CreateWindowAndRenderer(w, h, flags, &sdlWindow, &sdlRenderer);
	sdlWindow = SDL_CreateWindow("Cubosphere", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
	SetHWRender(hw);

	return true;
	}

bool Game::UpdateWindow(int w,int h,int hw,int fs) {
	textures.clear();

	if (AntiAliasing) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			constexpr int naa=4; // FIXME: WTF?
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, naa);
			}
	else {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			}

	//OutVideoInfo();
	screenwidth=w;
	screenheight=h;
	int res = SDL_SetWindowFullscreen(sdlWindow, fs ? SDL_WINDOW_FULLSCREEN : 0);
	SDL_SetWindowSize(sdlWindow, w, h);
	SetHWRender(hw);
	if (not (sdlWindow and sdlRenderer and !res)) { return false; }
	return true;
	}

void Game::Quit() {
	events.Close();
	}

void Game::HandleInput() {
	keyboard.HandleKeys();
	joysticks.HandleKeys();
	}


static int framecounter=0;
static double ftime=0;

void Game::HandleEvents() {
	events.HandleEvents();
	}

void Game::GameLoopFrame() {
	if (!GameLua.CallVAIfPresent("GameLoop")) {
			HandleEvents(); //This will parse keyboard and mouse input
			//Call the input's Handler
			HandleInput();
			//Do the physics
			Think();
			//Do the Render-Work
			Render();
			}

	}

void Game::GameLoop() {

	double maxframeaccu=0;


	while (!events.Closed()) {
			//Get the elapsed time
			oldtime=time;
			time = SDL_GetTicks();
			elapsed=time-oldtime;
			elapsed/=1000.0;

			maxframeaccu+=elapsed;
			if (maxframes>0) {
					if (maxframeaccu<1.0/maxframes) { continue; }
					else {
							elapsed=maxframeaccu;
							maxframeaccu=0;
							}
					}

			framecounter++;
			ftime+=elapsed;
			if (ftime>=1.0) {
					double fps=framecounter/ftime;

					FPS=(int)fps;
					ftime=0;
					framecounter=0;
					}

			if (minframes>0) {
					if (elapsed>1.0/minframes) {
							elapsed=1.0/minframes;
							}
					}





			TheGame=(CuboGame*)this;

			GameLoopFrame();


			}
	}


int Game::Init() {

	TheGame=(CuboGame*)this;
	mouse.Initialize();
	joysticks.Initialize();
	events.SetMouse(&mouse);
	events.SetKeyboard(&keyboard);
	events.SetJoystick(&joysticks);
	FPS=0;
	glReady=0;
	return 0;
	}

void Game::SetGameLoopSource(std::string s) {
	if (s=="") { GameLoopSource="-"; } //stdin
	else { GameLoopSource=s; }
	}

void Game::Start() {
	TheGame=(CuboGame*)this;
	CuboConsole::GetInstance()->Init();



	GameLua.LoadUserLibs();
	// DEBUGGING CODE
	/*
	{
		std::vector<std::string> cmds;
		cmds.push_back("local j = require 'Joystick'");
		cmds.push_back("local joy = j.Get(0)");
		cmds.push_back("io.write('Joy count: ', j.Count(), '\\n')");
		cmds.push_back("if joy:IsConnected() then");
		cmds.push_back("io.write('Name: ', joy:GetName(), '\\n')");
		cmds.push_back("io.write('GUID: ', joy:GetGUID(), '\\n')");
		cmds.push_back("else");
		cmds.push_back("io.write('No joys connected\\n')");
		cmds.push_back("end");
		GameLua.ExecStrings(cmds);
	}*/

//Prepare Looper
	if (GameLoopSource!="") {
			std::string line;
			std::istream * inp=NULL;
			std::ifstream file;
			if (GameLoopSource=="-") { inp=&std::cin; }
			else {
					file.open(GameLoopSource.c_str());
					inp=&file;
					}
			std::vector<std::string> cmds;
			while (inp->good()) {
					getline((*inp), line);
					cmds.push_back(line);
					}
			GameLua.ExecStrings(cmds);
			}
	if (GameLoopSource=="") {
			std::vector<std::string> defaultcmds;
			defaultcmds.push_back("function GameLoop()");
			defaultcmds.push_back(" GAME_HandleEvents();");
			defaultcmds.push_back(" GAME_HandleInput();");
			defaultcmds.push_back(" GAME_Think();");
			defaultcmds.push_back(" GAME_Render();");
			defaultcmds.push_back("end;");
			GameLua.ExecStrings(defaultcmds);
			}

	GameLua.CallVAIfPresent("Init");

	time=oldtime=SDL_GetTicks();
	GameLoop();
	}






////////////////////////////////////////////////////////////////7

static SDL_Surface* flip_vertical(SDL_Surface* sfc) {
	SDL_Surface* result = SDL_CreateRGBSurface(sfc->flags, sfc->w, sfc->h,
			sfc->format->BytesPerPixel * 8, sfc->format->Rmask, sfc->format->Gmask,
			sfc->format->Bmask, sfc->format->Amask);
	const auto pitch = sfc->pitch;
	const auto pxlength = pitch*sfc->h;
	auto pixels = static_cast<unsigned char*>(sfc->pixels) + pxlength;
	auto rpixels = static_cast<unsigned char*>(result->pixels) ;
	for(auto line = 0; line < sfc->h; ++line) {
			memcpy(rpixels,pixels,pitch);
			pixels -= pitch;
			rpixels += pitch;
			}
	return result;
	}

void CuboGame::SaveFramePic(std::string fname, int nw,int nh) {

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int w=viewport[2];
	int h=viewport[3];

	int nSize = w*h*3;

	auto pixels = std::make_unique<GLubyte[]>(nSize);
	//GLubyte *pixels = new GLubyte [nSize];

	glReadPixels(0, 0, w, h, GL_RGB,
			GL_UNSIGNED_BYTE, pixels.get());

	//Resizing
	if ( ((nw>=0) && (w!=nw)) || ((nh>=0) && (h!=nh))) {
			if (nw<0) { nw=w; } if (nh<0) { nh=h; }
			int nSize2 = nw*nh* 3;
			auto pixels2 = std::make_unique<GLubyte[]>(nSize2);
			gluScaleImage(GL_RGB,w,h,GL_UNSIGNED_BYTE,pixels.get(),nw,nh,GL_UNSIGNED_BYTE,pixels2.get());
			pixels = std::move(pixels2);
			w=nw; h=nh;
			nSize=nSize2;
			}

	std::string ext=fname.substr(fname.find_last_of(".") + 1);
	if (ext=="tga") {
			auto fScreenshot = fopen(fname.c_str(),"wb");
			//convert to BGR format
			for(int i = 0; i < nSize; i+=3) {
					std::swap(pixels[i], pixels[i+2]);
					}

			unsigned char TGAheader[12]= {0,0,2,0,0,0,0,0,0,0,0,0};
			unsigned char header[6] = {(unsigned char)(w%256),(unsigned char)(w/256),
									   (unsigned char)(h%256),(unsigned char)(h/256),24,0
									  };

			fwrite(TGAheader, sizeof(unsigned char), 12, fScreenshot);
			fwrite(header, sizeof(unsigned char), 6, fScreenshot);
			fwrite(pixels.get(), sizeof(GLubyte), nSize, fScreenshot);
			fclose(fScreenshot);



			}
	else if (ext=="jpg") {
			SDL_Surface* img = SDL_CreateRGBSurfaceFrom(pixels.get(), w, h, 24, w*3, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
			auto img2 = flip_vertical(img);
			IMG_SaveJPG(img2, fname.c_str(), 100);
			SDL_FreeSurface(img);
			SDL_FreeSurface(img2);
			}
	else if (ext=="png") {
			SDL_Surface* img = SDL_CreateRGBSurfaceFrom(pixels.get(), w, h, 24, w*3, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
			auto img2 = flip_vertical(img);
			IMG_SavePNG(img2, fname.c_str());
			SDL_FreeSurface(img);
			SDL_FreeSurface(img2);
			}

	}


void CuboGame::ScreenShot(void) {

	std::string ext="jpg";

	char cFileName[64];
	std::string fname;
	int nShot=1;
	while (nShot < 128) {
			std::string form="screenshot%03d."+ext;
			sprintf(cFileName,form.c_str(),nShot);
			fname=cFileName;
			auto fw = g_BaseFileSystem()->GetFileForWriting("/user/"+fname);
			if (!fw) { return; }
			if (fw->WillOverwrite()) {
					++nShot;
					if (nShot > 127) {
							coutlog("Screenshot limit of 128 reached! Delete some in your Cubosphere User dir",2);
							return;
							}
					continue;
					}
			fname=fw->GetHDDName();
			break;



			}
	SaveFramePic(fname);

	}

void CuboGame::JoyAxisHandle(int joys,int axis,float val,float pval) {
	if (CuboConsole::GetInstance()->IsActive()) { return ; }

	if (MenuActive) {
			menu.JoyAxisChange(joys,axis,val,pval);
			}
	if (GameActive) {
			lvl.JoyAxisChange(joys,axis,val,pval);
			}
	}

int CuboGame::StartLevel(std::string lname,int normal_user_edit) {

	if (normal_user_edit!=2 || lname!="") {
			auto finfo=GetFileName(lname,normal_user_edit!=0 ? FILE_USERLEVEL : FILE_LEVEL,".ldef");
			if (!finfo) {std::string uls=(normal_user_edit!=0 ? "Userlevel" : "Level"); coutlog("Cannot find "+uls+": "+lname,2); return 0 ;}
			}

	if (normal_user_edit==1) {
			g_Vars()->SetVar("StartUserLevel",lname);
			g_Vars()->SetVar("StartLevel","");
			g_Vars()->SetVar("Editor_LoadLevel","");
			g_Vars()->SetVar("EditorMode",0);
			menu.LoadDef("startgame");
			}
	else if (normal_user_edit==0) {
			g_Vars()->SetVar("StartUserLevel","");
			g_Vars()->SetVar("StartLevel",lname);
			g_Vars()->SetVar("Editor_LoadLevel","");
			g_Vars()->SetVar("EditorMode",0);
			menu.LoadDef("startgame");
			}
	else if (normal_user_edit==2) {
			g_Vars()->SetVar("StartUserLevel","");
			g_Vars()->SetVar("Editor_LoadLevel",lname);
			g_Vars()->SetVar("StartLevel","");
			g_Vars()->SetVar("EditorMode",1);
			menu.LoadDef("editorstart");

			}

	GameActive=0;
	MenuActive=1;
	menu.PostThink();
	return 1;
	}


void CuboGame::KeyHandle(int ident,int down,int toggle) {
	if (ident==-1) { exit(0); }
	if ((ident==CuboConsole::GetInstance()->GetToggleKey() ) && down && toggle) {
			CuboConsole::GetInstance()->Toggle();
			return;
			}

	if (CuboConsole::GetInstance()->IsActive()) {
			//Send key to console
			CuboConsole::GetInstance()->KeyHandle(ident,down,toggle);
			return;
			}

	if (CuboConsole::GetInstance()->CheckBindKey(ident,down,toggle)) {return;}

	if (MenuActive) {
			menu.SendKey(ident,down,toggle);
			}
	if (GameActive) {
			lvl.SendKey(ident,down,toggle);
			}
	}

void CuboGame::TextInputHandle(std::string& inp) {
	if (MenuActive) {
			menu.SendTextInput(inp);
			}
	if (GameActive) {
			lvl.SendTextInput(inp);
			}
	}

void CuboGame::DiscreteJoyHandle(int joy,int button,int dir,int down,int toggle) {
	if (CuboConsole::GetInstance()->IsActive()) { return; }

	if (MenuActive) {
			menu.SendJoyButton(joy,button,dir,down,toggle);
			}
	if (GameActive) {
			lvl.SendJoyButton(joy,button,dir,down,toggle);
			}
	}


int CuboGame::AddActor(std::string aname) {
	int res=move.size();
	move.resize(move.size()+1);
	move[res]=new TCuboMovement();
	move[res]->Init(&lvl);
	move[res]->SetType(res,aname);
	move[res]->SetID(res);
	return res;
	}

void CuboGame::DeleteActor(int index) {
	if ((index<0) || (index>=(int)(move.size()))) { return; }
	if (move[index]) {delete move[index]; move[index]=NULL;}
	move.erase(move.begin()+index); //CAREFUL! SHIFTS THE ACTORS INDICES
	for (unsigned int i=0; i<move.size(); i++) { move[i]->SetID(i); }
	}


int CuboGame::AddEnemy(std::string aname) {
	int res=move.size();
	move.resize(move.size()+1);
	move[res]=new CuboEnemy();
	move[res]->Init(&lvl);
	move[res]->SetType(res,aname);
	return res;
	}

void CuboGame::Clear() {
	g_SpriteEnvs()->Clear();
	for (unsigned int i=0; i<move.size(); i++) if (move[i]) {delete move[i]; move[i]=NULL;}
	move.clear();
	basis.clear();
//move.resize(0);
	g_PathGraphs()->Clear();
	lvl.clear();
	}

int CuboGame::AddBasis() {
	int res=basis.size();
	basis.resize(res+1);
	return res;
	}


int CuboGame::Init() {
	Game::Init();
	freecam=0;
	Vector2d wh;
	maxphyselapsed=1000; //Means only one phys calc
	FlushOrFinishBeforeSwap=0;
	StartBootScript("boot.lua");

	//menu.LoadDef("init");
	// if (CurrentMod()!="") menu.PostThink(); //To invoke another Call of Loading the "init.mdef"
	MenuActive=1;
	GameActive=0;
	NewMenuActive=1;
	NewGameActive=0;
	RenderPassID=0;
	keyboard.Init();
	keyboard.SetHandler(keyhandle);
	keyboard.SetTextHandler(texthandle);

	//joysticks.SetAxisHandler(joyaxishandle);
	joysticks.SetDiscreteHandler(joyhandle);


	player.push_back(new CuboPlayer(0));

	cam.goUp(2*CUBO_SCALE);
	Vector2d nearfar(2.0,6000);
	cam.setNearFar(nearfar);
	font.Init();
	return 0;
	}


void CuboGame::LoadSky(std::string name) {
// sky.LoadTextures(name,&textures);
	sky.LoadSkybox(name);
	}


void CuboGame::Think() {

	//if (mouse.GetButton(0).pressed) move[0].Jump();


	/*   T2dVector ppos;
	   ppos=mouse.getRelativeMotion();
	   cam.rotateSide(ppos.v*0.002);
	   cam.rotateY(-ppos.u*0.002);
	*/
	g_Sounds()->Think(GetElapsed());

	if (GameActive) {

#ifdef PARALLELIZE
//cout << "Game loop with " << omp_get_num_threads() << " of " << omp_get_max_threads() << " threads" << endl;
			omp_set_num_threads(omp_get_max_threads());

#endif
			int numcalcs=1;
			double oldelapsed=elapsed;
			double interpolelapsed=elapsed;

			if (GetElapsed()>maxphyselapsed) {
					numcalcs=(int)ceil(GetElapsed()/maxphyselapsed);

					interpolelapsed=elapsed/(float)numcalcs;
					}

#ifdef PARALLELIZE
			#pragma omp parallel for
#endif
			for (unsigned int i=0; i<move.size(); i++) {

					move[i]->Think();
					}

			elapsed=interpolelapsed;
			for (int calcstep=1; calcstep<=numcalcs; calcstep++) {
#ifdef PARALLELIZE
					#pragma omp parallel for
#endif
					for (unsigned int i=0; i<move.size(); i++) {
							//move[i].Think();
							move[i]->InterpolateMove(lvl.Elapsed());
							}


					//  cout << "lvl activeis " << lvl.GetCollisionChecksActive() << endl;
					lvl.CheckCollides();
					if (lvl.GetCollisionChecksActive()) {
							for (unsigned int a=0; a<g_Game()->NumActors(); a++) {
									if (!(move[a]->IsPlayer())) { continue; }
									for (unsigned int e=0; e<g_Game()->NumActors(); e++)
										if (!(move[e]->IsPlayer())) { move[a]->CheckEnemyCollision(move[e]); }
										else if (a!=e) { move[a]->CheckPlayerCollision(move[e]); }
									}
							}
					}
			elapsed=oldelapsed;

#ifdef PARALLELIZE
			#pragma omp parallel for
#endif

			for (unsigned int i=0; i<move.size(); i++) {
					move[i]->FinalizeMove();

					}



			lvl.Think();

#ifdef PARALLELIZE
			#pragma omp parallel for
#endif

			for (unsigned int i=0; i<move.size(); i++) {
					move[i]->PostLevelThink();

					}


			if (!(player[0]->InCameraPan())) {
					int CamPlayer=cam.GetPlayer();
					if (CamPlayer>=0) { move[CamPlayer]->SetCamPos(&cam); }
					}
			else {
					player[0]->SetCameraPos(elapsed,&cam);
					}
			cam.Invalidate(); //This will recalc everythink

			g_SpriteEnvs()->Think(elapsed);

			}
	if (MenuActive) { menu.Think(); }
	}

void CuboGame::PreRender(int wo,int ho) {

	//const SDL_VideoInfo* vidinfo = SDL_GetVideoInfo();
	int w,h;
	SDL_GetWindowSize(sdlWindow, &w, &h);

	Vector2d widthheight;

	if (wo<=0) { wo=w; }
	if (ho<=0) { ho=h; }


	widthheight.uv(wo,ho);
	cam.setScreenWidthHeight(widthheight);
	if (GameActive) { lvl.FrameRenderStart(); }
	cam.think();

	}

void CuboGame::AfterRenderLevel() {

	cam.postthink();
	if (GameActive) { lvl.FrameRenderEnd(); }
	}

void CuboGame::Render() {

	if (!glReady) { return; }


//AntiAliasing=1;

	if ((!g_PostEffect()) || (!GameActive)) {
			if (!AntiAliasing) {
					PreRender();
					glClearColor(0,0,0,1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					RenderPass();
					AfterRenderLevel();
					}
			else {
//      glClearAccum(0, 0, 0, 0);
					//    glClear(GL_ACCUM_BUFFER_BIT);
					glEnable(GL_MULTISAMPLE);
					PreRender();
					glClearColor(0,0,0,1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					RenderPass();
					AfterRenderLevel();
					glDisable(GL_MULTISAMPLE);
					//  glAccum(GL_ACCUM, 1.0);

					//glAccum(GL_RETURN, 0.5);
					}
			}
	else {
			PreRender();
			g_PostEffect()->CallRender();
			AfterRenderLevel();
			}





	if (GameActive) { lvl.DrawHUD(); }
	if (MenuActive) { menu.Render(); }

	CuboConsole::GetInstance()->Render();

	if (FlushOrFinishBeforeSwap==1) {
			glFlush();
			}
	else if (FlushOrFinishBeforeSwap==2) {
			glFinish();
			}

	SDL_GL_SwapWindow(sdlWindow);


	if (GameActive || NewGameActive) { lvl.PostThink(); }
	if (MenuActive || NewMenuActive) { menu.PostThink(); }

	GameActive=NewGameActive;
	MenuActive=NewMenuActive;
	}

void CuboGame::RenderPass() {
	if (GameActive) {
			//sky.Render(cam.getPos());
//cout << "RENDERPASS" << endl;
			glColor4f(1,1,1,1);

			sky.Render();

			lvl.Render(&cam);
			for (unsigned int i=0; i<move.size(); i++) { move[i]->Render(); }


			lvl.SortDistRenderList();
			lvl.DistRender(&cam);
			g_SpriteEnvs()->RenderAfterLevel();
			InvalidateMaterial();
			// lvl.FrameRenderEnd();
			RenderPassID++;
			}
	}

void CuboGame::SpecialRenderPass(std::string nam,int defrender) {
	if (GameActive) {
			sky.SpecialRender(nam,defrender);
			lvl.SpecialRender(&cam,nam,defrender);
			for (unsigned int i=0; i<move.size(); i++) { move[i]->SpecialRender(nam,defrender); }
			lvl.SortDistRenderList();
			lvl.DistRender(&cam); //You have to keep in mind, that the special DistRender has to be set up in the individual lua script with SetVar, GetVar etc
			}
	}

void CuboGame::Reload(std::vector<std::string> & extratoks) {
	int all=0; // Implicits => All
	int lua=0; // Implicits =>
	int textures=0; // Implicits =>
	int models=0; // Implicits =>
	int sounds=0; // Implicits =>
	int level=0; // Implicits =>
	int actordefs=0;
	int mdldefs=0;
	int menudefs=0;
	int tdefs=0, bdefs=0, idefs=0;
	int skydef=0;
	int language=0;

	if (!extratoks.size())  {
			FreeMedia();
			if (GameActive) { lvl.Reload(); }
			if (MenuActive) { menu.Reload(); } return;
			}


	for (unsigned int i=0; i<extratoks.size(); i++) {
			if (extratoks[i]=="all") { all=1; }
			else if (extratoks[i]=="lua") { lua=1; }

			else if (extratoks[i]=="actordefs") { actordefs=1; }
			else if (extratoks[i]=="blockdefs") { bdefs=1; }
			else if (extratoks[i]=="enemydefs") { actordefs=1; }
			else if (extratoks[i]=="itemdefs") { idefs=1; }
			else if (extratoks[i]=="mdldefs") { mdldefs=1; }
			else if (extratoks[i]=="menudefs") { menudefs=1; }
			else if (extratoks[i]=="textures") { textures=1; }
			else if (extratoks[i]=="mdls") { models=1; }
			else if (extratoks[i]=="sounds") { sounds=1; }
			else if (extratoks[i]=="texdefs") { tdefs=1; }
			else if (extratoks[i]=="level") { level=1; }
			else if (extratoks[i]=="skydef") { skydef=1; }
			else if (extratoks[i]=="language") { language=1; }
			else { coutlog("switch "+extratoks[i]+" not defined",2); }
			}



	if (textures || all) { this->textures.Reload(); }
	if (sounds || all) { g_Sounds()->Reload(); }
	if (models || all) { mdls.Reload(); }

	if (actordefs || all || lua) { adefs.Reload(); }
	if (mdldefs || all || lua) { mdefs.Reload(); }

	if (tdefs || all || lua) { lvl.GetTexDefs()->Reload(); }
	if (bdefs || all || lua) { lvl.GetBlockDefs()->Reload(); }
	if (idefs || all || lua) { lvl.GetItemDefs()->Reload(); }

	if (skydef || all || lua) { sky.Reload(); }
	if (language || all ) { ReloadLanguage(); }

	if (menudefs || all || lua) { menu.Reload(); }

	if (level || all ) { lvl.Reload(); }

	}

void CuboGame::FreeMedia() {
	// glReady=0;
	mdefs.clear();
	mdls.Clear();
	adefs.clear();
	g_SpriteEnvs()->Clear();
	g_ParticleDefs()->clear();
	lvl.clearAll();
	textures.clear();
///What about the sounds?
	font.ClearCache();
	GetShaders()->clear();
	if (g_PostEffect()) { g_PostEffect()->UnPrecache(); }
	}

bool CuboGame::InitGL(int w,int h,int hw,int fs) {
	FreeMedia();
	return Game::InitGL(w,h,hw,fs);
	}

bool CuboGame::UpdateWindow(int w, int h, int hw, int fs) {
	FreeMedia(); // FIXME: is required?
	return Game::UpdateWindow(w,h,hw,fs);
	}


CuboGame::~CuboGame() {
	for (unsigned int i=0; i<player.size(); i++) if (player[i]) {delete player[i]; player[i]=NULL;}
	}

void CuboBasis::InvertMatrix() {
	Matrix3d sub;
	matrix.getSubMatrix(&sub);
	sub=sub.inverse();
	matrix.setSubMatrix(sub);
	}



//////////////////////////LUA IMPLEMENTATION//////////////////////////


int BASIS_New(lua_State *state) {
	int res=g_Game()->AddBasis();
	LUA_SET_NUMBER(state, res);
	return 1;
	}

int BASIS_SetAxisRotate(lua_State *state) {
	Vector3d pos=Vector3FromStack(state);
	float angle=LUA_GET_DOUBLE(state);
	Vector3d axis=Vector3FromStack(state);

	int index=LUA_GET_INT(state);


	CuboBasis *b=g_Game()->GetBasis(index);
	b->setPos(pos);
	b->SetBasisAxisRotate(axis,angle);
	return 0;
	}

int BASIS_Set(lua_State *state) {
	Vector3d pos=Vector3FromStack(state);
	Vector3d dir=Vector3FromStack(state);
	Vector3d up=Vector3FromStack(state);
	Vector3d side=Vector3FromStack(state);

	int index=LUA_GET_INT(state);


	CuboBasis *b=g_Game()->GetBasis(index);
	b->setPos(pos);
	b->SetBasis(side,up,dir);
	return 0;
	}

int BASIS_Invert(lua_State *state) {

	int index=LUA_GET_INT(state);


	CuboBasis *b=g_Game()->GetBasis(index);
	b->InvertMatrix();
	return 0;
	}

int BASIS_SetScale(lua_State *state) {
	Vector3d s=Vector3FromStack(state);
	int index=LUA_GET_INT(state);

	g_Game()->GetBasis(index)->setScale(s);

	return 0;
	}


int BASIS_SetPos(lua_State *state) {
	Vector3d s=Vector3FromStack(state);
	int index=LUA_GET_INT(state);

	g_Game()->GetBasis(index)->setPos(s);

	return 0;
	}

int BASIS_GetSide(lua_State *state) {
	int index=LUA_GET_INT(state);
	Vector3d v=g_Game()->GetBasis(index)->getSide();
	LUA_SET_VECTOR3(state, v);
	return 1;
	}

int BASIS_GetUp(lua_State *state) {
	int index=LUA_GET_INT(state);
	Vector3d v=g_Game()->GetBasis(index)->getUp();
	LUA_SET_VECTOR3(state, v);
	return 1;
	}

int BASIS_GetDir(lua_State *state) {
	int index=LUA_GET_INT(state);
	Vector3d v=g_Game()->GetBasis(index)->getDir();
	LUA_SET_VECTOR3(state, v);
	return 1;
	}

int BASIS_AxisRotate(lua_State *state) {
	float angle=LUA_GET_DOUBLE(state);
	Vector3d s=Vector3FromStack(state);
	int index=LUA_GET_INT(state);

	g_Game()->GetBasis(index)->rotateV(angle,s);

	return 0;
	}

int BASIS_Push(lua_State *state) {
	int index=LUA_GET_INT(state);
	g_Game()->GetBasis(index)->think();
// g_Game()->GetBasis(index)->PushMult();
	return 0;
	}

int BASIS_Pop(lua_State *state) {
	int index=LUA_GET_INT(state);
	g_Game()->GetBasis(index)->postthink();
	return 0;
	}

int BASIS_ChaseCam(lua_State *state) {
	int actor=LUA_GET_INT(state);
	int index=LUA_GET_INT(state);
	g_Game()->GetActorMovement(actor)->SetCamPos (g_Game()->GetBasis(index));
	return 0;
	}



void LUA_BASIS_RegisterLib() {
	g_CuboLib()->AddFunc("BASIS_New",BASIS_New);
	g_CuboLib()->AddFunc("BASIS_Invert",BASIS_Invert);
	g_CuboLib()->AddFunc("BASIS_SetScale",BASIS_SetScale);
	g_CuboLib()->AddFunc("BASIS_Push",BASIS_Push);
	g_CuboLib()->AddFunc("BASIS_Pop",BASIS_Pop);
	g_CuboLib()->AddFunc("BASIS_SetPos",BASIS_SetPos);
	g_CuboLib()->AddFunc("BASIS_AxisRotate",BASIS_AxisRotate);
	g_CuboLib()->AddFunc("BASIS_SetAxisRotate",BASIS_SetAxisRotate);

	g_CuboLib()->AddFunc("BASIS_ChaseCam",BASIS_ChaseCam); ///Gets the position from a ACTOR-Chase-Cam
	g_CuboLib()->AddFunc("BASIS_Set",BASIS_Set);
	g_CuboLib()->AddFunc("BASIS_GetSide",BASIS_GetSide);
	g_CuboLib()->AddFunc("BASIS_GetUp",BASIS_GetUp);
	g_CuboLib()->AddFunc("BASIS_SetDir",BASIS_GetDir);
	}




int GAME_HandleEvents(lua_State *state) {
	g_Game()->HandleEvents();
	return 0;
	}

int GAME_Think(lua_State *state) {
	g_Game()->Think();
	return 0;
	}


int GAME_HandleInput(lua_State *state) {
	g_Game()->HandleInput();
	return 0;
	}

int GAME_GetRenderPassID(lua_State *state) {
	LUA_SET_NUMBER(state, g_Game()->GetRenderPassID());
	return 1;
	}

int GAME_Render(lua_State *state) {
	g_Game()->Render();
	return 0;
	}

int GAME_SetGameLoopSource(lua_State *state) {
	std::string s;
	s=LUA_GET_STRING(state);
	g_Game()->SetGameLoopSource(s);
	return 0;
	}

int GAME_SetFlushOrFinishBeforeSwap(lua_State *state) {
	int i=LUA_GET_INT(state);
	g_Game()->SetFlushOrFinishBeforeSwap(i);
	return 0;
	}

int GAME_GetFlushOrFinishBeforeSwap(lua_State *state) {
	int i= g_Game()->GetFlushOrFinishBeforeSwap();
	LUA_SET_NUMBER(state, i);
	return 1;
	}

void LUA_GAME_RegisterLib() {
	g_CuboLib()->AddFunc("GAME_SetGameLoopSource",GAME_SetGameLoopSource);
	g_CuboLib()->AddFunc("GAME_HandleEvents",GAME_HandleEvents);
	g_CuboLib()->AddFunc("GAME_HandleInput",GAME_HandleInput);
	g_CuboLib()->AddFunc("GAME_Think",GAME_Think);
	g_CuboLib()->AddFunc("GAME_Render",GAME_Render);
	g_CuboLib()->AddFunc("GAME_GetRenderPassID",GAME_GetRenderPassID);
	g_CuboLib()->AddFunc("GAME_SetFlushOrFinishBeforeSwap",GAME_SetFlushOrFinishBeforeSwap);
	g_CuboLib()->AddFunc("GAME_GetFlushOrFinishBeforeSwap",GAME_GetFlushOrFinishBeforeSwap);


	}
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
