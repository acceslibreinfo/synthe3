/*
 * Synthé 3 - A speech synthetizer software for french
 *
 * Copyright (C) 1985-2015 by Michel MOREL <michel.morel@unicaen.fr>.
 *
 * Synthé 3 comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU Lesser General Public License, as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version. Please see the file LICENSE-LGPL for details.
 *
 * Web Page: https://github.com/acceslibreinfo/synthe3
 *
 * This software is maintained by Shérab <Sebastien.Hinderer@ens-lyon.org>.
 */

/////////////////////////////////////////////////////
//	Synthé : fonctions mathématiques
/////////////////////////////////////////////////////

#include <math.h>
#include "SynCalcul.h"

//Fenêtre croissante (sinusoïdale), position x, entre 0 et 1
float fenCroit(float x) {
	if (x<0) x=0;
	else if (x>1) x=1;
	return (1-(float)cos(x*PI))/2;
}

//Fenêtre décroissante (sinusoïdale), position x, entre 1 et 0
float fenDecroit(float x) {
	if (x<0) x=0;
	else if (x>1) x=1;
	return (1+(float)cos(x*PI))/2;
}

