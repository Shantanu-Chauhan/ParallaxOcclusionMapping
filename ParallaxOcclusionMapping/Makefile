########################################################################
# Makefile for Linux
#

ifeq ($(v), tr)
    VFLAG := -DTR_SOLUTION
    ODIR := tobjs
    shaders = lighting.vert lighting.frag
    udflags = -USOLUTION -DTR_SOLUTION
else ifeq ($(v), sol)
    VFLAG := -DTR_SOLUTION -DSOLUTION
    ODIR := sobjs
    shaders = lightingSol.vert lightingSol.frag 
    udflags = -DSOLUTION -DTR_SOLUTION
else
    VFLAG := 
    ODIR := bobjs
    shaders = lighting.vert lighting.frag 
    udflags = -USOLUTION -UTR_SOLUTION
endif

ifdef c
    pkgName=CS$(c)-framework
else
    pkgName=Graphics-framework
endif

# ifndef v
# $(error This makefile needs a flag: v=basic, v=tr, or v=sol)
# endif

# Where the compilter will place, and the linker will find, the .o files
vpath %.o  $(ODIR)

# Search for the libs directory
ifneq (,$(wildcard libs))
    LIBDIR := libs
else
    ifneq (,$(wildcard ../libs))
        LIBDIR := ../libs
    else
        ifneq (,$(wildcard ../../libs))
            LIBDIR := ../../libs
        else
            LIBDIR := ../../../libs
        endif
    endif
endif

CXX = g++
CFLAGS = -g $(VFLAG) -I. -I$(LIBDIR)/freeglut -I$(LIBDIR)/glm -I$(LIBDIR) -I/usr/include  -I/usr/include/GL/ 
CXXFLAGS = -std=c++11 $(CFLAGS) -DVK_TAB=9

LIBS =  -L/usr/lib/x86_64-linux-gnu -L../$(LIBDIR) -L/usr/lib -L/usr/local/lib -lglbinding -lglut -lX11 -lGLU -lGL

CPPsrc = framework.cpp interact.cpp transform.cpp scene.cpp texture.cpp shapes.cpp object.cpp shader.cpp simplexnoise.cpp fbo.cpp
Csrc = rply.c

headers = framework.h interact.h texture.h shapes.h object.h rply.h scene.h shader.h transform.h simplexnoise.h fbo.h
srcFiles = $(CPPsrc) $(Csrc) $(shaders) $(headers)
extraFiles = framework.vcxproj Makefile box.ply room.ply textures

pkgDir = /home/gherron/packages
objs = $(patsubst %.cpp,%.o,$(CPPsrc)) $(patsubst %.c,%.o,$(Csrc))
target = $(ODIR)/framework.exe

$(target): $(objs)
	@echo Link $(target)
	cd $(ODIR) && $(CXX) -g  -o ../$@  $(objs) $(LIBS)

help:
	@echo "Try:"
	@echo "    make -j8       run  // for base level -- no transformations or shading"
	@echo "    make -j8 v=tr  run  // for transformation level -- no shading    "              
	@echo "    make -j8 v=sol run  // for full solution level"
	@echo "Also:"
	@echo "    make       c=251 zip // For CS541 -- bare bones"
	@echo "    make       c=541 zip // For CS541 -- bare bones"
	@echo "    make v=tr  c=300 zip // For CS300 -- includes transformations"
	@echo "    make v=sol c=562 zip // For CS562 -- include everything"

run: $(target)
	LD_LIBRARY_PATH="$(LIBDIR);$(LD_LIBRARY_PATH)" ./$(target)

what:
	@echo VPATH = $(VPATH)
	@echo LIBS = $(LIBDIR)
	@echo VFLAG = $(VFLAG)
	@echo objs = $(objs)
	@echo shaders = $(shaders)
	@echo pkgName = $(pkgName)
	@echo srcFiles = $(srcFiles)
	@echo extraFiles = $(extraFiles)
	@echo udflags = $(udflags)

clean:
	rm -rf tobjs sobjs bobjs dependencies

%.o: %.cpp
	@echo Compile $<  $(VFLAG)
	@mkdir -p $(ODIR)
	@$(CXX) -c $(CXXFLAGS) $< -o $(ODIR)/$@

%.o: %.c
	@echo Compile $< $(VFLAG)
	@mkdir -p $(ODIR)
	@$(CC) -c $(CFLAGS) $< -o $(ODIR)/$@

zip:
	rm -rf $(pkgDir)/$(pkgName) $(pkgDir)/$(pkgName).zip
	mkdir $(pkgDir)/$(pkgName)
	cp $(srcFiles) $(pkgDir)/$(pkgName)
	cp -r $(extraFiles) $(pkgDir)/$(pkgName)
	cp -r ../libs $(pkgDir)/$(pkgName)
	rm -rf $(pkgDir)/$(pkgName)/libs/.hg $(pkgDir)/$(pkgName)/libs/Eigen* $(pkgDir)/$(pkgName)/libs/assimp
	cd $(pkgDir)/$(pkgName);  unifdef $(udflags)  -m  $(srcFiles) || /bin/true

	cd $(pkgDir);  zip -r $(pkgName).zip $(pkgName); rm -rf $(pkgName)

ws:
	unix2dos $(srcFiles)
	@echo
	@echo ========= TABS:
	@grep -P '\t' $(srcFiles)

dependencies: 
	g++ -MM $(CXXFLAGS) $(CPPsrc) > dependencies

include dependencies
