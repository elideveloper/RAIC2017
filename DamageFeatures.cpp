#include "DamageFeatures.h"

DamageFeatures::DamageFeatures()
	: damageVsT(0.0),
		damageVsI(0.0),
		damageVsF(0.0),
		damageVsH(0.0),
		damageVsA(0.0),
		defenceFromT(0.0),
		defenceFromI(0.0),
		defenceFromF(0.0),
		defenceFromH(0.0)
{
}

void DamageFeatures::refresh()
{
	this->damageVsT = 0.0;
	this->damageVsI = 0.0;
	this->damageVsF = 0.0;
	this->damageVsH = 0.0;
	this->damageVsA = 0.0;
	this->defenceFromT = 0.0;
	this->defenceFromI = 0.0;
	this->defenceFromF = 0.0;
	this->defenceFromH = 0.0;
}
