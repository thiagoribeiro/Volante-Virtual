#ifndef _CONTROLE_H
#define _CONTROLE_H



//#include "preRequisitos.h"
#include "GranTurismOgre.h"
#include "Veiculo.h"
#include "metodos.h"
#include "FSM.h"
#include "FSMclass.h"
//#include "SDL.h"
//#include "SoundManager.h"

//PARA O SOCKET

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
//------------
//PARA A THREAD
#include <unistd.h>
#include <pthread.h>







class Veiculo;

using namespace Ogre;
//
//-------------------

class Controle {
protected:
	Veiculo* _v;
	OIS::Keyboard *mInputDevice;
	SceneManager *mSceneManager;
	RenderWindow *rWindow;
	//int somId,channel;b
public:
	void setVeiculo(Veiculo *v) {
		_v=v;

	}
	
	Veiculo* getVeiculo() {
		return _v;
	}
	
	void setDevices(OIS::Keyboard *ir,SceneManager *ScM,RenderWindow *rw,int som) {
		mInputDevice=ir;
		mSceneManager=ScM;
		rWindow=rw;
		//somId=som;
		//channel=INVALID_SOUND_CHANNEL;
	}

	virtual void atualizar(const FrameEvent& evt) {};
	virtual Vector3 getDirecao() { return Vector3(0,0,0); };
};






/*
class controle_Volante : public Controle {
private:
	SDL_Joystick *mJoystick;
	Vector3 posicaoAnt;

	Real nivelar(Real valor) {
		if(valor>32000) {
			valor=32000;
		}
		else if(valor<-32000) {
			valor=-32000;
		}
		else if(valor>0 && valor<10) {
			valor=0;
		}
		else if(valor<0 && valor>-10) {
			valor=0;
		}
		return valor;
	}

	Real direcao() {
		return (nivelar(SDL_JoystickGetAxis( mJoystick, 0 ))/64000.f);
	}
	Real acelerar_freiar() {
		return -1*(nivelar(SDL_JoystickGetAxis( mJoystick, 1 ))/32000.f);
	}

public:
	int setId(int id) {
		mJoystick = SDL_JoystickOpen( id );

		_v->setSteerSpeed(10.0f);

		return ( mJoystick == NULL );
	}

	controle_Volante() {
		posicaoAnt = Vector3(0,0,0);
	}

	~controle_Volante() {
		SDL_JoystickClose( mJoystick );
	}

	void atualizar(const FrameEvent& evt) {
		posicaoAnt = _v->getPosition();
		SDL_JoystickUpdate();

		Real af = acelerar_freiar();
		_v->setInputs(direcao(),af);

		//if(acelerar_freiar()) {
		if (af) {
			SoundManager::getSingletonPtr()->PlaySound(somId, _v->getSceneNode(), &channel,0.1f);
		}
		else if(channel!=INVALID_SOUND_CHANNEL) {
			SoundManager::getSingletonPtr()->StopSound(&channel);
		}

		_v->showBrakeLights(af<0);
	}
};
//
//----------
class controle_Joystick : public Controle {
private:
	SDL_Joystick *mJoystick;
	clock_t start, finish;
	Vector3 posicaoAnt;
	double tempo;


	Real nivelar(Real valor) {
		if(valor>32000) {
			valor=32000;
		}
		else if(valor<-32000) {
			valor=-32000;
		}
		else if(valor>0 && valor<10) {
			valor=0;
		}
		else if(valor<0 && valor>-10) {
			valor=0;
		}
		return valor;
	}

	Real direcao() {
		return (nivelar(SDL_JoystickGetAxis( mJoystick, 0 ))/64000.f);
	}
	int acelerar() {
		return SDL_JoystickGetButton( mJoystick, 1);
	}
	int freiar() {
		return SDL_JoystickGetButton( mJoystick, 0);
	}

public:
	int setId(int id) {
		mJoystick = SDL_JoystickOpen( id );

		_v->setSteerSpeed(1.0f);

		return ( mJoystick == NULL );
	}

	controle_Joystick() {
		posicaoAnt = Vector3(0,0,0);
	}

	~controle_Joystick() {
		SDL_JoystickClose( mJoystick );
	}

	Vector3 getDirecao() {
		return _v->getPosition() - posicaoAnt;
	}

	void atualizar(const FrameEvent& evt) {
		posicaoAnt = _v->getPosition();

		SDL_JoystickUpdate();

		int esq = direcao()<0;
		int dir = direcao()>0;
		int ac = acelerar();
		int fr = freiar();
		_v->setInputs(esq,dir,ac,fr);

		//if(acelerar()) {
		if (ac) {
			SoundManager::getSingletonPtr()->PlaySound(somId, _v->getSceneNode(), &channel,0.1f);
		}
		else if(channel!=INVALID_SOUND_CHANNEL) {
			SoundManager::getSingletonPtr()->StopSound(&channel);
		}



		//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getVelocity()));
		//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getChave()));
		//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getPista()->getDistanciaCpFreio(_v->getPosition(),_v->getIdFreio()))+"------"+Ogre::StringConverter::toString(_v->getPista()->getDistanciaCpAcelerador(_v->getPosition(),_v->getIdAcelerador())));
		//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getPista()->getControlPointFreio(_v->getIdFreio()).medio)+"----"+Ogre::StringConverter::toString(_v->getPosition()));

		_v->showBrakeLights(fr>0);
	}
};
*/

