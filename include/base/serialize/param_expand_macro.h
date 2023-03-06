
#pragma once

#define  EXTAND_ARGS(args) args

#define FOR_EACH_1(what, Param, ...)  what(Param)
#define FOR_EACH_2(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_1(what, __VA_ARGS__))
#define FOR_EACH_3(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_2(what, __VA_ARGS__))
#define FOR_EACH_4(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_3(what, __VA_ARGS__))
#define FOR_EACH_5(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_4(what, __VA_ARGS__))
#define FOR_EACH_6(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_5(what, __VA_ARGS__))
#define FOR_EACH_7(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_6(what, __VA_ARGS__))
#define FOR_EACH_8(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_7(what, __VA_ARGS__))
#define FOR_EACH_9(what, Param, ...)  what(Param), EXTAND_ARGS(FOR_EACH_8(what, __VA_ARGS__))
#define FOR_EACH_10(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_9(what, __VA_ARGS__))
#define FOR_EACH_11(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_10(what, __VA_ARGS__))
#define FOR_EACH_12(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_11(what, __VA_ARGS__))
#define FOR_EACH_13(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_12(what, __VA_ARGS__))
#define FOR_EACH_14(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_13(what, __VA_ARGS__))
#define FOR_EACH_15(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_14(what, __VA_ARGS__))
#define FOR_EACH_16(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_15(what, __VA_ARGS__))
#define FOR_EACH_17(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_16(what, __VA_ARGS__))
#define FOR_EACH_18(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_17(what, __VA_ARGS__))
#define FOR_EACH_19(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_18(what, __VA_ARGS__))
#define FOR_EACH_20(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_19(what, __VA_ARGS__))
#define FOR_EACH_21(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_20(what, __VA_ARGS__))
#define FOR_EACH_22(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_21(what, __VA_ARGS__))
#define FOR_EACH_23(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_22(what, __VA_ARGS__))
#define FOR_EACH_24(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_23(what, __VA_ARGS__))
#define FOR_EACH_25(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_24(what, __VA_ARGS__))
#define FOR_EACH_26(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_25(what, __VA_ARGS__))
#define FOR_EACH_27(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_26(what, __VA_ARGS__))
#define FOR_EACH_28(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_27(what, __VA_ARGS__))
#define FOR_EACH_29(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_28(what, __VA_ARGS__))
#define FOR_EACH_30(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_29(what, __VA_ARGS__))
#define FOR_EACH_31(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_30(what, __VA_ARGS__))
#define FOR_EACH_32(what, Param, ...) what(Param), EXTAND_ARGS(FOR_EACH_31(what, __VA_ARGS__))

#define CONCATENATE(arg1, arg2)  arg1##arg2  

//Parameters Def expand as: "arg", arg
#define PARAM_DEF(Param) #Param, Param 
#define PARAM_DEF_FOR_EACH_(N, ...) EXTAND_ARGS(CONCATENATE(FOR_EACH_, N)(PARAM_DEF,__VA_ARGS__))

#define FL_ARG_COUNT(...) EXTAND_ARGS(FL_INTERNAL_ARG_COUNT_PRIVATE(0, __VA_ARGS__,\
	64, 63, 62, 61, 60, \
	59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
	29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
	9,  8,  7,  6,  5,  4,  3,  2,  1,  0))

#define FL_INTERNAL_ARG_COUNT_PRIVATE(\
	_0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, \
	_10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
	_20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
	_30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
	_40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
	_50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
	_60, _61, _62, _63, _64, N, ...) N
