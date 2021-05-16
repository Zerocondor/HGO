GCC=g++ -g 
INCLUDES= -I libs/include
LIB_STATICS=libs/lib/libsha256.a
LIB_DIR=
DYNAMIC_LIB= -lpthread
TARGET=testHGO
SOURCES= main.cpp chain/block.cpp chain/blockchain.cpp chain/chainevent.cpp exceptions.cpp token/wallet.cpp


all:$(TARGET)


$(TARGET) : $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES) $(LIB_STATICS) $(INCLUDES) $(LIB_DIR) $(DYNAMIC_LIB) -o $(TARGET)

clean:
	rm -rf $(TARGET)
