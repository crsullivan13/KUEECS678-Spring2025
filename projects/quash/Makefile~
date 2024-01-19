STUDENTID = XXXXXXX
PROGNAME = quash

CC = gcc --std=gnu11
CFLAGS = -Wall -g


####################################################################
#                           IMPORTANT                              #
####################################################################
# Add files to their respective line to get this makefile to build #
# them.                                                            #
####################################################################
# NOTE: The submission scripts assume all files in `CFILELIST` end with
# .c and all files in `HFILES` end in .h
CFILELIST = quash.c command.c execute.c parsing/memory_pool.c parsing/parsing_interface.c parsing/parse.tab.c parsing/lex.yy.c
HFILELIST = quash.h command.h execute.h parsing/memory_pool.h parsing/parsing_interface.h parsing/parse.tab.h deque.h debug.h

# Add libraries that need linked as needed (e.g. -lm -lpthread)
LIBLIST =

# Include locations
INCLIST = ./src ./src/parsing

# Doxygen configuration file
DOXYGENCONF = quash.doxygen

####################################################################

SRCDIR = ./src/
OBJDIR = ./obj/

EXECNAME = $(patsubst %,./%,$(PROGNAME))

CFILES = $(patsubst %,$(SRCDIR)%,$(CFILELIST))
HFILES = $(patsubst %,$(SRCDIR)%,$(HFILELIST))
OFILES = $(patsubst %.c,$(OBJDIR)%.o,$(CFILELIST))

RAWC = $(patsubst %.c,%,$(addprefix $(SRCDIR), $(CFILELIST)))
RAWH = $(patsubst %.h,%,$(addprefix $(SRCDIR), $(HFILELIST)))

INCDIRS = $(patsubst %,-I%,$(INCLIST))

OBJINNERDIRS = $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(shell find $(SRCDIR) -type d))
SUBMISSIONDIRS = $(addprefix $(STUDENTID)-project1-quash/,$(shell find $(SRCDIR) -type d))

# Build the the quash executable
all: $(OBJINNERDIRS) $(PROGNAME)

debug: CFLAGS += -DDEBUG -gdwarf-2
debug: all

# Build the object directories
$(OBJINNERDIRS):
	$(foreach dir, $(OBJINNERDIRS), mkdir -p $(dir);)

# Build the quash program
$(PROGNAME): $(OFILES)
	$(CC) $(CFLAGS) $^ -o $(PROGNAME) $(LIBLIST)

# Generic build target for all compilation units. NOTE: Changing a
# header requires you to rebuild the entire project
$(OBJDIR)%.o: $(SRCDIR)%.c $(HFILES)
	$(CC) $(CFLAGS) -c $(INCDIRS) -o $@ $< $(LIBS)

%lex.yy.c: %parse.l
	lex -o $@ $<

%.tab.c %.tab.h: %.y
	bison -t --verbose --defines=$(dir $@)parse.tab.h -o $(dir $@)parse.tab.c $<

# Build and run the program
test: all
	./run_tests.bash -p

# Build the documentation for the project
doc: $(CFILES) $(HFILES) $(DOXYGENCONF) README.md
	doxygen $(DOXYGENCONF)

# Quickly build a submission then extract it. Useful for testing if
# we will be able to build your project from the code you gave us
testsubmit: submit unsubmit

# Build a safeassign friendly submission of the quash project
submit: clean src/parsing/parse.tab.c src/parsing/parse.tab.h src/parsing/lex.yy.c
#	Perform renaming copies across the Makefile and all .c and .h
#	files
	cp Makefile Makefile.txt
	$(foreach file, $(RAWH), cp $(file).h $(file)-h.txt;)
	$(foreach file, $(RAWC), cp $(file).c $(file)-c.txt;)

#	Make a temporary directory
	mkdir -p $(STUDENTID)-project1-quash
	$(foreach dir, $(SUBMISSIONDIRS), mkdir -p $(dir);)

#	Move all the renamed files into the temporary directory
	mv Makefile.txt $(STUDENTID)-project1-quash
	$(foreach file, $(RAWH), mv $(file)-h.txt $(STUDENTID)-project1-quash/$(file)-h.txt &&) \
	$(foreach file, $(RAWC), mv $(file)-c.txt $(STUDENTID)-project1-quash/$(file)-c.txt &&) \
	zip -r $(STUDENTID)-project1-quash.zip $(STUDENTID)-project1-quash # Create submission zip
	-rm -rf $(STUDENTID)-project1-quash # Remove temporary directory

# We will use this command to extract your submission and build
# it. Check to see if the build works before submitting.
unsubmit: $(STUDENTID)-project1-quash.zip
	unzip $< && \
	cd $(STUDENTID)-project1-quash && \
	mv Makefile.txt Makefile && \
	$(foreach file, $(RAWH), mv $(file)-h.txt $(file).h &&) \
	$(foreach file, $(RAWC), mv $(file)-c.txt $(file).c &&) \
	make

# Remove all generated files and directories
clean:
	-rm -rf $(PROGNAME) obj sandbox *~ $(STUDENTID)-project1-quash* src/parsing/parse.output valgrind_report.txt output_report.txt

deep-clean: clean
	-rm -rf doc src/parsing/parse.tab.c src/parsing/parse.tab.h src/parsing/lex.yy.c

%.c: %.y
%.c: %.l

.PHONY: all debug test submit unsubmit testsubmit doc clean deep-clean
