#include "app.h"

int main(){
    //FreeConsole();
    app = new _app();
    if(app->init()){ app->run(); }

}
