#echain utils module CmakeLists.txt -- libechain_utils.a

set(LIB_ECHAIN_UTILS echain_utils)
set(UTILS_SRC
    file.cpp logger.cpp net.cpp thread.cpp timestamp.cpp utils.cpp 
    crypto.cpp lrucache.hpp timer.cpp system.cpp sqlparser.cpp 
    exprparser.cpp sm3.cpp ecc_sm2.cpp
)

#generate static library file
add_library(${LIB_ECHAIN_UTILS} ${UTILS_SRC}) 

#specify compile options for target obj
target_compile_options(${LIB_ECHAIN_UTILS}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
)
