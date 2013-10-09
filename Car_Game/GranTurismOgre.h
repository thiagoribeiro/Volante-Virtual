#ifndef _GTOGRE_H
#define _GTOGRE_H

#include "OgreOde_Core.h"
#include "OgreOde_Prefab.h"
#include "OgreOde_Loader.h"

#include "ExampleApplication.h"

#include "line3D.h"
#include <time.h>
#include "controle.h"
#include "Pista.h"
#include "Veiculo.h"
#include "metodos.h"
#include "FSM.h"
#include "FSMclass.h"




class Veiculo;


//-------------------


class GranTurismOgreFrameListener : public ExampleFrameListener
{
public:
	GranTurismOgreFrameListener(SceneManager *ScM, RenderWindow* win, Camera* cam,Real time_step,OgreOde_Prefab::Vehicle *vehicle,OgreOde_Prefab::Vehicle *Mvehicle,Root *root,SceneNode *zombie_node, OgreOde_Prefab::Ragdoll *zombie_ragdoll, OgreOde::World *world);
	GranTurismOgreFrameListener(RenderWindow* win, Camera* cam,Real time_step,Root* root,OgreOde::World *world);
	~GranTurismOgreFrameListener();
	bool frameStarted(const FrameEvent& evt);
	void changeCar();

private:
	void updateInfo();
    OgreOde::StepHandler *_stepper;
    OgreOde::World *_world;
	OgreOde_Prefab::Vehicle *_vehicle;
	char _drive;
	OgreOde_Loader::DotLoader *dotOgreOdeLoader;


	int som,somVaca,channelSom,channelVaca;


	bool PAUSE;
	AnimationState* _anim_state;
	Real _animation_speed;
	SceneNode *_zombie_node;
	OgreOde_Prefab::Ragdoll *_zombie_ragdoll;

	Camera *_cam;

	SceneNode *objNode;
	int qtdePontos;
	bool SwitchKeyB,SwitchKeyM,SwitchKeyN,SwitchKeyV;

	SceneManager *mSceneMgr;
	RenderWindow *rWindow;
	int proximoCP, atualCP;
    bool ladoCP, mudouCP;

	float steerMax,steerValue, steerFactor,steerDescFactor;

	//Bissecao *bissecao;


	Pista *pista;
	Veiculo *mVeiculo;
	Veiculo *iaVeiculo;


	Obstaculos *obstaculos;

	FILE *arquivo;

	int ITERACOES;
	Vector3 carro_pos;

	FSMclass *pFSMclass;
	int tempo;
};



class GranTurismOgreApplication : public ExampleApplication,public OgreOde::CollisionListener
{
public:
    GranTurismOgreApplication();
    ~GranTurismOgreApplication();

protected:

    virtual void chooseSceneManager(void);
	virtual void setupResources(void);
    virtual void createCamera(void);
    void createScene(void);
	void createFrameListener(void);

	virtual bool collision(OgreOde::Contact* contact);

	protected:
		OgreOde::World *_world;
		OgreOde_Prefab::Vehicle *_vehicle;
		OgreOde_Prefab::Vehicle *_Mvehicle;
		OgreOde::TriangleMeshGeometry *_track;

		SceneNode *_zombie_node;
		OgreOde_Prefab::Ragdoll *_zombie_ragdoll;

		Real _time_step;
};



#endif
