CXX := g++
CXXFLAGS := -W -Wall
SRCDIR := src/
SUBDIRS := net/ core/ parser/ mesh/
SRCS := $(wildcard $(addprefix $(SRCDIR), $(addsuffix *.cpp,$(SUBDIRS))))
OBJDIR := obj/
OBJS := $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cpp,%.o,$(SRCS)))
LIBS := -lboost_thread

all: ipovermesh

ipovermesh: $(OBJS)
	@echo "Building target $@"
	@$(CXX) $(LIBS) $(SRCDIR)main.cpp -o"$@" $^
	@echo "Target built"

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo "Compiled $(notdir $<)"

clean:
	@-rm -R $(OBJDIR)
	@echo "Repository cleaned"

mrproper: clean
	@-rm ipovermesh

.PHONY: all clean mrproper
