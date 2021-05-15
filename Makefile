GCC=g++
SOURCES= main.cpp chain/block.cpp chain/blockchain.cpp chain/exceptions.cpp
INCLUDES= -I libs/include
LIB_STATICS=libs/lib/libsha256.a
LIB_DIR=
DYNAMIC_LIB=
TARGET=testHGO


all:$(TARGET)


$(TARGET) : $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES) $(LIB_STATICS) $(INCLUDES) -o $(TARGET)

clean: $(TARGET)
	rm -rf $(TARGET)
