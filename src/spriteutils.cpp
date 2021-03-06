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

#include "spriteutils.hpp"
#include "cuboboxes.hpp"
#include "globals.hpp"
#include "game.hpp"
#include <iostream>
#include <algorithm>

Vector3d v_camside,v_camdir,v_camup;
Vector3d v_corner1,v_corner2;


SpriteEmitter * LUA_GET_EMITTER_FUNC(lua_State * state) {
	unsigned int id=(unsigned int)LUA_GET_DOUBLE(state);
	unsigned int emi=id & 0xFFFF;
	unsigned int env=(id >> 16) & 0xFFFF;
	return g_SpriteEnvs()->GetEnv(env)->GetEmitter(emi);
	}

#define LUA_GET_EMITTER LUA_GET_EMITTER_FUNC(state)

SpriteDef * LUA_GET_SPRITETYPE_FUNC(lua_State * state) {
	unsigned int id=(unsigned int)LUA_GET_DOUBLE(state);
	unsigned int emi=id & 0xFFFF;
	unsigned int env=(id >> 16) & 0xFFFF;

	return g_ParticleDefs()->GetDefPtr(env)->GetSpriteDefs()[emi];
	}

#define LUA_GET_SPRITETYPE LUA_GET_SPRITETYPE_FUNC(state)

int SpriteEnvironment::AddEmitter(int defindex) {
	int index=-1;
	for (unsigned int i=0; i<emitters.size(); i++) if (emitters[i]->Died()) {index=i; break;}
	if (index>=0) { delete emitters[index];}
	else { index=emitters.size(); emitters.push_back(NULL);}
	emitters[index]=new SpriteEmitter();
	emitters[index]->SetID( (id << 16) | ((index) & 0xFFFF));
	emitters[index]->Init(defindex,this);
	died=0;
	return emitters[index]->GetID();
	}

void SpriteEnvironment::AttachOnActor(int aid) {
	attachedid=aid;
	attachtype=SPRITE_ENV_ATTACHED_TO_ACTOR;
	RefreshBasis();
	}


void SpriteEnvironment::AttachOnSide(int sid) {
	attachedid=sid;
	attachtype=SPRITE_ENV_ATTACHED_TO_SIDE;
	RefreshBasis();
	}

void SpriteEnvironment::Clear() {
	for (unsigned int i=0; i<emitters.size(); i++) if (emitters[i])   { delete emitters[i]; emitters[i]=NULL;}
	emitters.clear();
	}

SpriteEnvironment::~SpriteEnvironment() {
	Clear();
	}

void SpriteEnvironment::RefreshBasis() {
	if (attachtype==SPRITE_ENV_ATTACHED_TO_WORLD) { return; }
	else if (attachtype==SPRITE_ENV_ATTACHED_TO_SIDE) {
			CuboBlockSide *bs;
			bs=g_Game()->GetLevel()->GetBlockSide(attachedid);
			if (!bs) { return; }
			vpos=bs->GetMidpoint();
			vup=bs->GetNormal();
			vdir=bs->GetTangent();
			vside=vdir.cross(vup);
			}
	else if (attachtype==SPRITE_ENV_ATTACHED_TO_ACTOR) {
			TCuboMovement *am;
			am=g_Game()->GetActorMovement(attachedid);
			if (!am) { return; }
			vpos=am->GetPos();
			vdir=am->GetDir();
			vside=am->GetSide();
			vup=am->GetUp();
			}
	base.setCol(0,vside);
	base.setCol(1,vup);
	base.setCol(2,vdir);
	}


void SpriteEnvironment::Think(double elap) {
	elapsed=elap;
	RefreshBasis();
	int remaining=0;
	for (unsigned int i=0; i<emitters.size(); i++) {
			if (!(emitters[i]->Died())) {
					remaining++;
					double mti=emitters[i]->GetMaxTimeInterval();
					if ( elapsed<mti || mti<=0) {
							emitters[i]->Think();
							}
					else {
							int numcalcs=(int)ceil(elap/mti);
							if (numcalcs>64) { numcalcs=64; } //There must be an upper bound!
							elapsed=elap/(double)numcalcs;
							for (int c=0; c<numcalcs; c++) { emitters[i]->Think(); }
							elapsed=elap;
							}

					}
			}
	if (!remaining) { died=1; }
	}





//////////////7

