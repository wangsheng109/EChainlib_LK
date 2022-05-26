#echain common module CmakeLists.txt -- libechain_common.a

set(LIB_ECHAIN_COMMON echain_common)
set(COMMON_SRC
    configure_base.cpp general.cpp storage.cpp private_key.cpp ca.cpp
    daemon.cpp ca_manager.cpp argument.cpp pb2json.cpp cfca.cpp network.cpp data_secret_key.cpp
)

#generate static library file
add_library(${LIB_ECHAIN_COMMON} ${COMMON_SRC}) 

#specify compile options for target obj
target_compile_options(${LIB_ECHAIN_COMMON}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
)
