# Makefile pour synthé

EXE = libsynthe.so
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin
#SYNTHE_VOIX = $(HOME)/opt/synthe/Michel.seg
#SYNTHE_TAB = $(HOME)/opt/synthe/Synthe.tab
OPT = -DALSA -g3 -ggdb3 -fPIC -Wall -pthread
GCC = g++

# Binaire synthé
all: $(EXE)

# SynCalcul
$(OBJ_DIR)/SynCalcul.o: $(SRC_DIR)/SynCalcul.cpp $(SRC_DIR)/SynCalcul.h
	$(GCC) $(OPT) -c $< -o $@

# alsa
$(OBJ_DIR)/alsa.o: $(SRC_DIR)/alsa.cpp $(SRC_DIR)/SynSon.h
	$(GCC) $(OPT) -c $< -o $@

# SynVoix
$(OBJ_DIR)/SynVoix.o: $(SRC_DIR)/SynVoix.cpp $(SRC_DIR)/SynVoix.h $(SRC_DIR)/SynMain.h
	$(GCC) $(OPT) -c $< -o $@

# SynGlobal
$(OBJ_DIR)/SynGlobal.o: $(SRC_DIR)/SynGlobal.cpp $(SRC_DIR)/SynGlobal.h
	$(GCC) $(OPT) -c $< -o $@

# SynSon
$(OBJ_DIR)/SynSon.o: $(SRC_DIR)/SynSon.cpp $(SRC_DIR)/SynSon.h $(SRC_DIR)/SynGlobal.h
	$(GCC) $(OPT) -c $< -o $@

# SynParle
$(OBJ_DIR)/SynParle.o: $(SRC_DIR)/SynParle.cpp $(SRC_DIR)/SynParle.h $(SRC_DIR)/SynSon.h $(SRC_DIR)/SynMain.h $(SRC_DIR)/SynCalcul.h $(SRC_DIR)/SynVoix.h
	$(GCC) $(OPT) -c $< -o $@

# SynTrans
$(OBJ_DIR)/SynTrans.o: $(SRC_DIR)/SynTrans.cpp $(SRC_DIR)/SynTrans.h $(SRC_DIR)/SynMain.h $(SRC_DIR)/SynVoix.h
	$(GCC) $(OPT) -c $< -o $@

# SynTex
$(OBJ_DIR)/SynTex.o: $(SRC_DIR)/SynTex.cpp $(SRC_DIR)/Synthe.h $(SRC_DIR)/SynMain.h $(SRC_DIR)/SynParle.h $(SRC_DIR)/SynTrans.h
	$(GCC) $(OPT) -c $< -o $@

# Synthe
$(OBJ_DIR)/Synthe.o: $(SRC_DIR)/Synthe.cpp $(SRC_DIR)/Synthe.h $(SRC_DIR)/SynMain.h $(SRC_DIR)/SynVoix.h
	$(GCC) $(OPT) -c $< -o $@

# Bibliothèque finale
libsynthe.so: $(OBJ_DIR)/SynCalcul.o $(OBJ_DIR)/SynGlobal.o $(OBJ_DIR)/SynParle.o $(OBJ_DIR)/SynSon.o $(OBJ_DIR)/alsa.o $(OBJ_DIR)/SynTex.o $(OBJ_DIR)/Synthe.o $(OBJ_DIR)/SynTrans.o $(OBJ_DIR)/SynVoix.o
	$(GCC) -shared $(OPT) -lasound -o $(BIN_DIR)/$(EXE) $(OBJ_DIR)/SynCalcul.o $(OBJ_DIR)/SynGlobal.o $(OBJ_DIR)/SynParle.o $(OBJ_DIR)/SynSon.o $(OBJ_DIR)/alsa.o $(OBJ_DIR)/SynTex.o $(OBJ_DIR)/Synthe.o $(OBJ_DIR)/SynTrans.o $(OBJ_DIR)/SynVoix.o

# Main de test
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/Synthe.h
	$(GCC) $(OPT) -c $< -o $@

main: $(OBJ_DIR)/main.o
	$(GCC) $(OPT) $(OBJ_DIR)/main.o -lasound -L$(BIN_DIR)/ -lsynthe -o $(BIN_DIR)/syntest

# Client console pour faire prononcer du texte
$(OBJ_DIR)/synthe_cmd.o: $(SRC_DIR)/synthe_cmd.cpp $(SRC_DIR)/Synthe.h
	$(GCC) $(OPT) -c $< -o $@

synthe_cmd: $(OBJ_DIR)/synthe_cmd.o
	$(GCC) $(OPT) $(OBJ_DIR)/synthe_cmd.o -lasound -L$(BIN_DIR)/ -lsynthe -o $(BIN_DIR)/synthe

# Suppression des fichiers objets
clean:
	rm -f $(OBJ_DIR)/*.o

# Suppression des fichiers objets et des exécutables
distclean: clean
	rm -f $(BIN_DIR)/$(EXE)