double SpriteEmitter::GetCullInformation() {

	if (!(g_Game()->GetCam()->SphereInFrustum(cull_center,cull_radius))) {
			//cout << "Emitter culled" << endl;
			return -1;
			}
	Vector3d d=g_Game()->GetCam()->getPos()-cull_center;
	cam_dist=d.sqrlength();
	return cam_dist;
	}

void SpriteEmitter::RenderAfterLevel() {
	if (colmultiply.w<=0  || (!sprites.size())) { return; }// Invisble


	std::vector<SpriteDef*> & sdefs=g_ParticleDefs()->GetDefPtr(defindex)->GetSpriteDefs();

	int firstsprite=-1;
	for (unsigned int i=0; i<sprites.size(); i++) {
			if (sprites[i].timeleft>0) { firstsprite=i; break;}
			}
	if (firstsprite==-1) { return; }

	int ctext=sdefs[sprites[firstsprite].typ]->textureind;
	g_Game()->GetTextures()->activate(ctext,0);

	glBegin(GL_QUADS);
//glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(v_camdir.x,v_camdir.y,v_camdir.z);
	for (unsigned int i=0; i<sprites.size(); i++) {
			if (sprites[i].timeleft>0) {
					if (sdefs[sprites[i].typ]->textureind!=ctext) {
							glEnd();
							ctext=sdefs[sprites[i].typ]->textureind;
							g_Game()->GetTextures()->activate(ctext,0);
							glBegin(GL_QUADS);
//     glBegin(GL_TRIANGLE_STRIP);
							}

					sprites[i].col.xyzw(sprites[i].col.x*colmultiply.x,sprites[i].col.y*colmultiply.y,sprites[i].col.z*colmultiply.z,sprites[i].col.w*colmultiply.w);
					sprites[i].scale*=scalefactor;
					sdefs[sprites[i].typ]->Render(sprites[i]);
					}
			}
	glEnd();
	}

Vector3d SpriteEmitter::GetPos(int rel) {
	if (rel==relpos) {return pos;}
	else {
			if (relpos) {  //Relative to world
					Vector3d r=env->base*pos;
					r=r+env->vpos;
					return r;
					}
			else { //Absolute to relative
					Vector3d r=pos-env->vpos;
					Matrix3d ib=env->base.inverse();
					return ib*r;
					}

			}
	}

void SpriteEmitter::SetPos(Vector3d p,int rel) {
	relpos=rel;
	pos=p;
	}

void SpriteEmitter::Init(int defind,SpriteEnvironment * envi) {
	varholder.clear();
	died=0;
	defindex=defind;
	env=envi;
	pos.xyz(0,0,0);
	relpos=1;
	g_ParticleDefs()->GetDefPtr(defindex)->Call_EmitterConstructor(id);
	}

void SpriteEmitter::Think() {
///TODO: Advance physics

	Vector3d adv=grav*(env->elapsed*gravfactor);
	vel=vel+adv;
	adv=vel*env->elapsed;
	pos=pos+adv;

	Vector3d minpos(100000000000.0,1000000000000.0,1000000000000.0);
	Vector3d maxpos=-minpos;
	double maxscale=0;

	std::vector<SpriteDef*> & sdefs=g_ParticleDefs()->GetDefPtr(defindex)->GetSpriteDefs();
	int counter=0;
	for (unsigned int i=0; i<sprites.size(); i++) if (sprites[i].timeleft>0) {



				double ip=1.0-sprites[i].timeleft/sprites[i].lifetime;
				sprites[i].scale=sdefs[sprites[i].typ]->scalef->GetValue(ip);
				if (sprites[i].scale>maxscale) { maxscale=sprites[i].scale; }
				sprites[i].col=sdefs[sprites[i].typ]->GetCol(ip,sprites[i].rgbphase);
				sprites[i].grav=sdefs[sprites[i].typ]->gravf->GetValue(ip);

				Vector3d adv=grav*(sprites[i].grav*env->elapsed);
				sprites[i].vel=sprites[i].vel+adv;
				adv=sprites[i].vel*env->elapsed;
				sprites[i].pos=sprites[i].pos+adv;
				sprites[i].rot+=sprites[i].rotspeed*env->elapsed;
				minpos.Minimize(sprites[i].pos); maxpos.Maximize(sprites[i].pos);
				sprites[i].timeleft-=env->elapsed;
				if (sprites[i].timeleft>0) { counter++; }
				}
	activesprites=counter;
	ParticleDef *pdef=g_ParticleDefs()->GetDefPtr(defindex);
	pdef->Call_EmitterThink(id,env->elapsed);

//We can compute the bounding area
	cull_center=maxpos+minpos; cull_center=cull_center*0.5;
	Vector3d diff=maxpos-minpos; diff=diff*0.5;
	cull_radius=diff.MaxValue();
	cull_radius+=maxscale*scalefactor*sqrt(2.0); //Maximal offs
	}

