TARGET = test
LIBS = -lglut -lGLU -lGL -lXmu -lXext -lXi -lX11 -lm
SRCS = test.cpp

default: $(TARGET)

.PHONY: all clean

all: default

.o:
	$(CXX) -c $<

$(TARGET): $(SRCS:.cpp=.o)
	$(CXX) $< -Wall $(LIBS) -o $@

clean:
	rm -f $.o $(TARGET)
