#pragma once
#include <ESPAsyncWebServer.h>
#include "event_list.h"

// Register web UI routes on the provided server instance and bind to given EventList
void setupWebUiRoutes(AsyncWebServer &server, EventList &events);
