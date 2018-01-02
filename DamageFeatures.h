#pragma once

#ifndef DAMAGE_FEATURES_H_
#define DAMAGE_FEATURES_H_

struct DamageFeatures {
	double damageVsT;
	double damageVsI;
	double damageVsF;
	double damageVsH;
	double damageVsA;
	double defenceFromT;
	double defenceFromI;
	double defenceFromF;
	double defenceFromH;

	// �������� ����������� ����������� � ����������!!!

	DamageFeatures();
	void refresh();
};

#endif