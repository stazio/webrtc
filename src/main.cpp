#include <iostream>
#include "version.h"

int main(int argc, char** argv) {
    std::cout << "Starting " << PROJECT_NAME << " v. [" << GIT_REV << "]" <<std::endl;
}
