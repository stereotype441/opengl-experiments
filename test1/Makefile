# TODO: dependencies

TARGET = test
LIBS = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm
SRCS = test.cpp lwo_parser.cpp mesh.cpp
GLSL_SRCS = vertex.glsl

default: $(TARGET)

.PHONY: all clean

all: default

%.glsl-o: %.glsl
	ld -s -r -o $@ -b binary $<

elfegab.lwo-o: ../models/elfegab.lwo
	ld -s -r -o $@ -b binary $<

test.cpp: mesh.h lwo_parser.h vectors.h
mesh.cpp: mesh.h
lwo_parser.cpp: vectors.h lwo_parser.h

%.o: %.cpp
	$(CXX) -g -c $<

$(TARGET): $(SRCS:.cpp=.o) $(GLSL_SRCS:.glsl=.glsl-o) elfegab.lwo-o
	$(CXX) -g $^ -Wall $(LIBS) -o $@

clean:
	rm -f *.o *.glsl-o *.lwo-o $(TARGET)