//
//-----------------
class controle_Teclado : public Controle {
private:
	OIS::KeyCode esq,dir,freio,acel;
	OIS::Keyboard *mInputDevice;
	Vector3 posicaoAnt;

	bool acelerar() {
		return mInputDevice->isKeyDown(acel);
	}
	bool freiar() {
		return mInputDevice->isKeyDown(freio);
	}
	bool esquerda() {
		return mInputDevice->isKeyDown(esq);
	}
	bool direita() {
		return mInputDevice->isKeyDown(dir);
	}
public:
	controle_Teclado() {
		posicaoAnt = Vector3(0,0,0);
	}
	void setKeys(OIS::KeyCode _esq,OIS::KeyCode _dir,OIS::KeyCode _acel,OIS::KeyCode _freio,OIS::Keyboard *ir);
	Vector3 getDirecao();
	void atualizar(const FrameEvent& evt);


};

class controle_IA : public Controle {
protected:
	Pista* pista;
	RenderWindow* rWindow;
	float angulacao12;
	float angulacao23;
	float distancia;
	//Bissecao *bis1,*bis2;
	float tempo[43], tempoAux,tempoVolta;
	Vector3 ponto[43], pontoAux,pontoAtual;
	clock_t startTime,endTime, recover;
	FSMclass *iaFSMclass;
	Setor* cp_aux;
	Real acelerar;
	Real deltaS, deltaV, t;


public:
	controle_IA() {
		angulacao12=0;
		angulacao23=0;
		distancia=0;
		//bis1 = new Bissecao();
		//bis2 = new Bissecao();
		recover=clock();

		deltaS=0;

		iaFSMclass = NULL;
		try
		{
			iaFSMclass = new FSMclass(STATE_ID_RACING_LINE);
		}
		catch( ... )
		{
			throw;
		}
		iaFSMclass->CreateIAFSM();
	}

	~controle_IA() {
		//free(bis1);
		//free(bis2);
		free(tempo);
		free(ponto);
	}

	void setup(Pista* _pista,RenderWindow *_rWindow);

	Vector3 getDirecao();

	void atualizar(const FrameEvent& evt);
};

class controle_Network: public Controle {
public:

	Vector3 posicaoAnt;
	float accel_break;
	float left_right;

	int socketId;
	unsigned int size;

	sockaddr_in serverAddr;


	Real nivelar(Real valor) {
		if(valor>32000) {
			valor=32000;
		}
		else if(valor<-32000) {
			valor=-32000;
		}
		else if(valor>0 && valor<10) {
			valor=0;
		}
		else if(valor<0 && valor>-10) {
			valor=0;
		}
		return valor;
	}

	Real direcao() {
		//return (nivelar(SDL_JoystickGetAxis( mJoystick, 0 ))/64000.f);
		return left_right;
	}
	Real acelerar_freiar() {
		//return -1*(nivelar(SDL_JoystickGetAxis( mJoystick, 1 ))/32000.f);
		return accel_break;
	}


public:

	~controle_Network() {
		close(socketId);
	}
	void setup();
	void atualizar(const FrameEvent& evt);
};
#endif
