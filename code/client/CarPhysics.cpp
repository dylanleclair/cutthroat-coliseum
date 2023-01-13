#include "CarPhysics.h"

#include <iostream>

// component serializer/deserializer
// it holds a copy of the component that can be used to
CarPhysicsConfig::CarPhysicsConfig(CarPhysics &carPhysics) : m_car_config(carPhysics) {
    // read in config file
    auto result = toml::parse_file(FILEPATH);
    if (!result) {
        std::cerr << "Couldn't parse CarPhysics.toml" << result.error() << "\n";
        std::exit(2);
    }

    table = std::move(result).table();
}

void CarPhysicsConfig::serialize() {
    **table["suspension_force"].as_floating_point() = m_car_config.m_suspension_force;
    **table["acceleration"].as_floating_point() = m_car_config.m_acceleration;
    std::cout << "\n\nSerializing these values IN MEMORY\n";
    std::cout << "acceleration: " <<m_car_config.m_acceleration << "\n";
    std::cout << "suspension force: " << m_car_config.m_suspension_force << "\n\n";
    std::cout << "Which form this table:\n";
    std::cout << table << "\n\n";
    std::cout << "To this file (relative to executable):\n";
    std::cout << ASSETS_FILEPATH << "\n\n";

    std::ofstream(ASSETS_FILEPATH) << table;
}

// NOTE(beau) crashes if keys are not in file
void CarPhysicsConfig::deserialize() {
    // NOTE(beau) crash if keys not found
    m_car_config.m_suspension_force = *( table["suspension_force"].value<float>() );
    m_car_config.m_acceleration = *( table["acceleration"].value<float>() );

    std::cout << "\n\nDeserialized this table: " << "\n";
    std::cout << table << "\n\n";
    std::cout << "From this file (relative to executable): " << "\n";
    std::cout << FILEPATH << "\n\n";
    std::cout << "To these values in memory:\n";
    std::cout << "acceleration: " <<m_car_config.m_acceleration << "\n";
    std::cout << "suspension force : " << m_car_config.m_suspension_force << "\n\n";
}
