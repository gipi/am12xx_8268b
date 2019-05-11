if you want to support the function of writting MAC address to EEPROM,you should follow steps blow:
step 1:
     enter sdk/include,open "sys_at24c02.h",then modify "#define EEPROM_TYPE 0"  to "#define EEPROM_TYPE X"
     "X" is 1 if EEPROM is IIC interface;"X" is 2 if EEPROM is SPI interface.
step 2:
     enter case/project_kit/project_qc/batch/scripts if chip type is 1211 or 1220,
     enter case/project_kit/project_qc/batch/scripts/1213 if chip type is 1213,
     then open usb_process.sh ,modify "eeprom_type=0"  to "eeprom_type=X"
     "X" is 1 if EEPROM is IIC interface;"X" is 2 if EEPROM is SPI interface.
step 3:
     make firmware.