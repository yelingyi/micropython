include("$(MPY_DIR)/extmod/asyncio")
add_library("unix-ffi", "$(MPY_LIB_DIR)/unix-ffi")
require("mip-cmdline")
require("ssl")
