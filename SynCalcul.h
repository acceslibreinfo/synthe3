/*
 * Synthé 3 - A speech synthetizer software for french
 *
 * Copyright (C) 1985-2014 by Michel MOREL <michel.morel@unicaen.fr>.
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

#ifndef SYN_CALCUL_H_
#define SYN_CALCUL_H_

////////////////////////////////////////////////////
// Synthé : fonctions mathématiques
////////////////////////////////////////////////////

//Constantes : valeurs de référence
#define VOLUME_REF 10	//devrait être 15, mais on le pousse un peu
#define HAUTEUR_REF 6
#define DEBIT_REF 3

//Constantes mathématiques
const double PI=3.14159;

//Fonctions
extern float fenCroit(float x);
extern float fenDecroit(float x);

#endif
