#pragma once

#include "Core/application.h"

int main() {
    RayTracer::Application application(1000, 1000);
    application.run();
    return 0;
}