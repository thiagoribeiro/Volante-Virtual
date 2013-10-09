/*
 * Pista.h
 *
 *  Created on: Jun 18, 2011
 *      Author: Thiago
 */

#ifndef PISTA_H_
#define PISTA_H_

#include "OgreOde_Core.h"
#include "OgreOde_Prefab.h"
#include "OgreOde_Loader.h"
#include "line3D.h"
#include "metodos.h"

using namespace Ogre;

typedef struct {
	Vector3 v1,v2,medio,guia;
	Vector3 direcao;
	float  modulo,vel;
	Radian angulo;
	Vector2 vetor;
} tCpoint;

typedef struct {
	Vector3 pos;
	bool colidiu;
	Real distancia;
	Entity *obj;
} relatorioObstaculo;

typedef struct {
	tCpoint checkpoint;
	int curva;
} structSetor;


extern Entity *obj;
extern SceneNode *objNode;


//
//-------------------

class Setor {
protected:
	int posicao;
	tCpoint cpFrente,cpTras;
	int curva;
	Real distancia,distanciaLateral;
	Setor *proximo, *anterior;
public:
	Setor(tCpoint frente,tCpoint tras,int _curva, int pos) {
		posicao=pos;

		cpFrente=frente;
		cpTras=tras;

		if(_curva>0) {
			curva = _DIREITA;
		}
		else {
			curva = _ESQUERDA;
		}
		setCpTras(tras);
		setCpFrente(frente);

	//	cpTras.angulo= Ogre::Math::ACos((((cpTras.vetor.x*cpTras.vetor.x)+(cpTras.vetor.y*cpTras.vetor.y))/(cpTras.modulo*cpFrente.modulo)));


		distancia=per_Dist(this,cpTras.medio);
		distanciaLateral=per_DistLateral(this,(cpTras.v2+cpFrente.v2)*0.5);

	}

	Vector3 pontoMedioL() {
		return (cpTras.v1+cpFrente.v1)*0.5;
	}
	Vector3 pontoMedioR() {
		return (cpTras.v2+cpFrente.v2)*0.5;
	}
	void setCpTras(tCpoint t) {
		cpTras=t;

		cpTras.medio=(cpTras.v1+cpTras.v2)*0.5;
		cpTras.vetor = Vector2(cpTras.v1.x-cpTras.v2.x,cpTras.v1.z-cpTras.v2.z);
		cpTras.modulo = Ogre::Math::Sqrt(Ogre::Math::Sqr(cpTras.vetor.x)+Ogre::Math::Sqr(cpTras.vetor.y));
	}
	void setCpFrente(tCpoint t) {
		cpFrente=t;

		cpFrente.medio=(cpFrente.v1+cpFrente.v2)*0.5;
		cpFrente.vetor = Vector2(cpFrente.v1.x-cpFrente.v2.x,cpFrente.v1.z-cpFrente.v2.z);
		cpFrente.modulo = Ogre::Math::Sqrt(Ogre::Math::Sqr(cpFrente.vetor.x)+Ogre::Math::Sqr(cpFrente.vetor.y));
		cpFrente.guia = cpFrente.medio;
	}
	void setAngulo(Radian angle) {
		cpTras.angulo=angle;
	}
	Radian getAngulo() {
		return cpTras.angulo;
	}
	int atravessouLateral(Vector3 ponto,int lado) {
		if(lado==_DIREITA)
			return ladoReta(ponto.x,ponto.z,cpTras.v2.x,cpTras.v2.z,cpFrente.v2.x,cpFrente.v2.z);
		else
			return !ladoReta(ponto.x,ponto.z,cpTras.v1.x,cpTras.v1.z,cpFrente.v1.x,cpFrente.v1.z);
	}
	int atravessouFrente(Vector3 ponto) {
		return !ladoReta(ponto.x,ponto.z,cpFrente.v1.x,cpFrente.v1.z,cpFrente.v2.x,cpFrente.v2.z);
	}
	int atravessouTras(Vector3 ponto) {
		return ladoReta(ponto.x,ponto.z,cpTras.v1.x,cpTras.v1.z,cpTras.v2.x,cpTras.v2.z);
	}
	int ehCurva() {
		return cpTras.angulo>Radian(10);
	}
	int getDirecaoCurva() {
		return curva;
	}
	tCpoint getFrente() {
		return cpFrente;
	}
	tCpoint getTras() {
		return cpTras;
	}
	Setor* getProxSetor() {
		return proximo;
	}
	Setor* getAnteriorSetor() {
		return anterior;
	}
	int getPos() {
		return posicao;
	}
	Real getDistancia() {
		return distancia;
	}
	Real getDistanciaLateral() {
		return distanciaLateral;
	}
	Real getDistanciaPontoNoSetor(Vector3 ponto) {
		return per_Dist(this,ponto);
	}
	Real getDistanciaPontoLateral(Vector3 ponto) {
		return per_DistLateral(this,ponto);
	}
	void setProxSetor(Setor *prox) {
		proximo=prox;
		if(ladoReta(proximo->getFrente().medio.x,proximo->getFrente().medio.z,cpTras.medio.x,cpTras.medio.z,cpFrente.medio.x,cpFrente.medio.z)) {
			curva=_DIREITA;
		}
		else {
			curva=_ESQUERDA;
		}
	}
	void setAnteriorSetor(Setor *ant) {
		anterior=ant;
	}
	Vector3 getVetorDirecao() {
		return cpFrente.medio-cpTras.medio;
	}

