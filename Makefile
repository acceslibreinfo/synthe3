library = libsynthe.so
program = synthe

library_source_files = \
  SynCalcul.cpp SynGlobal.cpp SynParle.cpp SynSon.cpp alsa.cpp \
  SynTex.o Synthe.o SynTrans.o SynVoix.o

library_object_files := $(library_source_files:.cpp=.o) 

CPPFLAGS = -DALSA
CXXFLAGS = -g3 -ggdb3 -fPIC -Wall -pthread
LDFLAGS = -L.
LDLIBS = -lasound -lsynthe

.PHONY : all clean distclean

all: $(program)

$(program): main.o $(library)
	$(CC) $(CXXFLAGS) -o $@ -L. -lasound -lsynthe $<

$(library): $(library_object_files)
	$(CXX) -shared $(CPPFLAGS) -lasound -o $@ $^

SynCalcul.o: SynCalcul.cpp SynCalcul.h

alsa.o: alsa.cpp SynSon.h

SynVoix.o: SynVoix.cpp SynVoix.h SynMain.h

SynGlobal.o: SynGlobal.cpp SynGlobal.h

SynSon.o: SynSon.cpp SynSon.h SynGlobal.h

SynParle.o: SynParle.cpp SynParle.h SynSon.h SynMain.h SynCalcul.h SynVoix.h

SynTrans.o: SynTrans.cpp SynTrans.h SynMain.h SynVoix.h

SynTex.o: SynTex.cpp Synthe.h SynMain.h SynParle.h SynTrans.h

Synthe.o: Synthe.cpp Synthe.h SynMain.h SynVoix.h

main.o: main.cpp Synthe.h

clean:
	rm -f *.o

distclean: clean
	rm -f $(LIBRARY)