Vector3d SpriteEmitter::RelativePosToWorldPos(Vector3d rp) {
	Vector3d res(0,0,0);
	Vector3d offs;
	offs=env->vside;
	res=res+offs*rp.x;
	offs=env->vup;
	res=res+offs*rp.y;
	offs=env->vdir;
	res=res+offs*rp.z;
	res=res+GetPos(0);
	return res;
	}

void SpriteEmitter::SpawnSprite(int typ,double theta,double phi,double vel) {
	int p=-1;
//Ensure the sprites to be type sorted


	for (unsigned int i=0; i<sprites.size(); i++) {
			if (sprites[i].timeleft<0) {p=i; break;}
			}
	if (p==-1) { p=sprites.size(); sprites.push_back(Sprite()); }

	SpriteDef * sd=g_ParticleDefs()->GetDefPtr(defindex)->GetSpriteDefs()[typ & 0xFFFF];
	sprites[p].typ=typ & 0xFFFF;
	sprites[p].lifetime=sd->lifetime; sprites[p].timeleft=sprites[p].lifetime;
	sprites[p].pos=GetPos(0);
//sprites[p].col=sd->GetCol(0,0);
	Vector3d d=env->vup*cos(theta);
	double v=sin(theta)*cos(phi);
	Vector3d offs=env->vside*v;
	d=d+offs;
	v=sin(theta)*sin(phi);
	offs=env->vdir*v;
	d=d+offs;
	sprites[p].vel=d*vel;
	sprites[p].scale=sd->scalef->GetValue(0);
	sprites[p].rotspeed=0; sprites[p].rot=0;
	sprites[p].rgbphase=0;
	lastsprite=p;
	}

/////////////////

void SpriteDef::Render(Sprite & s) {


	glColor4f(s.col.x,s.col.y,s.col.z,s.col.w);


	double scale=s.scale;

	if (s.rot==0) {
			double px=s.pos.x,py=s.pos.y,pz=s.pos.z;
			glTexCoord2f(0,0); glVertex3f(px+v_corner1.x*scale,py+v_corner1.y*scale,pz+v_corner1.z*scale);
			glTexCoord2f(1,0);  glVertex3f(px+v_corner2.x*scale,py+v_corner2.y*scale,pz+v_corner2.z*scale);
			glTexCoord2f(1,1); glVertex3f(px-v_corner1.x*scale,py-v_corner1.y*scale,pz-v_corner1.z*scale);
			glTexCoord2f(0,1); glVertex3f(px-v_corner2.x*scale,py-v_corner2.y*scale,pz-v_corner2.z*scale);

			}
	else {
			float si=sin(s.rot)*scale;
			float co=cos(s.rot)*scale;
			Vector3d c1=s.pos+(v_corner1*co+v_corner2*si);
			Vector3d c2=s.pos+(v_corner2*co-v_corner1*si);
			Vector3d c3=s.pos-(v_corner1*co+v_corner2*si);
			Vector3d c4=s.pos+(v_corner1*si-v_corner2*co);
			glTexCoord2f(0,0); glVertex3f(c1.x,c1.y,c1.z);
			glTexCoord2f(1,0);  glVertex3f(c2.x,c2.y,c2.z);
			glTexCoord2f(1,1); glVertex3f(c3.x,c3.y,c3.z);
			glTexCoord2f(0,1); glVertex3f(c4.x,c4.y,c4.z);


			}



	}


/////////////////

SpriteEnvironment* SpriteEnvironments::GetEnv(int i) {
	if (i<0  || i>=(int)(envs.size())) { return NULL; }
	return envs[i];
	}

int SpriteEnvironments::CreateEnv() {
	envs.push_back(new SpriteEnvironment());
	envs.back()->SetID(envs.size()-1);
	return envs.size()-1;
	}


void SpriteEnvironments::Clear() {
	for (unsigned int i=0; i<envs.size(); i++) if (envs[i]) {delete envs[i]; envs[i]=NULL;}
	envs.clear();
	}

