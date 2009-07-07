#ifndef PyPy_h
#define PyPy_h

#include <ScriptEvaluator.h>

namespace WebCore {
	class String;
    class ScriptSourceCode;

  class PyPyScriptEvaluator: public ScriptEvaluator {
	public:
		~PyPyScriptEvaluator(){}
		
    bool matchesMimeType(const String& mimeType);
		void evaluate(const String& mimeType, const ScriptSourceCode& sourceCode, void *context);
	};
}

#endif
