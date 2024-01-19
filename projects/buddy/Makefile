STUDENT_LASTNAMES = XXXXXXX
PROGNAME = buddy

CC = gcc -std=gnu11
CFLAGS = -Wall -g

####################################################################
#                           IMPORTANT                              #
####################################################################
# Add files to their respective line to get this makefile to build #
# them.                                                            #
####################################################################
# NOTE: The submission scripts assume all files in `CFILES` end with
# .c and all files in `HFILES` end in .h
CFILES = simulator.c buddy.c
HFILES = buddy.h list.h

# Add libraries that need linked as needed (e.g. -lm -lpthread)
LIBS =

ZIPNAME = project3-buddy

DOXYGENCONF = $(PROGNAME).doxygen

OBJFILES = $(patsubst %.c,%.o,$(CFILES))
EXECNAME = $(patsubst %,./%,$(PROGNAME))

RAWC = $(patsubst %.c,%,$(CFILES))
RAWH = $(patsubst %.h,%,$(HFILES))

# Build the buddy executable
$(PROGNAME): $(OBJFILES)
	$(CC) $(CFLAGS) $^ -o $(PROGNAME) $(LIBS)

# Build the documentation and the buddy program
all: doc $(PROGNAME)

# Generic build target for all compilation units. NOTE: Changing a
# header requires you to rebuild the entire project
%.o: %.c $(HFILES)
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

# Build and run the program
test: $(PROGNAME)
	./run_tests.bash -d

# Build the documentation for the project
doc: $(CFILES) $(HFILES) $(DOXYGENCONF) README.md
	doxygen $(DOXYGENCONF)
#	-ln -s doc/html/index.html index.html

# Quickly build a submission then extract it. Useful for testing if
# we will be able to build your project from the code you gave us
testsubmit: submit unsubmit

# Build a safeassign friendly submission of the buddy project
submit: clean
#	Perform renaming copies across the Makefile and all .c and .h
#	files
	cp Makefile Makefile.txt
	$(foreach file, $(RAWH), cp $(file).h $(file)-h.txt;)
	$(foreach file, $(RAWC), cp $(file).c $(file)-c.txt;)

#	Make a temporary directory
	mkdir -p $(STUDENT_LASTNAMES)-$(ZIPNAME)

#	Move all the renamed files into the temporary directory
	mv Makefile.txt $(STUDENT_LASTNAMES)-$(ZIPNAME)
	mv $(HFILES:.h=-h.txt) $(STUDENT_LASTNAMES)-$(ZIPNAME)
	mv $(CFILES:.c=-c.txt) $(STUDENT_LASTNAMES)-$(ZIPNAME)

#	Compress and remove temporary directory
	zip -r $(STUDENT_LASTNAMES)-$(ZIPNAME).zip $(STUDENT_LASTNAMES)-$(ZIPNAME)
	-rm -rf $(STUDENT_LASTNAMES)-$(ZIPNAME)

# We will use this command to extract your submission and build
# it. Check to see if the build works before submitting.
unsubmit: $(STUDENT_LASTNAMES)-$(ZIPNAME).zip
	unzip $< && \
	cd $(STUDENT_LASTNAMES)-$(ZIPNAME) && \
	mv Makefile.txt Makefile && \
	$(foreach file, $(RAWH), mv $(file)-h.txt $(file).h &&) \
	$(foreach file, $(RAWC), mv $(file)-c.txt $(file).c &&) \
	make $(PROGNAME)

# Remove all generated files and directories
clean:
	-rm -rf $(PROGNAME) *.o *~ $(STUDENT_LASTNAMES)-$(ZIPNAME)*

# Remove all generated documentation files and directories
clean-doc:
	-rm -rf doc index.html

.PHONY: all test submit unsubmit testsubmit clean
