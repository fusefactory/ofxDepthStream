#ifdef OPENNI_AVAILABLE
  #include "OpenNI.h"
#else
  // create "dummy" openni::VideoStream type
  namespace openni {
    using VideoStream = int;
  }
#endif

namespace persee {
  class Formatter {
    public:
      void process(openni::VideoStream& stream);
      const char* getData() const { return data; }
      int getSize(){ return size; }

    private:

      #ifdef OPENNI_AVAILABLE
        const char* data = NULL;
      #else
        static const size_t BUF_SIZE = 640*480*2;
        char data[BUF_SIZE];
      #endif
      unsigned int size=0;
  };
}
