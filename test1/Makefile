# TODO: dependencies

TARGET = test
LIBS = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm
SRCS = test.cpp generated.cpp
GLSL_SRCS = vertex.glsl

default: $(TARGET)

.PHONY: all clean

all: default

generated.h generated.cpp: code_gen.py
	python code_gen.py

%.glsl-o: %.glsl
	ld -s -r -o $@ -b binary $<

test.cpp: generated.h

%.o: %.cpp
	$(CXX) -g -c $<

$(TARGET): $(SRCS:.cpp=.o) $(GLSL_SRCS:.glsl=.glsl-o)
	$(CXX) -g $^ -Wall $(LIBS) -o $@

clean:
	rm -f *.o $(TARGET) generated.*
