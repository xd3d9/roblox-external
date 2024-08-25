#pragma once
#include <vector>

#include "../globals.hpp"
#include <mutex>



class Player {
public:
    bool isLocalPlayer = false;
    bool isSameTeam = false;


    instance_t playerInstance;
    instance_t headInstance;
    instance_t hrpInstance;
    instance_t humanoid;
    instance_t modelinstance;
    instance_t leftHandInstance;
    instance_t rightHandInstance;
    instance_t leftLowerArmInstance;
    instance_t rightLowerArmInstance;
    instance_t leftArmInstance;
    instance_t rightArmInstance;
    instance_t leftUpperArmInstance;
    instance_t rightUpperArmInstance;
    instance_t leftFootInstance;
    instance_t rightFootInstance;
    instance_t leftLegInstance;
    instance_t rightLegInstance;
    instance_t leftLowerLegInstance;
    instance_t rightLowerLegInstance;
    instance_t leftUpperLegInstance;
    instance_t rightUpperLegInstance;
    instance_t upperTorsoInstance;
    instance_t lowerTorsoInstance;

    vector2_t hrpPos2d;
    instance_t armorPath;
    vector2_t leftHandInstance2d;
    vector2_t rightHandInstance2d;
    vector2_t leftLowerArmInstance2d;
    vector2_t rightLowerArmInstance2d;
    vector2_t leftUpperArmInstance2d;
    vector2_t rightUpperArmInstance2d;
    vector2_t leftArmInstance2d;
    vector2_t rightArmInstance2d;
    vector2_t leftFootInstance2d;
    vector2_t rightFootInstance2d;
    vector2_t leftLegInstance2d;
    vector2_t rightLegInstance2d;
    vector2_t leftLowerLegInstance2d;
    vector2_t leftUpperLegInstance2d;
    vector2_t rightLowerLegInstance2d;
    vector2_t rightUpperLegInstance2d;
    vector2_t upperTorsoInstance2d;
    vector2_t lowerTorsoInstance2d;

    vector3_t hrpPos3d;
    vector3_t leftHandInstance3d;
    vector3_t rightHandInstance3d;
    vector3_t leftLowerArmInstance3d;
    vector3_t rightLowerArmInstance3d;
    vector3_t leftUpperArmInstance3d;
    vector3_t rightUpperArmInstance3d;
    vector3_t leftArmInstance3d;
    vector3_t rightArmInstance3d;
    vector3_t leftFootInstance3d;
    vector3_t rightFootInstance3d;
    vector3_t leftLegInstance3d;
    vector3_t rightLegInstance3d;
    vector3_t leftLowerLegInstance3d;
    vector3_t rightLowerLegInstance3d;
    vector3_t leftUpperLegInstance3d;
    vector3_t rightUpperLegInstance3d;
    vector3_t upperTorsoInstance3d;
    vector3_t lowerTorsoInstance3d;
    vector3_t velocity;

    matrix3_t hrpRot;

    std::string playerName;

    Player() :
        playerInstance(),
        headInstance(),
        hrpInstance(),
        humanoid(),
        modelinstance(),
        leftHandInstance(),
        rightHandInstance(),
        leftLowerArmInstance(),
        rightLowerArmInstance(),
        leftArmInstance(),
        rightArmInstance(),
        leftUpperArmInstance(),
        rightUpperArmInstance(),
        leftFootInstance(),
        rightFootInstance(),
        leftLegInstance(),
        rightLegInstance(),
        leftLowerLegInstance(),
        rightLowerLegInstance(),
        leftUpperLegInstance(),
        rightUpperLegInstance(),
        upperTorsoInstance(),
        lowerTorsoInstance(),
        hrpPos2d(),
        armorPath(),
        leftHandInstance2d(),
        rightHandInstance2d(),
        leftArmInstance2d(),
        rightArmInstance2d(),
        leftLowerArmInstance2d(),
        rightLowerArmInstance2d(),
        leftUpperArmInstance2d(),
        rightUpperArmInstance2d(),
        leftFootInstance2d(),
        rightFootInstance2d(),
        leftLegInstance2d(),
        rightLegInstance2d(),
        leftLowerLegInstance2d(),
        leftUpperLegInstance2d(),
        rightLowerLegInstance2d(),
        rightUpperLegInstance2d(),
        upperTorsoInstance2d(),
        lowerTorsoInstance2d(),
        // Initializing 3D positions
        hrpPos3d(),
        leftHandInstance3d(),
        rightHandInstance3d(),
        leftLowerArmInstance3d(),
        rightLowerArmInstance3d(),
        leftUpperArmInstance3d(),
        rightUpperArmInstance3d(),
        leftArmInstance3d(),
        rightArmInstance3d(),
        leftFootInstance3d(),
        rightFootInstance3d(),
        leftLegInstance3d(),
        rightLegInstance3d(),
        leftLowerLegInstance3d(),
        rightLowerLegInstance3d(),
        leftUpperLegInstance3d(),
        rightUpperLegInstance3d(),
        upperTorsoInstance3d(),
        lowerTorsoInstance3d(),
        playerName(),
        velocity(),
        hrpRot()
    {}
};


Player find_local_player();

vector3_t getLocalPlayerPosition();

extern std::vector<Player> playerList;