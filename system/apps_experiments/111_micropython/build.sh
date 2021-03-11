# bez libm
set -e

mkdir -p build
cd build

arm-none-eabi-gcc -g -Wall -Os -Werror -fno-common -mcpu=cortex-m3 -mthumb -msoft-float -fno-exceptions -Wno-psabi -DLA104 -MD -c \
../source/py/mpstate.c \
../source/py/nlr.c \
../source/py/nlrx86.c \
../source/py/nlrx64.c \
../source/py/nlrthumb.c \
../source/py/nlrpowerpc.c \
../source/py/nlrxtensa.c \
../source/py/nlrsetjmp.c \
../source/py/malloc.c \
../source/py/gc.c \
../source/py/pystack.c \
../source/py/qstr.c \
../source/py/vstr.c \
../source/py/mpprint.c \
../source/py/unicode.c \
../source/py/mpz.c \
../source/py/reader.c \
../source/py/lexer.c \
../source/py/parse.c \
../source/py/scope.c \
../source/py/compile.c \
../source/py/emitcommon.c \
../source/py/emitbc.c \
../source/py/asmbase.c \
../source/py/asmx64.c \
../source/py/emitnx64.c \
../source/py/asmx86.c \
../source/py/emitnx86.c \
../source/py/asmthumb.c \
../source/py/emitnthumb.c \
../source/py/emitinlinethumb.c \
../source/py/asmarm.c \
../source/py/emitnarm.c \
../source/py/asmxtensa.c \
../source/py/emitnxtensa.c \
../source/py/emitinlinextensa.c \
../source/py/emitnxtensawin.c \
../source/py/formatfloat.c \
../source/py/parsenumbase.c \
../source/py/parsenum.c \
../source/py/emitglue.c \
../source/py/persistentcode.c \
../source/py/runtime.c \
../source/py/runtime_utils.c \
../source/py/scheduler.c \
../source/py/nativeglue.c \
../source/py/pairheap.c \
../source/py/ringbuf.c \
../source/py/stackctrl.c \
../source/py/argcheck.c \
../source/py/warning.c \
../source/py/profile.c \
../source/py/map.c \
../source/py/obj.c \
../source/py/objarray.c \
../source/py/objattrtuple.c \
../source/py/objbool.c \
../source/py/objboundmeth.c \
../source/py/objcell.c \
../source/py/objclosure.c \
../source/py/objcomplex.c \
../source/py/objdeque.c \
../source/py/objdict.c \
../source/py/objenumerate.c \
../source/py/objexcept.c \
../source/py/objfilter.c \
../source/py/objfloat.c \
../source/py/objfun.c \
../source/py/objgenerator.c \
../source/py/objgetitemiter.c \
../source/py/objint.c \
../source/py/objint_longlong.c \
../source/py/objint_mpz.c \
../source/py/objlist.c \
../source/py/objmap.c \
../source/py/objmodule.c \
../source/py/objobject.c \
../source/py/objpolyiter.c \
../source/py/objproperty.c \
../source/py/objnone.c \
../source/py/objnamedtuple.c \
../source/py/objrange.c \
../source/py/objreversed.c \
../source/py/objset.c \
../source/py/objsingleton.c \
../source/py/objslice.c \
../source/py/objstr.c \
../source/py/objstrunicode.c \
../source/py/objstringio.c \
../source/py/objtuple.c \
../source/py/objtype.c \
../source/py/objzip.c \
../source/py/opmethods.c \
../source/py/sequence.c \
../source/py/stream.c \
../source/py/binary.c \
../source/py/builtinimport.c \
../source/py/builtinevex.c \
../source/py/builtinhelp.c \
../source/py/modarray.c \
../source/py/modbuiltins.c \
../source/py/modcollections.c \
../source/py/modgc.c \
../source/py/modio.c \
../source/py/modmath.c \
../source/py/modcmath.c \
../source/py/modmicropython.c \
../source/py/modstruct.c \
../source/py/modsys.c \
../source/py/moduerrno.c \
../source/py/modthread.c \
../source/py/vm.c \
../source/py/bc.c \
../source/py/showbc.c \
../source/py/repl.c \
../source/py/smallint.c \
../source/py/frozenmod.c \
../source/lib/utils/pyexec.c \
../source/frozen/_frozen_mpy.c \
../source/glue.c \
  -I../source/

