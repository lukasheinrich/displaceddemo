# Makefile for ATLAS prompt and displaced searches
PYTHIA8=/code/pythia8215
PYTHIAINC=$(PYTHIA8)/include/
PYTHIALIB=$(PYTHIA8)/lib/
FASTJET3=/usr/local/lib
FJINC=$(FASTJET3)/include
FJLIB=$(FASTJET3)/lib
OBJECTS=displacedRecoVertex.o ToyDetector-ATLAS.o
# OBJECTS=trackStudies.o ToyDetector-ATLAS.o
# OBJECTS=crossSectionsDGS.o
INCLUDE=-I$(PYTHIAINC) -I./ -I$(FJINC)

displacedRecoVertex: $(OBJECTS)
	$(CXX) -std=c++11 -o displacedRecoVertex $(OBJECTS) -L$(PYTHIALIB) -lpythia8 \
	-L$(FJLIB) -lfastjet
# crossSectionsDGS: $(OBJECTS)
# 	$(CXX) -o crossSectionsDGS $(OBJECTS) -L$(PYTHIALIB) -lpythia8 \
# 	-L$(FJLIB) -lfastjet
%.o: %.cc
	$(CXX) -std=c++11 -c $^ $(INCLUDE)