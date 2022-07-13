/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_WEB_EVENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_WEB_EVENT_H

namespace OHOS::Ace {

enum DialogEventType {
    DIALOG_EVENT_ALERT = 0,
    DIALOG_EVENT_BEFORE_UNLOAD = 1,
    DIALOG_EVENT_CONFIRM = 2,
    DIALOG_EVENT_PROMPT = 3
};

class WebConsoleLog : public AceType {
    DECLARE_ACE_TYPE(WebConsoleLog, AceType)
public:
   WebConsoleLog() = default;
    ~WebConsoleLog() = default;

    virtual int GetLineNumber() = 0;
    virtual std::string GetLog() = 0;
    virtual int GetLogLevel() = 0;
    virtual std::string GetSourceId() = 0;
};

class WebFileSelectorParam : public AceType {
    DECLARE_ACE_TYPE(WebFileSelectorParam, AceType)
public:
    WebFileSelectorParam() = default;
    ~WebFileSelectorParam() = default;

    virtual std::string GetTitle() = 0;
    virtual int GetMode() = 0;
    virtual std::string GetDefaultFileName() = 0;
    virtual std::vector<std::string> GetAcceptType() = 0;
    virtual bool IsCapture() = 0;
};

class ACE_EXPORT WebError : public AceType {
    DECLARE_ACE_TYPE(WebError, AceType)

public:
    WebError(const std::string& info, int32_t code) : info_(info), code_(code) {}
    ~WebError() = default;

    const std::string& GetInfo() const
    {
        return info_;
    }

    int32_t GetCode() const
    {
        return code_;
    }

private:
    std::string info_;
    int32_t code_;
};

class ACE_EXPORT WebResponse : public AceType {
    DECLARE_ACE_TYPE(WebResponse, AceType)

public:
    WebResponse(const std::map<std::string, std::string>& headers, const std::string& data, const std::string& encoding,
        const std::string& mimeType, const std::string& reason, int32_t statusCode)
        : headers_(headers), data_(data), encoding_(encoding), mimeType_(mimeType), reason_(reason),
          statusCode_(statusCode)
    {}
    WebResponse() = default;
    ~WebResponse() = default;

    const std::map<std::string, std::string>& GetHeaders() const
    {
        return headers_;
    }

    const std::string& GetData() const
    {
        return data_;
    }

    const std::string& GetEncoding() const
    {
        return encoding_;
    }

    const std::string& GetMimeType() const
    {
        return mimeType_;
    }

    const std::string& GetReason() const
    {
        return reason_;
    }

    int32_t GetStatusCode() const
    {
        return statusCode_;
    }

    void SetHeadersVal(std::string& key, std::string& val)
    {
        headers_[key] = val;
    }

    void SetData(std::string& data)
    {
        data_ = data;
    }

    void SetEncoding(std::string& encoding)
    {
        encoding_ = encoding;
    }

    void SetMimeType(std::string& mimeType)
    {
        mimeType_ = mimeType;
    }

    void SetReason(std::string& reason)
    {
        reason_ = reason;
    }

    void SetStatusCode(int32_t statusCode)
    {
        statusCode_ = statusCode;
    }

private:
    std::map<std::string, std::string> headers_;
    std::string data_;
    std::string encoding_;
    std::string mimeType_;
    std::string reason_;
    int32_t statusCode_;
};

class ACE_EXPORT WebRequest : public AceType {
    DECLARE_ACE_TYPE(WebRequest, AceType)

public:
    WebRequest(const std::map<std::string, std::string>& headers, const std::string& method, const std::string& url,
        bool hasGesture, bool isMainFrame, bool isRedirect)
        : headers_(headers), method_(method), url_(url), hasGesture_(hasGesture), isMainFrame_(isMainFrame),
          isRedirect_(isRedirect)
    {}
    ~WebRequest() = default;

    const std::map<std::string, std::string>& GetHeaders() const
    {
        return headers_;
    }

    const std::string& GetMethod() const
    {
        return method_;
    }

    const std::string& GetUrl() const
    {
        return url_;
    }

    bool HasGesture() const
    {
        return hasGesture_;
    }

    bool IsMainFrame() const
    {
        return isMainFrame_;
    }

    bool IsRedirect() const
    {
        return isRedirect_;
    }

private:
    std::map<std::string, std::string> headers_;
    std::string method_;
    std::string url_;
    bool hasGesture_;
    bool isMainFrame_;
    bool isRedirect_;
};

class ACE_EXPORT Result : public AceType {
    DECLARE_ACE_TYPE(Result, AceType)

public:
    Result() = default;
    ~Result() = default;

