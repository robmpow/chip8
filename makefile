CXX:= g++
CXX_VERSION:= c++14
SRCDIR:= src
BUILDDIR:= build
TARGET:= bin/chip8

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
#SOURCES := src/main.cpp src/renderer.cpp src/chip8.cpp src/chip8_util.cpp
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CXXFLAGS := -Wall -Werror
LIB := -lX11 -lGL -lXrender -lGLEW

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CXX) -std=$(CXX_VERSION) $^ -o $(TARGET) $(LIB)"; $(CXX) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CXX) -std=$(CXX_VERSION) $(CXXFLAGS) $(INC) -c -o $@ $<"; $(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

debug: clean
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: clean
