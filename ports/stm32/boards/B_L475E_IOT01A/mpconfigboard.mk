# The stm32l475 does not have a LDC controller which is
# the only difference to the stm32l476 - so reuse some files.
AF_FILE = boards/stm32l476_af.csv
LD_FILES = boards/stm32l476xg.ld boards/common_basic.ld
OPENOCD_CONFIG = boards/openocd_stm32l4.cfg