    virtual void Confirm() = 0;
    virtual void Confirm(const std::string& message) = 0;
    virtual void Cancel() = 0;
};

class ACE_EXPORT FileSelectorResult : public AceType {
    DECLARE_ACE_TYPE(FileSelectorResult, AceType)

public:
    FileSelectorResult() = default;
    ~FileSelectorResult() = default;

    virtual void HandleFileList(std::vector<std::string>& fileList) = 0;
};

class ACE_EXPORT WebDialogEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(WebDialogEvent, BaseEventInfo);

public:
    WebDialogEvent(const std::string& url, const std::string& message, const std::string& value, 
        const DialogEventType& type, const RefPtr<Result>& result)
        : BaseEventInfo("WebDialogEvent"), url_(url), message_(message), value_(value), type_(type), result_(result) {}
    ~WebDialogEvent() = default;

    const std::string& GetUrl() const
    {
        return url_;
    }

    const std::string& GetMessage() const
    {
        return message_;
    }

    const std::string& GetValue() const
    {
        return value_;
    }

    const RefPtr<Result>& GetResult() const
    {
        return result_;
    }

    const DialogEventType& GetType() const
    {
        return type_;
    }

private:
    std::string url_;
    std::string message_;
    std::string value_;
    DialogEventType type_;
    RefPtr<Result> result_;
};

class ACE_EXPORT AuthResult : public AceType {
    DECLARE_ACE_TYPE(AuthResult, AceType)

public:
    AuthResult() = default;
    ~AuthResult() = default;

    virtual bool Confirm(std::string &userName, std::string &pwd) = 0;
    virtual bool IsHttpAuthInfoSaved() = 0;
    virtual void Cancel() = 0;
};

class ACE_EXPORT WebHttpAuthEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(WebHttpAuthEvent, BaseEventInfo);

public:
    WebHttpAuthEvent(const RefPtr<AuthResult>& result, const std::string &host, const std::string &realm)
        : BaseEventInfo("WebHttpAuthEvent"), result_(result), host_(host), realm_(realm) {}
    ~WebHttpAuthEvent() = default;

    const RefPtr<AuthResult>& GetResult() const
    {
        return result_;
    }

    const std::string& GetHost() const
    {
        return host_;
    }

    const std::string& GetRealm() const
    {
        return realm_;
    }
private:
    RefPtr<AuthResult> result_;
    std::string host_;
    std::string realm_;
};

class ACE_EXPORT WebGeolocation : public AceType {
    DECLARE_ACE_TYPE(WebGeolocation, AceType)

public:
    WebGeolocation() = default;
    ~WebGeolocation() = default;

    virtual void Invoke(const std::string& origin, const bool& allow, const bool& retain) = 0;
};

class ACE_EXPORT LoadWebPageStartEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebPageStartEvent, BaseEventInfo);

public:
    explicit LoadWebPageStartEvent(const std::string& url) : BaseEventInfo
        ("LoadWebPageStartEvent"), loadedUrl_(url) {}
    ~LoadWebPageStartEvent() = default;

    const std::string& GetLoadedUrl() const
    {
        return loadedUrl_;
    }

private:
    std::string loadedUrl_;
};

class ACE_EXPORT LoadWebPageFinishEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebPageFinishEvent, BaseEventInfo);

public:
    explicit LoadWebPageFinishEvent(const std::string& url) : BaseEventInfo
        ("LoadWebPageFinishEvent"), loadedUrl_(url) {}
    ~LoadWebPageFinishEvent() = default;

    const std::string& GetLoadedUrl() const
    {
        return loadedUrl_;
    }

private:
    std::string loadedUrl_;
};

class ACE_EXPORT LoadWebProgressChangeEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebProgressChangeEvent, BaseEventInfo);

public:
    explicit LoadWebProgressChangeEvent(const int& newProgress)
        : BaseEventInfo("LoadWebProgressChangeEvent"), newProgress_(newProgress) {}
    ~LoadWebProgressChangeEvent() = default;

    const int& GetNewProgress() const
    {
        return newProgress_;
    }

private:
    int newProgress_;
};

class ACE_EXPORT LoadWebTitleReceiveEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebTitleReceiveEvent, BaseEventInfo);

public:
    explicit LoadWebTitleReceiveEvent(const std::string& title) : BaseEventInfo
        ("LoadWebTitleReceiveEvent"), title_(title) {}
    ~LoadWebTitleReceiveEvent() = default;

    const std::string& GetTitle() const
    {
        return title_;
    }

