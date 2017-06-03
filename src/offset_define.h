#include <stddef.h>

#define DEFINE_OFFSET(FIRST, MEMBER) \
	unsigned OFF_##MEMBER = offsetof(FIRST, MEMBER)

#define DEFINE_OFFSET2(FIRST, SECOND, MEMBER) \
	unsigned OFF_##SECOND##_##MEMBER = offsetof(FIRST, SECOND.MEMBER)

#define DEFINE_OFFSET_WName(NAME, FIRST, MEMBER) \
	unsigned OFF_##NAME = offsetof(FIRST, MEMBER)

#define DEFINE_OFFSET_WName2(NAME, FIRST, SECOND, MEMBER) \
	unsigned OFF_##NAME = offsetof(FIRST, SECOND.MEMBER)

