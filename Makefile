GCC=g++
INCLUDES= -I libs/include
LIB_STATICS=libs/lib/libsha256.a
LIB_DIR=
DYNAMIC_LIB=
TARGET=testHGO
SOURCES= main.cpp chain/block.cpp chain/blockchain.cpp exceptions.cpp token/wallet.cpp


all:$(TARGET)


$(TARGET) : $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES) $(LIB_STATICS) $(INCLUDES) -o $(TARGET)

clean:
	rm -rf $(TARGET)
