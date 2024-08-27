/* Copyright 2021 UFACTORY Inc. All Rights Reserved.
 *
 * Software License Agreement (BSD License)
 *
 * Author: Vinman <vinman.cub@gmail.com>
 ============================================================================*/

#include "xarm_planner/xarm_planner.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
using namespace std;

#define HOST "127.0.0.1" // Dirección IP del servidor
#define PORT 12345       // Puerto en el que el servidor está escuchando
#define BUFFER_SIZE 1024

int action = 0;
bool contador = false;

void exit_sig_handler(int signum)
{
    fprintf(stderr, "[test_xarm_planner_api_pose] Ctrl-C caught, exit process...\n");
    exit(-1);
}

int main(int argc, char** argv)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir la dirección IP a binario
    if (inet_pton(AF_INET, HOST, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Dirección inválida o no soportada" << std::endl;
        return -1;
    }

    // Conectarse al servidor
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error en la conexión" << std::endl;
        return -1;
    }

    std::cout << "Conectado al servidor en " << HOST << ":" << PORT << std::endl;

    rclcpp::init(argc, argv);
    rclcpp::NodeOptions node_options;
    node_options.automatically_declare_parameters_from_overrides(true);
    std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("test_xarm_planner_api_pose", node_options);
    RCLCPP_INFO(node->get_logger(), "test_xarm_planner_api_pose start");

    signal(SIGINT, exit_sig_handler);

    int dof;
    node->get_parameter_or("dof", dof, 7);
    std::string robot_type;
    node->get_parameter_or("robot_type", robot_type, std::string("xarm"));
    std::string group_name = robot_type;
    if (robot_type == "xarm" || robot_type == "lite")
        group_name = robot_type + std::to_string(dof);

    RCLCPP_INFO(node->get_logger(), "namespace: %s, group_name: %s", node->get_namespace(), group_name.c_str());

    xarm_planner::XArmPlanner planner(node, group_name);

    // Prisma Triangular
    geometry_msgs::msg::Pose target_pose1;
    target_pose1.position.x = 0.1;
	target_pose1.position.y = 0.1;
	target_pose1.position.z = 0.2;
	target_pose1.orientation.x = 0;
	target_pose1.orientation.y = 1;
	target_pose1.orientation.z = 0;
	target_pose1.orientation.w = 0;
    
    while (rclcpp::ok())
    {
        // Recibir datos del servidor
        int valread = recv(sock, buffer, BUFFER_SIZE, 0);

        // Verificar si no hay más datos
        if (valread <= 0) {
            continue;
        }

        // Convertir los datos recibidos a string
        std::string data(buffer, valread);

        // Procesar solo el primer dígito del valor
        try {
            std::string first_digit(1, data.at(0)); // Extraer el primer carácter
            action = std::stoi(first_digit); // Convertir el primer carácter a un entero
            std::cout << "Primer dígito recibido del servidor: " << action << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: dato recibido está vacío" << std::endl;
            continue;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Valor recibido no es un dígito válido: " << data << std::endl;
            continue;
        }

        if (contador == false){
            planner.planPoseTarget(target_pose1);
            planner.executePath();
            contador = true;
            cout << "Entré 1 vez" << endl;
        } else if (contador == true) {
            if (action == 1){ // Arriba
                if (target_pose1.position.z >= 0.49){
                    target_pose1.position.z = 0.5;
                } else {
                    target_pose1.position.z += 0.05;
                }
                planner.planPoseTarget(target_pose1);
            } else if (action == 2){ // Abajo
                if (target_pose1.position.z <= 0.16){
                    target_pose1.position.z = 0.15;
                } else {
                    target_pose1.position.z -= 0.05;
                }
                planner.planPoseTarget(target_pose1);
            } else if (action == 3){ // Izquierda
                if (target_pose1.position.y >= 0.24){
                    target_pose1.position.y = 0.25;
                } else {
                    target_pose1.position.y += 0.05;
                }
                planner.planPoseTarget(target_pose1);
            } else if (action == 4){ // Derecha
                if (target_pose1.position.y <= -0.26){
                    target_pose1.position.y = -0.25;
                } else {
                    target_pose1.position.y -= 0.05;
                }
                planner.planPoseTarget(target_pose1);
            } else if (action == 5){ // Atrás
                if (target_pose1.position.x <= 0.31){
                    target_pose1.position.x = 0.3;
                } else {
                    target_pose1.position.x -= 0.05;
                }
                planner.planPoseTarget(target_pose1);
            } else if (action == 6){ // Adelante
                if (target_pose1.position.x >= 0.39){
                    target_pose1.position.x = 0.4;
                } else {
                    target_pose1.position.x += 0.05;
                }
                planner.planPoseTarget(target_pose1);
            }
            //planner.planPoseTarget(target_pose1);
            planner.executePath();
            cout << "Entré a actualizar" << endl;
        }
        cout << "pos z " << target_pose1.position.z << endl;
        cout << "pos y " << target_pose1.position.y << endl;
        cout << "pos x " << target_pose1.position.x << endl;
    }

    // Cerrar el socket
    close(sock);
    RCLCPP_INFO(node->get_logger(), "test_xarm_planner_api_pose over");
    return 0;
}