SpriteEnvironments::~SpriteEnvironments() { Clear();}

void SpriteEnvironments::Think(double elapsed) {
	if (timescale<0.00001) { return; }
	elapsed*=timescale;
	for (unsigned int i=0; i<envs.size(); i++) {if (envs[i] && (!(envs[i]->Died()))) envs[i]->Think(elapsed);}
	}

bool SpriteEmitter::cmp_emitterdists(const SpriteEmitter *a, const SpriteEmitter *b) {
	return a->cam_dist > b->cam_dist;
	}

void SpriteEnvironments::RenderAfterLevel() {
	if (!envs.size()) { return; }

	//Get the visible emitters
	renderemitters.clear();
	for (unsigned int envi=0; envi<envs.size(); envi++) {
			for (unsigned int emi=0; emi<envs[envi]->emitters.size(); emi++) {
					double emidist=envs[envi]->emitters[emi]->GetCullInformation();
					if (emidist>0) { renderemitters.push_back(envs[envi]->emitters[emi]); }
					}
			}
	sort(renderemitters.begin(),renderemitters.end(),SpriteEmitter::cmp_emitterdists);

	glDisable(GL_LIGHTING);
	glDepthMask(false);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	v_camdir=g_Game()->GetCam()->getDir();
	v_camup=g_Game()->GetCam()->getUp();
	v_camside=g_Game()->GetCam()->getSide();

	v_corner1=v_camup+v_camside;
	v_corner2=v_camup-v_camside;

	for (unsigned int i=0; i<renderemitters.size(); i++) { renderemitters[i]->RenderAfterLevel(); }



	glColor4f(1,1,1,1);
	glDepthMask(true);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	}


SpriteEnvironments gSpriteEnvs;
SpriteEnvironments * g_SpriteEnvs() {return &gSpriteEnvs;}

//////////////////////////////////////////////////

ParticleDef::~ParticleDef() {
	for (unsigned int i=0; i<spritedefs.size(); i++) if (spritedefs[i]) { delete spritedefs[i]; spritedefs[i]=NULL;}
	spritedefs.resize(0);
	}

void ParticleDef::Call_EmitterThink(int eid,double elapsed) {
	lua.CallVAIfPresent("EmitterThink", {{eid, elapsed}});
	}

void ParticleDef::Call_EmitterConstructor(int eid) {
	lua.CallVAIfPresent("EmitterConstructor", {{eid}});
	}


int ParticleDef::NewSpriteType(int tind) {
	spritedefs.push_back(new SpriteDef());
	spritedefs.back()->textureind=tind;
	return ((myid << 16) | ((spritedefs.size()-1) & 0xFFFF));
	}



ParticleDefServer gParticleDefs;

ParticleDefServer * g_ParticleDefs() {return &gParticleDefs;}


////////////////LUA-IMPLEMENT////////////////////

LuaParticleLib gParticleLib;

LuaParticleLib * g_ParticleLib() {return &gParticleLib;}


int PARTICLE_CreateEnvOnActor(lua_State *state) {
	int act=LUA_GET_INT(state);
	int ei=gSpriteEnvs.CreateEnv();
	gSpriteEnvs.GetEnv(ei)->AttachOnActor(act);
	LUA_SET_NUMBER(state, ei);
	return 1;
	}


int PARTICLE_CreateEnvOnSide(lua_State *state) {
	int side=LUA_GET_INT(state);
	int ei=gSpriteEnvs.CreateEnv();
	gSpriteEnvs.GetEnv(ei)->AttachOnSide(side);
	LUA_SET_NUMBER(state, ei);
	return 1;
	}

int PARTICLE_LoadDef(lua_State *state) {
	std::string s=LUA_GET_STRING(state);
	int r=gParticleDefs.AddDef(s);
	LUA_SET_NUMBER(state, r);
	return 1;
	}

int PARTICLE_SetTimeScale(lua_State *state) {
	double s=LUA_GET_DOUBLE(state);
	gSpriteEnvs.SetTimeScale(s);
	return 0;
	}


int PARTICLE_AddEmitter(lua_State *state) {
	int envid=LUA_GET_INT(state);
	int emdef=LUA_GET_INT(state);
	unsigned int emid= gSpriteEnvs.GetEnv(envid)->AddEmitter(emdef);
	LUA_SET_NUMBER(state, emid);
	return 1;
	}

