Obj_OS := $(patsubst %.cpp, %.o, $(wildcard os/*.cpp))
Obj_utils := $(patsubst %.cpp, %.o, $(wildcard utils/*.cpp))
Obj_com := $(patsubst %.cpp, %.o, $(wildcard common/*.cpp))
Obj_config := $(patsubst %.cpp, %.o, $(wildcard config/*.cpp))
Obj_mysql := $(patsubst %.cpp, %.o, $(wildcard mysql/*.cpp))
Obj_test := $(patsubst %.cpp, %.o, $(wildcard test/*.cpp))
Obj_3rd := $(patsubst %.cpp, %.o, $(wildcard 3rd/*.cpp))
Obj_ec := $(patsubst %.cpp, %.o, $(wildcard ec/*.cpp))

OBJS :=  $(Obj_OS) $(Obj_ec) $(Obj_utils) $(Obj_com) $(Obj_test) $(Obj_config) $(Obj_mysql) $(Obj_3rd)

INC :=-I/usr/include -I/usr/include/boost -I./ -I./os -I./config -I./mysql -I./utils -I./common -I./test -I./3rd/websocketpp -I/usr/include -I/usr/include/mysql

#Libs := -lpthread -lm -lz -ldl -lmysqlclient -lrt -lcrypto -lssl -lboost_system -lboost_filesystem -lprotobuf -ljson -lpcreposix -lpcrecpp -lpcre -lbz2 -lsodium -L/lib64/ -L/usr/lib64/ -L/usr/lib64/mysql
Libs := -lc -lpthread -lm -lz -ldl -lmysqlclient -lrt -lcrypto -lssl -lboost_system -lboost_filesystem -ljson_linux-gcc-7.2.0_libmt -L/lib64/ -L/usr/lib64/ -L/usr/lib64/mysql

DIR_LIB :=./lib

CFLAGS := -c -g -D_LINUX_ -fpic
LDFLAGS := -shared -fpic
ARFLAGS := -rc
CC := g++
#LD := ld
LD := g++
AR := ar
COPY  = cp
SHARE_LIB := libEChainLKAPI.so
STATIC_LIB := libEChainLKAPI.a

all : $(OBJS)
	$(LD) $(LDFLAGS) -o $(DIR_LIB)/$(SHARE_LIB) $(OBJS) $(INC) $(Libs)   
	$(AR) $(ARFLAGS) -o $(DIR_LIB)/$(STATIC_LIB) $(OBJS) 
     
	
$(OBJS) : %.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@ $(INC) $(Libs) 
	
clean:
	rm -rf $(OBJS) $(DIR_LIB)/*.so $(DIR_LIB)/*.a