	void setGuia(Vector3 guia) {
		cpFrente.guia=guia;
	}

};


//
//---------------
class Obstaculos {
protected:
	Entity **lista;
	int n,i;
	RenderWindow *rWindow;
public:
	Obstaculos(RenderWindow* win) {
		rWindow = win;
		n=10;
		i=0;
		lista = (Entity**) malloc(sizeof(Entity*)*n);
	}
	~Obstaculos() {
		free(lista);

	}
	int add(Entity* obj) {
		lista[i]= obj;
		i++;
		return i-1;
	}
	relatorioObstaculo vaiBater(int id, Vector3 direcao) {
		std::pair<bool,Real> resultado;

		relatorioObstaculo result;
		result.colidiu=false;
		result.distancia=0;
		result.pos=Vector3(0,0,0);

		Entity* obj = lista[id];

		Vector3 pos = obj->getParentNode()->getPosition();
		Ray raio = Ray(pos,direcao);
		//rWindow->setDebugText(Ogre::StringConverter::toString(lista[id]->getWorldBoundingBox().getCenter()));
		for(int j=0;j<i;j++) {
			if(j!=id) {
				resultado = raio.intersects(lista[j]->getWorldBoundingBox());
				if(resultado.first) {
					result.pos=lista[j]->getParentNode()->getPosition();
					result.colidiu=true;
					result.distancia=resultado.second;
					result.obj=lista[j];
					break;
				}
			}
		}

		return result;
	}
};




class Pista {
private:
	Setor* primeiroSetor;
	SceneManager *mSceneMgr;
	RenderWindow *rWindow;
	Camera *camera;
	OgreOde::World *world;
	int nCP;		// quantidade de checkpoints
	int nControlPoints; //quantidade de pontos de controle, referente ao controle de velocidade
	tCpoint *cpFreio, *cpAcelerador;	//control points do freio e do acelerador

public:
	Pista(SceneManager *ScM, RenderWindow* win, Camera* cam, OgreOde::World *_world) {
		world=_world;
		rWindow=win;
		camera=cam;
		mSceneMgr=ScM;
	}

