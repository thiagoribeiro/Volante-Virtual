/*
 * metodos.cpp
 *
 *  Created on: Jun 18, 2011
 *      Author: Thiago
 */
#include <math.h>
#include "OgreOde_Core.h"
#include "OgreOde_Prefab.h"
#include "OgreOde_Loader.h"
#include "Pista.h"

using namespace Ogre;

float eq_reta_perpendicular(float px,float py,float xa,float ya,float xb,float yb) {
	return (1/((ya-yb)/(xb-xa)))*px - ((xa*yb-xb*ya)/(xb-xa)) - py;
}
float eq_reta(float px,float py,float xa,float ya,float xb,float yb) {
  return (px*(ya-yb)+py*(xb-xa)+(xa*yb-xb*ya));
}
bool ladoReta(float px,float py,float xa,float ya,float xb,float yb) {
	return eq_reta(px,py,xa,ya,xb,yb)>0;
}
float steering(float ax,float ay,float bx,float by,float cx,float cy) {
	//return ((ay-by)/(ax-bx)-(by-cy)/(bx-cx))*0.01;
	float resultado = ((ay-by)/(ax-bx)-(by-cy)/(bx-cx))*0.01;
	float absoluto = abs(resultado);
	bool negativo = absoluto>resultado;

	if(negativo) {
		absoluto*=-1;
	}

	if (absoluto>0.1 || absoluto<-0.1)
		return absoluto;
	else
		return 0;
}

float Dist(Vector3 A, Vector3 B)
{
	return Ogre::Math::Sqrt(Ogre::Math::Sqr(B.x-A.x) + Ogre::Math::Sqr(B.z-A.z));
}

float DistPontoReta(Vector3 ponto,Vector3 retaA,Vector3 retaB) {
	float dist1 = Dist(ponto,retaA);
	float dist2 = Dist(ponto,retaB);
	if(dist1<dist2) {
		return dist1;
	}
	else {
		return dist2;
	}
}

float per_Dist(Setor* setor,Vector3 pos) {
	Vector3 aux=setor->getFrente().medio - pos;
	aux.y=0.0f;
	return aux.dotProduct(setor->getVetorDirecao());
}
float per_DistLateral(Setor* setor,Vector3 pos) {
	const Vector3 pMedioV1 = (setor->getTras().v1+setor->getFrente().v1)*0.5;
	//Vector3 aux= pMedioV1- pos ;
	//aux.y=0.0f;
	//return aux.dotProduct(pMedioV1.perpendicular());
	return Dist(pMedioV1,pos);
}

float aiClamp(float fValue,float min,float max)
{
	if(fValue<min)
		return min;
	else if(fValue>max)
		return max;
	else
		return fValue;
}

float CalcAngle(Vector3 target, Vector3 position, Vector3 XDir, Vector3 ZDir, RenderWindow *mWin)
{
	// Calc turn angle to reach the target.
	Vector3 TargetHeading = target - position;

	float fX=TargetHeading.dotProduct(XDir);
	float fZ=TargetHeading.dotProduct(-ZDir);

	float fAngle=atan2f(fX,fZ);

	float fSteering;
	if(fAngle>0) {
		fSteering=fAngle-3.14;
	}
	else {
		fSteering=fAngle+3.14;
	}
	//mWin->setDebugText(Ogre::StringConverter::toString((float)(fSteering*0.7)));
//		fSteering = -fAngle/10; ///.35f;
		//fSteering = aiClamp(fSteering, -1.f, 1.f);
	return aiClamp(fSteering,-1.f,1.f);
}
Radian CalcSetorAngle(Setor* s1, Setor* s2, RenderWindow *mWin)
{
	Real dProduto = s1->getTras().vetor.dotProduct(s2->getTras().vetor);
	Real modulo = s1->getTras().modulo*s2->getTras().modulo;

	return Ogre::Math::ACos(dProduto/modulo);
}
