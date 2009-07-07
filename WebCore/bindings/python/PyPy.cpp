#include "config.h"

#include "PlatformString.h"

#include <ScriptSourceCode.h>

#include "PlatformString.h"
#include <wtf/Platform.h>
#include <parser/SourceProvider.h>
#include "ScriptController.h"
#include <parser/SourceCode.h>
#include "PyPy.h"
#include <stdio.h>

extern "C" char *RPython_StartupCode();
extern "C" long interpret(const char *, char *);

namespace WebCore {
  class String;

  int started_up = 0;

  bool PyPyScriptEvaluator::matchesMimeType(const String& mimeType)
  {
    if (mimeType == String("python")) {
      return(true);
    }
    return(false);
  }

  void PyPyScriptEvaluator::evaluate(const String& mimeType, const ScriptSourceCode& sourceCode, void *context)
	{
    if (mimeType == String("python")) {
        if (!started_up) {
            started_up = 1;
            RPython_StartupCode();
        }
    }

    interpret(sourceCode.jsSourceCode().toString().UTF8String().c_str(), (char*)context);
  }

}
