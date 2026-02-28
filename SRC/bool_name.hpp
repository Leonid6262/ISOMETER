#pragma once

enum class StatusRet     { ERROR       = 0, SUCCESS = 1 };
enum class State         { OFF         = 0, ON      = 1 };
enum class StatusTO      { NOT_EXPIRED = 0, EXPIRED = 1, NO_RECEPTION = 2};
enum class EVarType {
    NONE,
    sshort,
    ushort,
    vfloat,
    vbool,
    char2b,
    short2b,
    text,
    eb_0, eb_1, eb_2,  eb_3,  eb_4,  eb_5,  eb_6,  eb_7,
    eb_8, eb_9, eb_10, eb_11, eb_12, eb_13, eb_14, eb_15
}; 
