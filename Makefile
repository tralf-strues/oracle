Options = -Wall -Wpedantic

SrcDir = src
BinDir = bin
Intermediates = $(BinDir)/intermediates
LibDir = libs

LIBS = $(LibDir)/file_manager.a $(LibDir)/log_generator.a 
DEPS = $(SrcDir)/binary_tree.h $(SrcDir)/oracle.h $(SrcDir)/ui.h $(LibDir)/file_manager.h $(LibDir)/log_generator.h $(LibDir)/stack.h 

$(BinDir)/oracle.exe: $(Intermediates)/main.o $(Intermediates)/oracle.o $(Intermediates)/ui.o $(Intermediates)/binary_tree.o $(LIBS) $(DEPS)
	g++ -o $(BinDir)/oracle.exe $(Intermediates)/main.o $(Intermediates)/oracle.o $(Intermediates)/ui.o $(Intermediates)/binary_tree.o $(LIBS)

$(Intermediates)/main.o: $(SrcDir)/main.cpp $(DEPS)
	g++ -o $(Intermediates)/main.o -c $(SrcDir)/main.cpp $(Options)

$(Intermediates)/oracle.o: $(SrcDir)/oracle.cpp $(DEPS)
	g++ -o $(Intermediates)/oracle.o -c $(SrcDir)/oracle.cpp $(Options)

$(Intermediates)/ui.o: $(SrcDir)/ui.cpp $(DEPS)
	g++ -o $(Intermediates)/ui.o -c $(SrcDir)/ui.cpp $(Options)

$(Intermediates)/binary_tree.o: $(SrcDir)/binary_tree.cpp $(DEPS)
	g++ -o $(Intermediates)/binary_tree.o -c $(SrcDir)/binary_tree.cpp $(Options)