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

	// Õ¿œ»—¿“‹  ŒÕ—“–” “Œ–  Œœ»–Œ¬¿Õ»ﬂ » œ–»—¬Œ≈Õ»ﬂ!!!

	DamageFeatures();
	void refresh();
};

#endif