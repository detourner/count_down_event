#ifndef WEB_UI_H
#define WEB_UI_H

#include <ESPAsyncWebServer.h>
#include "event_list.h"

typedef void (*CallbackTypeWebUi)(void);

class EventListWebUi
{
    public:
        EventListWebUi();

        void begin(AsyncWebServer &server, EventList &events, CallbackTypeWebUi callback = nullptr);
       

        // Register web UI routes on the provided server instance and bind to given EventList
        void setupWebUiRoutes(AsyncWebServer &server, EventList &events);

        void notifyClients(void);

    private:

        void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
                   AwsEventType type, void * arg, uint8_t *data, size_t len);

        AsyncWebServer* _server = nullptr;
        EventList* _events = nullptr;
        AsyncWebSocket _ws{"/ws"};

        CallbackTypeWebUi _callback;


};

#endif // WEB_UI_H