arm-none-eabi-g++ -g -Os -Werror -fno-common -mcpu=cortex-m3 -mthumb -msoft-float -fno-exceptions -fno-rtti -fno-threadsafe-statics -Wno-psabi -MD -D DS213 -D _ARM -D STM32F10X_HD -c \
  ../source/platform.cpp \
  ../source/main.cpp \
  ../../../os_host/source/framework/Serialize.cpp \
  -I../source/ \
  -I../../../os_library/include/ 

arm-none-eabi-gcc -g -fPIC -mcpu=cortex-m3 -mthumb -o output.elf -nostartfiles -T ../source/app.lds \
  mpstate.o \
  nlr.o \
  nlrx86.o \
  nlrx64.o \
  nlrthumb.o \
  nlrpowerpc.o \
  nlrxtensa.o \
  nlrsetjmp.o \
  malloc.o \
  gc.o \
  pystack.o \
  qstr.o \
  vstr.o \
  mpprint.o \
  unicode.o \
  mpz.o \
  reader.o \
  lexer.o \
  parse.o \
  scope.o \
  compile.o \
  emitcommon.o \
  emitbc.o \
  asmbase.o \
  asmx64.o \
  emitnx64.o \
  asmx86.o \
  emitnx86.o \
  asmthumb.o \
  emitnthumb.o \
  emitinlinethumb.o \
  asmarm.o \
  emitnarm.o \
  asmxtensa.o \
  emitnxtensa.o \
  emitinlinextensa.o \
  emitnxtensawin.o \
  formatfloat.o \
  parsenumbase.o \
  parsenum.o \
  emitglue.o \
  persistentcode.o \
  runtime.o \
  runtime_utils.o \
  scheduler.o \
  nativeglue.o \
  pairheap.o \
  ringbuf.o \
  stackctrl.o \
  argcheck.o \
  warning.o \
  profile.o \
  map.o \
  obj.o \
  objarray.o \
  objattrtuple.o \
  objbool.o \
  objboundmeth.o \
  objcell.o \
  objclosure.o \
  objcomplex.o \
  objdeque.o \
  objdict.o \
  objenumerate.o \
  objexcept.o \
  objfilter.o \
  objfloat.o \
  objfun.o \
  objgenerator.o \
  objgetitemiter.o \
  objint.o \
  objint_longlong.o \
  objint_mpz.o \
  objlist.o \
  objmap.o \
  objmodule.o \
  objobject.o \
  objpolyiter.o \
  objproperty.o \
  objnone.o \
  objnamedtuple.o \
  objrange.o \
  objreversed.o \
  objset.o \
  objsingleton.o \
  objslice.o \
  objstr.o \
  objstrunicode.o \
  objstringio.o \
  objtuple.o \
  objtype.o \
  objzip.o \
  opmethods.o \
  sequence.o \
  stream.o \
  binary.o \
  builtinimport.o \
  builtinevex.o \
  builtinhelp.o \
  modarray.o \
  modbuiltins.o \
  modcollections.o \
  modgc.o \
  modio.o \
  modmath.o \
  modcmath.o \
  modmicropython.o \
  modstruct.o \
  modsys.o \
  moduerrno.o \
  modthread.o \
  vm.o \
  bc.o \
  showbc.o \
  repl.o \
  smallint.o \
  frozenmod.o \
  main.o \
  pyexec.o \
  platform.o \
  Serialize.o \
  _frozen_mpy.o \
  glue.o \
  -lbios_la104 -L../../../os_library/build

arm-none-eabi-objdump -d -S output.elf > output.asm

nm --print-size --size-sort -gC output.elf | grep " B " > symbols_ram.txt
nm --print-size --size-sort -gC output.elf | grep " T " > symbols_rom.txt
nm --print-size --size-sort -gC output.elf > symbols_all.txt

find . -type f -name '*.o' -delete
find . -type f -name '*.d' -delete

../../../../tools/elfstrip/elfstrip output.elf 111micro.elf
