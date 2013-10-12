OBJS = main.o socket_manager.o http_manager.o log.o util.o
CC = g++
SRC_DIR = .
INC_DIR = .
OBJ_DIR = objs/
INCLUDE = -I $(SRC_DIR) -I $(INC_DIR) -I $(OBJ_DIR)

vpath %.h $(INC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.o $(OBJ_DIR)

server : main.o $(OBJS)
	$(CC) $(addprefix $(OBJ_DIR), $(OBJS)) -o server.o 
	
main.o : main.cpp socket_manager.h http_manager.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@ 
	
socket_manager.o: socket_manager.cpp socket_manager.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@

http_manager.o: http_manager.cpp http_manager.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@

log.o : log.cpp log.h 
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@

util.o : util.cpp util.h
	$(CC) -c $< $(INCLUDE) -o $(OBJ_DIR)$@

clean:
	-rm $(OBJ_DIR)*.o server.o
