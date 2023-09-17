#include "externs.h"

int timesGameRestarted = 0;

inline void TransformFromObjectSpace(CEntity* self, CVector& outPos, const CVector& offset)
{
    if(self->m_matrix)
    {
        outPos = *self->m_matrix * offset;
        return;
    }
    TransformPoint((RwV3d&)outPos, self->m_placement, (RwV3d&)offset);
}
inline CVector TransformFromObjectSpace(CEntity* ent, const CVector& offset)
{
    auto result = CVector();
    if (ent->m_matrix)
    {
        result = *ent->m_matrix * offset;
        return result;
    }

    TransformPoint((RwV3d&)result, ent->m_placement, (RwV3d&)offset);
    return result;
}

CLEO_Fn(IS_ON_MISSION)
{
    UpdateCompareFlag((CRunningScript*)handle, *OnAMissionFlag && *(ScriptSpace + *OnAMissionFlag));
}
CLEO_Fn(IS_ON_SAMP)
{
    UpdateCompareFlag((CRunningScript*)handle, (
        aml->GetLibHandle("libsamp.so") != NULL || aml->GetLibHandle("libvoice.so") != NULL || aml->GetLibHandle("libAlyn_SAMPMOBILE.so") != NULL || aml->HasMod("net.rusjj.resamp")
    ));
}
CLEO_Fn(IS_ON_CUTSCENE)
{
    UpdateCompareFlag((CRunningScript*)handle, *ms_running);
}
CLEO_Fn(FIX_CHAR_GROUND_BRIGHTNESS_AND_FADE_IN)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    int ground = cleo->ReadParam(handle)->i;
    int brightness = cleo->ReadParam(handle)->i;
    int fade = cleo->ReadParam(handle)->i;

    if(!ped) return;

    if (fade && ped->m_pRwClump) SetClumpAlpha(ped->m_pRwClump, 0);

    CVector *pedPos = NULL;
    if(ground)
    {
        pedPos = &ped->GetPosition();

        bool foundGround;
        CEntity *foundEntity;
        float foundZ = FindGroundZFor3DCoord(pedPos->x, pedPos->y, pedPos->z, &foundGround, &foundEntity);
        if (foundGround)
        {
            pedPos->z = GetDistanceFromCentreOfMassToBaseOfModel(ped) + foundZ;
            ped->Teleport(*pedPos);
        }
    }

    if(brightness)
    {
        if (!pedPos) pedPos = &ped->GetPosition();

        CEntity *outEntity;
        CColPoint outColPoint;
        if (ProcessVerticalLine(*pedPos, -1000.0, outColPoint, outEntity, 1, 0, 0, 0, 0, 0, NULL))
        {
            ped->m_fContactSurfaceBrightness =
                      outColPoint.m_nLightingB.day
                    * 0.033333334f
                    * (1.0f - *m_fDNBalanceParam)
                    + outColPoint.m_nLightingB.night
                    * 0.033333334f
                    * *m_fDNBalanceParam;
        }
    }
}
CLEO_Fn(IS_WEAPON_FIRE_TYPE)
{
    eWeaponType weaponType = (eWeaponType)cleo->ReadParam(handle)->i;
    int fireType = cleo->ReadParam(handle)->i;

    bool ok = false;
    auto weaponInfo = GetWeaponInfo(weaponType, 1);
    if (!weaponInfo) GetWeaponInfo(weaponType, 0);
    if (weaponInfo) ok = weaponInfo->m_nWeaponFire == fireType;
    UpdateCompareFlag((CRunningScript*)handle, ok);
}
CLEO_Fn(GET_CURRENT_SAVE_SLOT)
{
    cleo->GetPointerToScriptVar(handle)->i = g_nCurrentSaveSlot;
}
CLEO_Fn(IS_GAME_FIRST_START)
{
    UpdateCompareFlag((CRunningScript*)handle, timesGameRestarted == 0);
}
CLEO_Fn(SET_CHAR_COORDINATES_SIMPLE)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    ped->m_matrix->pos.x = cleo->ReadParam(handle)->f;
    ped->m_matrix->pos.y = cleo->ReadParam(handle)->f;
    ped->m_matrix->pos.z = cleo->ReadParam(handle)->f;
}
CLEO_Fn(FRAME_MOD)
{
    int frame = cleo->ReadParam(handle)->i;
    UpdateCompareFlag((CRunningScript*)handle, *m_FrameCounter % frame == 0);
}
CLEO_Fn(RANDOM_PERCENT)
{
    int percent = cleo->ReadParam(handle)->i;
    int random = rand() % 101;
    UpdateCompareFlag((CRunningScript*)handle, random <= percent);
}
CLEO_Fn(GET_TRAILER_FROM_CAR)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    int trailerRef = -1;
    if(vehicle)
    {
        CVehicle* trailer = vehicle->m_pVehicleBeingTowed;
        if(trailer) trailerRef = GetVehicleRef(trailer);
    }
    cleo->GetPointerToScriptVar(handle)->i = trailerRef;
    UpdateCompareFlag((CRunningScript*)handle, trailerRef != -1);
}
CLEO_Fn(GET_CAR_FROM_TRAILER)
{
    CVehicle *trailer = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    int vehicleRef = -1;
    if(trailer)
    {
        CVehicle* vehicle = trailer->m_pTowingVehicle;
        if(vehicle) vehicleRef = GetVehicleRef(vehicle);
    }
    cleo->GetPointerToScriptVar(handle)->i = vehicleRef;
    UpdateCompareFlag((CRunningScript*)handle, vehicleRef != -1);
}
CLEO_Fn(GET_CAR_DUMMY_COORD)
{
    bool bResult = false;
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    int dummyId = cleo->ReadParam(handle)->i;
    int worldCoords = cleo->ReadParam(handle)->i;
    int invertX = cleo->ReadParam(handle)->i;

    CVector posn = { 0.0f, 0.0f, 0.0f };

    if(0)
    {
        // CVector *vector = vehFuncs_Ext_GetVehicleDummyPosAdapted(vehicle, dummyId);
        // if (!vector) goto GET_CAR_DUMMY_COORD_OFFSET_getByModelInfo;
        // posn = *vector;
    }
    else
    {
      GET_CAR_DUMMY_COORD_OFFSET_getByModelInfo:
        CVehicleModelInfo *vehModelInfo = (CVehicleModelInfo *)(ms_modelInfoPtrs[vehicle->m_nModelIndex]);
        if (vehModelInfo) posn = vehModelInfo->m_VehicleStructure->m_positions[dummyId];
    }

    if (posn.x != 0.0f && posn.y != 0.0f && posn.z != 0.0f) bResult = true;
    if (invertX) posn.x = -posn.x;
    if (worldCoords) posn = TransformFromObjectSpace(vehicle, posn);

    cleo->GetPointerToScriptVar(handle)->f = posn.x;
    cleo->GetPointerToScriptVar(handle)->f = posn.y;
    cleo->GetPointerToScriptVar(handle)->f = posn.z;
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(IS_CHEAT_ACTIVE)
{
    int i = cleo->ReadParam(handle)->i;
    UpdateCompareFlag((CRunningScript*)handle, m_aCheatsActive[i]);
}
CLEO_Fn(CHANGE_PLAYER_MONEY)
{
    int playerNum = cleo->ReadParam(handle)->i;
    int mode = cleo->ReadParam(handle)->i;
    int value = cleo->ReadParam(handle)->i;
    CPlayerPed* player = FindPlayerPed(playerNum);

    CPlayerInfo* info;
    if(player && (info = GetPlayerInfoForThisPlayerPed(player)))
    {
        switch(mode)
        {
            case 0:
                info->m_nMoney = value;
                info->m_nDisplayMoney = value;
                break;

            case 1:
                info->m_nMoney += value;
                break;

            case 2:
                info->m_nMoney -= value;
                break;

            case 3:
                info->m_nMoney -= value;
                if(info->m_nMoney < 0) info->m_nMoney = 0;
                break;

            case 4:
                if(info->m_nMoney > 0) info->m_nMoney -= value;
                break;
        }
    }
}
CLEO_Fn(CAR_HORN)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    if(!vehicle) return;

    vehicle->m_cHorn = 1;
    vehicle->PlayCarHorn();
}
CLEO_Fn(GET_STRING_LENGTH)
{
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    int len = strnlen(buf, 128);
    cleo->GetPointerToScriptVar(handle)->i = len;
    UpdateCompareFlag((CRunningScript*)handle, len > 0);
}
CLEO_Fn(COPY_STRING)
{
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    int len = strnlen(buf, 128);

    uint8_t paramType = **(uint8_t**)((int)handle + GetPCOffset());
    char* pointer;
    if(paramType >= 1 && paramType <= 8)
    {
        pointer = (char*)cleo->ReadParam(handle)->i;
    }
    else
    {
        pointer = (char*)cleo->GetPointerToScriptVar(handle);
    }
    memcpy(pointer, buf, len);
    pointer[len] = 0;
}
CLEO_Fn(GET_CAR_ALARM)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    int mode = -1;
    if(vehicle)
    {
        if(vehicle->CarAlarmState == 0) mode = 0;
        else mode = 1 + (vehicle->CarAlarmState != 0xFFFF);
    }
    cleo->GetPointerToScriptVar(handle)->i = mode;
}
CLEO_Fn(SET_CAR_ALARM)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    int mode = cleo->ReadParam(handle)->i;
    if(vehicle)
    {
        switch(mode)
        {
            case 0: vehicle->CarAlarmState = 0; return;
            case 1: vehicle->CarAlarmState = 0xFFFF; return;
            case 2: vehicle->CarAlarmState = 0xFFFE; return;
        }
    }
}
CLEO_Fn(GET_CHAR_MAX_HEALTH)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    float maxHealth = 0.0f;
    if(ped->m_nPedType <= PED_TYPE_PLAYER2)
    {
        maxHealth = (float)(GetPlayerInfoForThisPlayerPed((CPlayerPed*)ped)->m_nMaxHealth);
    }
    else
    {
        maxHealth = ped->m_fMaxHealth;
    }
    cleo->GetPointerToScriptVar(handle)->f = maxHealth;
}
CLEO_Fn(GET_CHAR_HEALTH_PERCENT)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    float maxHealth = 0.0f;
    if(ped->m_nPedType <= PED_TYPE_PLAYER2)
    {
        maxHealth = (float)(GetPlayerInfoForThisPlayerPed((CPlayerPed*)ped)->m_nMaxHealth);
    }
    else
    {
        maxHealth = ped->m_fMaxHealth;
    }
    float curHealth = ped->m_fHealth;
    cleo->GetPointerToScriptVar(handle)->f = (curHealth / maxHealth) * 100.0f;
}
CLEO_Fn(GET_CURRENT_CAMERA_MODE)
{
    cleo->GetPointerToScriptVar(handle)->i = TheCamera->m_apCams[TheCamera->m_nCurrentActiveCam].Mode;
}
CLEO_Fn(GET_CAR_COLLISION_INTENSITY)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    float intensity = 0.0f;
    if(vehicle) intensity = vehicle->m_fDamageIntensity;
    cleo->GetPointerToScriptVar(handle)->f = intensity;
    UpdateCompareFlag((CRunningScript*)handle, intensity > 0);
}
CLEO_Fn(GET_CAR_COLLISION_COORDINATES)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    CVector pos;
    if(vehicle) pos = vehicle->m_vecLastCollisionPosn;
    cleo->GetPointerToScriptVar(handle)->f = pos.x;
    cleo->GetPointerToScriptVar(handle)->f = pos.y;
    cleo->GetPointerToScriptVar(handle)->f = pos.z;
}
CLEO_Fn(STREAM_CUSTOM_SCRIPT_FROM_LABEL)
{
    
}
CLEO_Fn(GET_LAST_CREATED_CUSTOM_SCRIPT)
{
    
}
CLEO_Fn(GET_OBJECT_CENTRE_OF_MASS_TO_BASE_OF_MODEL)
{
    CObject *object = GetObjectFromRef(cleo->ReadParam(handle)->i);
    float dist = 999999.0f;
    if(object) dist = GetDistanceFromCentreOfMassToBaseOfModel(object);
    cleo->GetPointerToScriptVar(handle)->f = dist;
    UpdateCompareFlag((CRunningScript*)handle, dist != 999999.0f);
}
CLEO_Fn(GET_MODEL_TYPE)
{
    int modelId = cleo->ReadParam(handle)->i;
    int type = -1;
    if(modelId >= 0)
    {
        CBaseModelInfo* modelInfo = ms_modelInfoPtrs[modelId];
        if(modelInfo) type = modelInfo->GetModelType();
    }
    cleo->GetPointerToScriptVar(handle)->i = type;
}
CLEO_Fn(IS_STRING_EQUAL)
{
    bool bResult = true;
    char stringA[128], stringB[128], ignoreChar[2];
    CLEO_ReadStringEx(handle, stringA, sizeof(stringA));
    CLEO_ReadStringEx(handle, stringB, sizeof(stringB));
    int maxSize = cleo->ReadParam(handle)->i;
    int caseSensitive = cleo->ReadParam(handle)->i;
    CLEO_ReadStringEx(handle, ignoreChar, sizeof(ignoreChar));

    if(!caseSensitive)
    {
        toupper(stringA);
        toupper(stringB);
    }

    int i = 0;
    while (i < maxSize && stringA[i] != 0 && stringB[i] != 0)
    {
        if (stringA[i] != ignoreChar[0] && stringB[i] != ignoreChar[0])
        {
            if (stringA[i] != stringB[i])
            {
                bResult = false;
                break;
            }
        }
        ++i;
    }
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(IS_STRING_COMMENT)
{
    bool bResult = false;
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));

    unsigned int i = 0;
    while (buf[i] == ' ' && i <= 127) ++i;
    if (buf[i] == '#' || buf[i] == ';' || (buf[i] == '/' && buf[i + 1] == '/')) bResult = true;
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(DOES_CAR_HAVE_PART_NODE)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    int nodeId = cleo->ReadParam(handle)->i;
    bool bResult = false;
    if(vehicle) bResult = ((CAutomobile*)vehicle)->m_CarNodes[nodeId] != NULL;
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(GET_CURRENT_CHAR_WEAPONINFO)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    eWeaponType weaponType = ped->m_Weapons[ped->m_byteCurrentWeaponSlot].m_nType;
    CWeaponInfo *weaponInfo = GetWeaponInfo(weaponType, GetWeaponSkill(ped, weaponType));
    cleo->GetPointerToScriptVar(handle)->i = (int)weaponInfo;
    UpdateCompareFlag((CRunningScript*)handle, weaponInfo != NULL);
}
CLEO_Fn(GET_WEAPONINFO)
{
    int weaponId = cleo->ReadParam(handle)->i;
    int skill = cleo->ReadParam(handle)->i;
    CWeaponInfo *weaponInfo = GetWeaponInfo((eWeaponType)weaponId, skill);
    cleo->GetPointerToScriptVar(handle)->i = (int)weaponInfo;
    UpdateCompareFlag((CRunningScript*)handle, weaponInfo != NULL);
}
CLEO_Fn(GET_WEAPONINFO_MODELS)
{
    CWeaponInfo *weaponInfo = (CWeaponInfo*)cleo->ReadParam(handle)->i;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_nModelId1;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_nModelId2;
}
CLEO_Fn(GET_WEAPONINFO_FLAGS)
{
    CWeaponInfo *weaponInfo = (CWeaponInfo*)cleo->ReadParam(handle)->i;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_nFlags;
}
CLEO_Fn(GET_WEAPONINFO_ANIMGROUP)
{
    CWeaponInfo *weaponInfo = (CWeaponInfo*)cleo->ReadParam(handle)->i;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_eAnimGroup;
}
CLEO_Fn(GET_WEAPONINFO_TOTAL_CLIP)
{
    CWeaponInfo *weaponInfo = (CWeaponInfo*)cleo->ReadParam(handle)->i;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_nAmmoClip;
}
CLEO_Fn(GET_WEAPONINFO_FIRE_TYPE)
{
    CWeaponInfo *weaponInfo = (CWeaponInfo*)cleo->ReadParam(handle)->i;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_nWeaponFire;
}
CLEO_Fn(GET_WEAPONINFO_SLOT)
{
    CWeaponInfo *weaponInfo = (CWeaponInfo*)cleo->ReadParam(handle)->i;
    cleo->GetPointerToScriptVar(handle)->i = weaponInfo->m_nSlot;
}
CLEO_Fn(GET_CHAR_WEAPON_STATE)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->m_Weapons[ped->m_byteCurrentWeaponSlot].m_nState;
}
CLEO_Fn(GET_CHAR_WEAPON_CLIP)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->m_Weapons[ped->m_byteCurrentWeaponSlot].m_nAmmoInClip;
}
CLEO_Fn(GET_CHAR_COLLISION_SURFACE)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->m_nContactSurface;
}
CLEO_Fn(GET_CHAR_COLLISION_LIGHTING)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->f = ped->m_fContactSurfaceBrightness;
}
CLEO_Fn(GET_CAR_COLLISION_SURFACE)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->f = vehicle->m_fContactSurfaceBrightness;
}
CLEO_Fn(GET_CAR_COLLISION_LIGHTING)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = vehicle->m_nContactSurface;
}
CLEO_Fn(IS_CHAR_REALLY_IN_AIR)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    UpdateCompareFlag((CRunningScript*)handle, !ped->physicalFlags.bOnSolidSurface && !ped->physicalFlags.bSubmergedInWater);
}
CLEO_Fn(IS_CAR_REALLY_IN_AIR)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    UpdateCompareFlag((CRunningScript*)handle, !vehicle->physicalFlags.bOnSolidSurface && !vehicle->physicalFlags.bSubmergedInWater);
}
CLEO_Fn(IS_OBJECT_REALLY_IN_AIR)
{
    CObject *object = GetObjectFromRef(cleo->ReadParam(handle)->i);
    UpdateCompareFlag((CRunningScript*)handle, !object->physicalFlags.bOnSolidSurface && !object->physicalFlags.bSubmergedInWater);
}
CLEO_Fn(SIMULATE_OBJECT_DAMAGE)
{
    CObject *object = GetObjectFromRef(cleo->ReadParam(handle)->i);
    float damage = cleo->ReadParam(handle)->f;
    int type = cleo->ReadParam(handle)->i;
    ObjectDamage(object, damage, NULL, NULL, NULL, (eWeaponType)type);
}
CLEO_Fn(REQUEST_PRIORITY_MODEL)
{
    int model = cleo->ReadParam(handle)->i;
    RequestModel(model, eStreamingFlags::STREAMING_KEEP_IN_MEMORY | eStreamingFlags::STREAMING_MISSION_REQUIRED | eStreamingFlags::STREAMING_PRIORITY_REQUEST);
    AddToResourceManager(ScriptResourceManager, model, 2, (CRunningScript*)handle);
}
CLEO_Fn(LOAD_ALL_PRIORITY_MODELS_NOW)
{
    TimerSuspend();
    LoadAllRequestedModels(false);
    TimerResume();
}
CLEO_Fn(LOAD_SPECIAL_CHARACTER_FOR_ID)
{
    int id = cleo->ReadParam(handle)->i;
    CBaseModelInfo* modelInfo = ms_modelInfoPtrs[id];
    if(!modelInfo)
    {
        CPedModelInfo* pedModelInfo = AddPedModel(id);
        SetColModel(pedModelInfo, ms_colModelPed1, false);
        CPedModelInfo* basePedModelInfo = (CPedModelInfo*)ms_modelInfoPtrs[290];
        pedModelInfo->m_defaultPedType = ePedType::PED_TYPE_CIVMALE;
        pedModelInfo->m_defaultPedStats = ePedStats::PEDSTAT_STD_MISSION;
        if(basePedModelInfo)
        {
            pedModelInfo->m_motionAnimGroup = basePedModelInfo->m_motionAnimGroup;
            pedModelInfo->m_drivesWhichCars = basePedModelInfo->m_drivesWhichCars;
            pedModelInfo->m_flags = basePedModelInfo->m_flags;
            pedModelInfo->m_radio1 = basePedModelInfo->m_radio1;
            pedModelInfo->m_radio2 = basePedModelInfo->m_radio2;
            pedModelInfo->m_Race = basePedModelInfo->m_Race;
            pedModelInfo->m_AudioPedType = basePedModelInfo->m_AudioPedType;
            pedModelInfo->m_FirstVoice = basePedModelInfo->m_FirstVoice;
            pedModelInfo->m_LastVoice = basePedModelInfo->m_LastVoice;
            pedModelInfo->m_NextVoice = basePedModelInfo->m_NextVoice;
        }
    }
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    RequestSpecialModel(id, buf, eStreamingFlags::STREAMING_KEEP_IN_MEMORY | eStreamingFlags::STREAMING_MISSION_REQUIRED);
    SpecialCharacterModelsUsed.insert(id);
    AddToResourceManager(ScriptResourceManager, id, 2, (CRunningScript*)handle);
}
CLEO_Fn(UNLOAD_SPECIAL_CHARACTER_FROM_ID)
{
    int id = cleo->ReadParam(handle)->i;
    if(RemoveFromResourceManager(ScriptResourceManager, id, 2, (CRunningScript*)handle))
    {
        SetMissionDoesntRequireModel(id);
    }
    CBaseModelInfo *baseModelInfo = ms_modelInfoPtrs[id];
    if(baseModelInfo && baseModelInfo->m_nRefCount == 0) RemoveModel(id);
}
CLEO_Fn(GET_MODEL_BY_NAME)
{
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    int id = -1;
    CBaseModelInfo *baseModelInfo = GetModelInfoByName(buf, &id);
    cleo->GetPointerToScriptVar(handle)->i = id;
    UpdateCompareFlag((CRunningScript*)handle, id > -1 && baseModelInfo);
}
CLEO_Fn(IS_MODEL_AVAILABLE_BY_NAME)
{
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    unsigned int offset, size;
    CDirectory::DirectoryInfo* dir = FindItem(*ms_pExtraObjectsDir, (const char*)buf, offset, size);
    UpdateCompareFlag((CRunningScript*)handle, dir != NULL);
}
CLEO_Fn(GET_MODEL_DOESNT_EXIST_IN_RANGE)
{
    bool bResult = false;
    int start = cleo->ReadParam(handle)->i;
    int end = cleo->ReadParam(handle)->i;
    int id;

    for (id = start; id <= end; ++id)
    {
        if(!IsObjectInCdImage(id) || (ms_aInfoForModel[id].m_status != 1 && SpecialCharacterModelsUsed.find(id) != SpecialCharacterModelsUsed.end()))
        {
            bResult = true;
            break;
        }
    }
    if (!bResult) id = -1;
    cleo->GetPointerToScriptVar(handle)->i = id;
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(REMOVE_ALL_UNUSED_MODELS)
{
    RemoveAllUnusedModels();
}
CLEO_Fn(REMOVE_MODEL_IF_UNUSED)
{
    int id = cleo->ReadParam(handle)->i;
    CBaseModelInfo *baseModelInfo = ms_modelInfoPtrs[id];
    if(baseModelInfo && baseModelInfo->m_nRefCount == 0) RemoveModel(id);
}
CLEO_Fn(IS_CHAR_ON_FIRE)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    UpdateCompareFlag((CRunningScript*)handle, ped && ped->m_pFire != NULL);
}
CLEO_Fn(GET_CLOSEST_COP_NEAR_CHAR)
{
    
}
CLEO_Fn(GET_CLOSEST_COP_NEAR_POS)
{
    
}
CLEO_Fn(GET_ANY_CHAR_NO_SAVE_RECURSIVE)
{
    
}
CLEO_Fn(GET_ANY_CAR_NO_SAVE_RECURSIVE)
{
    
}
CLEO_Fn(GET_ANY_OBJECT_NO_SAVE_RECURSIVE)
{
    
}
CLEO_Fn(SET_CHAR_ARRESTED)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    ped->m_nPedState = ePedState::PEDSTATE_ARRESTED;
}
CLEO_Fn(GET_CHAR_PEDSTATE)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->m_nPedState;
}
CLEO_Fn(GET_CHAR_PROOFS)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->physicalFlags.bBulletProof;
    cleo->GetPointerToScriptVar(handle)->i = ped->physicalFlags.bFireProof;
    cleo->GetPointerToScriptVar(handle)->i = ped->physicalFlags.bExplosionProof;
    cleo->GetPointerToScriptVar(handle)->i = ped->physicalFlags.bCollisionProof;
    cleo->GetPointerToScriptVar(handle)->i = ped->physicalFlags.bMeleeProof;
}
CLEO_Fn(GET_CAR_PROOFS)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = vehicle->physicalFlags.bBulletProof;
    cleo->GetPointerToScriptVar(handle)->i = vehicle->physicalFlags.bFireProof;
    cleo->GetPointerToScriptVar(handle)->i = vehicle->physicalFlags.bExplosionProof;
    cleo->GetPointerToScriptVar(handle)->i = vehicle->physicalFlags.bCollisionProof;
    cleo->GetPointerToScriptVar(handle)->i = vehicle->physicalFlags.bMeleeProof;
}
CLEO_Fn(GET_OBJECT_PROOFS)
{
    CObject *object = GetObjectFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = object->physicalFlags.bBulletProof;
    cleo->GetPointerToScriptVar(handle)->i = object->physicalFlags.bFireProof;
    cleo->GetPointerToScriptVar(handle)->i = object->physicalFlags.bExplosionProof;
    cleo->GetPointerToScriptVar(handle)->i = object->physicalFlags.bCollisionProof;
    cleo->GetPointerToScriptVar(handle)->i = object->physicalFlags.bMeleeProof;
}
CLEO_Fn(IS_CHAR_WEAPON_VISIBLE_SET)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    CPlayerData* data = ped->m_pPlayerData;
    UpdateCompareFlag((CRunningScript*)handle, data && data->m_bRenderWeapon);
}
CLEO_Fn(GET_CHAR_STAT_ID)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = (int)ped->m_pPedStat;
}
CLEO_Fn(GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS)
{
    CVector offset, posReturn;
    offset.x = cleo->ReadParam(handle)->f;
    offset.y = cleo->ReadParam(handle)->f;
    offset.z = cleo->ReadParam(handle)->f;

    RwV3dTransformPoint(posReturn, offset, TheCamera->m_matCameraMatrix);

    cleo->GetPointerToScriptVar(handle)->f = posReturn.x;
    cleo->GetPointerToScriptVar(handle)->f = posReturn.y;
    cleo->GetPointerToScriptVar(handle)->f = posReturn.z;
}
CLEO_Fn(GET_OFFSET_FROM_MATRIX_IN_WORLD_COORDS)
{
    CMatrix* matrix = (CMatrix*)cleo->ReadParam(handle)->i;

    CVector offset, posReturn;
    offset.x = cleo->ReadParam(handle)->f;
    offset.y = cleo->ReadParam(handle)->f;
    offset.z = cleo->ReadParam(handle)->f;

    RwV3dTransformPoint(posReturn, offset, *matrix);

    cleo->GetPointerToScriptVar(handle)->f = posReturn.x;
    cleo->GetPointerToScriptVar(handle)->f = posReturn.y;
    cleo->GetPointerToScriptVar(handle)->f = posReturn.z;
}
CLEO_Fn(SET_CAR_COORDINATES_SIMPLE)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    vehicle->m_matrix->pos.x = cleo->ReadParam(handle)->f;
    vehicle->m_matrix->pos.y = cleo->ReadParam(handle)->f;
    vehicle->m_matrix->pos.z = cleo->ReadParam(handle)->f;
}
CLEO_Fn(GET_CHAR_DAMAGE_LAST_FRAME)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
}
CLEO_Fn(GET_CAR_WEAPON_DAMAGE_LAST_FRAME)
{
    
}
CLEO_Fn(IS_ON_SCRIPTED_CUTSCENE)
{
    UpdateCompareFlag((CRunningScript*)handle, TheCamera->m_WideScreenOn);
}
CLEO_Fn(GET_MODEL_PED_TYPE_AND_STAT)
{
    bool bResult = false;
    int model = cleo->ReadParam(handle)->i;
    int type = -1, stat = -1;
    CPedModelInfo *pedModelInfo = (CPedModelInfo*)ms_modelInfoPtrs[model];
    if (pedModelInfo)
    {
        bResult = true;
        type = pedModelInfo->m_defaultPedType;
        stat = pedModelInfo->m_defaultPedStats;
    }
    cleo->GetPointerToScriptVar(handle)->i = type;
    cleo->GetPointerToScriptVar(handle)->i = stat;
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(PASS_TIME)
{
    int mins = cleo->ReadParam(handle)->i;
    PassTime(mins);
}
CLEO_Fn(GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED)
{
    int seed = cleo->ReadParam(handle)->i;
    int min = cleo->ReadParam(handle)->i;
    int max = cleo->ReadParam(handle)->i - 1;

    unsigned int resetSeed = rand() / 2;
    srand(seed);
    cleo->GetPointerToScriptVar(handle)->i = rand() % max + min;
    srand((time(NULL) / 2) + resetSeed);
}
CLEO_Fn(GENERATE_RANDOM_FLOAT_IN_RANGE_WITH_SEED)
{
    int seed = cleo->ReadParam(handle)->i;
    float min = cleo->ReadParam(handle)->f;
    float max = cleo->ReadParam(handle)->f;

    unsigned int resetSeed = rand() / 2;
    srand(seed);
    cleo->GetPointerToScriptVar(handle)->f = fmaf((float)rand() / (float)RAND_MAX, max - min, min);
    srand((time(NULL) / 2) + resetSeed);
}
CLEO_Fn(LOCATE_CAMERA_DISTANCE_TO_COORDINATES)
{
    bool bResult = false;
    CVector pos;
    pos.x = cleo->ReadParam(handle)->f;
    pos.y = cleo->ReadParam(handle)->f;
    pos.z = cleo->ReadParam(handle)->f;
    float radius = cleo->ReadParam(handle)->f;
    if ((TheCamera->m_apCams[TheCamera->m_nCurrentActiveCam].Source - pos).MagnitudeSqr() <= radius * radius) bResult = true;
    UpdateCompareFlag((CRunningScript*)handle, bResult);
}
CLEO_Fn(GET_FX_SYSTEM_POINTER)
{
    int hndl = cleo->ReadParam(handle)->i;
    if(hndl >= 0)
    {
        hndl = GetActualScriptThingIndex(hndl, 1);
        if(hndl >= 0)
        {
            cleo->GetPointerToScriptVar(handle)->i = (int)(ScriptEffectSystemArray[hndl].pFXSystem);
            UpdateCompareFlag((CRunningScript*)handle, true);
            return;
        }
    }
    cleo->GetPointerToScriptVar(handle)->i = 0;
    UpdateCompareFlag((CRunningScript*)handle, false);
}
CLEO_Fn(ADD_FX_SYSTEM_PARTICLE)
{
    int hndl = cleo->ReadParam(handle)->i;

    RwV3d pos;
    pos.x = cleo->ReadParam(handle)->f;
    pos.y = cleo->ReadParam(handle)->f;
    pos.z = cleo->ReadParam(handle)->f;

    RwV3d vel;
    vel.x = cleo->ReadParam(handle)->f;
    vel.y = cleo->ReadParam(handle)->f;
    vel.z = cleo->ReadParam(handle)->f;

    float size = cleo->ReadParam(handle)->f;

    float brightness = cleo->ReadParam(handle)->f;

    float r = cleo->ReadParam(handle)->f;
    float g = cleo->ReadParam(handle)->f;
    float b = cleo->ReadParam(handle)->f;
    float a = cleo->ReadParam(handle)->f;

    float lastFactor = cleo->ReadParam(handle)->f;

    if(hndl >= 0)
    {
        hndl = GetActualScriptThingIndex(hndl, 1);
        if(hndl >= 0)
        {
            FxSystem_c *fx = ScriptEffectSystemArray[hndl].pFXSystem;
            FxPrtMult_c *prtMult = new FxPrtMult_c(r, g, b, a, size, 0.0, lastFactor);
            AddParticle(fx, &pos, &vel, 0.0f, prtMult, -1.0f, brightness, 0.0f, 0);
        }
    }
}
CLEO_Fn(IS_FX_SYSTEM_AVAILABLE_WITH_NAME)
{
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    UpdateCompareFlag((CRunningScript*)handle, FindFxSystemBP(g_fxMan, buf) != NULL);
}
CLEO_Fn(SET_STRING_UPPER)
{
    char* str = (char*)cleo->ReadParam(handle)->i;
    int i = -1;
    while(str[++i]) str[i] = std::toupper(str[i]);
}
CLEO_Fn(SET_STRING_LOWER)
{
    char* str = (char*)cleo->ReadParam(handle)->i;
    int i = -1;
    while(str[++i]) str[i] = std::tolower(str[i]);
}
CLEO_Fn(STRING_FIND)
{
    int mode = cleo->ReadParam(handle)->i;
    char buf[128];
    CLEO_ReadStringEx(handle, buf, sizeof(buf)); std::string str = buf;
    CLEO_ReadStringEx(handle, buf, sizeof(buf)); std::string strFind = buf;
    
    size_t found = std::string::npos;
    if (mode == 0) found = str.find(strFind);
    else if (mode == 1) found = str.rfind(strFind);

    if (found != std::string::npos)
    {
        cleo->GetPointerToScriptVar(handle)->i = found;
        UpdateCompareFlag((CRunningScript*)handle, true);
    }
    else
    {
        cleo->GetPointerToScriptVar(handle)->i = -1;
        UpdateCompareFlag((CRunningScript*)handle, false);
    }
}
CLEO_Fn(CUT_STRING_AT)
{
    char* str = (char*)cleo->ReadParam(handle)->i;
    str[cleo->ReadParam(handle)->i] = 0;
}
CLEO_Fn(IS_STRING_CHARACTER_AT)
{
    char str[128], cha[4];
    CLEO_ReadStringEx(handle, str, sizeof(str));
    CLEO_ReadStringEx(handle, cha, sizeof(cha));
    int index = cleo->ReadParam(handle)->i;
    UpdateCompareFlag((CRunningScript*)handle, (str[index] == cha[0]));
}
CLEO_Fn(GET_CHAR_RANDOM_SEED)
{
    CEntity *ent = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ent->m_nRandomSeed;
}
CLEO_Fn(GET_CAR_RANDOM_SEED)
{
    CEntity *ent = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ent->m_nRandomSeed;
}
CLEO_Fn(GET_OBJECT_RANDOM_SEED)
{
    CEntity *ent = GetObjectFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ent->m_nRandomSeed;
}
CLEO_Fn(GET_CHAR_MOVE_STATE)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->m_nMoveState;
}
CLEO_Fn(DONT_DELETE_CHAR_UNTIL_TIME)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    unsigned int time = cleo->ReadParam(handle)->i + *m_snTimeInMilliseconds;
    if(ped->m_nTimeTillWeNeedThisPed < time) ped->m_nTimeTillWeNeedThisPed = time;
}
CLEO_Fn(DONT_DELETE_CAR_UNTIL_TIME)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    unsigned int time = cleo->ReadParam(handle)->i + *m_snTimeInMilliseconds;
    if(vehicle->DontUseSmallerRemovalRange < time) vehicle->DontUseSmallerRemovalRange = time;
}
CLEO_Fn(GET_TIME_CHAR_IS_DEAD)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    if(ped->IsDead()) cleo->GetPointerToScriptVar(handle)->i = ped->m_nTimeOfDeath - *m_snTimeInMilliseconds;
    else cleo->GetPointerToScriptVar(handle)->i = -1;
}
CLEO_Fn(GET_TIME_CAR_IS_DEAD)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    if(vehicle->m_fHealth <= 0.0f) cleo->GetPointerToScriptVar(handle)->i = vehicle->m_nTimeOfDeath - *m_snTimeInMilliseconds;
    else cleo->GetPointerToScriptVar(handle)->i = -1;
}
CLEO_Fn(SET_CHAR_IGNORE_DAMAGE_ANIMS)
{
    
}
CLEO_Fn(LOCATE_CHAR_DISTANCE_TO_CHAR)
{
    CEntity *ent1 = GetPedFromRef(cleo->ReadParam(handle)->i);
    CEntity *ent2 = GetPedFromRef(cleo->ReadParam(handle)->i);
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - ent2->GetPosition()).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_CHAR_DISTANCE_TO_CAR)
{
    CEntity *ent1 = GetPedFromRef(cleo->ReadParam(handle)->i);
    CEntity *ent2 = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - ent2->GetPosition()).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_CHAR_DISTANCE_TO_OBJECT)
{
    CEntity *ent1 = GetPedFromRef(cleo->ReadParam(handle)->i);
    CEntity *ent2 = GetObjectFromRef(cleo->ReadParam(handle)->i);
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - ent2->GetPosition()).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_CAR_DISTANCE_TO_OBJECT)
{
    CEntity *ent1 = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    CEntity *ent2 = GetObjectFromRef(cleo->ReadParam(handle)->i);
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - ent2->GetPosition()).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_CAR_DISTANCE_TO_CAR)
{
    CEntity *ent1 = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    CEntity *ent2 = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - ent2->GetPosition()).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_OBJECT_DISTANCE_TO_OBJECT)
{
    CEntity *ent1 = GetObjectFromRef(cleo->ReadParam(handle)->i);
    CEntity *ent2 = GetObjectFromRef(cleo->ReadParam(handle)->i);
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - ent2->GetPosition()).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_CHAR_DISTANCE_TO_COORDINATES)
{
    CEntity *ent1 = GetPedFromRef(cleo->ReadParam(handle)->i);
    CVector pos;
    pos.x = cleo->ReadParam(handle)->f;
    pos.y = cleo->ReadParam(handle)->f;
    pos.z = cleo->ReadParam(handle)->f;
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - pos).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_CAR_DISTANCE_TO_COORDINATES)
{
    CEntity *ent1 = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    CVector pos;
    pos.x = cleo->ReadParam(handle)->f;
    pos.y = cleo->ReadParam(handle)->f;
    pos.z = cleo->ReadParam(handle)->f;
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - pos).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_OBJECT_DISTANCE_TO_COORDINATES)
{
    CEntity *ent1 = GetObjectFromRef(cleo->ReadParam(handle)->i);
    CVector pos;
    pos.x = cleo->ReadParam(handle)->f;
    pos.y = cleo->ReadParam(handle)->f;
    pos.z = cleo->ReadParam(handle)->f;
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - pos).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(LOCATE_ENTITY_DISTANCE_TO_ENTITY)
{
    CEntity *ent1 = (CEntity*)(cleo->ReadParam(handle)->i);
    CVector pos;
    pos.x = cleo->ReadParam(handle)->f;
    pos.y = cleo->ReadParam(handle)->f;
    pos.z = cleo->ReadParam(handle)->f;
    float radius = cleo->ReadParam(handle)->f;

    UpdateCompareFlag((CRunningScript*)handle, (ent1->GetPosition() - pos).MagnitudeSqr() <= radius * radius);
}
CLEO_Fn(GET_ENTITY_COORDINATES)
{
    CEntity *ent = (CEntity*)(cleo->ReadParam(handle)->i);
    CVector& pos = ent->GetPosition();
    cleo->GetPointerToScriptVar(handle)->f = pos.x;
    cleo->GetPointerToScriptVar(handle)->f = pos.y;
    cleo->GetPointerToScriptVar(handle)->f = pos.z;
}
CLEO_Fn(GET_ENTITY_HEADING)
{
    CEntity *ent = (CEntity*)(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->f = ent->GetHeading();
}
CLEO_Fn(IS_CAR_OWNED_BY_PLAYER)
{
    CVehicle *vehicle = (CVehicle*)(cleo->ReadParam(handle)->i);
    UpdateCompareFlag((CRunningScript*)handle, vehicle->vehicleFlags.bHasBeenOwnedByPlayer);
}
CLEO_Fn(SET_CAR_OWNED_BY_PLAYER)
{
    CVehicle *vehicle = (CVehicle*)(cleo->ReadParam(handle)->i);
    vehicle->vehicleFlags.bHasBeenOwnedByPlayer = (cleo->ReadParam(handle)->i != 0);
}
CLEO_Fn(GET_MODEL_INFO)
{
    int model = cleo->ReadParam(handle)->i;
    CBaseModelInfo* modelInfo = NULL;
    if(model >= 0) modelInfo = ms_modelInfoPtrs[model];
    cleo->GetPointerToScriptVar(handle)->i = (int)modelInfo;
    UpdateCompareFlag((CRunningScript*)handle, modelInfo != NULL);
}
CLEO_Fn(GET_CAR_ANIMGROUP)
{
    CVehicle *vehicle = (CVehicle*)(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = vehicle->m_pHandling->AnimGroup;
}
CLEO_Fn(GET_CHAR_FEAR)
{
    CPed *ped = GetPedFromRef(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = ped->m_pPedStat->m_nFear;
}
CLEO_Fn(IS_CAR_CONVERTIBLE)
{
    CVehicle *vehicle = (CVehicle*)(cleo->ReadParam(handle)->i);
    UpdateCompareFlag((CRunningScript*)handle, vehicle->m_pHandling->m_bConvertible);
}
CLEO_Fn(GET_CAR_VALUE)
{
    CVehicle *vehicle = (CVehicle*)(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->i = vehicle->m_pHandling->nMonetaryValue;
}
CLEO_Fn(GET_CAR_PEDALS)
{
    CVehicle *vehicle = (CVehicle*)(cleo->ReadParam(handle)->i);
    cleo->GetPointerToScriptVar(handle)->f = vehicle->m_fGasPedal;
    cleo->GetPointerToScriptVar(handle)->f = vehicle->m_fBrakePedal;
}
CLEO_Fn(GET_LOADED_LIBRARY)
{
    char buf[64];
    CLEO_ReadStringEx(handle, buf, sizeof(buf));
    void* library = dlopen(buf, RTLD_LAZY);
    cleo->GetPointerToScriptVar(handle)->i = (int)library;
    UpdateCompareFlag((CRunningScript*)handle, library != NULL);
}
CLEO_Fn(SET_CAR_MODEL_ALPHA)
{
    CEntity *ent = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    if (ent->m_pRwObject)
    {
        SetRwObjectAlpha(ent, cleo->ReadParam(handle)->i);
        UpdateCompareFlag((CRunningScript*)handle, true);
    }
    else
    {
        UpdateCompareFlag((CRunningScript*)handle, false);
    }
}
CLEO_Fn(SET_CHAR_MODEL_ALPHA)
{
    CEntity *ent = GetPedFromRef(cleo->ReadParam(handle)->i);
    if (ent->m_pRwObject)
    {
        SetRwObjectAlpha(ent, cleo->ReadParam(handle)->i);
        UpdateCompareFlag((CRunningScript*)handle, true);
    }
    else
    {
        UpdateCompareFlag((CRunningScript*)handle, false);
    }
}
CLEO_Fn(SET_OBJECT_MODEL_ALPHA)
{
    CEntity *ent = GetObjectFromRef(cleo->ReadParam(handle)->i);
    if (ent->m_pRwObject)
    {
        SetRwObjectAlpha(ent, cleo->ReadParam(handle)->i);
        UpdateCompareFlag((CRunningScript*)handle, true);
    }
    else
    {
        UpdateCompareFlag((CRunningScript*)handle, false);
    }
}
CLEO_Fn(GET_LOCAL_TIME)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    cleo->GetPointerToScriptVar(handle)->i = 1900 + tm.tm_year;
    cleo->GetPointerToScriptVar(handle)->i = 1 + tm.tm_mon;
    cleo->GetPointerToScriptVar(handle)->i = tm.tm_wday;
    cleo->GetPointerToScriptVar(handle)->i = tm.tm_mday;
    cleo->GetPointerToScriptVar(handle)->i = tm.tm_hour;
    cleo->GetPointerToScriptVar(handle)->i = tm.tm_min;
    cleo->GetPointerToScriptVar(handle)->i = tm.tm_sec;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    cleo->GetPointerToScriptVar(handle)->i = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}
CLEO_Fn(SET_SCRIPT_VAR)
{
    
}
CLEO_Fn(GET_SCRIPT_VAR)
{
    
}
CLEO_Fn(SET_CAR_DOOR_WINDOW_STATE)
{
    CVehicle *vehicle = GetVehicleFromRef(cleo->ReadParam(handle)->i);
    unsigned int door = cleo->ReadParam(handle)->u;
    unsigned int state = cleo->ReadParam(handle)->u;
    if(state)
    {
        SetWindowOpenFlag(vehicle, door);
    }
    else
    {
        ClearWindowOpenFlag(vehicle, door);
    }
}

DECL_HOOKv(InitialiseWhenRestarting)
{
    InitialiseWhenRestarting();

    ++timesGameRestarted;
}
void Misc_Patch()
{
    HOOKPLT(InitialiseWhenRestarting, pGTASA + 0x672014);
}