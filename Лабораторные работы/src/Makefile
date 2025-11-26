CXX = g++
CXXFLAGS = -std=c++17 -O2 -g -Wall -Wextra -Wpedantic
INCLUDES = -I. -Iabilities -Iadditional -IcontrolGame -Icore \
           -IcontrolGame/controllerGame -IcontrolGame/controllerGame/console \
           -IcontrolGame/controllerGame/GUI
LIBS = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

SRC_DIRS = abilities additional controlGame core \
           controlGame/controllerGame controlGame/controllerGame/console \
           controlGame/controllerGame/GUI

SRCS = $(wildcard *.cpp) \
       $(wildcard abilities/*.cpp) \
       $(wildcard additional/*.cpp) \
       $(wildcard controlGame/*.cpp) \
       $(wildcard controlGame/controllerGame/*.cpp) \
       $(wildcard controlGame/controllerGame/console/*.cpp) \
       $(wildcard controlGame/controllerGame/GUI/*.cpp) \
       $(wildcard core/*.cpp)

OBJS = $(SRCS:.cpp=.o)

TARGET = battleship

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild