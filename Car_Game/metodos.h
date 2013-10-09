/*
 * metodos.h
 *
 *  Created on: Jun 18, 2011
 *      Author: Thiago
 */

#ifndef METODOS_H_
#define METODOS_H_

#include "OgreOde_Core.h"
#include "OgreOde_Prefab.h"
#include "OgreOde_Loader.h"
#include "Pista.h"

#define _VEICULO		9901
#define _DIREITA		9922
#define _ESQUERDA		9923
#define RACE_LENGTH		3

class Setor;

using namespace Ogre;

float Dist(Vector3 A, Vector3 B);
float DistPontoReta(Vector3 ponto,Vector3 retaA,Vector3 retaB);
bool ladoReta(float px,float py,float xa,float ya,float xb,float yb);
float CalcAngle(Vector3 target, Vector3 position, Vector3 XDir, Vector3 ZDir, RenderWindow *mWin);
Radian CalcSetorAngle(Setor* s1, Setor* s2, RenderWindow *mWin);
float per_Dist(Setor* setor,Vector3 pos);
float per_DistLateral(Setor* setor,Vector3 pos);

#endif /* METODOS_H_ */
