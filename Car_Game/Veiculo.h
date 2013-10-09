/*
 * Veiculo.h
 *
 *  Created on: Jun 18, 2011
 *      Author: Thiago
 */

#ifndef VEICULO_H_
#define VEICULO_H_


#include "Pista.h"
#include "controle.h"
#include "metodos.h"

class Controle;

using namespace Ogre;

typedef struct {
	Vector3 pos;
	double foraDaPista;
	double tempo;
} treinamento;




class Entidade {
protected:
	OgreOde::World *_world;

	SceneManager *mSceneMgr;
	RenderWindow *rWindow;
	Camera *camera;
	OgreOde::World *world;
public:
	void inicializar(SceneManager *ScM, RenderWindow* win, Camera* cam, OgreOde::World *_world) {
		mSceneMgr=ScM;
		rWindow=win;
		camera=cam;
		world=_world;
	}

	int estaProximo(Vector3 ponto,float raio);
	int getTipo();
};



class Veiculo : public Entidade {
protected:
	Pista *pista;
	Controle *_controle;
	int nVoltas, classificacao;	// cp = checkpoint atual; nVoltas = numero de voltas do carro na pista; classificacao = posição de classificacao do carro na corrida
	String luzId;
	Setor *cp;
	int atravessouFrenteCP,	atravessouTrasCP, atravessouEsquerdaCP, atravessouDireitaCP;
	int idFreio, idAcelerador; //idFreio = indice do control point freio, idAcelerador = indice do control point acelerador
	OgreOde_Prefab::Vehicle *_vehicle;
	OgreOde_Loader::DotLoader *dotOgreOdeLoader;

	// variaveis pra treinamento
	FILE *arqptr;
	treinamento *ptTreino;
	int idTreino;
	char *_arqGen;
	double tempo, tempoFP;	//FP = fora da pista
	int idFP;
	clock_t start, finish, startFP, finishFP;
	Obstaculos *obstaculos;
	int idObstaculo;

public:

	void criar(Ogre::String carro,Ogre::String arquivo,Pista *_pista, Controle *_c,char *arqGen,Obstaculos *obst) {
		pista=_pista;
		cp=pista->getPrimeiroSetor();

		idFreio = 0;
		idAcelerador = 0;
		this->idTreino = 0;

		idFP = 0;

		_arqGen = arqGen;

		start = clock();
		this->ptTreino = (treinamento *) malloc(pista->getQtdeCheckPoints()*sizeof(treinamento));

		setControle(_c);
		dotOgreOdeLoader = new OgreOde_Loader::DotLoader(world);
		_vehicle = static_cast <OgreOde_Prefab::Vehicle *> (dotOgreOdeLoader->loadObject ("subaru.ogreode", carro));
		//this->load(arquivo);



		obstaculos=obst;
		//idObstaculo = obstaculos->add(_vehicle->getEntity());

	}
	~Veiculo(){
		free(this->ptTreino);
	}
	Controle* getControle() {
		return _controle;

	}
	void setControle(Controle *_c);

	void criarLuzFreio(Vector2 YZ,Real Xesq, Real Xdir, String material,ColourValue cor, String Id)
	{

		luzId=Id;
		//SceneNode* freios = mSceneMgr->getRootSceneNode()->createChildSceneNode("freios"+Id);
		SceneNode* freios = mSceneMgr->createSceneNode("freios"+Id);
		Light* _freio1 = new Light("freio1"+Id);
		_freio1->setType(Light::LT_SPOTLIGHT);
		_freio1->setDirection(Vector3::NEGATIVE_UNIT_Z);
		_freio1->setCastShadows(true);
		_freio1->setDiffuseColour(cor);
		_freio1->setSpecularColour(1, 1, 1);
		_freio1->setAttenuation(800,1,0.0005,0);
		_freio1->setSpotlightRange(Degree(80),Degree(90));
		SceneNode *luz = freios->createChildSceneNode("luz1"+Id);
		luz->setScale(0.01,0.01,0.01);
		BillboardSet* bbs = mSceneMgr->createBillboardSet("bFreio1"+Id, 1);
		bbs->setMaterialName(material);
		Billboard* bb = bbs->createBillboard(0,0,0,cor);
		// attach
		luz->attachObject(_freio1);
		luz->attachObject(bbs);
		//luz->setPosition(cam->getPosition()+Vector3(0,2,0));
		luz->setPosition(Vector3(Xesq,YZ.x,YZ.y));
		//freios->addChild(luz);
		_freio1 = new Light("freio2"+Id);
		_freio1->setType(Light::LT_SPOTLIGHT);
		_freio1->setDirection(Vector3::NEGATIVE_UNIT_Z);
		_freio1->setCastShadows(true);
		_freio1->setDiffuseColour(cor);
		_freio1->setSpecularColour(1, 1, 1);
		_freio1->setAttenuation(800,1,0.0005,0);
		_freio1->setSpotlightRange(Degree(80),Degree(90));
		luz = freios->createChildSceneNode("luz2"+Id);
		luz->setScale(0.01,0.01,0.01);
		bbs = mSceneMgr->createBillboardSet("bFreio2"+Id, 1);
		bbs->setMaterialName(material);
		bb = bbs->createBillboard(0,0,0,cor);
		// attach
		luz->attachObject(_freio1);
		luz->attachObject(bbs);
		//luz->setPosition(cam->getPosition()+Vector3(0,2,0));
		luz->setPosition(Vector3(Xdir,YZ.x,YZ.y));
		freios->setVisible(false);
		_vehicle->getSceneNode()->addChild(freios);
	}

