# Makefile pour synthé

EXE = libsynthe.so
#SYNTHE_VOIX = $(HOME)/opt/synthe/Michel.seg
#SYNTHE_TAB = $(HOME)/opt/synthe/Synthe.tab
OPT = -DALSA -g3 -ggdb3 -fPIC -Wall -pthread
GCC = g++

# Binaire synthé
all: $(EXE)

# SynCalcul
SynCalcul.o: SynCalcul.cpp SynCalcul.h
	$(GCC) $(OPT) -c $< -o $@

# alsa
alsa.o: alsa.cpp SynSon.h
	$(GCC) $(OPT) -c $< -o $@

# SynVoix
SynVoix.o: SynVoix.cpp SynVoix.h SynMain.h
	$(GCC) $(OPT) -c $< -o $@

# SynGlobal
SynGlobal.o: SynGlobal.cpp SynGlobal.h
	$(GCC) $(OPT) -c $< -o $@

# SynSon
SynSon.o: SynSon.cpp SynSon.h SynGlobal.h
	$(GCC) $(OPT) -c $< -o $@

# SynParle
SynParle.o: SynParle.cpp SynParle.h SynSon.h SynMain.h SynCalcul.h SynVoix.h
	$(GCC) $(OPT) -c $< -o $@

# SynTrans
SynTrans.o: SynTrans.cpp SynTrans.h SynMain.h SynVoix.h
	$(GCC) $(OPT) -c $< -o $@

# SynTex
SynTex.o: SynTex.cpp Synthe.h SynMain.h SynParle.h SynTrans.h
	$(GCC) $(OPT) -c $< -o $@

# Synthe
Synthe.o: Synthe.cpp Synthe.h SynMain.h SynVoix.h
	$(GCC) $(OPT) -c $< -o $@

# Bibliothèque finale
libsynthe.so: SynCalcul.o SynGlobal.o SynParle.o SynSon.o alsa.o SynTex.o Synthe.o SynTrans.o SynVoix.o
	$(GCC) -shared $(OPT) -lasound -o $(EXE) SynCalcul.o SynGlobal.o SynParle.o SynSon.o alsa.o SynTex.o Synthe.o SynTrans.o SynVoix.o

# Main de test
main.o: main.cpp Synthe.h
	$(GCC) $(OPT) -c $< -o $@

main: main.o
	$(GCC) $(OPT) main.o -lasound -L. -lsynthe -o syntest

# Client console pour faire prononcer du texte
synthe_cmd.o: synthe_cmd.cpp Synthe.h
	$(GCC) $(OPT) -c $< -o $@

synthe_cmd: synthe_cmd.o
	$(GCC) $(OPT) synthe_cmd.o -lasound -L -lsynthe -o synthe

# Suppression des fichiers objets
clean:
	rm -f *.o

# Suppression des fichiers objets et des exécutables
distclean: clean
	rm -f $(EXE)
