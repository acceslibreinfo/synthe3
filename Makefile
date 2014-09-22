# Synthé 3 - A speech synthetizer software for french
#
# Copyright (C) 1985-2014 by Michel MOREL <michel.morel@unicaen.fr>.
#
# Synthé 3 comes with ABSOLUTELY NO WARRANTY.
#
# This is free software, placed under the terms of the
# GNU Lesser General Public License, as published by the Free Software
# Foundation; either version 2.1 of the License, or (at your option) any
# later version. Please see the file LICENSE-LGPL for details.
#
# Web Page: https://github.com/acceslibreinfo/synthe3
#
# This software is maintained by Shérab <Sebastien.Hinderer@ens-lyon.org>.

library = synthe
library_extension = so
library_name = lib$(library).$(library_extension) 
program = synthe

library_source_files = \
  SynCalcul.cpp SynGlobal.cpp SynParle.cpp SynSon.cpp alsa.cpp \
  SynTex.cpp Synthe.cpp SynTrans.cpp SynVoix.cpp

library_object_files := $(library_source_files:.cpp=.o) 

CPPFLAGS = -DALSA
CXXFLAGS = -g3 -ggdb3 -fPIC -Wall -pthread
LDFLAGS = -L.
LDLIBS = -lasound -l$(library)

.PHONY : all clean distclean

all: $(program)

$(program): main.o $(library_name)
	$(CC) $(CXXFLAGS) -o $@ -L. -lasound -l$(library) $<

$(library_name): $(library_object_files)
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
	rm -f $(library_name) $(program)
