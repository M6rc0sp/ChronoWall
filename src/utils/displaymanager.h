#pragma once
#include <QString>

class DisplayManager {
public:
    static QString findDisplay();

private:
    static bool testDisplay(const QString &display);
};
