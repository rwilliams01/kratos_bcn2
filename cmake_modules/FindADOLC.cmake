#FIND_PATH(ADOL_C_INCLUDE_DIR adolc.h
#    "${ADOL_C_DIR}/include/adolc"
#)

FIND_LIBRARY(ADOL_C_LIBRARY adolc PATHS "${ADOL_C_DIR}/lib64" NO_DEFAULT_PATH)

message("ADOL_C_DIR:" ${ADOL_C_DIR})
message("ADOL_C_LIBRARY:" ${ADOL_C_LIBRARY})

IF(ADOL_C_LIBRARY)
    SET(ADOL_C_INCLUDE_DIR "${ADOL_C_DIR}/include")
    IF(ADOL_C_LIBRARY)
        SET(ADOL_C_LIBRARIES ${ADOL_C_LIBRARY})
        SET(ADOL_C_FOUND TRUE)
        message("ADOL-C found at" ${ADOL_C_DIR})
    ELSE()
        message("finding ADOL-C failed, please try to set the var ADOL_C_DIR")
    ENDIF()
ELSE()
    message("finding ADOL-C failed, please try to set the var ADOL_C_DIR")
ENDIF()