private:
    std::string title_;
};

class ACE_EXPORT UrlLoadInterceptEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(UrlLoadInterceptEvent, BaseEventInfo);

public:
    explicit UrlLoadInterceptEvent(const std::string& data) : BaseEventInfo
        ("UrlLoadInterceptEvent"), data_(data) {}
    ~UrlLoadInterceptEvent() = default;

    const std::string& GetData() const
    {
        return data_;
    }

private:
    std::string data_;
};

class ACE_EXPORT LoadWebGeolocationHideEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebGeolocationHideEvent, BaseEventInfo);

public:
    explicit LoadWebGeolocationHideEvent(const std::string& origin)
        : BaseEventInfo("LoadWebGeolocationHideEvent"), origin_(origin) {}
    ~LoadWebGeolocationHideEvent() = default;

    const std::string& GetOrigin() const
    {
        return origin_;
    }

private:
    std::string origin_;
};

class ACE_EXPORT LoadWebGeolocationShowEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebGeolocationShowEvent, BaseEventInfo);

public:
    LoadWebGeolocationShowEvent(const std::string& origin, const RefPtr<WebGeolocation>& webGeolocation)
        : BaseEventInfo("LoadWebGeolocationShowEvent"), origin_(origin), webGeolocation_(webGeolocation) {}
    ~LoadWebGeolocationShowEvent() = default;

    const std::string& GetOrigin() const
    {
        return origin_;
    }

    const RefPtr<WebGeolocation>& GetWebGeolocation() const
    {
        return webGeolocation_;
    }

private:
    std::string origin_;
    RefPtr<WebGeolocation> webGeolocation_;
};

class ACE_EXPORT DownloadStartEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(DownloadStartEvent, BaseEventInfo);

public:
    DownloadStartEvent(const std::string& url, const std::string& userAgent, const std::string& contentDisposition,
        const std::string& mimetype, long contentLength) : BaseEventInfo("DownloadStartEvent"), url_(url),
        userAgent_(userAgent), contentDisposition_(contentDisposition), mimetype_(mimetype),
        contentLength_(contentLength) {}
    ~DownloadStartEvent() = default;

    const std::string& GetUrl() const
    {
        return url_;
    }

    const std::string& GetUserAgent() const
    {
        return userAgent_;
    }

    const std::string& GetContentDisposition() const
    {
        return contentDisposition_;
    }

    const std::string& GetMimetype() const
    {
        return mimetype_;
    }

    long GetContentLength() const
    {
        return contentLength_;
    }

private:
    std::string url_;
    std::string userAgent_;
    std::string contentDisposition_;
    std::string mimetype_;
    long contentLength_;
};

class ACE_EXPORT ReceivedErrorEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(ReceivedErrorEvent, BaseEventInfo);

public:
    ReceivedErrorEvent(const RefPtr<WebRequest>& request, const RefPtr<WebError>& error)
        : BaseEventInfo("ReceivedErrorEvent"), request_(request), error_(error)
    {
        LOGI("ReceivedErrorEvent constructor");
    }
    ~ReceivedErrorEvent() = default;

    const RefPtr<WebRequest>& GetRequest() const
    {
        return request_;
    }

    const RefPtr<WebError>& GetError() const
    {
        return error_;
    }

private:
    RefPtr<WebRequest> request_;
    RefPtr<WebError> error_;
};

class ACE_EXPORT ReceivedHttpErrorEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(ReceivedHttpErrorEvent, BaseEventInfo);

public:
    ReceivedHttpErrorEvent(const RefPtr<WebRequest>& request, const RefPtr<WebResponse>& response)
        : BaseEventInfo("ReceivedHttpErrorEvent"), request_(request), response_(response)
    {
        LOGI("ReceivedHttpErrorEvent constructor");
    }
    ~ReceivedHttpErrorEvent() = default;

    const RefPtr<WebRequest>& GetRequest() const
    {
        return request_;
    }

    const RefPtr<WebResponse>& GetResponse() const
    {
        return response_;
    }

private:
    RefPtr<WebRequest> request_;
    RefPtr<WebResponse> response_;
};

class ACE_EXPORT OnInterceptRequestEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(OnInterceptRequestEvent, BaseEventInfo);

public:
    OnInterceptRequestEvent(const RefPtr<WebRequest>& request)
        : BaseEventInfo("OnInterceptRequestEvent"), request_(request)
    {
        LOGI("OnInterceptRequestEvent constructor");
    }
    ~OnInterceptRequestEvent() = default;

    const RefPtr<WebRequest>& GetRequest() const
    {
        return request_;
    }

