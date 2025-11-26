#include "GameController.h"

#include "console/TerminalInputHandler.h"
#include "console/ConsoleRenderer.h"
#include "GUI/GUIInputHandler.h"
#include "GUI/GUIRenderer.h"
#include "Renderer.h"

// Явная инстанциация шаблонов — ДОЛЖНА быть в .cpp, не в .h
template class GameController<TerminalInputHandler, Renderer<ConsoleRenderer>>;
template class GameController<GUIInputHandler,     Renderer<GUIRenderer>>;
