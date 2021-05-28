GCC=g++ -g 
INCLUDES= -I libs/include -I .
LIB_STATICS=libs/lib/libsha256.a
LIB_DIR=
DYNAMIC_LIB= -lpthread -lssl -lcrypto

TARGET_TESTHGO=testHGO
SOURCES_TESTHGO=test/testhgo.cpp

TARGET_TESTPROTOCOL=testProtocol
SOURCES_TESTPROTOCOL=test/hgo_protocol.cpp

TARGET_TESTP2P=testP2p
SOURCES_TESTP2P=test/testp2p.cpp

SOURCES= chain/block.cpp \
chain/blockchain.cpp \
chain/chainevent.cpp \
exceptions.cpp \
token/wallet.cpp \
token/hgo_key.cpp \
hgo_protocol/network_manager.cpp \
hgo_protocol/p2p_server.cpp \

TARGET_MASTERNODE=masternode
SOURCES_MASTERNODE=masternode/main.cpp masternode/application.cpp

TARGET_WALLET=wallet
SOURCES_WALLET=wallet/main.cpp wallet/application.cpp

all: bin test

test: $(TARGET_TESTHGO) $(TARGET_TESTPROTOCOL) $(TARGET_TESTP2P)

bin: $(TARGET_MASTERNODE) $(TARGET_WALLET)

$(TARGET_MASTERNODE) : $(SOURCES_MASTERNODE) $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES_MASTERNODE) $(SOURCES) $(LIB_STATICS) $(INCLUDES) $(LIB_DIR) $(DYNAMIC_LIB) -o ./bin/$(TARGET_MASTERNODE)

$(TARGET_WALLET) : $(SOURCES_WALLET) $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES_WALLET) $(SOURCES) $(LIB_STATICS) $(INCLUDES) $(LIB_DIR) $(DYNAMIC_LIB) -o ./bin/$(TARGET_WALLET)

$(TARGET_TESTHGO) : $(SOURCES_TESTHGO) $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES_TESTHGO) $(SOURCES) $(LIB_STATICS) $(INCLUDES) $(LIB_DIR) $(DYNAMIC_LIB) -o $(TARGET_TESTHGO)

$(TARGET_TESTPROTOCOL) : $(SOURCES_TESTPROTOCOL) $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES_TESTPROTOCOL) $(SOURCES) $(LIB_STATICS) $(INCLUDES) $(LIB_DIR) $(DYNAMIC_LIB) -o $(TARGET_TESTPROTOCOL)

$(TARGET_TESTP2P) : $(SOURCES_TESTP2P) $(SOURCES) $(LIB_STATICS)
	$(GCC) $(SOURCES_TESTP2P) $(SOURCES) $(LIB_STATICS) $(INCLUDES) $(LIB_DIR) $(DYNAMIC_LIB) -o $(TARGET_TESTP2P)	

clean:
	rm -rf $(TARGET_TESTHGO) $(TARGET_TESTPROTOCOL) $(TARGET_TESTP2P) ./bin/$(TARGET_MASTERNODE) ./bin/$(TARGET_WALLET)
