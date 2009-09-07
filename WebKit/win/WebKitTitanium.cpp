#include "config.h"
#include "WebKit.h"
#include <WebCore/StringHash.h>
#include <wtf/Vector.h>
#include <WebCore/ScriptElement.h>
#include "WebCoreSupport/WebFrameLoaderClient.h"
#include "TitaniumProtocols.h"
#include "WebKitTitanium.h"
#include "WebMutableURLRequest.h"

void setNormalizeURLCallback(NormalizeURLCallback cb) {
    WebCore::TitaniumProtocols::NormalizeCallback = cb;
}

void setURLToFileURLCallback(URLToFileURLCallback cb) {
    WebCore::TitaniumProtocols::URLCallback = cb;
}

void setCanPreprocessCallback(CanPreprocessURLCallback cb) {
    WebCore::TitaniumProtocols::CanPreprocessCallback = cb;
}

void setPreprocessCallback(PreprocessURLCallback cb) {
    WebCore::TitaniumProtocols::PreprocessCallback = cb;
}

IWebURLRequest* STDMETHODCALLTYPE createWebURLRequest() {
	WebMutableURLRequest *request = WebMutableURLRequest::createInstance();
	
	return request;
}
