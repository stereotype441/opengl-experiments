# TODO: dependencies

TARGET = test
LIBS = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm
SRCS = test.cpp generated.cpp

default: $(TARGET)

.PHONY: all clean

all: default

generated.h generated.cpp: code_gen.py
	python code_gen.py

test.cpp: generated.h

.o:
	$(CXX) -c $<

$(TARGET): $(SRCS:.cpp=.o)
	$(CXX) $^ -Wall $(LIBS) -o $@

clean:
	rm -f $.o $(TARGET) generated.*
