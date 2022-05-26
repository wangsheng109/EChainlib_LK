#echain ed25519 module CmakeLists.txt -- libechain_ed25519.a

set(LIB_ECHAIN_ED25519 echain_ed25519)
aux_source_directory(${ECHAIN_SRC_DIR}/3rd/ed25519-donna/ ED25519_SRC)
add_library(${LIB_ECHAIN_ED25519} ${ED25519_SRC}) 

target_compile_options(${LIB_ECHAIN_ED25519} PUBLIC -DASIO_STANDALONE)
