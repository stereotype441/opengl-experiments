TARGET = test
LIBS = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm
SRCS = test.cpp lwo_parser.cpp mesh.cpp
GLSL_SRCS = vertex.glsl fragment.glsl

# See http://mad-scientist.net/make/autodep.html
MAKEDEPEND = $(CPP) $(CPPFLAGS) $< \
               | sed -n 's/^\# *[0-9][0-9]* *"\([^"<]*\)".*/$@: \1/p' \
               | sort -u > $*.d


default: $(TARGET)

-include $(SRCS:.cpp=.P)

.PHONY: all clean

all: default

%.glsl-o: %.glsl
	ld -s -r -o $@ -b binary $<

elfegab.lwo-o: ../models/elfegab.lwo
	ld -s -r -o $@ -b binary $<

%.o: %.cpp
	@$(MAKEDEPEND); \
	cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	    -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d
	$(CXX) -g -c $<

$(TARGET): $(SRCS:.cpp=.o) $(GLSL_SRCS:.glsl=.glsl-o) elfegab.lwo-o
	$(CXX) -g $^ -Wall $(LIBS) -o $@

clean:
	rm -f *.o *.glsl-o *.lwo-o *.P $(TARGET)
