/*
 * Veiculo.cpp
 *
 *  Created on: Jul 15, 2011
 *      Author: aluno
 */
#include "Veiculo.h"

void Veiculo::setControle(Controle *_c)
 {
		_controle = _c;
		_controle->setVeiculo(this);
	}



void Veiculo::step(const FrameEvent& evt,const bool PAUSE)
{
		if(cp->atravessouFrente(_vehicle->getPosition())) {

			if (idFP == 1)
			{
				finishFP = clock();
				tempoFP = (double)(finishFP - startFP) / CLOCKS_PER_SEC;
				this->ptTreino[idTreino-1].foraDaPista += tempoFP;
				idFP = 0;
			}

			Setor* auxcp=cp->getProxSetor();

			finish = clock();
			tempo = (double)(finish - start) / CLOCKS_PER_SEC;

			if (idTreino == 0)
				ptTreino[this->getPista()->getQtdeCheckPoints()-1].tempo = tempo;
			else
				ptTreino[idTreino-1].tempo = tempo;

			tempo = 0.0;
			start = clock();

			this->ptTreino[idTreino].pos = _vehicle->getPosition();
			this->idTreino++;

			if(auxcp->getPos()<1) {
				nVoltas++;

				if (nVoltas != 0) {
					this->RegistrarTreino(this->ptTreino);

				}

				this->idTreino = 0;
			}

			this->ptTreino[idTreino].foraDaPista = 0;

			cp=auxcp;
			atravessouFrenteCP=true;
			atravessouTrasCP=false;
		}
		else if(cp->atravessouTras(_vehicle->getPosition())) {
			cp=cp->getAnteriorSetor();
			atravessouFrenteCP=false;
			atravessouTrasCP=true;
		}

		if (pista->atravessouControlPointFreio(_vehicle->getPosition(),idFreio))
		{
			idFreio++;
			if (idFreio == pista->getQtdeControlPoints())
				idFreio = 0;
		}

		if (((idFreio-1 == idAcelerador)||(idAcelerador==pista->getQtdeControlPoints()-1))&&(pista->atravessouControlPointAcelerador(_vehicle->getPosition(),idAcelerador)))
		{
			idAcelerador++;
			if (idAcelerador == pista->getQtdeControlPoints())
				idAcelerador = 0;
		}

		int ladoD = this->getSetorAtual()->atravessouLateral(_vehicle->getPosition() ,_DIREITA);
		int ladoE = this->getSetorAtual()->atravessouLateral(_vehicle->getPosition() ,_ESQUERDA);
		if(ladoE || ladoD)
		{
			if (idFP == 0)
			{
				tempoFP = 0.0;
				startFP = clock();
				idFP = 1;
			}
		}
		else
		{
			if (idFP == 1)
			{
				finishFP = clock();
				tempoFP = (double)(finishFP - startFP) / CLOCKS_PER_SEC;
				this->ptTreino[idTreino-1].foraDaPista += tempoFP;
				idFP = 0;
			}
		}

		_controle->atualizar(evt);
		_vehicle->update(evt.timeSinceLastFrame);
}


relatorioObstaculo Veiculo::vaiBater() {
	return obstaculos->vaiBater(idObstaculo,this->getControle()->getDirecao());
}
