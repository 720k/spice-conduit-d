#pragma once
#include <QDebug>

#define cRST ("\e[0m")
#define cEOL (cRST)
#define cYLW ("\e[38;5;226m")
#define cORG ("\e[38;5;214m")
#define cGRN ("\e[38;5;070m")
#define cRED ("\e[38;5;160m")
#define cBLU ("\e[38;5;027m")

#define cLV0 ("\e[38;5;177m")
#define cLV1 ("\e[38;5;148m")
#define cLV2 ("\e[38;5;228m")

#define CON (qDebug().noquote())
