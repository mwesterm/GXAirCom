set variant=nopsRam
set addresses=0x4009095c:0x3ffd2310 0x40090bd9:0x3ffd2330 0x40087f3d:0x3ffd2350 0x40088069:0x3ffd2380 0x4014502b:0x3ffd23a0 0x4013e4dd:0x3ffd2660 0x4013e439:0x3ffd26b0 0x4009589d:0x3ffd26e0 0x40082cca:0x3ffd2700 0x40087e35:0x3ffd2720 0x4000bec7:0x3ffd2740 0x401b0635:0x3ffd2760 0x401b0375:0x3ffd2780 0x400fc699:0x3ffd27a0 0x400fcd96:0x3ffd27d0 0x400fd239:0x3ffd27f0 0x400f827f:0x3ffd2810 0x400ea9b9:0x3ffd2830 0x40091bea:0x3ffd2b60

set addr2linePath=C:\Users\gereic\.platformio\packages\toolchain-xtensa32\bin\

%addr2linePath%xtensa-esp32-elf-addr2line.exe -fipC -e ..\.pio\build\%variant%\firmware.elf %addresses%
REM %addr2linePath%xtensa-esp32-elf-addr2line.exe -fipC -e ..\.pio\build\%variant%\firmware.elf %1