int EMITTER_NewSpriteType(lua_State *state) {
	int tind=LUA_GET_INT(state);
	int pdefi=LUA_GET_INT(state);
	int r=gParticleDefs.GetDefPtr(pdefi)->NewSpriteType(tind);
	LUA_SET_NUMBER(state, r);
	return 1;
	}

int EMITTER_SpawnSprite(lua_State *state) {
	double vel=LUA_GET_DOUBLE(state);
	double phi=LUA_GET_DOUBLE(state);
	double theta=LUA_GET_DOUBLE(state);
	int typ=LUA_GET_INT(state);
	SpriteEmitter * emitt=LUA_GET_EMITTER;
	emitt->SpawnSprite(typ,theta,phi,vel);
	return 0;
	}

int EMITTER_GetActiveSprites(lua_State *state) {
	SpriteEmitter *em=LUA_GET_EMITTER;
	int res=em->GetActiveSprites();
	LUA_SET_NUMBER(state, res);
	return 1;
	}


int EMITTER_SetVar(lua_State *state) {
	unsigned int id=(unsigned int)lua_tonumber(state,1); lua_remove(state,1);
	unsigned int emi=id & 0xFFFF;
	unsigned int env=(id >> 16) & 0xFFFF;
	SpriteEmitter *em=g_SpriteEnvs()->GetEnv(env)->GetEmitter(emi);
	em->GetVarHolder()->StoreVar(state);
	return 0;
	}


int EMITTER_GetVar(lua_State *state) {
	unsigned int id=(unsigned int)lua_tonumber(state,1); lua_remove(state,1);
	unsigned int emi=id & 0xFFFF;
	unsigned int env=(id >> 16) & 0xFFFF;
	SpriteEmitter *em=g_SpriteEnvs()->GetEnv(env)->GetEmitter(emi);
	em->GetVarHolder()->GetVar(state);
	return 1;
	}

int EMITTER_SetPos(lua_State *state) {
	int relative=LUA_GET_INT(state);
	Vector3d p=Vector3FromStack(state);
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->SetPos(p,relative);
	return 0;
	}

int EMITTER_SetVelocity(lua_State *state) {
	Vector3d p=Vector3FromStack(state);
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->SetVel(p);
	return 0;
	}

int EMITTER_SetGravity(lua_State *state) {
	double f=LUA_GET_DOUBLE(state);
	Vector3d p=Vector3FromStack(state);
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->SetGrav(p,f);
	return 0;
	}

int EMITTER_Clear(lua_State *state) {
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->Clear();
	return 0;
	}

int EMITTER_Die(lua_State *state) {
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->Die();
	return 0;
	}

int EMITTER_SetMaxTimeInterval(lua_State *state) {
	double mti=LUA_GET_DOUBLE(state);
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->SetMaxTimeInterval(mti);
	return 0;
	}

int EMITTER_SetColorMultiply(lua_State *state) {
	Vector4d p=Vector4FromStack(state);
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->SetColorMultiply(p);
	return 0;
	}

int EMITTER_SetScaleMultiply(lua_State *state) {
	double s=LUA_GET_DOUBLE(state);
	SpriteEmitter *em=LUA_GET_EMITTER;
	em->SetScaleMultiply(s);
	return 0;
	}

int SPRITETYPE_SetLifeTime(lua_State *state) {
	double lt=LUA_GET_DOUBLE(state);
	SpriteDef * st=LUA_GET_SPRITETYPE;
	st->SetLifeTime(lt);
	return 0;
	}


InterpolationFunction * CreateInterFunc(lua_State *state) {
	int clampmode=LUA_GET_INT(state);
	double p4=LUA_GET_DOUBLE(state);
	double p3=LUA_GET_DOUBLE(state);
	double p2=LUA_GET_DOUBLE(state);
	double p1=LUA_GET_DOUBLE(state);
	std::string typ=LUA_GET_STRING(state);
	InterpolationFunction *result;
	if (typ=="const") { result=new InterpolationFunctionConst(p1); }
	else if (typ=="linear") { result=new InterpolationFunctionLinear(); }
	else if (typ=="sin") { result=new InterpolationFunctionSin(); }
	else { result=new InterpolationFunction(); }
	result->SetParameters(p1,p2,p3,p4,clampmode);
	return result;
	}

int SPRITETYPE_SetGravityFunction(lua_State *state) {
	InterpolationFunction *ifunc=CreateInterFunc(state);
	SpriteDef * st=LUA_GET_SPRITETYPE;
	st->SetGravityf(ifunc);
	return 0;
	}


