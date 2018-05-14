CMD = g++ -std=c++0x -Ofast
#CMD = g++ -std=c++0x -Wall -O3
# -DSCRIPT_CYCLE

TARGET = RouteSubmodular
SOURCE = io_ud.h Index.h Index.cpp RouteSearch.h RouteSearch.cpp Main.cpp
OBJ = $(TARGET).o
$(TARGET):
	$(CMD) $(LIBRA) $(SOURCE) -o $(TARGET)
clean:
	rm -f $(OBJ) $(TARGET)
