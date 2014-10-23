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

//Classe Transcription
class Transcription {
private:
	char* texteLec;
	char* texteEcr;
	short iLec;	//lecture
	short iEcr;	//écriture
	unsigned char carac;	//carac alpha
	unsigned char categ;	//sa catégorie
	short ctBava;
	char liaison;
	char* ptArbre;
	char nombre[12];

public:
	void phonemePhoneme(char* texte, char* texPhon);
	void minMajNFois(char* texteAlphaLec, char* texteAlphaEcr);
	void graphemePhoneme(char* texte, char* texPhon);
	void traiteNombre();
	void phonChif(char* ptChif);	//Phonétise le chiffre
	void finNombre(unsigned short int& iN);
	unsigned char carSuiv();
	void ecritPhon (char c);
	void ecrit (char c);
};
