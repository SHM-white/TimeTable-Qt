#include "WindowEffects.h"
#include <QWidget>
#ifdef Q_OS_WIN
#include <QLibrary>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace tt {
bool applyAcrylic(QWidget* window,bool enabled,const QColor& tint) {
#ifdef Q_OS_WIN
    if(!window)return false;
    const auto handle=reinterpret_cast<HWND>(window->winId());
    static QLibrary dwm(QStringLiteral("dwmapi"));
    using SetAttribute=HRESULT (WINAPI*)(HWND,DWORD,LPCVOID,DWORD);
    const auto setAttribute=reinterpret_cast<SetAttribute>(dwm.resolve("DwmSetWindowAttribute"));
    if(setAttribute) {
        // Attribute 38 is accepted only on recent Windows; older DWM rejects it.
        const int backdrop=enabled?3:1;
        if(SUCCEEDED(setAttribute(handle,38,&backdrop,sizeof(backdrop))))return enabled;
    }
    static QLibrary user(QStringLiteral("user32"));
    struct AccentPolicy { int state;int flags;DWORD color;int animation; };
    struct AttributeData { int attribute;void* data;SIZE_T size; };
    using SetComposition=BOOL (WINAPI*)(HWND,AttributeData*);
    const auto setComposition=reinterpret_cast<SetComposition>(user.resolve("SetWindowCompositionAttribute"));
    if(setComposition) {
        const DWORD color=(DWORD(tint.alpha())<<24)|(DWORD(tint.blue())<<16)|(DWORD(tint.green())<<8)|DWORD(tint.red());
        AccentPolicy accent{enabled?4:0,2,color,0};
        AttributeData data{19,&accent,sizeof(accent)};
        if(setComposition(handle,&data))return enabled;
    }
    // Windows 7 compatibility: dynamically resolve the older DWM blur API.
    struct BlurBehind { DWORD flags;BOOL enable;HRGN region;BOOL transition; };
    using EnableBlur=HRESULT (WINAPI*)(HWND,const BlurBehind*);
    const auto enableBlur=reinterpret_cast<EnableBlur>(dwm.resolve("DwmEnableBlurBehindWindow"));
    if(enableBlur) {
        const BlurBehind blur{1,enabled?TRUE:FALSE,nullptr,FALSE};
        if(SUCCEEDED(enableBlur(handle,&blur)))return enabled;
    }
#else
    Q_UNUSED(window);Q_UNUSED(enabled);Q_UNUSED(tint);
#endif
    return false;
}
}