	void showBrakeLights(bool show)
	{
		SceneNode* freios = mSceneMgr->getSceneNode("freios"+luzId);
		freios->setVisible(show);
	}

	void RegistrarTreino(treinamento *ptTreino)
	{
		int i;
		double total=0;
		double tfp=0;

		arqptr = fopen(_arqGen,"a+");

		fprintf(arqptr,"Volta %d\n",nVoltas);

		for(i=0; i<pista->getQtdeCheckPoints(); i++)
		{
			fprintf(arqptr,"Posicao(%d): %.2f, %.2f, %.2f\tTempo: %g segundos\tTempo Fora da pista: %g segundos\n",i,ptTreino[i].pos.x,ptTreino[i].pos.y,ptTreino[i].pos.z,ptTreino[i].tempo,ptTreino[i].foraDaPista);
			total += ptTreino[i].tempo;
			tfp+=ptTreino[i].foraDaPista;
		}

		fprintf(arqptr,"Tempo da volta: %g segundos\n\n",total);
		fclose(arqptr);
		double fo = total + 1.5 * tfp;
		Ogre::LogManager::getSingletonPtr()->logMessage("Tempo da Volta: "+Ogre::StringConverter::toString((Real)total)+" FO "+Ogre::StringConverter::toString((Real)fo));

		/*
		OverlayElement* over;
		if(nVoltas==1) {
			over = OverlayManager::getSingleton().getOverlayElement("Example/DynTex/Param_K");
			over->setCaption("1: "+Ogre::StringConverter::toString((Real)total)+" FO: "+Ogre::StringConverter::toString((Real)fo));
		} else if(nVoltas==2) {
			over = OverlayManager::getSingleton().getOverlayElement("Example/DynTex/Param_F");
			over->setCaption("2: "+Ogre::StringConverter::toString((Real)total)+" FO: "+Ogre::StringConverter::toString((Real)fo));
		} else if(nVoltas==3) {
			over = OverlayManager::getSingleton().getOverlayElement("Example/DynTex/Param_A0");
			over->setCaption("3: "+Ogre::StringConverter::toString((Real)total)+" FO: "+Ogre::StringConverter::toString((Real)fo));
		}
		*/

	}

	void step(const FrameEvent& evt,const bool PAUSE);


	int estaProximo(Vector3 target,float raio) {
		return (Ogre::Math::Abs(Dist(_vehicle->getPosition(),target))<raio);
	}

	Real getVelocityKm()
	{
		return ((_vehicle->getVelocity()*100)/32);
	}

	Setor* getSetorAtual(){return cp;}

	Pista* getPista(){return pista;}

	int getIdAcelerador()
	{
		return idAcelerador;
	}

	int getIdFreio()
	{
		return idFreio;
	}

	void initNumeroVoltas(){this->nVoltas = -1;}
	int getNumeroVoltas(){return this->nVoltas;}

	void setClassificacao(int pos){this->classificacao = pos;}
	int getClassificacao(){return this->classificacao;}

	relatorioObstaculo vaiBater();

	void setInputs(Real steering,Real throttle,Real brake) {
		_vehicle->setInputs(steering,throttle, brake);
	}
	void setInputs(Real steering,Real throttle_brake) {
		_vehicle->setInputs(steering, throttle_brake);
	}
	void setInputs(bool left,bool right,bool throttle,bool brake) {
		_vehicle->setInputs(left,right,throttle,brake);
	}
	const Vector3& getPosition() {
		return _vehicle->getPosition();
	}
	const Quaternion& getOrientation() {
		return _vehicle->getOrientation();
	}
	void setPosition(const Vector3& position) {
		_vehicle->setPosition(position);
	}
	SceneNode* getSceneNode(){
		return _vehicle->getSceneNode();
	}
	Entity*	getEntity(){
		return _vehicle->getEntity();
	}
	OgreOde::Body* getBody(){
		return _vehicle->getBody();
	}
	OgreOde::Geometry* getGeometry(){
		return _vehicle->getGeometry();
	}
	void update(Ogre::Real time) {
		_vehicle->update(time);
	}
	void setSteerSpeed(Ogre::Real speed) {
		printf("U %d\n\n",(int)_vehicle->getWheelCount());

		_vehicle->getWheel(0)->setSteerSpeed(speed);
		printf("X\n\n");
		_vehicle->getWheel(1)->setSteerSpeed(speed);
		printf("Z\n\n");

	}
	Real getVelocity() {
		return _vehicle->getVelocity();
	}
};

#endif /* VEICULO_H_ */
