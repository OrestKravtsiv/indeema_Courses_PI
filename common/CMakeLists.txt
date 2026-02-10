# 1. Create a static library target named 'common'
add_library(common STATIC
src/logger.cpp
)
# 2. Define the header search path
target_include_directories(common
PUBLIC
include
)
