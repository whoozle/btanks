#include "registrar.h"
#include "resource_manager.h"

void Registrar::registerObject(const std::string &name, Object *object) {
	ResourceManager->registerObject(name, object);
}
