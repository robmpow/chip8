CXX:= g++
CXX_VERSION:= c++14
SRCDIR:= src
BUILDDIR:= build
TARGETDIR:= bin
TESTDIR := test
TARGET:= chip8
TEST_TARGET := chip8_test

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TEST_SOURCES := test/chip8_test.cpp src/chip8.cpp src/chip8_util.cpp src/logger_impl.cpp src/logger.cpp
TEST_OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(patsubst $(TESTDIR)/%,$(BUILDDIR)/%,$(TEST_SOURCES:.$(SRCEXT)=.o)))
override CXX_FLAGS += -Wall -Werror -pedantic
LIB := -lSDL2_ttf
SDL_LIBS := $(shell sdl2-config --libs)
TEST_LIBS := -lboost_unit_test_framework

$(TARGETDIR)/$(TARGET): $(OBJECTS) 
	@echo " Linking..."
	@mkdir -p $(TARGETDIR)
	@echo " $(CXX) -std=$(CXX_VERSION) $^ -o $(TARGETDIR)/$(TARGET) $(SDL_LIBS) $(LIB)"; $(CXX) -std=$(CXX_VERSION) $^ -o $(TARGETDIR)/$(TARGET) $(SDL_LIBS) $(LIB)

$(TARGETDIR)/$(TEST_TARGET): $(TEST_OBJECTS)
	@echo " Linking..."
	@mkdir -p $(TARGETDIR)
	@echo " $(CXX) -std=$(CXX_VERSION) $^ -o $(TARGETDIR)/$(TARGET) $(SDL_LIBS) $(LIB)"; $(CXX) -std=$(CXX_VERSION) $^ -o $(TARGETDIR)/$(TEST_TARGET) $(SDL_LIBS) $(LIB) $(TEST_LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CXX) -std=$(CXX_VERSION) $(CXX_FLAGS) $(INC) -c -o $@ $<"; $(CXX) -std=$(CXX_VERSION) $(CXX_FLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CXX) -std=$(CXX_VERSION) $(CXX_FLAGS) $(INC) -c -o $@ $<"; $(CXX) -std=$(CXX_VERSION) $(CXX_FLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGETDIR)/$(TARGET) $(TARGETDIR)/$(TEST_TARGET)"; $(RM) -r $(BUILDDIR) $(TARGETDIR)/$(TARGET) $(TARGETDIR)/$(TEST_TARGET)

test: CXX_FLAGS := $(CXX_FLAGS) -ggdb
test: $(TARGETDIR)/$(TEST_TARGET)

debug: CXX_FLAGS := $(CXX_FLAGS) -ggdb
debug: clean
debug: $(TARGETDIR)/$(TARGET)

.PHONY: clean test debug
