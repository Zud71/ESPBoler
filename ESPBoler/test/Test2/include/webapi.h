#include "config.h"

void setupWebApi();
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);