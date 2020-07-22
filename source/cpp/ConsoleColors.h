#pragma once
#include <QDebug>

#define cRST ("\u001B[0m")
#define cEOL (cRST)
#define cYLW ("\u001B[38;5;226m")
#define cORG ("\u001B[38;5;214m")
#define cGRN ("\u001B[38;5;070m")
#define cRED ("\u001B[38;5;160m")
#define cBLU ("\u001B[38;5;027m")

#define cLV0 ("\u001B[38;5;177m")
#define cLV1 ("\u001B[38;5;148m")
#define cLV2 ("\u001B[38;5;228m")

#define CON (qDebug().noquote())
