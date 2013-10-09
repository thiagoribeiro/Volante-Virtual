/*
 * controle.cpp
 *
 *  Created on: Jun 18, 2011
 *      Author: aluno
 */
#include "controle.h"


#define NW_BUFLEN 512
#define NW_NPACK 10
#define NW_PORT 9930

#define SRV_IP "127.0.0.1"

#define NW_READY 1
#define NW_SEND_DATA 2
struct comandos {
	float left_right;
	float accel_break;
	int status;

};

sockaddr_in serverAddr;
sockaddr &serverAddrCast=(sockaddr&) serverAddr;

void controle_Network::setup() {
	socketId=socket(AF_INET,SOCK_DGRAM,0);
	printf("ID: %d\n\n\n",socketId);
	comandos cmd_snd, cmd_rcv;


	//sockaddr_in serverAddr;
	//sockaddr &serverAddrCast=(sockaddr&) serverAddr;
	extern sockaddr_in  serverAddr;
	extern sockaddr &serverAddrCast;
	//_v->setSteerSpeed(Real(1));

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port =htons(NW_PORT);
	if (inet_aton(SRV_IP, &serverAddr.sin_addr)==0) {
			fprintf(stderr, "inet_aton() failed\n");
			exit(1);
		}

	size=sizeof(serverAddr);

	cmd_snd.left_right=0;
	cmd_snd.accel_break=0;
	cmd_snd.status=NW_READY;
	sendto(socketId,&cmd_snd,sizeof(struct comandos),0,&serverAddrCast,size);
/*
	int i=0;
	while(i<10) {

		recvfrom(socketId,&cmd_rcv,sizeof(struct comandos), MSG_DONTWAIT ,&serverAddrCast,&size);
		printf("Mensagem: [%2.4f | %2.4f | %d]\n\n",cmd_rcv.left_right,cmd_rcv.accel_break,cmd_rcv.status);
		sleep(1);
		i++;
	}
*/
	printf("SERVER: %p \n",serverAddr.sin_addr);
	accel_break=0.5;
	left_right=0;

}
void controle_Network::atualizar(const FrameEvent& evt) {
	extern sockaddr_in  serverAddr;
	extern sockaddr &serverAddrCast;

	//printf("ID: %d\n\n\n",socketId);
	//sockaddr &serverAddrCast=(sockaddr&) serverAddr;
	//printf("SERVER: %p \n",serverAddr.sin_addr);

	comandos cmd_rcv;
	cmd_rcv.accel_break=0;
	cmd_rcv.left_right=0;
	cmd_rcv.status=0;
	recvfrom(socketId,&cmd_rcv,sizeof(struct comandos), MSG_DONTWAIT ,&serverAddrCast,&size);

	if(cmd_rcv.status==NW_SEND_DATA) {

		accel_break=(float)cmd_rcv.accel_break;
		left_right=(float)cmd_rcv.left_right;
		printf("Mensagem: [%2.4f | %2.4f | %d]\n\n",cmd_rcv.left_right,cmd_rcv.accel_break,cmd_rcv.status);
	}



	_v->setInputs(Real(left_right),Real(accel_break));
	_v->showBrakeLights(accel_break<0);

}

void controle_Teclado::setKeys(OIS::KeyCode _esq,OIS::KeyCode _dir,OIS::KeyCode _acel,OIS::KeyCode _freio,OIS::Keyboard *ir) {
	acel=_acel;
	freio=_freio;
	dir=_dir;
	esq=_esq;
	mInputDevice=ir;

	//CORRIGIR!!!
	//_v->setSteerSpeed(Real(1));

	printf("Cs\n\n");
}

Vector3 controle_Teclado::getDirecao() {
	return _v->getPosition() - posicaoAnt;
}
void controle_Teclado::atualizar(const FrameEvent& evt) {
	posicaoAnt = _v->getPosition();
	_v->setInputs(esquerda(),direita(),acelerar(),freiar());

	/*
	if(acelerar()) {
		SoundManager::getSingletonPtr()->PlaySound(somId, _v->getSceneNode(), &channel,0.1f);
	}
	else if(channel!=INVALID_SOUND_CHANNEL) {
		SoundManager::getSingletonPtr()->StopSound(&channel);
	}
	*/

	_v->showBrakeLights(freiar());
}





