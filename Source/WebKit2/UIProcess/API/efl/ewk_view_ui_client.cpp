/*
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "WKString.h"
#include "ewk_view_private.h"
#include "ewk_view_ui_client_private.h"
#include <Ecore_Evas.h>

static inline Evas_Object* toEwkView(const void* clientInfo)
{
    return static_cast<Evas_Object*>(const_cast<void*>(clientInfo));
}

static void closePage(WKPageRef, const void* clientInfo)
{
    ewk_view_page_close(toEwkView(clientInfo));
}

static WKPageRef createNewPage(WKPageRef, WKURLRequestRef, WKDictionaryRef, WKEventModifiers, WKEventMouseButton, const void* clientInfo)
{
     return ewk_view_page_create(toEwkView(clientInfo));
}

static void runJavaScriptAlert(WKPageRef, WKStringRef alertText, WKFrameRef, const void* clientInfo)
{
    ewk_view_run_javascript_alert(toEwkView(clientInfo), WKEinaSharedString(alertText));
}

static bool runJavaScriptConfirm(WKPageRef, WKStringRef message, WKFrameRef, const void* clientInfo)
{
    return ewk_view_run_javascript_confirm(toEwkView(clientInfo), WKEinaSharedString(message));
}

static WKStringRef runJavaScriptPrompt(WKPageRef, WKStringRef message, WKStringRef defaultValue, WKFrameRef, const void* clientInfo)
{
    WKEinaSharedString value = ewk_view_run_javascript_prompt(toEwkView(clientInfo), WKEinaSharedString(message), WKEinaSharedString(defaultValue));
    return value ? WKStringCreateWithUTF8CString(value) : 0;
}

#if ENABLE(INPUT_TYPE_COLOR)
static void showColorPicker(WKPageRef, WKStringRef initialColor, WKColorPickerResultListenerRef listener, const void* clientInfo)
{
    WebCore::Color color = WebCore::Color(WebKit::toWTFString(initialColor));
    ewk_view_color_picker_request(toEwkView(clientInfo), color.red(), color.green(), color.blue(), color.alpha(), listener);
}

static void hideColorPicker(WKPageRef, const void* clientInfo)
{
    ewk_view_color_picker_dismiss(toEwkView(clientInfo));
}
#endif

#if ENABLE(SQL_DATABASE)
static unsigned long long exceededDatabaseQuota(WKPageRef, WKFrameRef, WKSecurityOriginRef, WKStringRef databaseName, WKStringRef displayName, unsigned long long currentQuota, unsigned long long currentOriginUsage, unsigned long long currentDatabaseUsage, unsigned long long expectedUsage, const void* clientInfo)
{
    return ewk_view_database_quota_exceeded(toEwkView(clientInfo), WKEinaSharedString(databaseName), WKEinaSharedString(displayName), currentQuota, currentOriginUsage, currentDatabaseUsage, expectedUsage);
}
#endif

static void focus(WKPageRef, const void* clientInfo)
{
    evas_object_focus_set(toEwkView(clientInfo), true);
}

static void unfocus(WKPageRef, const void* clientInfo)
{
    evas_object_focus_set(toEwkView(clientInfo), false);
}

static void takeFocus(WKPageRef, WKFocusDirection, const void* clientInfo)
{
    // FIXME: this is only a partial implementation.
    evas_object_focus_set(toEwkView(clientInfo), false);
}

static WKRect getWindowFrame(WKPageRef, const void* clientInfo)
{
    int x, y, width, height;

    Ecore_Evas* ee = ecore_evas_ecore_evas_get(evas_object_evas_get(toEwkView(clientInfo)));
    ecore_evas_request_geometry_get(ee, &x, &y, &width, &height);

    return WKRectMake(x, y, width, height);
}

static void setWindowFrame(WKPageRef, WKRect frame, const void* clientInfo)
{
    Ecore_Evas* ee = ecore_evas_ecore_evas_get(evas_object_evas_get(toEwkView(clientInfo)));
    ecore_evas_move_resize(ee, frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
}

void ewk_view_ui_client_attach(WKPageRef pageRef, Evas_Object* ewkView)
{
    WKPageUIClient uiClient;
    memset(&uiClient, 0, sizeof(WKPageUIClient));
    uiClient.version = kWKPageUIClientCurrentVersion;
    uiClient.clientInfo = ewkView;
    uiClient.close = closePage;
    uiClient.createNewPage = createNewPage;
    uiClient.runJavaScriptAlert = runJavaScriptAlert;
    uiClient.runJavaScriptConfirm = runJavaScriptConfirm;
    uiClient.runJavaScriptPrompt = runJavaScriptPrompt;
    uiClient.takeFocus = takeFocus;
    uiClient.focus = focus;
    uiClient.unfocus = unfocus;
    uiClient.getWindowFrame = getWindowFrame;
    uiClient.setWindowFrame = setWindowFrame;
#if ENABLE(SQL_DATABASE)
    uiClient.exceededDatabaseQuota = exceededDatabaseQuota;
#endif

#if ENABLE(INPUT_TYPE_COLOR)
    uiClient.showColorPicker = showColorPicker;
    uiClient.hideColorPicker = hideColorPicker;
#endif

    WKPageSetPageUIClient(pageRef, &uiClient);
}