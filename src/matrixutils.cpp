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

#include "matrixutils.hpp"

#include <GL/glew.h>
#include <SDL.h>

#include "luautils.hpp"


GlutMatrix4d::GlutMatrix4d(): mode(GL_MODELVIEW) {}
GlutMatrix4d::GlutMatrix4d(float scal): Matrix4d(scal), mode(GL_MODELVIEW) {}


void GlutMatrix4d::glPushMult() {
//glMatrixMode(mode);
	glPushMatrix();
	glMultMatrixf((GLfloat*)(&m));
	}

void GlutMatrix4d::glMult() {
//glMatrixMode(mode);
	glMultMatrixf((GLfloat*)(&m));
	}


void GlutMatrix4d::glPop() {
//glMatrixMode(mode);
	glPopMatrix();
	}

void GlutMatrix4d::glPushLoad() {
//glMatrixMode(mode);
	glPushMatrix();
	glLoadMatrixf((GLfloat*)(&m));
	}

void GlutMatrix4d::glLoad() {
//glMatrixMode(mode);
	glLoadMatrixf((GLfloat*)(&m));
	}

const Vector3d GlutMatrix4d::getPos() { //Returns the last col
	Vector3d res(m[12],m[13],m[14]);
	return res;
	}

void GlutMatrix4d::setPos(const Vector3d& v) { //sets the last c
	setCol(3,v);
	}

void GlutMatrix4d::assign(Matrix4d* other) {
	float* mom=other->getValueMem();
	for (int i=0; i<16; i++) {
			Matrix4d::m[i]=mom[i];
			}
	}


/////////////LUA-IMPLEMENT///////////////////


int MATRIX_Push(lua_State *state) {
	glPushMatrix();
	return 0;
	}

int MATRIX_Pop(lua_State *state) {
	glPopMatrix();
	return 0;
	}
int MATRIX_Translate(lua_State *state) {
	Vector3d v=Vector3FromStack(state);
	glTranslatef(v.x,v.y,v.z);
	return 0;
	}

int MATRIX_Identity(lua_State *state) {
	glLoadIdentity();
	return 0;
	}

int MATRIX_AxisRotate(lua_State *state) {
	float angle=LUA_GET_DOUBLE(state);
	Vector3d v=Vector3FromStack(state);
	glRotatef(angle,v.x,v.y,v.z);
	return 0;
	}

int MATRIX_ScaleUniform(lua_State *state) {
	float f=LUA_GET_DOUBLE(state);
	glScalef(f,f,f);
	return 0;
	}

int MATRIX_Scale(lua_State *state) {
	Vector3d v=Vector3FromStack(state);
	glScalef(v.x,v.y,v.z);
	return 0;
	}

int MATRIX_MultBase(lua_State *state) {
	Vector3d p=Vector3FromStack(state);
	Vector3d d=Vector3FromStack(state);
	Vector3d u=Vector3FromStack(state);
	Vector3d s=Vector3FromStack(state);

	Matrix3d m(s,u,d);

	GlutMatrix4d m4(1);
	m4.setSubMatrix(m);
	m4.setPos(p);

	m4.glMult();


	return 0;
	}



void LUA_MATRIX_RegisterLib() {
	g_CuboLib()->AddFunc("MATRIX_Push",MATRIX_Push);
	g_CuboLib()->AddFunc("MATRIX_Pop",MATRIX_Pop);
	g_CuboLib()->AddFunc("MATRIX_Translate",MATRIX_Translate);
	g_CuboLib()->AddFunc("MATRIX_MultBase",MATRIX_MultBase);
	g_CuboLib()->AddFunc("MATRIX_ScaleUniform",MATRIX_ScaleUniform);
	g_CuboLib()->AddFunc("MATRIX_AxisRotate",MATRIX_AxisRotate);
	g_CuboLib()->AddFunc("MATRIX_Identity",MATRIX_Identity);
	g_CuboLib()->AddFunc("MATRIX_Scale",MATRIX_Scale);
	}
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
