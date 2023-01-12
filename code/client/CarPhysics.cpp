#include "CarPhysics.h"

#include <iostream>

// component serializer/deserializer
// it holds a copy of the component that can be used to
CarPhysicsConfig::CarPhysicsConfig(bool serializing) {
    // read in config file
    auto result = toml::parse_file(FILEPATH);
    if (!result) {
        std::cerr << "Couldn't parse CarPhysics.toml" << result.error() << "\n";
        std::exit(2);
    }

    table = std::move(result).table();

    deserialize();
    if (serializing) serialize();
}

void CarPhysicsConfig::serialize() {
    **table["suspension_force"].as_floating_point() = tempdata.m_suspension_force;
    **table["acceleration"].as_floating_point() = tempdata.m_acceleration;
    std::cout << "Serializing: \n" << table << "\n";
    std::ofstream configFile(ASSETS_FILEPATH);
    configFile << table;
}

// NOTE(beau) crashes if keys are not in file
void CarPhysicsConfig::deserialize() {
    // NOTE(beau) crash if keys not found
    tempdata.m_suspension_force = *( table["suspension_force"].value<float>() );
    tempdata.m_acceleration = *( table["acceleration"].value<float>() );

    std::cout << "Deserialized:" << "\n";
    std::cout << tempdata.m_suspension_force << "\n";
    std::cout << tempdata.m_acceleration << "\n";
}
