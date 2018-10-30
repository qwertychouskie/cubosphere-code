/**
Copyright (C) 2010 Chriddo

This program is free software;
you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, see <http://www.gnu.org/licenses/>.
**/


#ifndef EVENT_H_G
#define EVENT_H_G

#ifdef WIN32

#ifdef MINGW_CROSS_COMPILE
#include <GL/glew.h>
#else
#include <GL\glew.h>
#endif
// #include <GL\gl.h>
#include <windows.h>
#include <SDL.h>


#else

#include <GL/glew.h>
#include <SDL/SDL.h>

#endif

#include "mouse.hpp"
#include "keyboard.hpp"
#include "joystick.hpp"

class TEventManager
	{
	protected:
		int windowclose;
		TKeyboard *keyb;
		TMouse *mouse;
		TJoystickServer *joy;
	public:
		TEventManager() : windowclose(0) {};
		void HandleEvents();
		void Close() {windowclose=1;}
		int Closed() {return windowclose;}
		void SetMouse(TMouse *m) {mouse=m;}
		void SetKeyboard(TKeyboard *kb) {keyb=kb;}
		void SetJoystick(TJoystickServer *ys) {joy=ys;}
	};


#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