/*
#include "xarm_planner/xarm_planner.h"

void exit_sig_handler(int signum)
{
    fprintf(stderr, "[test_xarm_planner_api_pose] Ctrl-C caught, exit process...\n");
    exit(-1);
}

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::NodeOptions node_options;
    node_options.automatically_declare_parameters_from_overrides(true);
    std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("test_xarm_planner_api_pose", node_options);
    RCLCPP_INFO(node->get_logger(), "test_xarm_planner_api_pose start");

    signal(SIGINT, exit_sig_handler);

    int dof;
    node->get_parameter_or("dof", dof, 7);
    std::string robot_type;
    node->get_parameter_or("robot_type", robot_type, std::string("xarm"));
    std::string group_name = robot_type;
    if (robot_type == "xarm" || robot_type == "lite")
        group_name = robot_type + std::to_string(dof);

    RCLCPP_INFO(node->get_logger(), "namespace: %s, group_name: %s", node->get_namespace(), group_name.c_str());

    xarm_planner::XArmPlanner planner(node, group_name);

    // Prisma Triangular
    geometry_msgs::msg::Pose target_pose1;
    target_pose1.position.x = 0.2;
	target_pose1.position.y = 0.1;
	target_pose1.position.z = 0.15;
	target_pose1.orientation.x = 0;
	target_pose1.orientation.y = 1;
	target_pose1.orientation.z = 0;
	target_pose1.orientation.w = 0;

    geometry_msgs::msg::Pose target_pose2;
    target_pose2.position.x = 0.2;
	target_pose2.position.y = -0.1;
	target_pose2.position.z = 0.3;
	target_pose2.orientation.x = 0;
	target_pose2.orientation.y = 1;
	target_pose2.orientation.z = 0;
	target_pose2.orientation.w = 0;

    geometry_msgs::msg::Pose target_pose3;
    target_pose3.position.x = 0.4;
	target_pose3.position.y = 0.2;
	target_pose3.position.z = 0.3;
	target_pose3.orientation.x = 0;
	target_pose3.orientation.y = 1;
	target_pose3.orientation.z = 0;
	target_pose3.orientation.w = 0;

    geometry_msgs::msg::Pose target_pose4;
    target_pose4.position.x = 0.2;
	target_pose4.position.y = 0.1;
	target_pose4.position.z = 0.3;
	target_pose4.orientation.x = 0;
	target_pose4.orientation.y = 1;
	target_pose4.orientation.z = 0;
	target_pose4.orientation.w = 0;

    geometry_msgs::msg::Pose target_pose5;
    target_pose5.position.x = 0.2;
	target_pose5.position.y = 0.1;
	target_pose5.position.z = 0.4;
	target_pose5.orientation.x = 0;
	target_pose5.orientation.y = 1;
	target_pose5.orientation.z = 0;
	target_pose5.orientation.w = 0;

    geometry_msgs::msg::Pose target_pose6;
    target_pose6.position.x = 0.2;
	target_pose6.position.y = 0.3;
	target_pose6.position.z = 0.4;
	target_pose6.orientation.x = 0;
	target_pose6.orientation.y = 1;
	target_pose6.orientation.z = 0;
	target_pose6.orientation.w = 0;

    geometry_msgs::msg::Pose target_pose7;
    target_pose7.position.x = 0.4;
	target_pose7.position.y = 0.2;
	target_pose7.position.z = 0.4;
	target_pose7.orientation.x = 0;
	target_pose7.orientation.y = 1;
	target_pose7.orientation.z = 0;
	target_pose7.orientation.w = 0;

    while (rclcpp::ok())
    {
        // Prisma Triangular
        planner.planPoseTarget(target_pose1);
        planner.executePath();

        planner.planPoseTarget(target_pose2);
        planner.executePath();

        planner.planPoseTarget(target_pose3);
        planner.executePath();
        
        planner.planPoseTarget(target_pose1);
        planner.executePath();
                
        planner.planPoseTarget(target_pose5);
        planner.executePath();

        planner.planPoseTarget(target_pose6);
        planner.executePath();

        planner.planPoseTarget(target_pose7);
        planner.executePath();

        planner.planPoseTarget(target_pose5);
        planner.executePath();

        planner.planPoseTarget(target_pose1);
        planner.executePath();

        planner.planPoseTarget(target_pose3);
        planner.executePath();

        planner.planPoseTarget(target_pose7);
        planner.executePath();

        planner.planPoseTarget(target_pose6);
        planner.executePath();

        planner.planPoseTarget(target_pose2);
        planner.executePath();
    }

    RCLCPP_INFO(node->get_logger(), "test_xarm_planner_api_pose over");
    return 0;
}*/