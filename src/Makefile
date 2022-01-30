CC = gcc
CXX = g++
CXXFLAGS = -std=c++2a -fPIC
RENDERFLAGS= -lGLEW -lOpenGL -lglfw -lstudxml
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -D_GLIBCXX_DEBUG
else
CXXFLAGS += -O3 -march=native -Wall
endif

default:	help

all:	RayTracing.exe

RayTracing.exe:	main.cpp shader.hpp shader.cpp camera.hpp objects.hpp objects.cpp OpenGLcontext.cpp OpenGLcontext.hpp
	$(CXX) $(CXXFLAGS) -o RayTracing.exe main.cpp shader.cpp objects.cpp OpenGLcontext.cpp $(RENDERFLAGS) 

help: 
	@echo "Available targets : "
	@echo "    all      		 : compile all executables"
	@echo "    RayTracing.exe          : compile the RayTracing executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"

clean:
	@rm -f *.exe *~ 

