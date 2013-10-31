OBJS = main.o file.o log.o util.o
CC = g++
SRC_DIR = .
INC_DIR = .
OBJ_DIR = objs/
INCLUDE = -I $(SRC_DIR) -I $(INC_DIR) -I $(OBJ_DIR)

vpath %.h 	$(INC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.o 	$(OBJ_DIR)

server : main.o $(OBJS)
	$(CC) $(addprefix $(OBJ_DIR), $(OBJS)) -o shit.o 
	
main.o : main.cpp file.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@ 

file.o : file.cpp file.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@ 
	
log.o : log.cpp log.h 
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@

util.o : util.cpp util.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@

clean:
	-rm $(OBJ_DIR)*.o file.o
