#include "Utils/Debug/Logger.h"
#include "src/TestManager.h"

int main(int argc, char* argv[]) {
    Quasi::Debug::QInfo$("Execution Directory: {}", argv[0]);
    Quasi::Debug::QInfo$("Project Directory: {}", Test::PROJECT_DIRECTORY);

    Quasi::HashMap<Quasi::String, int> map;
    map.Insert("high", 2);
    map.Insert("main", 0);
    map.Insert("shadow", 4);
    map.Insert("outline", 1);
    map.Insert("glow", 3);
    for (const auto& [key, val] : map) {

        Quasi::Debug::QInfo$("{}: {} but got {}", key, val, map.Get(key));
    }

    Test::TestManager tManager;
    tManager.OnInit();
    while (tManager.WindowIsOpen())
        tManager.OnRun();
    tManager.OnDestroy();
}