	void loadPontosGuia(char* arq) {
		FILE *arquivo;
		float ax,ay,az;
		Setor* cp;

		arquivo = fopen(arq,"r");
		cp = this->getPrimeiroSetor();
		for(int i=0;i<this->getQtdeCheckPoints();i++) {
			fscanf(arquivo,"%f",&ax);
			fscanf(arquivo,"%f",&ay);
			fscanf(arquivo,"%f",&az);
			cp->setGuia(Vector3(ax,ay,az));

			cp=cp->getProxSetor();

		}
		fclose(arquivo);
	}

	void loadCheckPoints(char* arq,int showPoints)
	{
		int i;
		int cpoints;	// quantidade de pontos
		int cpTotal;	// quantidade de control points total do arquivo
		FILE *arquivo;
		float ax,ay,az,bx,by,bz;
		int ladoCurva,vel;
		Vector3 pReta;
		Line3D *myLine;
		SceneNode *myNode;
		Entity *obj;
		SceneNode *objNode;
		char tmp[10];
		int output;


		arquivo = fopen(arq,"r");
		fseek(arquivo,0,SEEK_CUR);

		fscanf(arquivo, "%d",&cpoints);
		nCP = (int)cpoints/2;
		structSetor *checkPoint = (structSetor*) malloc(sizeof(structSetor)*nCP);
		SceneNode *nodesCP = mSceneMgr->getRootSceneNode()->createChildSceneNode("nodesCP");

		for(i=0;i<nCP;i++)
		{
			fscanf(arquivo,"%f",&ax);
			fscanf(arquivo,"%f",&ay);
			fscanf(arquivo,"%f",&az);
			fscanf(arquivo,"%f",&bx);
			fscanf(arquivo,"%f",&by);
			fscanf(arquivo,"%f",&bz);
			fscanf(arquivo,"%d",&ladoCurva);

			checkPoint[i].curva = ladoCurva;
			checkPoint[i].checkpoint.v1=Vector3(ax,ay,az);
			checkPoint[i].checkpoint.v2=Vector3(bx,by,bz);

/*
			if(showPoints) {

				//obj = mSceneMgr->createEntity( "pontoA"+Ogre::StringConverter::toString(i), "sphere.mesh" );		// esfera = mais vertices
				obj = mSceneMgr->createEntity( "pontoA"+Ogre::StringConverter::toString(i), "cube.mesh" );			// box = menos vertices
				objNode = nodesCP->createChildSceneNode( "ptNodeA"+Ogre::StringConverter::toString(i),Vector3(ax,ay,az));
				objNode->setScale(.005,.05,.005);
				objNode->attachObject( obj );

				//obj = mSceneMgr->createEntity( "pontoB"+Ogre::StringConverter::toString(i), "sphere.mesh" );		//idem ponto A
				obj = mSceneMgr->createEntity( "pontoB"+Ogre::StringConverter::toString(i), "cube.mesh" );			//idem ponto A
				objNode = nodesCP->createChildSceneNode( "ptNodeB"+Ogre::StringConverter::toString(i),Vector3(bx,by,bz));
				objNode->setScale(.005,.05,.005);
				objNode->attachObject( obj );

				myLine = new Line3D();
				myLine->addPoint(checkPoint[i].checkpoint.v1);
				myLine->addPoint(checkPoint[i].checkpoint.v2);
				myLine->drawLines();

				myNode = nodesCP->createChildSceneNode();
			    myNode->attachObject(myLine);

			}
*/
		}
		checkPoint[nCP-1].checkpoint.angulo= (((checkPoint[nCP-1].checkpoint.vetor.x*checkPoint[0].checkpoint.vetor.x)+(checkPoint[nCP-1].checkpoint.vetor.y*checkPoint[0].checkpoint.vetor.y))/(checkPoint[nCP-1].checkpoint.modulo*checkPoint[0].checkpoint.modulo));

		fscanf(arquivo, "%d",&cpTotal);
		nControlPoints = (int)(cpTotal / 2);
		cpFreio = (tCpoint*) malloc(nControlPoints*sizeof(tCpoint));
		cpAcelerador = (tCpoint*) malloc(nControlPoints*sizeof(tCpoint));

		int j=0,w=0;
		for(i=0;i<cpTotal;i++)
		{


			fscanf(arquivo,"%f",&ax);
			fscanf(arquivo,"%f",&ay);
			fscanf(arquivo,"%f",&az);
			fscanf(arquivo,"%f",&bx);
			fscanf(arquivo,"%f",&by);
			fscanf(arquivo,"%f",&bz);
			fscanf(arquivo,"%d",&vel);

			if (((int)((i+1)%2)) != 0)
			{

				cpFreio[j].v1 = Vector3(ax,ay,az);
				cpFreio[j].v2 = Vector3(bx,by,bz);
				cpFreio[j].medio = (cpFreio[j].v1 + cpFreio[j].v2)*0.5;
				cpFreio[j].vetor = Vector2(ax-bx,az-bz);
				cpFreio[j].modulo = Ogre::Math::Sqrt(Ogre::Math::Sqr(cpFreio[j].vetor.x)+Ogre::Math::Sqr(cpFreio[j].vetor.y));
				cpFreio[j].direcao = cpFreio[j].v2 - cpFreio[j].v1;
				cpFreio[j].direcao.perpendicular();
				cpFreio[j].vel = vel;

/*
				if(showPoints)
				{
					//obj = mSceneMgr->createEntity( "pontoA"+Ogre::StringConverter::toString(i), "sphere.mesh" );		// esfera = mais vertices
					obj = mSceneMgr->createEntity( "pAC"+Ogre::StringConverter::toString(i), "cubeRed.mesh" );			// box = menos vertices
					printf("\nBBBBBBBBBBBB");
					objNode = nodesCP->createChildSceneNode( "ptNAC"+Ogre::StringConverter::toString(i),Vector3(ax,ay+2,az));
					printf("\nCCCCCCCCCCCCCC");
					objNode->setScale(.25,1.5,.25);
					printf("\nDddddddddddddddddd");
					objNode->attachObject( obj );
					printf("\nEEEEEEEEEEEEEEEEEE");

					//obj = mSceneMgr->createEntity( "pontoB"+Ogre::StringConverter::toString(i), "sphere.mesh" );		//idem ponto A
					obj = mSceneMgr->createEntity( "pBC"+Ogre::StringConverter::toString(i), "cubeRed.mesh" );			//idem ponto A
					objNode = nodesCP->createChildSceneNode( "ptNBC"+Ogre::StringConverter::toString(i),Vector3(bx,by+2,bz));
					objNode->setScale(.25,1.5,.25);
					objNode->attachObject( obj );

					myLine = new Line3D();
					myLine->addPoint(cpFreio[j].v1);
					myLine->addPoint(cpFreio[j].v2);
					myLine->drawLines();

					myNode = nodesCP->createChildSceneNode();
					myNode->attachObject(myLine);

				}
*/
				j++;
			}
			else
			{

				cpAcelerador[w].v1 = Vector3(ax,ay,az);
				cpAcelerador[w].v2 = Vector3(bx,by,bz);
				cpAcelerador[w].medio = (cpAcelerador[w].v1 + cpAcelerador[w].v2)*0.5;
				cpAcelerador[w].vetor = Vector2(ax-bx,az-bz);
				cpAcelerador[w].modulo = Ogre::Math::Sqrt(Ogre::Math::Sqr(cpAcelerador[w].vetor.x)+Ogre::Math::Sqr(cpAcelerador[w].vetor.y));
				cpAcelerador[w].direcao = cpAcelerador[w].v2 - cpAcelerador[w].v1;
				cpAcelerador[w].direcao.perpendicular();
				cpAcelerador[w].vel = vel;
/*
				if(showPoints)
				{
					//obj = mSceneMgr->createEntity( "pontoA"+Ogre::StringConverter::toString(i), "sphere.mesh" );		// esfera = mais vertices
					obj = mSceneMgr->createEntity( "pAC"+Ogre::StringConverter::toString(i), "cubeBlue.mesh" );			// box = menos vertices
					objNode = nodesCP->createChildSceneNode( "ptNAC"+Ogre::StringConverter::toString(i),Vector3(ax,ay+2,az));
					objNode->setScale(.25,1.5,.25);
					objNode->attachObject( obj );

					//obj = mSceneMgr->createEntity( "pontoB"+Ogre::StringConverter::toString(i), "sphere.mesh" );		//idem ponto A
					obj = mSceneMgr->createEntity( "pBC"+Ogre::StringConverter::toString(i), "cubeBlue.mesh" );			//idem ponto A
					objNode = nodesCP->createChildSceneNode( "ptNBC"+Ogre::StringConverter::toString(i),Vector3(bx,by+2,bz));
					objNode->setScale(.25,1.5,.25);
					objNode->attachObject( obj );

					myLine = new Line3D();
					myLine->addPoint(cpAcelerador[w].v1);
					myLine->addPoint(cpAcelerador[w].v2);
					myLine->drawLines();

					myNode = nodesCP->createChildSceneNode();
					myNode->attachObject(myLine);
				}
*/
				w++;
			}

		}

		Setor *novo,*aux;
		for(i=0;i<nCP-1;i++) {

			novo = new Setor(checkPoint[i+1].checkpoint,checkPoint[i].checkpoint,checkPoint[i].curva,i);
			if(i>0) {
				novo->setAnteriorSetor(aux);
				aux->setAngulo(CalcSetorAngle(novo,aux,rWindow));
				aux->setProxSetor(novo);
			}
			else {
				primeiroSetor = novo;
			}
			aux=novo;


		}

		novo = new Setor(checkPoint[0].checkpoint,checkPoint[nCP-1].checkpoint,checkPoint[nCP-1].curva,nCP-1);
		novo->setAnteriorSetor(aux);
		aux->setAngulo(CalcSetorAngle(novo,aux,rWindow));
		novo->setProxSetor(primeiroSetor);
		aux->setProxSetor(novo);
		aux=novo;


		primeiroSetor->setAnteriorSetor(novo);
		novo->setAngulo(CalcSetorAngle(primeiroSetor,novo,rWindow));
		aux=primeiroSetor->getProxSetor();
		free(checkPoint);
		fclose(arquivo);


		mSceneMgr->getSceneNode("nodesCP")->flipVisibility();


	}


