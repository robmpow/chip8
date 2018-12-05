CXX:= g++
SRCDIR:= src
BUILDDIR:= build
TARGET:= bin/chip8

SRCEXT := cpp
#SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
SOURCES := src/main.cpp
OBJECTS := build/main.o #$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CXXFLAGS := -Wall -Werror
LIB := -lX11 -lGL -lXrender -lGLEW

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CXX) $^ -o $(TARGET) $(LIB)"; $(CXX) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<"; $(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

debug: clean
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: clean