int SPRITETYPE_SetScaleFunction(lua_State *state) {
	InterpolationFunction *ifunc=CreateInterFunc(state);
	SpriteDef * st=LUA_GET_SPRITETYPE;
	st->SetScalef(ifunc);
	return 0;
	}

int SPRITETYPE_SetColorFunction(lua_State *state) {
	InterpolationFunction *ifunc=CreateInterFunc(state);
	std::string col=LUA_GET_STRING(state);
	SpriteDef * st=LUA_GET_SPRITETYPE;
	if (col=="r") { st->SetColorRf(ifunc); }
	else if (col=="g") { st->SetColorGf(ifunc); }
	else if (col=="b") { st->SetColorBf(ifunc); }
	else if (col=="a") { st->SetColorAf(ifunc); }
	return 0;
	}



int SPRITE_SetRot(lua_State *state) {
	double rots=LUA_GET_DOUBLE(state);
	double rot=LUA_GET_DOUBLE(state);
	SpriteEmitter *emi=LUA_GET_EMITTER;
	Sprite *ls=emi->GetLastSprite();
	ls->rotspeed=rots;
	ls->rot=rot;
	return 0;
	}

int SPRITE_SetPos(lua_State *state) {
	int relative=LUA_GET_INT(state);
	Vector3d p=Vector3FromStack(state);
	SpriteEmitter * emi=LUA_GET_EMITTER;

	if (relative) {
			p=emi->RelativePosToWorldPos(p);
			}
	Sprite *ls=emi->GetLastSprite();
	ls->pos=p;
	return 0;
	}

int SPRITE_SetPhase(lua_State *state) {
	double rgb=LUA_GET_DOUBLE(state);
	SpriteEmitter * emi=LUA_GET_EMITTER;

	Sprite *ls=emi->GetLastSprite();
	ls->rgbphase=rgb;
	return 0;
	}


LuaParticleLib::LuaParticleLib() {
	AddFunc("PARTICLE_CreateEnvOnSide",PARTICLE_CreateEnvOnSide);
	AddFunc("PARTICLE_CreateEnvOnActor",PARTICLE_CreateEnvOnActor);

	AddFunc("PARTICLE_LoadDef",PARTICLE_LoadDef);
	AddFunc("PARTICLE_AddEmitter",PARTICLE_AddEmitter);
	AddFunc("PARTICLE_SetTimeScale",PARTICLE_SetTimeScale);



	AddFunc("EMITTER_NewSpriteType",EMITTER_NewSpriteType);
	AddFunc("EMITTER_SpawnSprite",EMITTER_SpawnSprite);
	AddFunc("EMITTER_GetActiveSprites",EMITTER_GetActiveSprites);
	AddFunc("EMITTER_GetVar",EMITTER_GetVar);
	AddFunc("EMITTER_SetVar",EMITTER_SetVar);
	AddFunc("EMITTER_SetPos",EMITTER_SetPos);
	AddFunc("EMITTER_SetVelocity",EMITTER_SetVelocity);
	AddFunc("EMITTER_Die",EMITTER_Die);
	AddFunc("EMITTER_Clear",EMITTER_Clear);
	AddFunc("EMITTER_SetGravity",EMITTER_SetGravity);
	AddFunc("EMITTER_SetMaxTimeInterval",EMITTER_SetMaxTimeInterval);
	AddFunc("EMITTER_SetColorMultiply",EMITTER_SetColorMultiply);
	AddFunc("EMITTER_SetScaleMultiply",EMITTER_SetScaleMultiply);

	//AddFunc("SPRITE_SetPos",Sprite_Sprite);

	AddFunc("SPRITETYPE_SetLifeTime",SPRITETYPE_SetLifeTime);
	AddFunc("SPRITETYPE_SetScaleFunction",SPRITETYPE_SetScaleFunction);
	AddFunc("SPRITETYPE_SetColorFunction",SPRITETYPE_SetColorFunction);
	AddFunc("SPRITETYPE_SetGravityFunction",SPRITETYPE_SetGravityFunction);

	AddFunc("SPRITE_SetRot",SPRITE_SetRot);
	AddFunc("SPRITE_SetPos",SPRITE_SetPos);
	AddFunc("SPRITE_SetPhase",SPRITE_SetPhase);
	}
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
