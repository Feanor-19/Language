CC=g++

CFLAGS = 	-Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef 				\
			-Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations 			\
			-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ 				\
			-Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion 		\
			-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2	 			\
			-Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers 			\
			-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo 		\
			-Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits 	\
			-Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

OBJ 	= obj
SRC 	= src
BIN 	= bin
CMN 	= .\..\common
LOGS 	= .\logs

COMMON_LIB_FOLDER = ./../../../mylibs/
LIB_NAMES 			= tree onegin
LIB_SUBST 			= -L$(COMMON_LIB_FOLDER)$(lib_name)/bin -l$(lib_name)
LIB_HEADERS_SUBST 	= -I $(COMMON_LIB_FOLDER)$(lib_name)/headers

LIBS 			= $(foreach lib_name,$(LIB_NAMES),$(LIB_SUBST))
LIB_HEADERS		= $(foreach lib_name,$(LIB_NAMES),$(LIB_HEADERS_SUBST))
SOURCES 		= $(wildcard $(SRC)/*.cpp)
OBJFILES 		= $(patsubst $(SRC)/%,$(OBJ)/%,$(SOURCES:.cpp=.o))
CMN_SOURCES		= $(wildcard $(CMN)/$(SRC)/*.cpp)
CMN_OBJFILES	= $(patsubst $(CMN)/$(SRC)/%,$(CMN)/$(OBJ)/%,$(CMN_SOURCES:.cpp=.o))
OUT 			= $(BIN)\backend.exe
DUMP_FOLDER 	= .\dumps
LATEX_FOLDER 	= .\LaTeX

$(OUT) : $(OBJFILES) $(CMN_OBJFILES)
	@$(CC) -o $@ $(CFLAGS) $^ $(LIB_HEADERS) $(LIBS) -I $(CMN)/$(SRC)

$(OBJ)/%.o : $(SRC)/%.cpp
	@$(CC) -c $(CFLAGS) -o $@ $< -I $(CMN)/$(SRC)

$(CMN)/$(OBJ)/%.o : $(CMN)/$(SRC)/%.cpp
	@$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: run
run:
	@$(OUT) -i .\\..\\compiler_tree.txt -o .\\..\\asm.txt --img-dumps $(DUMP_FOLDER)\\image_dumps\\

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(CMN_OBJFILES) $(OUT)

.PHONY: clean_dumps
clean_dumps:
	rm -r -f $(DUMP_FOLDER)
	mkdir $(DUMP_FOLDER)

.PHONY: clean_latex
clean_latex:
	rm -r -f $(LATEX_FOLDER)

.PHONY: clean_logs
clean_logs:
	rm -r -f $(LOGS)
	mkdir $(LOGS)
