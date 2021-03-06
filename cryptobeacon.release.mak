###########################################################
# Makefile generated by xIDE for uEnergy                   
#                                                          
# Project: cryptobeacon
# Configuration: Release
# Generated: Wed Mar 9 19:44:07 2016
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

XIDE_PROJECT=cryptobeacon
XIDE_CONFIG=Release
OUTPUT=cryptobeacon
OUTDIR=C:/Users/Neil/Documents/firmware/cryptobeacon
DEFS=

OUTPUT_TYPE=0
USE_FLASH=0
ERASE_NVM=1
CSFILE_CSR101x_A05=C:\Users\Neil\Documents\firmware\cryptobeacon\cryptobeacon_csr101x_A05.keyr
MASTER_DB=
LIBPATHS=
INCPATHS=
STRIP_SYMBOLS=0
OTAU_BOOTLOADER=0
OTAU_CSFILE=
OTAU_NAME=
OTAU_SECRET=
OTAU_VERSION=7

DBS=\
\
      app_gatt_db.db\
      battery_service_db.db\
      dev_info_service_db.db\
      gap_service_db.db\
      gatt_service_db.db\
      auth_service_db.db

INPUTS=\
      battery_service.c\
      beacon.c\
      debug_interface.c\
      dev_info_service.c\
      gap_service.c\
      gatt_access.c\
      hw_access.c\
      nvm_access.c\
      cryptobeacon.c\
      auth_service.c\
      leds.c\
      power_management.c\
      crypto.c\
      string_utils.c\
      $(DBS)

KEYR=\
      cryptobeacon_csr101x_A05.keyr


-include cryptobeacon.mak
include $(SDK)/genmakefile.uenergy
