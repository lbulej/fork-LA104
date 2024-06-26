//#include "ringbuf.h"

void EventLoop();
void _yield();
void _PrepareRun();

extern unsigned long _addressRamBegin;
extern unsigned long _addressRamEnd;
extern unsigned long _addressRomBegin;
extern unsigned long _addressRomEnd;

namespace GABUINO
{
  uint8_t* writePtr = nullptr;
  int writeCount = 0;
  bool running = false;
  bool trapped = false;
  bool shouldStop = false;
  int writeSum = 0;
  int userRetVal = 0x66667777;

//  RingBufCPP<uint16_t, 128> debugPrintBuffer;
//  char debugPrintBuffer[128] = {0};
  CArray<uint32_t> debugStackFrames;
  uint32_t debugStackFramesData[90]; // TODO: wasting!

  char tempBuf[200];


  int Write(uint32_t address, uint32_t count)
  {
    writeSum = 0;
    writePtr = (uint8_t*)address;
    writeCount = count;
    return 1;
  }

  void HandleWrite(uint8_t* buf, int count)
  {
//    BIOS::DBG::Print("c=%d,wc=%d,b=%s", count, writeCount, buf);
    writeCount -= count;
    _ASSERT(writeCount >= 0);
    while (count--)
    {
      writeSum += *buf;
      *writePtr++ = *buf++;
    }
  }

  int Read(uint32_t ptr, int n)
  {
    uint8_t* p = (uint8_t*)ptr;

    TERMINAL::Print("{bulk:%d,bps:8}", n);
    if (!TERMINAL::BulkTransfer(p, n))
      return 0;
    return 1;
  }

  int Done()
  {
    return writeSum;
  }

  void Zero(uint32_t ptr, uint32_t count)
  {
    uint8_t* p = (uint8_t*)ptr;
    while (count--)
      *p++ = 0;
  }

  int Exec(uint32_t ptr)
  {
    _ASSERT((ptr & 1) && (ptr >= 0x20005001));
    userRetVal = 0x66667777;

    typedef int(*TFunc)();
    _PrepareRun();
    shouldStop = false;
    running = true;
    userRetVal = ((TFunc)ptr)();
    running = false;

    TERMINAL::Print("_DBGEVENT(1)");

    return userRetVal;
  }

  int Running()
  {
    return running;
  }

  int ReturnValue()
  {
    return userRetVal;
  }

  int Stop()
  {
    shouldStop = true;
    return 39;
  }

  int GetKey2()
  {
    EventLoop();
    if (!shouldStop)
      return BIOS::KEY::GetKey();
    else
      return BIOS::KEY::EKey::Escape;
  }

  int DelayMs2(int duration)
  {
    while (duration>20)
    {
      EventLoop();
      BIOS::SYS::DelayMs(20);
      duration -= 20;
    }
    EventLoop();
    if (duration > 0)
      BIOS::SYS::DelayMs(duration);    
    return 0;
  }

  void DbgPrint2(const char * format, ...)
  {
    const char* prefix = "_DBGPRINT(`";
    const char* suffix = "`);";
    const int prefixlen = strlen(prefix);
    const int suffixlen = strlen(suffix);

    int len = strlen(format);
    for (int i=0; i<len; i++)
      if (format[i] == '%')
      {
        _ASSERT(strlen(format) < sizeof(tempBuf)-16);

	strcpy(tempBuf, prefix);
        va_list args;        
        va_start( args, format );
        vsprintf( tempBuf+prefixlen, format, args );
        strcat(tempBuf, suffix);

        _ASSERT(strlen(tempBuf) < sizeof(tempBuf)-16);
        TERMINAL::BulkTransfer((uint8_t*)tempBuf, strlen(tempBuf));
        return;
      }

    // static text
    int counter = 0;
    TERMINAL::BulkTransfer(prefixlen+suffixlen+len, [&](uint8_t* buf, int bytes)
    {
      for (int i=0; i<bytes; i++, counter++)
      {
        if (counter < prefixlen)
          buf[i] = prefix[counter];
        else if (counter < prefixlen+len)
          buf[i] = format[counter-prefixlen];
        else
        {
           _ASSERT(counter - prefixlen - len < suffixlen);
           buf[i] = suffix[counter - prefixlen - len];
        }
      }
    });
  }

