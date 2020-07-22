#include "Application.hpp"

#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>

int main() {
    plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
    plog::init(plog::verbose, &console_appender);

	Application app{};
	app.Run();
	return 0;
}
