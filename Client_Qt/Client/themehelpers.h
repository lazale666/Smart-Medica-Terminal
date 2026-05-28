#ifndef THEMEHELPERS_H
#define THEMEHELPERS_H

#include <QString>

namespace ThemeHelpers {

inline bool isLightTheme(const QString &bgColor)
{
    return bgColor.compare("#F5FBFF", Qt::CaseInsensitive) == 0
        || bgColor.compare("#ffffff", Qt::CaseInsensitive) == 0;
}

inline QString normalizeBgColor(const QString &bgColor)
{
    return isLightTheme(bgColor) ? "#F5FBFF" : "#07111F";
}

inline QString defaultFontColorForBg(const QString &bgColor)
{
    return isLightTheme(bgColor) ? "#0F2740" : "#D8F7FF";
}

inline QString titleColor(const QString &bgColor)
{
    return isLightTheme(bgColor) ? "#0F2740" : "#00E5FF";
}

inline QString statusOkColor(const QString &bgColor)
{
    return isLightTheme(bgColor) ? "#157A52" : "#31FFB7";
}

inline QString statusWarnColor(const QString &bgColor)
{
    return isLightTheme(bgColor) ? "#9A6A00" : "#FFCF5A";
}

inline QString statusErrorColor(const QString &bgColor)
{
    return isLightTheme(bgColor) ? "#B13A52" : "#FF5F7E";
}

} // namespace ThemeHelpers

#endif // THEMEHELPERS_H