  uint32_t GetProcAddress2(char* name)
  {
    if (strcmp(name, "_ZN4BIOS3KEY6GetKeyEv") == 0)
      return (uint32_t)GABUINO::GetKey2;
    if (strcmp(name, "_ZN4BIOS3DBG5PrintEPKcz") == 0)
      return (uint32_t)GABUINO::DbgPrint2;
    if (strcmp(name, "_ZN4BIOS3SYS7DelayMsEi") == 0)
      return (uint32_t)GABUINO::DelayMs2;
    return BIOS::SYS::GetProcAddress(name);
  }

  int _Frame() // called from ISR!
  {
    static uint32_t *SP;
    asm("mrs %0, msp" : "=r"(SP) : :);
    uint32_t* stack = SP+6;
    uint32_t** memPtrs = (uint32_t**)(BIOS::SYS::GetAttribute(BIOS::SYS::EAttribute::SystemMemoryRanges));
    uint32_t* endStack = (uint32_t*)*((uint32_t*)memPtrs[4]);
    // TODO: not verified!

    int count = 0;
    for (count=0; count<COUNT(debugStackFramesData) && stack < endStack; count++)
      debugStackFramesData[count] = *stack++;

    debugStackFrames.Init(debugStackFramesData, count);
    debugStackFrames.SetSize(count);
    return 0;
  }

  int Frame()
  {
    if (debugStackFrames[0] == 0xffffffff)
      return -1;
    TERMINAL::Print("{bulk:%d,bps:32}", debugStackFrames.GetSize()*4);
    TERMINAL::BulkTransfer((uint8_t*)debugStackFrames.GetData(), debugStackFrames.GetSize()*4);
    // mark invalid
    debugStackFrames[0] = 0xaaaaaaaa;
    return 90;
  }

  int Screenshot()
  {
    TERMINAL::Print("{bulk:%d,bps:16}", BIOS::LCD::Width*BIOS::LCD::Height*2);
    BIOS::LCD::BufferBegin(CRect(0, 0, BIOS::LCD::Width, BIOS::LCD::Height));
    int x = 0, y = 0;
    TERMINAL::BulkTransfer(BIOS::LCD::Width*BIOS::LCD::Height*2, [&](uint8_t* buf, int bytes)
    {
      for (int i=0; i<bytes; i+=2)
      {
        uint16_t color = BIOS::LCD::BufferRead();
        buf[i] = color>>8;
        buf[i+1] = (uint8_t)color;
      }
    });
    BIOS::LCD::BufferEnd();
    return 0;
  }

  int Resume()
  {
    trapped = false;
    return 0;
  }

  uint32_t crc32b(const uint8_t *message, int length) {
     uint32_t crc, mask;

     crc = 0xFFFFFFFF;
     while (length--) {
        crc = crc ^ *message++;
        for (int j = 7; j >= 0; j--) 
        {
           mask = -(crc & 1);
           crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
     }
     return ~crc;
  }

  int Identify()
  {
    int len = (uint8_t*)&_addressRomEnd-(uint8_t*)&_addressRomBegin;
    uint32_t hostCrc = crc32b((uint8_t*)&_addressRomBegin, len);

    uint32_t osCrc = *(uint32_t*)BIOS::SYS::GetAttribute(BIOS::SYS::EAttribute::FirmwareChecksum);
    const char* device = (const char*)BIOS::SYS::GetAttribute(BIOS::SYS::EAttribute::DeviceType); 
    uint32_t deviceUid = BIOS::SYS::GetAttribute(BIOS::SYS::EAttribute::SerialNumber);

    // TODO: must fit 64 bytes!
    TERMINAL::Print("{os:0x%08x,host:0x%08x,device:'%s',id:0x%08x}", 
      osCrc, hostCrc, device, deviceUid);
    return 0;
  }

/*
  int Trace()
  {
    static uint32_t *SP;
    asm("mrs %0, msp" : "=r"(SP) : :);

    uintptr_t estack = BIOS::SYS::GetAttribute(BIOS::SYS::EAttribute::EndStack)
    uint32_t *p = sp+6;
    while (i < 10 && p < &estack)
    {
        if ((*p >> 24) == 0x08)
            callers[i++] = *p;        
        p++;
    }

    return 0;
  }   

  void SaveStack()
  {
  }
*/
}

