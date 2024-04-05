BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
DEP_DIR = dependencies

CC = gcc
CFLAGS = -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR)
LDFLAGS = -lrt -lpthread -lprotobuf-c -lzookeeper_mt

EXECS = $(BIN_DIR)/table_server $(BIN_DIR)/table_client

LIB_OBJ = $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o

CLI_OBJ = $(OBJ_DIR)/table_client.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/sdmessage.pb-c.o

SER_OBJ = $(OBJ_DIR)/table_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/sdmessage.pb-c.o

all: libtable table-client table-server sdmessage

table-client: $(BIN_DIR)/table_client

table-server: $(BIN_DIR)/table_server

libtable: $(LIB_DIR)/libtable.a

sdmessage: $(OBJ_DIR)/sdmessage.pb-c.o

$(LIB_DIR)/libtable.a: $(LIB_OBJ)
	ar -rcs $(LIB_DIR)/libtable.a $(LIB_OBJ)
	
$(BIN_DIR)/table_client: $(CLI_OBJ) $(LIB_DIR)/libtable.a
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/table_server: $(SER_OBJ) $(LIB_DIR)/libtable.a
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/sdmessage.pb-c.o: sdmessage.proto

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -g -o $@ -c $<

include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -f $(DEP_DIR)/*.d $(OBJ_DIR)/*.o $(LIB_DIR)/*.a $(EXECS)