private:
    RefPtr<WebRequest> request_;
};

class ACE_EXPORT LoadWebRequestFocusEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebRequestFocusEvent, BaseEventInfo);

public:
    explicit LoadWebRequestFocusEvent(const std::string& url) : BaseEventInfo
        ("LoadWebRequestFocusEvent"), focusUrl_(url) {}
    ~LoadWebRequestFocusEvent() = default;

    const std::string& GetRequestFocus() const
    {
        return focusUrl_;
    }
private:
    std::string focusUrl_;
};

class ACE_EXPORT LoadWebOnFocusEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebOnFocusEvent, BaseEventInfo);

public:
    explicit LoadWebOnFocusEvent(const std::string& url) : BaseEventInfo
        ("LoadWebOnFocusEvent"), focusUrl_(url) {}
    ~LoadWebOnFocusEvent() = default;

    const std::string& GetOnFocus() const
    {
        return focusUrl_;
    }
private:
    std::string focusUrl_;
};

class ACE_EXPORT LoadWebConsoleLogEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(LoadWebConsoleLogEvent, BaseEventInfo);

public:
    LoadWebConsoleLogEvent(RefPtr<WebConsoleLog> message) : BaseEventInfo("LoadWebConsoleLogEvent"),
                                                            message_(message) {}
    ~LoadWebConsoleLogEvent() = default;

    const RefPtr<WebConsoleLog> GetMessage() const
    {
        return message_;
    }

private:
    RefPtr<WebConsoleLog> message_;
};

class ACE_EXPORT RenderExitedEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(RenderExitedEvent, BaseEventInfo);

public:
    RenderExitedEvent(int32_t exitedReason) : BaseEventInfo("RenderExitedEvent"), exitedReason_(exitedReason) {}
    ~RenderExitedEvent() = default;

    int32_t GetExitedReason() const
    {
        return exitedReason_;
    }

private:
    int32_t exitedReason_;
};

class ACE_EXPORT RefreshAccessedHistoryEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(RefreshAccessedHistoryEvent, BaseEventInfo);

public:
    RefreshAccessedHistoryEvent(const std::string& url, bool isRefreshed) :
                                BaseEventInfo("RefreshAccessedHistoryEvent"),
                                url_(url),  isRefreshed_(isRefreshed) {}

    ~RefreshAccessedHistoryEvent() = default;

    const std::string& GetVisitedUrl() const
    {
        return url_;
    }

    bool IsRefreshed() const
    {
        return isRefreshed_;
    }

private:
    std::string url_;
    bool isRefreshed_;
};

class ACE_EXPORT FileSelectorEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(FileSelectorEvent, BaseEventInfo);

public:
    FileSelectorEvent(const RefPtr<WebFileSelectorParam>& param,
                      const RefPtr<FileSelectorResult>& result)
        : BaseEventInfo("FileSelectorEvent"), param_(param), result_(result)
    {
        LOGI("FileSelectorEvent constructor");
    }
    ~FileSelectorEvent() = default;

    const RefPtr<WebFileSelectorParam>& GetParam() const
    {
        return param_;
    }

    const RefPtr<FileSelectorResult>& GetFileSelectorResult() const
    {
        return result_;
    }

private:
    RefPtr<WebFileSelectorParam> param_;
    RefPtr<FileSelectorResult> result_;
};

class ACE_EXPORT ResourceLoadEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(ResourceLoadEvent, BaseEventInfo);

public:
    explicit ResourceLoadEvent(const std::string& url) : BaseEventInfo
        ("ResourceLoadEvent"), loadUrl_(url) {}
    ~ResourceLoadEvent() = default;

    const std::string& GetOnResourceLoadUrl() const
    {
        return loadUrl_;
    }
private:
    std::string loadUrl_;
};

class ACE_EXPORT ScaleChangeEvent : public BaseEventInfo {
    DECLARE_RELATIONSHIP_OF_CLASSES(ScaleChangeEvent, BaseEventInfo);

public:
    ScaleChangeEvent(float oldScale, float newScale) : BaseEventInfo
        ("ScaleChangeEvent"), oldScale_(oldScale), newScale_(newScale) {}
    ~ScaleChangeEvent() = default;

    float GetOnScaleChangeOldScale() const
    {
        return oldScale_;
    }

    float GetOnScaleChangeNewScale() const
    {
        return newScale_;
    }
private:
    float oldScale_ = 0.0f;
    float newScale_ = 0.0f;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_WEB_EVENT_H
