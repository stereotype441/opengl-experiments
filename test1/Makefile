LIBS = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm
LIB_SRCS = lwo_parser.cpp mesh.cpp pmf.cpp gpu_wrappers.cpp file_tools.cpp
GLSL_SRCS = vertex.glsl fragment.glsl
SRCS = $(LIB_SRCS) test.cpp

# See http://mad-scientist.net/make/autodep.html
MAKEDEPEND = $(CPP) $(CPPFLAGS) $< \
               | sed -n 's/^\# *[0-9][0-9]* *"\([^"<]*\)".*/$@: \1/p' \
               | sort -u > $*.d


default: test

-include $(SRCS:.cpp=.P)

.PHONY: all clean default

all: default elfegab.pmf-dump

%.glsl-o: %.glsl
	ld -s -r -o $@ -b binary $<

elfegab.pmf-o: elfegab.pmf
	ld -s -r -o $@ -b binary $<

elfegab.pmf-dump: elfegab.pmf dump_pmf
	./dump_pmf > $@

%.o: %.cpp
	@$(MAKEDEPEND); \
	cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	    -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d
	$(CXX) -g -c $<

test: $(LIB_SRCS:.cpp=.o) test.cpp $(GLSL_SRCS:.glsl=.glsl-o) elfegab.pmf-o
	$(CXX) -g $^ -Wall $(LIBS) -o $@

build_tool: $(LIB_SRCS:.cpp=.o) build_tool.cpp
	$(CXX) -g $^ -Wall $(LIBS) -o $@

dump_pmf: $(LIB_SRCS:.cpp=.o) dump_pmf.cpp
	$(CXX) -g $^ -Wall $(LIBS) -o $@

elfegab.pmf: ../models/elfegab.lwo build_tool
	./build_tool

clean:
	rm -f *.o *.glsl-o *.pmf-o *.pmf-dump *.P test build_tool dump_pmf