	float DistVectorCP(tCpoint CP,Vector3 pos) // distancia entre pontos
	{
		Real aux = pow((CP.medio.x - pos.x),2) + pow((CP.medio.y - pos.y),2) + pow((CP.medio.z - pos.z),2);
		return sqrt(aux);
	}

	Real getDistanciaCpFreio(Vector3 ponto, int i) {
		return DistVectorCP(cpFreio[i],ponto);
	}

	Real getDistanciaCpAcelerador(Vector3 ponto, int i) {
		return DistVectorCP(cpAcelerador[i],ponto);
	}

	tCpoint getControlPointFreio(int i) {
		return cpFreio[i];
	}

	tCpoint getControlPointAcelerador(int i) {
		return cpAcelerador[i];
	}

	int getQtdeControlPoints() {
		return nControlPoints;
	}

	int getQtdeCheckPoints() {
		return nCP;
	}

	int atravessouControlPointFreio(Vector3 ponto,int i) {
		return !ladoReta(ponto.x,ponto.z,cpFreio[i].v1.x,cpFreio[i].v1.z,cpFreio[i].v2.x,cpFreio[i].v2.z);
	}

	int atravessouControlPointAcelerador(Vector3 ponto,int i) {
		return !ladoReta(ponto.x,ponto.z,cpAcelerador[i].v1.x,cpAcelerador[i].v1.z,cpAcelerador[i].v2.x,cpAcelerador[i].v2.z);
	}

	Setor* getPrimeiroSetor() {
		return primeiroSetor;
	}
};

#endif /* PISTA_H_ */
