#pragma once

#include "Core/application.h"

int main() {
    RayTracer::Application application(1000, 500);
    application.run();
    return 0;
}