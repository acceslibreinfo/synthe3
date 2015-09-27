/*
 * Synthé 3 - A speech synthetizer software for french
 *
 * Copyright (C) 1985-2015 by Michel MOREL <michel.morel@unicaen.fr>.
 *
 * Synthé 3 comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: https://github.com/acceslibreinfo/synthe3
 *
 * This software is maintained by Shérab <Sebastien.Hinderer@ens-lyon.org>.
 */

/* main.cpp: command-line test program */

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "Synthe.h"

int main( int argc, char** argv )
{
	initSynthe();

	//Prononce une phrase avec les paramètres par défaut (à -1) : volume, débit, hauteur, phonétique, modeLec, compta
	//puis 0 (pas de son) puis 1 (sortie wave), puis "essai.wav"
	//	synTexte("Ceci est une phrase que je prononce correctement.", -1, -1, -1, -1, -1, -1, 0, 1, "/tmp/essaiSynthe1.wav");
	//	synTexte("Ceci est une phrase que je prononce correctement.", -1, -1, -1, -1, -1, -1, 0, 1);
		synTexte("Ceci est une phrase que je prononce correctement.", -1, -1, -1, -1, -1, -1, 1, 0);
	while(synIndex()>0);	//attend la fin de la phrase (sinon coupe)
	/*	synTexte("Et la suivante.", -1, -1, -1, -1, -1, -1, 0, 1, "/tmp/essaiSynthe2.wav");
		while(synIndex()>0);	//attend la fin de la phrase (sinon coupe)
	synTexte("À l'école, les élèves écoutent la leçon donnée par le maître.", -1, -1, -1, -1, -1, -1, 0, 1, "/tmp/essaiSynthe3.wav");
	while(synIndex()>0);	//attend la fin de la phrase (sinon coupe)
	*/
		
	quitteSynthe();

	exit(0);
}