void controle_IA::setup(Pista* _pista,RenderWindow *_rWindow) {
		pista = _pista;
		rWindow=_rWindow;
		recover=clock();

		_v->setSteerSpeed(1.0f);

		Setor* cp = _v->getSetorAtual();
		cp_aux = cp;

		distancia= cp->getDistancia();

		//bis1->newPoint(cp->getFrente().v1,cp->getFrente().v2,_v->getPosition());
		//bis2->newPoint(cp->getProxSetor()->getFrente().v1,cp->getProxSetor()->getFrente().v2,_v->getPosition());
}

Vector3 controle_IA::getDirecao() {
		return _v->getSetorAtual()->getFrente().guia - _v->getPosition();
}

void controle_IA::atualizar(const FrameEvent& evt) {
		//DESVIO DE OBSTACULO
		Real distorcao=0;
		relatorioObstaculo resultado = _v->vaiBater();
		if((resultado.colidiu) && (resultado.distancia<45)) {
			//Vector3 tamanho = resultado.obj->getBoundingBox()->getCenter() - resultado.obj->getWorldBoundingBox()->getSize();
			//_v->getSetorAtual()->getDistanciaLateral()
			float distL = Dist(resultado.pos,_v->getSetorAtual()->pontoMedioL());
			float distR = Dist(resultado.pos,_v->getSetorAtual()->pontoMedioR());
			if(distL>distR) {
				distorcao=-0.15*(1 - (resultado.distancia/45));
			}
			else {
				distorcao=0.15*(1 - (resultado.distancia/45));
			}

			//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getSetorAtual()->getDistanciaLateral()));
			//_v->getSetorAtual()->getDistanciaPontoLateral(resultado.distancia);
		}
		Setor* cp = _v->getSetorAtual();
		acelerar=1;

		if(this->getVeiculo()->getNumeroVoltas()<0) {
			recover=clock();
		}
		if(cp->getPos() != cp_aux->getPos()) {
			recover=clock();
			//bis1->newPoint(cp->getFrente().v1,cp->getFrente().v2,_v->getPosition());
			//bis2->newPoint(cp->getProxSetor()->getFrente().v1,cp->getProxSetor()->getFrente().v2,_v->getPosition());
			distancia=cp->getDistancia();
		}
		if(((double)(clock() - recover) / CLOCKS_PER_SEC)>(distancia*0.01)) {
			this->getVeiculo()->getBody()->setLinearVelocity(Vector3(0,0,0));
			this->getVeiculo()->setPosition((0.5*(cp->getFrente().medio+cp->getTras().medio))+Vector3(0,0.5,0));
			this->getVeiculo()->getBody()->setOrientation(Quaternion::IDENTITY*Quaternion(CalcSetorAngle(pista->getPrimeiroSetor(),cp,rWindow),Vector3(0,1,0)));
			recover=clock()+5*CLOCKS_PER_SEC;
		}

		cp_aux = cp;
		//bis1->step();
		//bis2->step();
		const Real steerCP = CalcAngle(_v->getSetorAtual()->getFrente().guia,_v->getPosition(),_v->getOrientation().xAxis(),_v->getOrientation().zAxis(),rWindow);
		const Real steerNextCP = CalcAngle(_v->getSetorAtual()->getProxSetor()->getFrente().guia,_v->getPosition(),_v->getOrientation().xAxis(),_v->getOrientation().zAxis(),rWindow);
		//const Real multiplex = (100*Dist(_v->getPosition(),_v->getSetorAtual()->getFrente().guia)/distancia);
		const Real multiplex = (per_Dist(cp,_v->getPosition())/distancia);


		//SoundManager::getSingletonPtr()->PlaySound(somId, _v->getSceneNode(), &channel,1.0f);

		Real velo = _v->getVelocity();
		if(velo>30) {
			velo=30;
		}

		Real direcao = distorcao + ((steerCP*(multiplex)) + 0.8*(steerNextCP*(1-multiplex)));
		//rWindow->setDebugText(Ogre::StringConverter::toString(direcao-distorcao)+"     "+Ogre::StringConverter::toString(distorcao));

		//Real direcao = (steerCP*multiplex);//+ (steerNextCP*(1-multiplex));

		//Real direcao = (_v->getVelocity()/28)*(0.95*((steerCP*(multiplex)) + (2*steerNextCP*(1-multiplex)) ));

		//Real distanciaRelativa = _v->getSetorAtual()->getDistanciaPontoNoSetor(_v->getPosition());
		//if(((distanciaRelativa/distancia)>0.89) && ((distanciaRelativa/distancia)<0.91))
		if (pista->getDistanciaCpFreio(_v->getPosition(),_v->getIdFreio())>60)
		{
			//deltaS = (Ogre::Math::Pow(pista->getControlPointFreio(_v->getIdFreio()).vel ,2) - Ogre::Math::Pow(_v->getVelocity(),2))/(2*(-9.8));
			//deltaS = cp->getProxSetor()->getTras().angulo.valueDegrees()*(0.5*(_v->getVelocity() - pista->getControlPointFreio(_v->getIdFreio()).vel));
			//deltaS = abs((Ogre::Math::Pow(pista->getControlPointFreio(_v->getIdFreio()).vel ,2) - Ogre::Math::Pow(_v->getVelocity(),2))/(2*(9.80665))); //com base na formula proposta por marquinho
			//deltaS = 55;
			/*deltaV = _v->getVelocity()+2;// + abs(pista->getControlPointFreio(_v->getIdFreio()).vel - _v->getVelocity());
			t =  deltaV / 9.8; //32.1;
			deltaS = (deltaV * t) - (0.5 * 9.8 * Ogre::Math::Pow(t,2));
			deltaS = 60 - deltaS;*/
			Real t = _v->getVelocity() / 32.1;
			deltaS = ((_v->getVelocity()+10) * t) - (0.5 * 32.1 * Ogre::Math::Pow(t,2));
			deltaS = 60 - deltaS;
		}

		/*
		* Quanto menor a distancia,
		*/
		//deltaS = (_v->getVelocity() - pista->getControlPointFreio(_v->getIdFreio()).vel)*cos(cp->getTras().angulo.valueDegrees());
		//deltaS = cp->getProxSetor()->getTras().angulo.valueDegrees()+(0.5*(_v->getVelocity() - pista->getControlPointFreio(_v->getIdFreio()).vel));

		if((pista->getDistanciaCpFreio(_v->getPosition(),_v->getIdFreio())<deltaS)/* || (freiaPorra)*/) {
			//freia

			if (_v->getVelocity()>pista->getControlPointFreio(_v->getIdFreio()).vel) {
				//acelerar = _v->getVelocity()-pista->getControlPointFreio(_v->getIdFreio()).vel
				acelerar=-1.0;
			}
			else {
				acelerar = (0.5*pista->getControlPointFreio(_v->getIdFreio()).vel)/18.f;
				//freiaPorra=0;
			}
		}
		else if(_v->getIdFreio()>_v->getIdAcelerador()) {
			//seta velocidade constante
			acelerar = (0.5*pista->getControlPointFreio(_v->getIdFreio()-1).vel)/18.f;
		}
		else {
			acelerar=1;
		}
		/*
		if(pista->atravessouControlPointAcelerador(_v->getPosition(),_v->getIdAcelerador())) {
			//acelera maximo
			acelerar=1;
			freioSystem=2;
		}
		*/


		//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getVelocity())+"------"+Ogre::StringConverter::toString(pista->getControlPointFreio(_v->getIdFreio()).vel)+"---"+Ogre::StringConverter::toString(deltaS));
		//rWindow->setDebugText(Ogre::StringConverter::toString((controle_freio == 1) || ((_v->getSetorAtual()->getPos() == 3) && ((_v->getSetorAtual()->getDistanciaPontoNoSetor(_v->getPosition()) <= 393)))));
		//rWindow->setDebugText(Ogre::StringConverter::toString(_v->getIdFreio())+"------"+Ogre::StringConverter::toString(_v->getIdAcelerador()));
		//rWindow->setDebugText(Ogre::StringConverter::toString(t)+"---"+Ogre::StringConverter::toString(deltaS));

		_v->setInputs(direcao,acelerar);//+ 1*(steerNNextCP*(1-multiplex))

		//_v->setInputs(direcao,0.f,1.f);

		_v->showBrakeLights(acelerar<0);
}
